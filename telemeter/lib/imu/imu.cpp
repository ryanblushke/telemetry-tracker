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

#include "imu.h"

#define IC2_ADDRESS_IMU 0x28

int16_t IMU::selfTest(){
    int16_t BM;
    int16_t result;
    //Set to config mode
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x3D); // OPR_MODE Register
    Wire.write(0b00000000); // Set to CONFIGMODE
    Wire.endTransmission();

    // Execute Build In Self Test
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x3F); // SYS_TRIGGER Register
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS_IMU, 1);
    BM = Wire.read();

    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x3F); // SYS_TRIGGER Register
    Wire.write(BM | 0b00000001); // Trigger Self Test
    Wire.endTransmission();

    //TODO: Check result register
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x36); // ST_RESULT Register
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS_IMU, 1);
    result = Wire.read();
    return result;

}

void IMU::init() {
    // Reset the device
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x3F);  // SYS_TRIGGER Register
    Wire.write(0b01000000);  // Trigger reset
    Wire.endTransmission();

    // Change from config to active mode
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x3D);  // OPR_MODE Register
    Wire.write(0b00001100);  // Set to NDOF mode
    Wire.endTransmission();
}

void IMU::enterSleep() {
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x3E);  // PWR_MODE Register
    Wire.write(0b00000010);  // Set to Suspend mode
    Wire.endTransmission();
}

void IMU::queryData() {
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x1A);  // QUA_Data_w_LSB Register (The first one in the block)
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS_IMU, 20);
    RY = Wire.read() | Wire.read() << 8;  // Read 16 bit value, lsb first
    RR = Wire.read() | Wire.read() << 8;
    RP = Wire.read() | Wire.read() << 8;
    QW = Wire.read() | Wire.read() << 8;
    QX = Wire.read() | Wire.read() << 8;
    QY = Wire.read() | Wire.read() << 8;
    QZ = Wire.read() | Wire.read() << 8;
    AX = Wire.read() | Wire.read() << 8;
    AY = Wire.read() | Wire.read() << 8;
    AZ = Wire.read() | Wire.read() << 8;
}

void IMU::queryCalibrated() {
    Wire.beginTransmission(IC2_ADDRESS_IMU);
    Wire.write(0x35);  // CALIB_STAT Register
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS_IMU, 1);
    while (!Wire.available()) {;}
    uint8_t raw_value = Wire.read();
    cal_sys = (raw_value & 0b11000000) >> 6;
    cal_gyr = (raw_value & 0b00110000) >> 4;
    cal_acc = (raw_value & 0b00001100) >> 2;
    cal_mag = (raw_value & 0b00000011) >> 0;
}
