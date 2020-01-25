#ifndef IMU_H
#define IMU_H

#include <stdint.h>
 
class IMU {
public:
    void init();
    void enterSleep();
    void queryCalibrated();
    uint8_t cal_sys, cal_gyr, cal_acc, cal_mag; // 3=fully calibrated, 0=not calibrated
    void queryData();
    int16_t AX, AY, AZ;  // Linear acceleration - 1LSB=0.01m/s^2
    int16_t QW, QX, QY, QZ; // Quaternion orientation - Unitless, fullscale value 2^14
    int16_t RP, RR, RY; // Euler rotation angles, pitch is -180 to 180, roll is -90 to 90, yaw is 0 to 360, 1LSB = 1/16 of 1 degree
};
 
#endif