// Main file for receiver build

#include "receiver.h"

#define DEBUG true

byte data[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


// Relative data packet variables
uint8_t header = 0;
int16_t GPS_lat_rel_undiv = 0;
int16_t GPS_lng_rel_undiv = 0;
int16_t altitude_rel_undiv = 0;
float GPS_lat_rel = 0;
float GPS_lng_rel = 0;
float altitude_rel = 0;

// Absolute data packet variables
int32_t GPS_lat_abs_undiv = 0;
int32_t GPS_lng_abs_undiv = 0;
int32_t altitude_abs_undiv = 0;
float GPS_lat_abs = 0;
float GPS_lng_abs = 0;
int16_t altitude_abs = 0;


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

void decodeAbsolutePacket(){
  //Latitude Decode
  GPS_lat_abs_undiv = ((data[0] & 0x7F) << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
  if((data[0] & 0x80) == 0x80) {
    GPS_lat_abs_undiv = GPS_lat_abs_undiv * -1;
  }
  GPS_lat_abs = GPS_lat_abs_undiv;
  Serial.println("Latitude Absolute before div: ");
  Serial.println(GPS_lat_abs_undiv);
  GPS_lat_abs = GPS_lat_abs_undiv / (float)10000000;

  //Longtitude Decode
  GPS_lng_abs_undiv = ((data[4] & 0x7F) << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
  if((data[4] & 0x08) == 0x80) {
    GPS_lng_abs_undiv = GPS_lng_abs_undiv * -1;
  }
  GPS_lng_abs = GPS_lng_abs_undiv;
  Serial.println("Longitude Absolute before div: ");
  Serial.println(GPS_lng_abs_undiv);
  GPS_lng_abs = GPS_lng_abs_undiv / (float)10000000;

  //Altitude Decode
  Serial.println(data[8],HEX);
  Serial.println(data[9],HEX);
  altitude_abs = (data[8] << 6);
  Serial.println(data[9] >> 2,HEX);
  altitude_abs |= (data[9] >> 2);
   Serial.println(altitude_abs);
}

void decodeRelativePacket(){
  header = (data[0] & 0xE0) >> 5;

  //Latitude Decode
  GPS_lat_rel_undiv = ((data[0] & 0x0F) << 6) | ((data[1] & 0xFC) >> 2);
  if((data[0] & 0x10) == 0x10){
    GPS_lat_rel_undiv = GPS_lat_rel_undiv * -1;
  }
  GPS_lat_rel = GPS_lat_rel_undiv;
  GPS_lat_rel = GPS_lat_rel / 100000;

  //Longtitude Decode
  GPS_lng_rel_undiv = ((data[1] & 0x01) << 9) | (data[2] << 1) | ((data[3] & 0x10) >> 7);
  if((data[1] & 0x02) == 0x02){
    GPS_lng_rel_undiv = GPS_lng_rel_undiv * -1;
  }
  GPS_lng_rel = GPS_lng_rel_undiv;
  GPS_lng_rel = GPS_lng_rel / 100000;

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
    // radio.rx(data, 5);
    // for (int i = 0; i < 5; i++) {
    //   Serial.print(data[i], HEX);
    //   Serial.print(", ");
    // }
    radio.rx(data, 10);
    for (int i = 0; i < 10; i++) {
      Serial.print(data[i], HEX);
      Serial.print(", ");
    }
    Serial.println();
    //decodeRelativePacket();
    decodeAbsolutePacket();
    Serial.println();
    // Serial.print("Latitude Relative: ");
    // Serial.println(GPS_lat_rel, 5);
    // Serial.print("Longtitude Relative: ");
    // Serial.println(GPS_lng_rel, 5);
    // Serial.print("Altitude Relative: ");
    // Serial.println(altitude_rel);
    // Serial.println();
    Serial.println();
    Serial.print("Latitude Absolute: ");
    Serial.println(GPS_lat_abs);
    Serial.print("Longitude Absolute: ");
    Serial.println(GPS_lng_abs,8);
    Serial.print("Altitude Absolute: ");
    Serial.println(altitude_abs,8);
    Serial.println();
    Serial.print("RSSI: ");
    Serial.println(radio.rssi());
    Serial.print("SNR: ");
    Serial.println(radio.snr());
  }
}
