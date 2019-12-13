#include <Arduino.h>
#include <Wire.h>

#include "imu.h"

IMU imu;

void setup() {
    Wire.begin();
    imu.init();
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
}