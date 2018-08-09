cmake_minimum_required(VERSION 2.8.11)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set (CMAKE_C_FLAGS "--std=c99 -g ${CMAKE_C_FLAGS}")
endif()

include_directories("/usr/include/azureiot"
                    "/usr/include/azureiot/inc/"
                    "/usr/local/include/azureiot"
                    "/usr/local/include/azureiot/inc/")

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(SOURCE main.c bme280.c bme280.h wiring.h)
  add_executable(app ${SOURCE})

  target_link_libraries(app wiringPi
                          serializer
                          iothub_client
                          iothub_client_mqtt_transport
                          umqtt
                          aziotsharedutil
                          ssl
                          crypto
                          curl
                          pthread
                          m
                          ssl
                          crypto)
else()
  set(SOURCE main.c wiring.h)
  add_executable(app ${SOURCE})

  target_link_libraries(app serializer
                          iothub_client
                          iothub_client_mqtt_transport
                          umqtt
                          aziotsharedutil
                          ssl
                          crypto
                          curl
                          pthread
                          m)
endif()
