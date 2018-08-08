# Workshop on IoT Edge

This repository contains content for hands-on on IoT Edge. Whether you use this to learn at your own pace, to deliver a guided workshop or a hackathon, it is up to you. The following topics are included:

* Initializing a Raspberry Pi to act as IoT Edge Gateway
* Create an IoT Edge solution using Visual Studio Code
* Deploying code to an ARM-based IoT Edge Gateway with a Continuous Integration pipeline
* Using VSTS and a VSTS build agents to build ARM-based Linux containers

If you want to try yourself, these are the overall steps you should follow. Next to each step you have a "hints" document, with detailed descriptions about how to execute it.

## Option 1. ARM-based IoT Edge Gateway

For this option you need ARM-based hardware that represents your IoT Edge Gateway

1. Install Raspbian on a Raspberry Pi, as well as the IoT Edge bits. [Hints](docs/ARM-raspbian.md)
2. Create an IoT hub in Azure, and connect the Raspberry Pi to your IoT Hub as IoT Edge Device. [Hints](docs/iothub.md)
3. Create an Azure Container Registry. Create a service principal with read-only access to your ACR. [Hints](docs/acr.md)
4. Install the Azure IoT Edge extensions on VS Code, and create an IoT Edge Solution. Commit your code to a Github repository. [Hints](docs/vsc.md)
5. Create a Linux VM that we will use as VSTS Build Agent. Install the Azure SDK (CLI 2.0), the IoT extensions and QEMU (to build ARM-based containers). [Hints](docs/buildagent.md)
6. Create a CI pipeline in VSTS that will orchestrate the building/pushing of a container and the deployment to the IoT Edge device over IoT hub. [Hints](vsts.md)

## Option 2. x86-based IoT Edge Gateway

You can run these exercises without having to purchase ARM hardware. You can use this [Quickstart guide](https://docs.microsoft.com/en-us/azure/iot-edge/quickstart) or refer to these more generic documents:

1. Create a VM in Azure with Linux/Windows, install the IoT Edge bits
2. Create an IoT hub in Azure, and connect the IoT Edge software on your VM to your IoT Hub as IoT Edge Device. [Hints](docs/iothub.md)
3. Create an Azure Container Registry. Create a service principal with read-only access to your ACR. [Hints](docs/acr.md)
4. Install the Azure IoT Edge extensions on VS Code, and create an IoT Edge Solution. Commit your code to a Github repository. [Hints](docs/vsc.md)
5. Create a CI pipeline in VSTS that will orchestrate the building/pushing of a container and the deployment to the IoT Edge device over IoT hub. [Hints](vsts.md)
