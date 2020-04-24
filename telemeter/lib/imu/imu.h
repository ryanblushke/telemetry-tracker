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
#ifndef IMU_H
#define IMU_H

#include <stdint.h>

class IMU {
public:
    void init();
    void enterSleep();
    void queryCalibrated();
    int16_t selfTest();
    uint8_t cal_sys, cal_gyr, cal_acc, cal_mag; // 3=fully calibrated, 0=not calibrated
    void queryData();
    int16_t AX, AY, AZ;  // Linear acceleration - 1LSB=0.01m/s^2
    int16_t QW, QX, QY, QZ; // Quaternion orientation - Unitless, fullscale value 2^14
    int16_t RP, RR, RY; // Euler rotation angles, pitch is -180 to 180, roll is -90 to 90, yaw is 0 to 360, 1LSB = 1/16 of 1 degree
};

#endif
