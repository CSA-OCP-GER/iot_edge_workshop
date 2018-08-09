/*
*
* ©2017 Microsoft Corporation. All rights reserved. The text in this document is available under the 
* Creative Commons Attribution 4.0 License (https://creativecommons.org/licenses/by/4.0/legalcode), 
* additional terms may apply. All other content contained in this document (including, without limitation,
* trademarks, logos, images, etc.) are not included within the Creative Commons license grant. This 
* document does not provide you with any legal rights to any intellectual property in any Microsoft 
* product. 
*
* This document is provided "as-is." Information and views expressed in this document, including URL 
* and other Internet Web site references, may change without notice. You bear the risk of using it. 
* Some examples are for illustration only and are fictitious. No real association is intended or inferred.
* Microsoft makes no warranties, express or implied, with respect to the information provided here.
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "./wiring.h"

#ifdef __arm__
#include "./bme280.h"
#endif

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "iothub_client.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "iothubtransportmqtt.h"

//Uncomment this line to create dummy data if you do not have sensors
//#define SimulateData

int getRandom(int min, int max) 
{
    return ((rand() % (int)(((max) + 1) - (min))) + (min));
}

const int MAX_BLINK_TIMES = 20;
const int GREEN_LED_PIN = 7;
const int BLUE_LED_PIN = 22;
const int PHOTOCELL_PIN = 1;
const int BRIGHTNESS_MAX = 200000;
int totalBlinkTimes = 1;
int lastMessageSentTime = 0;
bool messagePending = false;

static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        writeToPin(GREEN_LED_PIN, HIGH);
        wait(100);
        writeToPin(GREEN_LED_PIN, LOW);
    }
    else
    {
        printf("[Device] Failed to send message to Azure IoT Hub\r\n");
    }

    messagePending = false;
}

static void sendMessageAndBlink(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle)
{
    setPinMode(PHOTOCELL_PIN, INPUT);
    #if (defined(__arm__) && !defined(SimulateData))
        int reading = 0;
        setPinMode(PHOTOCELL_PIN, OUTPUT);
        writeToPin(PHOTOCELL_PIN, LOW);
        wait(100);
        while ((readFromPin(PHOTOCELL_PIN) == LOW) && (reading < BRIGHTNESS_MAX))
	    {
	            reading += 1;
            }
    #else
        reading = getRandom(300, 1200);
    #endif

    #if (defined(__arm__) && !defined(SimulateData))
        if (check_bme_init() != 1) {
            // setup failed
            printf("failed check");
        }

        float temperature, humidity, pressure;
        if (bme280_read_sensors(&temperature, &pressure, &humidity) != 1) {
	    printf("failed read");
	}

        printf("{\"sensor\":\"bme280\", \"humidity\":%.2f, \"pressure\":%.2f, \"temperature\":%.2f }\n", humidity, pressure, temperature);
    #else
        float t = getRandom(0,100); // Random C Temperature
        float p = getRandom(800, 1013); // hPa
	float h = getRandom(0,100); // %

        printf("{\"sensor\":\"simulated\", \"humidity\":%.2f, \"pressure\":%.2f, \"temperature\":%.2f }\n", h, p, t);
    #endif

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "{ \"deviceId\": \"%s\", \"messageId\": %d, \"brightness\": %d }", "testDevice", totalBlinkTimes, reading);
    //snprintf(buffer, sizeof(buffer), "{ deviceId: %s, messageId: %d }", "testDevPi", totalBlinkTimes);

    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        printf("[Device] ERROR: unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            printf("[Device] ERROR: Failed to hand over the message to IoTHubClient\r\n");
        }
        else
        {
            lastMessageSentTime = milli();
            messagePending = true;
            printf("[Device] Sending message #%d: %s\r\n", totalBlinkTimes, buffer);
        }

        IoTHubMessage_Destroy(messageHandle);
    }
}

char *get_device_id(char *str)
{
    char *substr = strstr(str, "DeviceId=testDevPi");

    if (substr == NULL)
        return NULL;

    // skip "DeviceId="
    substr += 9;

    char *semicolon = strstr(substr, ";");
    int length = semicolon == NULL ? strlen(substr) : semicolon - substr;
    char *device_id = calloc(1, length + 1);
    memcpy(device_id, substr, length);
    device_id[length] = '\0';

    return device_id;
}

static char *readFile(char *fileName)
{
    FILE *fp;
    int size;
    char *buffer;

    fp = fopen(fileName, "rb");

    if (fp == NULL)
    {
        printf("[Device] ERROR: File %s doesn't exist!\n", fileName);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    // Allocate memory for entire content
    buffer = calloc(1, size + 1);

    if (buffer == NULL)
    {
        fclose(fp);
        printf("[Device] ERROR: Failed to allocate memory.\n");
        return NULL;
    }

    // Read the file into the buffer
    if (1 != fread(buffer, size, 1, fp))
    {
        fclose(fp);
        free(buffer);
        printf("[Device] ERROR: Failed to read the file %s into memory.\n", fileName);
        return NULL;
    }

    fclose(fp);

    return buffer;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("[Device] IoT Hub connection string should be passed as a parameter\r\n");
        return 1;
    }

    char device_id[257];
    char *device_id_src = get_device_id(argv[1]);

    if (device_id_src == NULL)
    {
        printf("[Device] ERROR: Cannot parse device id from IoT device connection string: %s\n", argv[1]);
        return 1;
    }

    snprintf(device_id, sizeof(device_id), "%s", device_id_src);
    free(device_id_src);

    setupWiringPi();
    setPinMode(BLUE_LED_PIN, OUTPUT);
    setPinMode(GREEN_LED_PIN, OUTPUT);

    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    (void)printf("[Device] Starting the IoTHub client sample MQTT...\r\n");

    if (platform_init() != 0)
    {
        printf("[Device] Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(argv[1], MQTT_Protocol)) == NULL)
        {
            (void)printf("[Device] ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            while ((totalBlinkTimes <= MAX_BLINK_TIMES) || messagePending)
            {
                if ((lastMessageSentTime + 2000 < milli()) && !messagePending)
                {
                    sendMessageAndBlink(iotHubClientHandle);
                    totalBlinkTimes++;
                }

                IoTHubClient_LL_DoWork(iotHubClientHandle);
                wait(100);
            }

            IoTHubClient_LL_Destroy(iotHubClientHandle);
        }
        platform_deinit();
    }

    return 0;
}
