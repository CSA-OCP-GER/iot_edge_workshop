// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Original code: https://github.com/Azure/iot-edge-v1/blob/master/v2/samples/azureiotedge-simulated-temperature-sensor/Program.cs


// disabling async warning as the SendSimulationData is an async method
// but we don't wait for it
#pragma warning disable CS4014

namespace AzureIotEdgeSimulatedTemperatureSensor
{
    using System;
    using System.IO;
    using System.Net;
    using System.Collections;
    using System.Collections.Generic;
    using System.Runtime.InteropServices;
    using System.Runtime.Loader;
    using System.Security.Cryptography.X509Certificates;
    using System.Net.Security;
    using System.Text;
    using System.Threading;
    using System.Threading.Tasks;
    using Microsoft.Azure.Devices.Client;
    using Microsoft.Azure.Devices.Shared;
    //using Microsoft.Azure.Devices.Client.Transport.Mqtt;
    using Newtonsoft.Json;

    class Program
    {
        private static int counter;

        private static volatile DesiredPropertiesData desiredPropertiesData;
        private static DataGenerationPolicy generationPolicy = new DataGenerationPolicy();

        private static volatile bool IsReset = false;

        static void Main(string[] args)
        {
            Init().Wait();

            // Wait until the app unloads or is cancelled
            var cts = new CancellationTokenSource();
            AssemblyLoadContext.Default.Unloading += (ctx) => cts.Cancel();
            Console.CancelKeyPress += (sender, cpe) => cts.Cancel();
            WhenCancelled(cts.Token).Wait();
        }

        // The following method is invoked by the RemoteCertificateValidationDelegate, it disables cert validation check
        // From https://docs.microsoft.com/en-us/dotnet/api/system.net.security.remotecertificatevalidationcallback?view=netframework-4.7.2
        public static bool ValidateServerCertificate(
            object sender,
            X509Certificate certificate,
            X509Chain chain,
            SslPolicyErrors sslPolicyErrors)
        {
            return true;
        }

        /// <summary>
        /// Handles cleanup operations when app is cancelled or unloads
        /// </summary>
        public static Task WhenCancelled(CancellationToken cancellationToken)
        {
            var tcs = new TaskCompletionSource<bool>();
            cancellationToken.Register(s => ((TaskCompletionSource<bool>)s).SetResult(true), tcs);
            return tcs.Task;
        }

        /// <summary>
        /// Initializes the DeviceClient and sets up the callback to receive
        /// messages containing temperature information
        /// </summary>
        static async Task Init()
        {
            bool DisableCertCheck = true;

            //DEBUG: print all environment variables
            Console.WriteLine("DEBUG: Printing environment variables:");
            foreach(DictionaryEntry e in Environment.GetEnvironmentVariables())
            {
                Console.WriteLine(e.Key  + ":" + e.Value);
            }
            Console.WriteLine("---");

            // From https://docs.microsoft.com/en-us/azure/iot-edge/how-to-create-transparent-gateway-linux#installation-on-the-downstream-device
            Console.WriteLine("DEBUG: installing root CA certificate...");
            string certPath = "/app/azure-iot-test-only.root.ca.cert.pem";
            if (!File.Exists(certPath))
            {
                Console.WriteLine($"Missing path to root CA certificate file: {certPath}");
                throw new InvalidOperationException("Missing certificate file.");
            }
            X509Store store = new X509Store(StoreName.Root, StoreLocation.CurrentUser);
            store.Open(OpenFlags.ReadWrite);
            store.Add(new X509Certificate2(X509Certificate2.CreateFromCertFile(certPath)));
            store.Close();

            // Code copied over from FilterModule
            AmqpTransportSettings amqpSetting = new AmqpTransportSettings(TransportType.Amqp_Tcp_Only);
            if (DisableCertCheck) {
                amqpSetting.RemoteCertificateValidationCallback = new RemoteCertificateValidationCallback (ValidateServerCertificate); 
            }
            ITransportSettings[] settings = { amqpSetting };
            // Open a connection to the Edge runtime
            ModuleClient ioTHubModuleClient = await ModuleClient.CreateFromEnvironmentAsync(settings);
            await ioTHubModuleClient.OpenAsync();
            Console.WriteLine("IoT Hub module client initialized.");



            /* Original code, part of it was in Main()

            // The Edge runtime gives us the connection string we need -- it is injected as an environment variable
            var connectionString = Environment.GetEnvironmentVariable("EdgeHubConnectionString");

            // Cert verification is not yet fully functional when using Windows OS for the container
            var bypassCertVerification = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);
            if (!bypassCertVerification) InstallCert();
            await Init(connectionString, bypassCertVerification);

            Console.WriteLine("Connection String {0}", connectionString);

            var mqttSetting = new MqttTransportSettings(TransportType.Mqtt_Tcp_Only);
            // During dev you might want to bypass the cert verification. It is highly recommended to verify certs systematically in production
            if (bypassCertVerification)
            {
                mqttSetting.RemoteCertificateValidationCallback = (sender, certificate, chain, sslPolicyErrors) => true;
            }
            ITransportSettings[] settings = { mqttSetting };

            // Open a connection to the Edge runtime
            var ioTHubModuleClient = DeviceClient.CreateFromConnectionString(connectionString, settings);
            await ioTHubModuleClient.OpenAsync();
            Console.WriteLine("IoT Hub module client initialized.");
            */

            var moduleTwin = await ioTHubModuleClient.GetTwinAsync();
            var moduleTwinCollection = moduleTwin.Properties.Desired;
            desiredPropertiesData = new DesiredPropertiesData(moduleTwinCollection);

            // callback for updating desired properties through the portal or rest api
            await ioTHubModuleClient.SetDesiredPropertyUpdateCallbackAsync(OnDesiredPropertiesUpdate, null);

            // this direct method will allow to reset the temperature sensor values back to their initial state
            await ioTHubModuleClient.SetMethodHandlerAsync("reset", ResetMethod, null);

            // we don't pass ioTHubModuleClient as we're not sending any messages out to the message bus
            await ioTHubModuleClient.SetInputMessageHandlerAsync("control", ControlMessageHandler, null);

            // as this runs in a loop we don't await
            SendSimulationData(ioTHubModuleClient);
        }

