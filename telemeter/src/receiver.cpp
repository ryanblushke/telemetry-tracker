// Main file for receiver build

#include "receiver.h"

#define DEBUG true
#define MAXVOLT 4.193

byte stateChange[1] = {0xFF};
byte data[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Relative data packet variables
uint8_t header = 0;
int16_t GPS_lat_rel_undiv = 0;
int16_t GPS_lng_rel_undiv = 0;
int16_t alt_rel = 0;

// Absolute data packet variables
int32_t GPS_lat_abs_undiv = 0;
int32_t GPS_lng_abs_undiv = 0;
int16_t alt_abs = 0;

// Battery voltage
int8_t battVoltBits = 0;
float battVolt = 0;

// State print flags
int idleStatePrinted = 0;
int armedStatePrinted = 0;
int activeStatePrinted = 0;
int landedStatePrinted = 0;

Radio radio;

enum State {
  IDLE = 1,
  ARMED = 2,
  ACTIVE = 3,
  DATA_TRANSFER = 5,
  TEST = 6
};

enum State curr_state = IDLE;

String serialBuffer = "";
// Checks for incoming characters, and returns a full line terminated by \r\n
// If not possible, returns ""
String lookForSerialCommand(){
  while (Serial.available()){
    serialBuffer += (char)Serial.read();
    if (serialBuffer.endsWith("\r\n")){
      String temp = serialBuffer;
      temp.remove(temp.length() - 2);
      serialBuffer = "";
      return temp;
    }
  }
  return "";
}

void decodeAbsolutePacket() {
  //Latitude Decode
  GPS_lat_abs_undiv = ((data[0] & 0x7F) << 24) | (data[1] << 16) |
                      (data[2] << 8) | data[3];
  if ((data[0] & 0x80) == 0x80) {
    GPS_lat_abs_undiv = GPS_lat_abs_undiv * -1;
    if (DEBUG) Serial.println("***NEGATIVE_LAT***");
  }
  if (DEBUG) Serial.println("Latitude Absolute before div: ");
  if (DEBUG) Serial.println(GPS_lat_abs_undiv);

  //Longitude Decode
  GPS_lng_abs_undiv = ((data[4] & 0x7F) << 24) | (data[5] << 16) |
                      (data[6] << 8) | data[7];
  if ((data[4] & 0x80) == 0x80) {
    GPS_lng_abs_undiv = GPS_lng_abs_undiv * -1;
    if (DEBUG) Serial.println("***NEGATIVE_LNG***");
  }
  if (DEBUG) Serial.println("Longitude Absolute before div: ");
  if (DEBUG) Serial.println(GPS_lng_abs_undiv);

  //Altitude Decode
  alt_abs = (data[8] << 6);
  alt_abs |= (data[9] >> 2);
  if (DEBUG) Serial.println(alt_abs);
}

// TODO: Updated design documentation differs from current implementation.
// This decoding format is now deprecated.
void decodeRelativePacket() {
  header = (data[0] & 0xE0) >> 5;

  //Latitude Decode
  GPS_lat_rel_undiv = ((data[0] & 0x0F) << 6) | ((data[1] & 0xFC) >> 2);
  if ((data[0] & 0x10) == 0x10) {
    GPS_lat_rel_undiv = GPS_lat_rel_undiv * -1;
  }

  //Longitude Decode
  GPS_lng_rel_undiv = ((data[1] & 0x01) << 9) | (data[2] << 1) | ((
                        data[3] & 0x10) >> 7);
  if ((data[1] & 0x02) == 0x02) {
    GPS_lng_rel_undiv = GPS_lng_rel_undiv * -1;
  }

  alt_rel = ((data[3] & 0x3F) << 4) | ((data[4] & 0xF0) >> 4);
  if ((data[3] & 0x40) == 0x40) {
    alt_rel = alt_rel * -1;
  }
  battVoltBits = data[4] & 0x0F;
  battVolt = (battVoltBits - 15) * 0.0510625 + MAXVOLT;
}

enum State idleHandler(void) {
  if (idleStatePrinted == 0) {
    Serial.println("STATECHANGE:IDLE");
    idleStatePrinted = 1;
  }
  radio.initialize(TX, 1);
  String msg = lookForSerialCommand();
  if (msg == "ARM") {
    for (int i = 0; i < 10; i++) {
      if (DEBUG) Serial.println("Transmitting ARM");
      radio.tx(stateChange, 1);
      delay(20);
    }
    return ARMED;
  }
  return IDLE;
}

enum State armedHandler(void) {
  radio.initialize(RX, 10);
  if (radio.dataready()) {
    radio.rx(data, 10);
    decodeAbsolutePacket();
    String absLat = "absLat";
    absLat.concat(GPS_lat_abs_undiv);
    Serial.println(absLat);
    String absLong = "absLong";
    absLong.concat(GPS_lng_abs_undiv);
    Serial.println(absLong);
    String absAlt = "absAlt";
    absAlt.concat(alt_abs);
    Serial.println(absAlt);
    String battVoltStr = "battVolt";
    battVoltStr.concat(battVoltBits);
    Serial.println(battVoltStr);
    if (armedStatePrinted == 0) {
      Serial.println("STATECHANGE:ARMED");
      armedStatePrinted = 1;
    }
    if (DEBUG) {
      Serial.print("RSSI: ");
      Serial.println(radio.rssi());
      Serial.print("SNR: ");
      Serial.println(radio.snr());
    }
    return ACTIVE;
  }
  return ARMED;
}

enum State activeHandler(void) {
  radio.initialize(RX, 5);
  if (landedStatePrinted == 0) {
    if (header == 7) {
      Serial.println("STATECHANGE:LANDED");
      landedStatePrinted = 1;
    }
  }
  if (radio.dataready()) {
    if (activeStatePrinted == 0) {
      Serial.println("STATECHANGE:ACTIVE");
      activeStatePrinted = 1;
    }
    radio.rx(data, 5);
    decodeRelativePacket();
    String relLat = "relLat";
    relLat.concat(GPS_lat_rel_undiv);
    Serial.println(relLat);
    String relLong = "relLong";
    relLong.concat(GPS_lng_rel_undiv);
    Serial.println(relLong);
    String relAlt = "relAlt";
    relAlt.concat(alt_rel);
    Serial.println(relAlt);
    String battVoltStr = "battVolt";
    battVoltStr.concat(battVoltBits);
    Serial.println(battVoltStr);
    if (DEBUG) {
      Serial.print("RSSI: ");
      Serial.println(radio.rssi());
      Serial.print("SNR: ");
      Serial.println(radio.snr());
    }
  }
  return ACTIVE;
}

enum State testHandler(void) {
  radio.initialize(RX, 5);
  String msg = Serial.readString();
  Serial.println("ACK: " + msg);

  if (radio.dataready()) {
    radio.rx(data, 5);
    for (int i = 0; i < 5; i++) {
      Serial.print(data[i], HEX);
      Serial.print(", ");
    }
    Serial.println();
    decodeRelativePacket();
    Serial.println();
    Serial.print("RSSI: ");
    Serial.println(radio.rssi());
    Serial.print("SNR: ");
    Serial.println(radio.snr());
  }
  return TEST;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  while (!Serial) {;} // Wait for serial channel to open
  Serial.println("Starting");
}


void loop() {
  switch (curr_state) {
  case IDLE:
    curr_state = idleHandler();
    break;
  case ARMED:
    curr_state = armedHandler();
    break;
  case ACTIVE:
    curr_state = activeHandler();
    break;
  case TEST:
    curr_state = testHandler();
    break;
  default:
    Serial.print("Error - Invalid State: ");
    Serial.println(curr_state);
  }
}
