// Timer stuff
// https://gist.github.com/jdneo/43be30d85080b175cb5aed3500d3f989

// GPS Stuff
// https://github.com/adafruit/Adafruit_GPS



#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <SPI.h>

#include "imu.h"
#include "pressure.h"
#include "flash.h"

//For SD Card
#include <SPI.h>
#include <SdFat.h>
// #include <Adafruit_GPS.h>
#include <TinyGPS++.h>

const int chipSelect = 10; 
SdFat sd;
SdFile file;


IMU imu;
PressureSensor pressureSensor;
// int cs_sdcard = 10;
// uint32_t analog_0 = 0;
// float analog_0_voltage = 0;
// #define CPU_HZ 48000000
// #define TIMER_PRESCALER_DIV 1024

// void startTimer(int frequencyHz);
// void setTimerFrequency(int frequencyHz);
// void TC3_Handler();
// uint32_t adcRead(void);

// bool isLEDOn = false;


Flash flash;

// Adafruit_GPS GPS(&Serial1);
TinyGPSPlus gps;

void fastBlink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();
    imu.init();
    flash.init();
    pressureSensor.init();
    
    // startTimer(10); // In Hz
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(8, OUTPUT); // Disable RFM96 radio SPI interface
    digitalWrite(8, HIGH);

    // while (!Serial) {;} // Wait for serial channel to open

    analogReadResolution(12);
    pinMode(A0, INPUT);
    REG_ADC_CTRLB = 0x0110; // PRESCALER=0x1, RESSEL=0x1 - 16 bit result
    REG_ADC_AVGCTRL = 0x0A; // Accumulate 1024 samples per read

    Serial.println("Starting");
    while (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
        Serial.println("initialization failed");
        fastBlink();
    }

    Serial1.begin(9600); // GPS connection
    Serial1.println("$PMTK251,38400*27"); // Set serial baud to 38400
    delay(300);
    Serial1.begin(38400);
    Serial1.println("$PMTK220,100*2F"); // Set update rate to 10HZ (Won't work at default baud)

}

uint32_t smoothedADCValue = 0;
float alpha = 0.01; // More alpha = faster response (1.0 = no smoothing, 0.001 = lots)
// bool first = true;

void updateADCReading() {
    uint32_t newValue = analogRead(A0) << 16;
    // if (REG_ADC_INTFLAG & 1 || first){ // If conversion done, start another
    //     first = false;
    //     uint32_t newValue = REG_ADC_RESULT;// << (16); // Scale up to be full scale 32 bit number
    smoothedADCValue = smoothedADCValue * (1.0 - alpha) + newValue * alpha;
    // smoothedADCValue = analogRead(A0);
    //     REG_ADC_SWTRIG = 2; //start the adc conversion
    // }
}

void logLineOfDataToSDCard() {
    // uint32_t starttime = millis();
    if (!file.open("data.txt", FILE_WRITE)) {
        Serial.println("File Open Error");
        fastBlink();
        return;
    }
    imu.queryCalibrated();
    imu.queryData();
    // Serial.println("Logging");
    file.print(millis()); // System Timestamp, mS
    file.print(',');
    file.print(pressureSensor.queryData(), 4);// Atmospheric pressure, pascels
    file.print(",");
    file.print(pressureSensor.temp, 4); // Temp, deg c
    file.print(",");
    file.print(imu.cal_sys);
    file.print(",");
    file.print(imu.cal_gyr);
    file.print(",");
    file.print(imu.cal_acc);
    file.print(",");
    file.print(imu.cal_mag);
    file.print(",");
    file.print(imu.AX);
    file.print(",");
    file.print(imu.AY);
    file.print(",");
    file.print(imu.AZ);
    file.print(",");
    file.print(imu.RP);
    file.print(",");
    file.print(imu.RR);
    file.print(",");
    file.print(imu.RY);
    file.print(",");
    file.print(imu.QW);
    file.print(",");
    file.print(imu.QX);
    file.print(",");
    file.print(imu.QY);
    file.print(",");
    file.print(imu.QZ);
    file.print(",");
    file.print(float(smoothedADCValue) / pow(2.0, 32.0) * 3.3, 8);
    file.print(",");
    file.print(smoothedADCValue); // Pitot tube data
    file.print(",");
    file.print(gps.satellites.value());
    file.print(",");
    file.print(gps.hdop.hdop(), 1);
    file.print(",");
    file.print(gps.location.age());
    file.print(",");
    file.print(gps.location.lat(), 7);
    file.print(",");
    file.print(gps.location.lng(), 8);
    file.print(",");
    file.print(gps.altitude.meters(), 2);
    file.print(",");
    file.print(gps.time.hour());
    file.print(",");
    file.print(gps.time.minute());
    file.print(",");
    file.print(gps.time.second());
    file.print(",");
    file.print(gps.time.centisecond());
    file.print(",");
    file.print(gps.course.deg(), 2);
    file.print(",");
    file.print(gps.speed.mps());
    file.println("");
    
    Serial.println(float(smoothedADCValue) / pow(2.0, 32.0) * 3.3, 8);

    // Serial.print(gps.satellites.value());
    // Serial.print(",");
    // Serial.print(gps.hdop.hdop(), 1);
    // Serial.print(",");
    // Serial.print(gps.location.age());
    // Serial.print(",");
    // Serial.println(gps.location.lat(), 7);

    if (gps.location.age() > 100){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(10);
        digitalWrite(LED_BUILTIN, LOW);
    }
    file.close();
    if (!file.sync() || file.getWriteError()) {
        Serial.println("Write error");
        fastBlink();
    }
    // Serial.println(millis() - starttime);
}

