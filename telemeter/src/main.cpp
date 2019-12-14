#include <Arduino.h>
#include <Wire.h>

#include "imu.h"
#include "pressure.h"

IMU imu;
PressureSensor pressureSensor;

void setup() {
    Wire.begin();
    imu.init();
    pressureSensor.init();
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}