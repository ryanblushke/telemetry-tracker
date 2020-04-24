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
#ifndef PRESSURE_H
#define PRESSURE_H

#include <stdint.h>

class PressureSensor {
private:
    // Calibration coefficients
    float PAR_T1, PAR_T2, PAR_T3;
    float PAR_P1, PAR_P2, PAR_P3, PAR_P4, PAR_P5;
    float PAR_P6, PAR_P7, PAR_P8, PAR_P9, PAR_P10, PAR_P11;
public:
    void init();
    void enterSleep();
    float queryData();  // Absolute pressure in Pascals
    float temp; // Updated on querying pressure
};

#endif
