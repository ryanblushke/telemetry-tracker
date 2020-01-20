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


IMU imu;
PressureSensor pressureSensor;
int cs_sdcard = 10;
uint32_t analog_0 = 0;
float analog_0_voltage = 0;
#define LED_PIN 13
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();
uint32_t adcRead(void);

bool isLEDOn = false;

void setup() {

    Wire.begin();
    imu.init();
    pressureSensor.init();
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    

    analogReadResolution(14);
    pinMode(A0, INPUT);

    startTimer(10); // In Hz

}

void loop() {

    // handle sensor saving stuff in the interrupt
}

void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;
  // Make sure the count is in a proportional position to where it was
  // to prevent any jitter or disconnect when changing the compare value.
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  Serial.println(TC->COUNT.reg);
  Serial.println(TC->CC[0].reg);
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
  while ( GCLK->STATUS.bit.SYNCBUSY == 1 ); // wait for sync

  TcCount16* TC = (TcCount16*) TC3;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Use match mode so that the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync

  setTimerFrequency(frequencyHz);

  // Enable the compare interrupt
  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.MC0 = 1;

  NVIC_EnableIRQ(TC3_IRQn);

  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // wait for sync
}

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  // If this interrupt is due to the compare register matching the timer count
  // we toggle the LED.
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1;
    
    // toggle onboard LED so show its working
    digitalWrite(LED_PIN, isLEDOn);
    isLEDOn = !isLEDOn;

    // Get Data from IMU
    imu.queryData();

    // Get data from BMP
    pressureSensor.queryData();

    // Get data from Analog (Pitot Tube)
    analog_0 = adcRead();
    analog_0_voltage = 3.3*((float)analog_0)/4095; //convert to volts
    Serial.println(analog_0_voltage);

    // Get data from GPS
        // im thinking this mostly done in loop(), with activity here to simply write
  }
}

uint32_t adcRead(void){
    //page 914 in Datasheet
    REG_ADC_CTRLA = 2; //enable ADC
    //REG_ADC_AVGCTRL
    REG_ADC_INPUTCTRL = 0x0F001800;
    REG_ADC_SWTRIG = 2; //start the adc conversion
    while (!(REG_ADC_INTFLAG & 1));
        return REG_ADC_RESULT;
}
