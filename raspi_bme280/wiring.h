/*
* Developing IoT Solutions with Azure IoT - Microsoft Sample Code - Copyright (c) 2017 - Licensed MIT
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

#ifndef WIRING_H_
#define WIRING_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef __arm__
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "bme280.h"
#define WIRINGPI_SETUP 1
#define SPI_CHANNEL 0
#define SPI_CLOCK 1000000L
#define SPI_SETUP 1 << 2
#define BME_INIT 1 << 3
static unsigned int BMEInitMark = 0;
#elif _WIN32
#include <Windows.h>
#else
#include <linux/time.h>
#include <unistd.h>
#endif

#ifndef __arm__
#define HIGH 1
#define LOW 0
#define OUTPUT 0
#define INPUT 1
static uint64_t epochMilli;
#endif

int setupWiringPi()
{
#ifdef __arm__
	int result;
	if (result = wiringPiSetup() == 0)
    {
        BMEInitMark |= WIRINGPI_SETUP;
    }
	return result;
#elif __linux__
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	epochMilli = (uint64_t)ts.tv_sec * (uint64_t)1000 + (uint64_t)(ts.tv_nsec / 1000000L);
	return 0;
#endif
}

void setPinMode(int pinNumber, int mode)
{
#ifdef __arm__
	pinMode(pinNumber, mode);
#else
	return; //no-op if not on Pi
#endif
}

void writeToPin(int pinNumber, int value)
{
#ifdef __arm__
	digitalWrite(pinNumber, value);
#else
	return; //no-op if not on Pi
#endif
}

int readFromPin(int pinNumber)
{
#ifdef __arm__
	digitalRead(pinNumber);
#else
	return; //no-op if not on Pi
#endif
}

void wait(int duration)
{
	fflush(stdout);

#ifdef __arm__
	delay(duration);
#elif _WIN32
	Sleep(duration);
#else
	usleep(duration * 1000);
#endif
}

unsigned int milli(void)
{
#ifdef __arm__
	return millis();
#elif __linux__
	uint64_t now;

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	now = (uint64_t)ts.tv_sec * (uint64_t)1000 + (uint64_t)(ts.tv_nsec / 1000000L);

	return (uint32_t)(now - epochMilli);
#endif
}

int mask_check(int check, int mask)
{
    return (check & mask) == mask;
}

// check whether the BMEInitMark's corresponding mark bit is set, if not, try to invoke corresponding init()
int check_bme_init()
{
    #ifdef __arm__
    // wiringPiSetup == 0 is successful
    if (mask_check(BMEInitMark, WIRINGPI_SETUP) != 1 && wiringPiSetup() != 0)
    {
        return -1;
    }
    BMEInitMark |= WIRINGPI_SETUP;

    // wiringPiSetup < 0 means error
    if (mask_check(BMEInitMark, SPI_SETUP) != 1 && wiringPiSPISetup(SPI_CHANNEL, SPI_CLOCK) < 0)
    {
        return -1;
    }
    BMEInitMark |= SPI_SETUP;

    // bme280_init == 1 is successful
    if (mask_check(BMEInitMark, BME_INIT) != 1 && bme280_init(SPI_CHANNEL) != 1)
    {
        return -1;
    }
    BMEInitMark |= BME_INIT;
    #endif

    return 1;

}

#endif // WIRING_H_
