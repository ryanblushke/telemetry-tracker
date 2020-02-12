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