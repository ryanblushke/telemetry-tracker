#include <Arduino.h>
#include <Wire.h>

#include "imu.h"

#define IC2_ADDRESS_IMU 0x28

Void IMU::selfTest(){
    int16_t BM;
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
    Wire.beginTransmission();
    Wire.write(0x36); // ST_RESULT Register
    
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
