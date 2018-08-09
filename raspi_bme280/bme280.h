/*
* IoT Hub Raspberry Pi C - Microsoft Sample Code - Copyright (c) 2017 - Licensed MIT
*
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

///////////////////////////////////////////////////////////////////////////////
//
// bme280.h:
// SPI based interface to read temperature, pressure and humidity samples from
// a BME280 module.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BME280_H_
#define BME280_H_


///////////////////////////////////////////////////////////////////////////////
// Call this after setting the chip select (or SPI Enable) pin (via
// bme280_set_cs_pin()), and before calling the bmp280_read function.
// Return: 0 if the module was not found.
//         1 if the module was readable, and verified to be a BMP280, and the
//           calibration data was read.
int bme280_init(int Chip_enable_to_use__i);

///////////////////////////////////////////////////////////////////////////////
// Prerequisite:
// You must call wiringPiSetup before calling this function. For example:
//  int Result__i = wiringPiSetup();
//  if (Result__i != 0) exit(Result__i);
// You must call wiringPiSPISetup before calling this function. For example:
//  int Spi_fd__i = wiringPiSPISetup(Spi_channel__i, Spi_clock__i);
//  if (Spi_fd__i < 0)
//  {
//    printf("Can't setup SPI, error %i calling wiringPiSPISetup(%i, %i)  %s\n",
//      Spi_fd__i, Spi_channel__i, Spi_clock__i, strerror(Spi_fd__i));
//    exit(Spi_fd__i);
//  }
//
// Param: Temp_C__fp  Pointer to a float to receive the current temperature in
//                    degrees Celcius. Only set if read is successful.
// Param: Pres_Pa__fp  Pointer to a float to receive the current pressure
//                     as hPa. Only set if read is successful.
// Param: Hum_pct__fp  Pointer to a float to receive the current humidity
//                     as a percentage. Only set if read is successful.
// Return: If wiringPi gets an error, this will be < 0
//         If the read attempts fail, this will be 1
//         If the read succeeds within the available retries, returns 0
int bme280_read_sensors(float * Temp_C__fp, float * Pres_Pa__fp, float * Hum_pct__fp);

#endif  // BME280_H_