        private static async Task SendSimulationData(ModuleClient deviceClient)
        {
            while(true)
            {
                try
                {
                    if(desiredPropertiesData.SendData)
                    {
                        counter++;
                        if(counter == 1)
                        {
                            // first time execution needs to reset the data factory
                            IsReset = true;
                        }

                        var messageBody = TemperatureDataFactory.CreateTemperatureData(counter, generationPolicy, IsReset);
                        IsReset = false;
                        var messageString = JsonConvert.SerializeObject(messageBody);
                        var messageBytes = Encoding.UTF8.GetBytes(messageString);
                        var message = new Message(messageBytes);
                        message.ContentEncoding = "utf-8"; 
                        message.ContentType = "application/json"; 

                        await deviceClient.SendEventAsync("temperatureOutput", message);
                        Console.WriteLine($"\t{DateTime.UtcNow.ToShortDateString()} {DateTime.UtcNow.ToLongTimeString()}> Sending message: {counter}, Body: {messageString}");

                    }
                    await Task.Delay(TimeSpan.FromSeconds(desiredPropertiesData.SendInterval));
                }
                catch(Exception ex)
                {
                    Console.WriteLine($"[ERROR] Unexpected Exception {ex.Message}" );
                    Console.WriteLine($"\t{ex.ToString()}");
                }
            }

        }

        private static Task OnDesiredPropertiesUpdate(TwinCollection twinCollection, object userContext)
        {
            desiredPropertiesData = new DesiredPropertiesData(twinCollection);
            return Task.CompletedTask;
        }


        private static Task<MethodResponse> ResetMethod(MethodRequest request, object userContext)
        {
            var response = new MethodResponse((int) HttpStatusCode.OK);
            Console.WriteLine("Received reset command via direct method invocation");
            Console.WriteLine("Resetting temperature sensor...");
            IsReset = true;
            return Task.FromResult(response);
        }

        private static Task<MessageResponse> ControlMessageHandler(Message message, object userContext)
        {
            var messageBytes = message.GetBytes();
            var messageString = Encoding.UTF8.GetString(messageBytes);

            Console.WriteLine($"Received message Body: [{messageString}]");

            try
            {
                var messages = JsonConvert.DeserializeObject<ControlCommand[]>(messageString);
                foreach (ControlCommand messageBody in messages)
                {
                    if (messageBody.Command == ControlCommandEnum.Reset)
                    {
                        Console.WriteLine("Resetting temperature sensor..");
                        IsReset = true;
                    }
                    else
                    {
                        //NoOp
                        Console.WriteLine("Received NOOP message");
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to deserialize control command with exception: [{ex.Message}]");
            }

            return Task.FromResult(MessageResponse.Completed);
        }
    }
}