uint32_t lastLogTime = 0;

void loop() {
    if (millis() > (lastLogTime + 100)) {
        lastLogTime = millis();
        logLineOfDataToSDCard();
    }
    updateADCReading();

    // Update GPS Parser
    while (Serial1.available()) {
        char c = Serial1.read();
        // Serial.write(c);
        gps.encode(c);
    }
}


// void setTimerFrequency(int frequencyHz) {
//     int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
//     TcCount16* TC = (TcCount16*) TC3;
//     // Make sure the count is in a proportional position to where it was
//     // to prevent any jitter or disconnect when changing the compare value.
//     TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
//     TC->CC[0].reg = compareValue;
//     Serial.println(TC->COUNT.reg);
//     Serial.println(TC->CC[0].reg);
//     while (TC->STATUS.bit.SYNCBUSY == 1);
// }

// void startTimer(int frequencyHz) {
//     REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
//     while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

//     TcCount16* TC = (TcCount16*) TC3;

//     TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
//     while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

//     // Use the 16-bit timer
//     TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
//     while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

//     // Use match mode so that the timer counter resets when the count matches the compare register
//     TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
//     while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

//     // Set prescaler to 1024
//     TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
//     while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

//     setTimerFrequency(frequencyHz);

//     // Enable the compare interrupt
//     TC->INTENSET.reg = 0;
//     TC->INTENSET.bit.MC0 = 1;

//     NVIC_EnableIRQ(TC3_IRQn);

//     TC->CTRLA.reg |= TC_CTRLA_ENABLE;
//     while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
// }

// void TC3_Handler() {
//     TcCount16* TC = (TcCount16*) TC3;
//     // If this interrupt is due to the compare register matching the timer count
//     // we toggle the LED.
//     if (TC->INTFLAG.bit.MC0 == 1) {
//         TC->INTFLAG.bit.MC0 = 1;
        
//         // toggle onboard LED so show its working
//         digitalWrite(LED_PIN, isLEDOn);
//         isLEDOn = !isLEDOn;

//         // Get Data from IMU
//         imu.queryData();

//         // Get data from BMP
//         pressureSensor.queryData();

//         // Get data from Analog (Pitot Tube)
//         analog_0 = adcRead();
//         analog_0_voltage = 3.3*((float)analog_0)/4095; //convert to volts
//         Serial.println(analog_0_voltage);

//         // Get data from GPS
//             // im thinking this mostly done in loop(), with activity here to simply write
//     }
// }

// uint32_t adcRead(void){
//     //page 914 in Datasheet
//     REG_ADC_CTRLA = 2; //enable ADC
//     //REG_ADC_AVGCTRL
//     REG_ADC_INPUTCTRL = 0x0F001800;
//     REG_ADC_SWTRIG = 2; //start the adc conversion
//     while (!(REG_ADC_INTFLAG & 1));
//         return REG_ADC_RESULT;
// }