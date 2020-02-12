#ifndef TELEMETER_H
#define TELEMETER_H

// Main file for telemeter build

#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <SPI.h>

//For SD Card
#include <SPI.h>
#include <SdFat.h>
#include <TinyGPS++.h>

#include "imu.h"
#include "pressure.h"
#include "flash.h"

void logLineOfDataToSDCard();

#endif
