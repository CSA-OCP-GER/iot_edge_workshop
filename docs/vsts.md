# Creating a VSTS Pipeline for Azure IoT Edge

After you have created a project and linked it to your Github repository, we can create a Build pipeline for Continuous Integration. The first thing we will do in our pipelin is defining some variables with the ACR credentials and the tag we want to assign to our Docker images.

![Variables](pics/vsts_variables.PNG "Variables")

The first step of the build pipeline will be getting the source code from Github. You should have this step automatically included in your pipeline, if you associated your project to Github.

![VSTS Pipeline Get Sources](pics/vsts_pipeline_get_sources.PNG "VSTS Pipeline Get Sources")

Since you removed all confidential information from the source code, you can replace the variables in VSTS to include the actual credentials. We will do that in the `module.json` file and in the `deployment.template.json file`. We will use a variable as well to set the tag to the build number. You will probably need to install the Token Replacement extension in your VSTS environment.

![Token Replacement](pics/vsts_pipeline_replace_module_json.PNG "Token Replacement")

![Token Replacement](pics/vsts_pipeline_replace_deployment_template_json.PNG "Token Replacement")

We are ready to start building our Docker image. We can use the Docker tasks to build and push the image.

![VSTS Build Image](pics/vsts_pipeline_build_image.PNG "VSTS Build Image")

![VSTS Push Image](pics/vsts_pipeline_push_image.PNG "VSTS Push Image")

The containers are created, now we can deploy them. The most comfortable way is using the Azure IoT Edge extension for that (you will probably have to install it too). Note that this extension can be configured to build and push the containers, but since we already did that with the Docker tasks, we are using it only to deploy those containers to the IoT hub:

![VSTS Azure IoT Edge Deployment](pics/vsts_pipeline_iot_edge_deploy.PNG "VSTS Azure IoT Edge Deployment")

Lastly, you could configure the pipeline to run automatically after every commit to the Github repository where the code is based on. You would do that in the Triggers section of the pipeline:

![VSTS Pipeline Triggers](pics/vsts_pipeline_triggers.PNG "VSTS Pipeline Triggers")