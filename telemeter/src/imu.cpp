#include <Arduino.h>
#include <Wire.h>

#include "imu.h"

#define IC2_ADDRESS 0x28

void IMU::init() {
    // Reset the device
    Wire.beginTransmission(IC2_ADDRESS);
    Wire.write(0x3F);  // SYS_TRIGGER Register
    Wire.write(0b01000000);  // Trigger reset
    Wire.endTransmission();
    
    // Change from config to active mode
    Wire.beginTransmission(IC2_ADDRESS);
    Wire.write(0x3D);  // OPR_MODE Register
    Wire.write(0b00001100);  // Set to NDOF mode
    Wire.endTransmission();
}

void IMU::enterSleep() {
    Wire.beginTransmission(IC2_ADDRESS);
    Wire.write(0x3E);  // PWR_MODE Register
    Wire.write(0b00000010);  // Set to Suspend mode
    Wire.endTransmission();
}

void IMU::queryData() {
    Wire.beginTransmission(IC2_ADDRESS);
    Wire.write(0x20);  // QUA_Data_w_LSB Register (The first one in the block)
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS, 14);
    QW = Wire.read() | Wire.read() << 8;  // Read 16 bit value, lsb first
    QX = Wire.read() | Wire.read() << 8; 
    QY = Wire.read() | Wire.read() << 8; 
    QZ = Wire.read() | Wire.read() << 8; 
    AX = Wire.read() | Wire.read() << 8; 
    AY = Wire.read() | Wire.read() << 8; 
    AZ = Wire.read() | Wire.read() << 8; 
}

void IMU::queryCalibrated() {
    Wire.beginTransmission(IC2_ADDRESS);
    Wire.write(0x35);  // CALIB_STAT Register
    Wire.endTransmission();
    Wire.requestFrom(IC2_ADDRESS, 1);
    while (!Wire.available()) {;}
    uint8_t raw_value = Wire.read();
    cal_sys = (raw_value & 0b11000000) >> 6;
    cal_gyr = (raw_value & 0b00110000) >> 4;
    cal_acc = (raw_value & 0b00001100) >> 2;
    cal_mag = (raw_value & 0b00000011) >> 0;
}