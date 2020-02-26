#ifndef RECEIVER_H
#define RECEIVER_H

// Main file for telemeter build

#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <SPI.h>

//For SD Card
#include <SPI.h>
//#include <SdFat.h>
//#include <TinyGPS++.h>

#include "imu.h"
#include "pressure.h"
#include "flash.h"
#include "radio.h"

void logLineOfDataToSDCard();

#endif
