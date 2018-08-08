# Create an Azure Container Registry

This guide will help you to create an Azure Container Registry, and a Service Principal with read-only authentication to that ACR. 

## Option 1: Azure CLI

As usual, let us define some variables:

```
acrname=iotacr1310
spname=iotedgeapp
sppwd=mysupersecretpassword
```

The variable `$rgname` is defined in a previous step ([IoT Hub](iothub.md)). If you didn't follow that guide, you can define that variable now:

```
rgname=iottest
```

Now you can create the Azure Container Registry. 

```
az acr create -g $rgname -n $acrname --sku Basic
```

The IoT Edge devices only need to download container images from our ACR. Therefore we will distribute read-only credentials, that allow to download images from our ACR, but not to push or delete images. The default ACR admin user has read-write privilege, and you cannot create additional admin users. But you can create a Service Principal with Reader access to the ACR, and use that SP as docker authentication credentials. 

```
acrid=$(az acr show -g $rgname -n $acrname --query id -o tsv)
az ad sp create-for-rbac -n $spname --role reader --scopes $acrid
az ad sp credential reset --id $spname -p $sppwd
acrurl=$(az acr show -g $rgname -n $acrname --query loginServer -o tsv)
appid=$(az ad sp show --id http://$spname --query appId -o tsv)
```

If you get an error with the az ad sp command, claiming that you need to use the --name option, please do so:

```
az ad sp credential reset --name $spname -p $sppwd
acrurl=$(az acr show -g $rgname -n $acrname --query loginServer -o tsv)
appid=$(az ad sp show --id http://$spname --query appId -o tsv)
```


Now you can test that Docker authentication works with these read-only credentials. If you tried to push an image, it would fail, but pulling images should work (note that you should have a working Docker installation to be able to test this step):

```
docker login -u $appid -p $sppwd $acrurl
```

In case you need the admin credentials for your ACR, you can get them as follows:

```
az acr update -g $rgname -n $acrname --set adminUserEnabled=true
acr_adminusr=$(az acr credential show -n $acrname -g $rgname --query username -o tsv)
acr_adminpwd=$(az acr credential show -n $acrname -g $rgname --query passwords[].value -o tsv | head -1)
```

After pushing images to your repository, you can have a look at them with these commands. In ACR parlance, a repository is the same as an image.

```
az acr repository list -n $acrname -o table
az acr repository show-tags -n $acrname --repository your_image_name
```

## Option 2: Powershell

Work in progres...