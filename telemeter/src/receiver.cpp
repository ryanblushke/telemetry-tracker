// Main file for receiver build

#include "receiver.h"

#define DEBUG true

Radio radio;

void fastBlink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
}


void setup() {
  Serial.begin(115200);
  radio.RXradioinit();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("Test");
  while (!Serial) {;} // Wait for serial channel to open

  Serial.println("Starting");
}


void loop() {
  String msg = Serial.readString();
  Serial.println("ACK: " + msg);

  if (radio.dataready()) {
    byte buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    radio.rx(buffer, 5);
    for (int i = 0; i < 5; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(", ");
    }
    Serial.println();
    Serial.print("RSSI: ");
    Serial.println(radio.rssi());
    Serial.print("SNR: ");
    Serial.println(radio.snr());
  }
}
