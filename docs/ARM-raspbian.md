# Install Raspbian on a Raspberry Pi with Azure IoT Edge code

There are multiple guides out there that will help you to install an OS (Raspbian in this case) on a Raspberry Pi. For example, you have some instructions in the official Raspberry documentation: https://www.raspberrypi.org/documentation/installation/installing-images/. You can download Raspbian from https://www.raspberrypi.org/downloads/raspbian/.

Note that the default credentials for Raspbian are pi/raspberry (pi is the username, raspberry hte password).

After you have an OS on your Raspberry Pi, you need to configured networking and SSH connectivity:
* Enable SSH: https://www.raspberrypi.org/documentation/remote-access/ssh/
* https://www.raspberrypi.org/learning/software-guide/wifi/

The last step is installing the Azure IoT Edge software. You can follow this guide to do so: https://docs.microsoft.com/en-us/azure/iot-edge/how-to-install-iot-edge-linux-arm. Note that the latest IoT Edge bits use a daemon to connect to an IoT Hub, instead of the old iotedgectl command. IoT Hub connectivity settings are specified in the file `/etc/iotedge/config.yaml`, but we do not have yet an IoT Hub to connect to.