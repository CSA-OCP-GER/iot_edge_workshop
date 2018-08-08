# Create an Azure IoT Hub

This guide will help you to create an Azure IoT Hub, and an IoT Edge Gateway. 

## Option 1: Azure CLI

In this guide we will use Linux. The commands for Windows should be similar, but the handling of the variables would be different (as well as some particularities about how Linux and Windows understand single and double quotes). But first things first: you need to install the Azure IoT extension for the Azure CLI:

```
az extension add --name azure-cli-iot-ext
```


Now let us define some variables that will help us along the way:

```
rgname=iottest
location=westeurope
iothubname=iothub1310
iotedgename=myedgepi
```


You can use this Azure CLI statements to create a resource group, where to deploy the rest of the artifacts of this lab.

```
az group create -n $rgname -l $location
```


Now you can deploy an Azure IoT hub to that resource group:

```
az iot hub create -g $rgname -n $iothubname --sku F1
iothubcxstr=$(az iot hub show-connection-string -n $iothubname -o tsv)
az iot hub device-identity create -n $iothubname -d $iotedgename --edge-enabled
iotedgecxstr=$(az iot hub device-identity show-connection-string -d $iotedgename -n $iothubname -o tsv)
```

## Option 2: Powershell

Work in progres...