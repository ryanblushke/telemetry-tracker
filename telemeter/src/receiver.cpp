// Main file for receiver build

#include "receiver.h"

#define DEBUG true

uint8_t header = 0;
uint16_t GPS_lat_rel = 0;
uint16_t GPS_lng_rel = 0;
uint16_t altitude_rel = 0;

Radio radio;

enum State {
  SLEEP = 0,
  IDLE = 1,
  ARMED = 2,
  ACTIVE = 3,
  LANDED = 4,
  DATA_TRANSFER = 5,
  TEST = 6
};

void decodeAbsolute(byte data[]){
}

void decodeRelative(byte data[]){
  header = (data[0] & 0xE0) >> 5;
  GPS_lat_rel = ((data[0] & 0x0F) << 6) | ((data[1] & 0xFC) >> 2);
  if((data[0] & 0x10) == 0x10){
    GPS_lat_rel = GPS_lat_rel * -1;
  }
  GPS_lng_rel = ((data[1] & 0x01) << 9) | (data[2] << 1) | ((data[3] & 0x10) >> 7);
  if((data[1] & 0x02) == 0x02){
    GPS_lng_rel = GPS_lng_rel * -1;
  }
  altitude_rel = ((data[3] & 0x3F) << 4) | ((data[4] & 0xF0) >> 4);
  if((data[3] & 0x40) == 0x40){
    altitude_rel = altitude_rel * -1;
  }
}

void setup() {
  Serial.begin(115200);
  radio.RXradioinit();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

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
    decodeRelative(buffer);
    Serial.println();
    Serial.print("Latitude Relative: ");
    Serial.println(GPS_lat_rel);
    Serial.print("Longtitude Relative: ");
    Serial.println(GPS_lng_rel);
    Serial.print("Altitude Relative: ");
    Serial.println(altitude_rel);
    Serial.println();
    Serial.print("RSSI: ");
    Serial.println(radio.rssi());
    Serial.print("SNR: ");
    Serial.println(radio.snr());
  }
}
