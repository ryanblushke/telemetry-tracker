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

#include "pressure.h"

#define IC2_ADDRESS_PRESSURE 0x77

void PressureSensor::init() {
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x7E);  // CMD Register
    Wire.write(0xB6);  // Trigger soft reset
    Wire.endTransmission();
    delay(10); // Wait for sensor to finish booting
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x1B);  // PWR_CTRL Register
    Wire.write(0b00110011);  // Put in Normal mode, pressure and temprature sensors on
    Wire.endTransmission();

    // Oversampling
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x1C);  // OSR Register
    Wire.write(0b00010101);  // 32x pressure, 2x temprature oversampling
    Wire.endTransmission();

    // Sampling rate. Must be slow enough to accomadate all oversampling
    // Can't be faster then 0x04 (12.5Hz) for 32x pressure, 2x temp
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x1D);  // ODR Register
    Wire.write(0x04);  // 12.5Hz
    Wire.endTransmission();

    // Filtering. At 12.5Hz sampling rate filter coefficients will give different settling times
    // 3 => 1s, 7 => 2s, 15 => 5s
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x1F);  // Config Register
    Wire.write(0b00000110); // coef 7
    Wire.endTransmission();

    // Read out calibration data
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x31);  // First register in block
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS_PRESSURE, 21);
    PAR_T1 = (Wire.read() | Wire.read() << 8) / pow(2, -8);
    PAR_T2 = (Wire.read() | Wire.read() << 8) / pow(2, 30);
    PAR_T3 = (int8_t)Wire.read() / pow(2, 48);
    PAR_P1 = ((int16_t)(Wire.read() | Wire.read() << 8) - pow(2, 14)) / pow(2, 20);
    PAR_P2 = ((int16_t)(Wire.read() | Wire.read() << 8) - pow(2, 14)) / pow(2, 29);
    PAR_P3 = (int8_t)Wire.read() / pow(2, 32);
    PAR_P4 = (int8_t)Wire.read() / pow(2, 37);
    PAR_P5 = (Wire.read() | Wire.read() << 8) / pow(2, -3);
    PAR_P6 = (Wire.read() | Wire.read() << 8) / pow(2, 6);
    PAR_P7 = (int8_t)Wire.read() / pow(2, 8);
    PAR_P8 = (int8_t)Wire.read() / pow(2, 15);
    PAR_P9 = (int16_t)(Wire.read() | Wire.read() << 8) / pow(2, 48);
    PAR_P10 = (int8_t)Wire.read() / pow(2, 48);
    PAR_P11 = (int8_t)Wire.read() / pow(2, 65);
}

void PressureSensor::enterSleep() {
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x1B);  // PWR_CTRL Register
    Wire.write(0b00000000);  // Put in Sleep mode
    Wire.endTransmission();
}

float PressureSensor::queryData() {
    Wire.beginTransmission(IC2_ADDRESS_PRESSURE);
    Wire.write(0x04);  // Pressure data Register 3 registers, lsb first
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS_PRESSURE, 6);
    float raw_pressure = Wire.read() | Wire.read() << 8 | Wire.read() << 16;
    float raw_temprature = Wire.read() | Wire.read() << 8 | Wire.read() << 16;

    float partial_data1, partial_data2, partial_data3, partial_data4, partial_out1, partial_out2;

    partial_data1 = (raw_temprature - PAR_T1);
    partial_data2 = (partial_data1 * PAR_T2);
    temp = partial_data2 + (partial_data1 * partial_data1) * PAR_T3;

    partial_data1 = PAR_P6 * temp;
    partial_data2 = PAR_P7 * (temp * temp);
    partial_data3 = PAR_P8 * (temp * temp * temp);
    partial_out1 = PAR_P5 + partial_data1 + partial_data2 + partial_data3;
    partial_data1 = PAR_P2 * temp;
    partial_data2 = PAR_P3 * (temp * temp);
    partial_data3 = PAR_P4 * (temp * temp * temp);
    partial_out2 = raw_pressure * (PAR_P1 + partial_data1 + partial_data2 + partial_data3);
    partial_data1 = raw_pressure * raw_pressure;
    partial_data2 = PAR_P9 + PAR_P10 * temp;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + (raw_pressure * raw_pressure * raw_pressure) * PAR_P11;
    float press = partial_out1 + partial_out2 + partial_data4;
    return press;
}
