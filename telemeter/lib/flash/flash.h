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
#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

class Flash {
private:
public:
  void init();
	uint8_t readByte(uint16_t addr);
	void writeByte(uint16_t addr, uint8_t data);
	void writeArray(uint8_t data[], uint8_t length, uint16_t baseaddr);
	void readArray(uint8_t data[], uint8_t length, uint16_t baseaddr);
};

#endif
