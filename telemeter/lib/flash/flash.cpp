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
#include <Arduino.h>
#include <Wire.h>
#include <cmath>

#include "flash.h"

#define PAGE_SIZE 256
#define MIN_ERASE_SIZE 4096

const int PIN_CS = 5;

void Flash::init() {
    pinMode(PIN_CS, OUTPUT);
	digitalWrite(PIN_CS, HIGH); // Disable
}

void Flash::writeArray(uint8_t data[], uint8_t length, uint16_t baseaddr){
	for(uint8_t i = 0; i < length; i++)
		writeByte(baseaddr + i, data[i]);
}

void Flash::readArray(uint8_t data[], uint8_t length, uint16_t baseaddr){
	for(uint8_t i = 0; i < length; i++)
		data[i] = readByte(baseaddr + i);
}
