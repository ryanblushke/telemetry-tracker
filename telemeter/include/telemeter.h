/*
    telemetry-tracker is a telemetry tracker system for model rockets/airplanes.

    Copyright (C) 2020 Alex McNabb, Ryan Blushke, Torban Peterson, Scott Seidle

    This file is part of telemetry-tracker.

    telemetry-tracker is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    telemetry-tracker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with telemetry-tracker.  If not, see <https://www.gnu.org/licenses/>.
 */
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
#include "radio.h"

void logLineOfDataToSDCard();

#endif
