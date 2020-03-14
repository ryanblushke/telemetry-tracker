// Main file for telemeter build

#include "telemeter.h"

#define DEBUG false
#define UPDATE true
#define SEND false
#define PRINTTIME false
#define FLASH false
#define NOGPS false
#define TIMEOUT 3000
#define VBATPIN A7
#define MAXVOLT 4.193

// Values recorded for 105mAh Cell, 1C discharge
// Output value is rounded down to the nearest threshold, so only 15 thresholds are used to map 16 possible outputs
// So we round down by dropping the lower threshold, or round up by dropping the upper one.
// uint16_t LIPO_LUT[16] = {0, 3376, 3429, 3468, 3498, 3523, 3542, 3563, 3588, 3617, 3650, 3687, 3727, 3773, 3823, 3882}; // Round down
uint16_t LIPO_LUT[16] = {0, 3429, 3468, 3498, 3523, 3542, 3563, 3588, 3617, 3650, 3687, 3727, 3773, 3823, 3882, 4193};  // Round up

enum State {
  SLEEP = 0,
  IDLE = 1,
  ARMED = 2,
  ACTIVE = 3,
  LANDED = 4,
  DATA_TRANSFER = 5,
  TEST = 6
};

enum State curr_state = IDLE;

byte stateChange[1] = {0x00};
byte data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
byte absData[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


//Flag for RX TX MODE
int rxMode = 0;
int byteMode = 0;

//Intermediate relative float values
float GPS_lat_rel_flt = 0;
float GPS_lng_rel_flt = 0;
float alt_rel_flt = 0;

//Intermediate absolute float values
double GPS_lat_abs_flt = 0;
double GPS_lng_abs_flt = 0;
double alt_abs_flt = 0;

//Final relative int16_t values
int16_t altitude_rel = 0;
int16_t GPS_lat_rel = 0;
int16_t GPS_lng_rel = 0;

//Final relative int64_t values
// TODO: altitude_abs should be int32_t with check for negative in encode
uint32_t altitude_abs = 0;
int32_t GPS_lat_abs = 0;
int32_t GPS_lng_abs = 0;

unsigned long timeout = 0;
int32_t initTimeout = 0;
int counter = 0;
const int chipSelect = 8;
SdFat sd;
SdFile file;
TinyGPSPlus gps;
uint32_t lastLogTime = 0;

IMU imu;
PressureSensor pressureSensor;
Flash flash;
Radio radio;

void fastBlink() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
}

void updateAbsoluteLocation() {
  if (NOGPS) {
    GPS_lat_abs_flt = 52.1248 * 10000000;
    GPS_lng_abs_flt = -106.6594 * 10000000;
    alt_abs_flt = 900;
    GPS_lat_abs = GPS_lat_abs_flt;
    GPS_lng_abs = GPS_lng_abs_flt;
    altitude_abs = alt_abs_flt;
  } else {
    GPS_lat_abs_flt = gps.location.lat() * 10000000;
    GPS_lng_abs_flt = gps.location.lng() * 10000000;
    alt_abs_flt = gps.altitude.meters();
    GPS_lat_abs = GPS_lat_abs_flt;
    GPS_lng_abs = GPS_lng_abs_flt;
    altitude_abs = alt_abs_flt;
  }

  if (UPDATE) {
    Serial.print("(int32_t) Absolute Latitude: ");
    Serial.println(GPS_lat_abs);
    Serial.print("(int32_t) Absolute Longitude: ");
    Serial.println(GPS_lng_abs);
    Serial.print("(uint32_t) Absolute Altitude: ");
    Serial.println(altitude_abs);
  }
}

// Need 32 bits (1 sign + 31 data) (8 bytes) to represent a max value of 1799999999
// Lat long total of 64 bits for both lat and long
// Need 13 bits (2 bytes) to represent a max value of 9850
// Total of  bits or  bytes
void encodeAbsolutePacket() {
  for (int i = 0; i < 10; i++) {
    absData[i] = 0x00;
  }
  if (GPS_lat_abs < 0) {
    GPS_lat_abs = GPS_lat_abs * -1;
    absData[0] = 0x80;
  }
  absData[0] |= (GPS_lat_abs & 0x7F000000) >> 24;
  absData[1] = (GPS_lat_abs & 0xFF0000) >> 16;
  absData[2] = (GPS_lat_abs & 0xFF00) >> 8;
  absData[3] = (GPS_lat_abs & 0xFF);
  if (GPS_lng_abs < 0) {
    GPS_lng_abs = GPS_lng_abs * -1;
    absData[4] |= 0x80;
  }
  absData[4] |= (GPS_lng_abs & 0x7F000000) >> 24;
  absData[5] = (GPS_lng_abs & 0xFF0000) >> 16;
  absData[6] = (GPS_lng_abs & 0xFF00) >> 8;
  absData[7] = (GPS_lng_abs & 0xFF);
  absData[8] = (altitude_abs & 0x3FC0) >> 6;
  absData[9] = (altitude_abs & 0x3F) << 2;
}

void updateRelativeLocation() {
  if (NOGPS) {
    GPS_lat_rel_flt = -.009 * 100000;
    GPS_lng_rel_flt = .009 * 100000;
    alt_rel_flt = 900;
    GPS_lat_rel = GPS_lat_rel_flt;
    GPS_lng_rel = GPS_lng_rel_flt;
    altitude_rel = alt_rel_flt;
  } else {
    GPS_lat_rel_flt = (gps.location.lat() - GPS_lat_abs) * 100000;
    GPS_lng_rel_flt = (gps.location.lng() - GPS_lng_abs) * 100000;
    alt_rel_flt = gps.altitude.meters() - altitude_abs;
    GPS_lat_rel = GPS_lat_rel_flt;
    GPS_lng_rel = GPS_lng_rel_flt;
    altitude_rel = alt_rel_flt;
  }

  if (UPDATE) {
    Serial.print("gps.location.lat(): ");
    Serial.println(gps.location.lat());
    Serial.print("gps.location.lng(): ");
    Serial.println(gps.location.lng());
    Serial.print("gps.altitude.meters(): ");
    Serial.println(gps.altitude.meters());
    Serial.print("(int16_t) Relative Latitude: ");
    Serial.println(GPS_lat_rel);
    Serial.print("(int16_t) Relative Longitude: ");
    Serial.println(GPS_lng_rel);
    Serial.print("(int16_t) Relative Altitude: ");
    Serial.println(altitude_rel);
  }
}

//Payload first 3 bits header, next 11 bits for lat (first bit sign),
//next 11 bits for long (first bit sign), next 11 bits altitude (first bit signed)
void encodeRelativePacket() {
  uint8_t GPS_lat_rel_top;
  uint8_t GPS_lat_rel_bot;
  uint8_t GPS_lng_rel_top;
  uint8_t GPS_lng_rel_mid;
  uint8_t GPS_lng_rel_bot;
  uint8_t altitude_rel_top;
  uint8_t altitude_rel_bot;

  if (curr_state == LANDED) {
    data[0] = 0xE0;
  } else {
    data[0] = 0x00;
  }

  if (GPS_lat_rel < 0) {
    GPS_lat_rel = GPS_lat_rel * -1;
    GPS_lat_rel_top = (GPS_lat_rel & 0x03C0) >> 6;
    GPS_lat_rel_bot = (GPS_lat_rel & 0x003F);
    data[0] &= 0xE0;
    data[0] |= GPS_lat_rel_top;
    data[0] |= 0x10;
    data[1] = (GPS_lat_rel_bot << 2);
  } else {
    GPS_lat_rel_top = (GPS_lat_rel & 0x03C0) >> 6;
    GPS_lat_rel_bot = (GPS_lat_rel & 0x003F);
    data[0] &= 0xE0;
    data[0] |= GPS_lat_rel_top;
    data[0] &= ~0x10;
    data[1] = (GPS_lat_rel_bot << 2);
  }

  if (GPS_lng_rel < 0) {
    GPS_lng_rel = GPS_lng_rel * -1;
    GPS_lng_rel_top = (GPS_lng_rel & 0x0200) >> 9;
    GPS_lng_rel_mid = (GPS_lng_rel & 0x01FE) >> 1;
    GPS_lng_rel_bot = (GPS_lng_rel & 0x0001);
    data[1] |= 0x02;
    data[1] &= ~0x01;
    data[1] |= GPS_lng_rel_top;
    data[2] = GPS_lng_rel_mid;
    data[3] = GPS_lng_rel_bot;
  } else {
    GPS_lng_rel_top = (GPS_lng_rel & 0x0200) >> 9;
    GPS_lng_rel_mid = (GPS_lng_rel & 0x01FE) >> 1;
    GPS_lng_rel_bot = (GPS_lng_rel & 0x0001);
    data[1] &= ~0x03;
    data[1] |= GPS_lng_rel_top;
    data[2] = GPS_lng_rel_mid;
    data[3] = GPS_lng_rel_bot;
  }

  if (altitude_rel < 0) {
    altitude_rel = altitude_rel * -1;
    altitude_rel_top = (altitude_rel & 0x3F0) >> 4;
    altitude_rel_bot = (altitude_rel & 0x00F);
    data[3] &= ~0x7F;
    data[3] |= 0x40;
    data[3] |= altitude_rel_top;
    data[4] = altitude_rel_bot << 4;
  } else {
    altitude_rel_top = (altitude_rel & 0x3F0) >> 4;
    altitude_rel_bot = (altitude_rel & 0x00F);
    data[3] &= ~0x7F;
    data[3] |= altitude_rel_top;
    data[4] = altitude_rel_bot << 4;
  }

  // Measure Battery Voltage
  float measuredvbat = analogRead(VBATPIN);
  if (DEBUG) {
    Serial.print("Raw Measured VBat: " );
    Serial.println(measuredvbat);
  }
  measuredvbat *= 2;    // resistor network divided by 2, so multiply back
  measuredvbat /= 4096; // Convert to decimal number
  measuredvbat *= 3300;  // Multiply by 3300mV, our reference voltage
  if (DEBUG) {
    Serial.print("VBat: " );
    Serial.println(measuredvbat);
  }
  uint8_t batt_state = 0;
  // Convert to battery state with LUT.
  for (int i = 15; i > 0; i--) {
    if (measuredvbat > LIPO_LUT[i]) {
      batt_state = i;
      break;
    }
  }
  if (DEBUG) {
    Serial.print("Measured State of Charge: ");
    Serial.println(batt_state);
  }
  data[4] &= ~0x0F;
  data[4] |= 0x0F & batt_state;
}

enum State sleepHandler(void) {
  return SLEEP;
}

enum State idleHandler(void) {
  if (rxMode != 1 || byteMode != 1) {
    if (DEBUG) Serial.println("Setting to Rx mode for 1 byte");
    radio.RXradioinit(1);
    rxMode = 1;
    byteMode = 1;
  }
  if (radio.dataready()) {
    radio.rx(stateChange, 1);
    Serial.println("CHANGED TO ARMED");
    Serial.print(stateChange[0], HEX);
    return ARMED;
  }
  return IDLE;
}

enum State armedHandler(void) {
  //Obtain Absolute position setup transmission for abs pos
  //TODO: decide how long to transmit abs position.
  //The current setup works, but is not the most reliable
  //Both of the devices must be set to the idle state in the current
  //configuration
  if (initTimeout == 1) initTimeout = 0;
  if (DEBUG) Serial.print("Waiting for GPS lock gps.location.lat(): ");
  if (DEBUG) Serial.println(gps.location.lat());

  if (NOGPS || gps.location.lat() != 0) {
    //Transmit absolute position to the receiver
    updateAbsoluteLocation();
    encodeAbsolutePacket();
    if (rxMode != 0 || byteMode != 10) {
      radio.TXradioinit(10);
      rxMode = 0;
      byteMode = 10;
    }
    for (int i = 0; i < 10; i++) {
      radio.tx(absData, 10);
      delay(20);
    }
    //Setup for active mode
    if (rxMode != 0 || byteMode != 5) {
      radio.TXradioinit(5);
      rxMode = 0;
      byteMode = 5;
    }

    //Wait for movement to move into active
    updateRelativeLocation();
    while ((!NOGPS) && (altitude_rel < 1)) {
      updateRelativeLocation();
      // Update GPS Parser
      while (Serial1.available()) {
        char c = Serial1.read();
        // Serial.write(c);
        gps.encode(c);
      }
    }
    return ACTIVE;
  }
  return ARMED;
}

enum State activeHandler(void) {
  if (initTimeout == 0) {
    timeout = millis() + TIMEOUT;
    initTimeout = 1;
  }
  updateRelativeLocation();
  encodeRelativePacket();
  if (SEND) Serial.println("Starting send");
  radio.tx(data, 5);
  if (SEND) Serial.println("Done send");
  imu.queryData();
  logLineOfDataToSDCard();
  if ((imu.AX > -20 && imu.AX < 20) && (imu.AY > -20 && imu.AY < 20)
      && (imu.AZ > -20 && imu.AZ < 20)) {
    if (PRINTTIME) {
      Serial.print("timeout - millis(): ");
      Serial.println(timeout - millis());
    }

    if (timeout - millis() > timeout) {
      if (DEBUG) Serial.println("Switched state to LANDED");
      return LANDED;
    }
  } else {
    if (PRINTTIME) Serial.println("timeout reset");
    timeout = millis() + TIMEOUT;
  }
  return ACTIVE;
}

enum State landedHandler(void) {
  if (DEBUG) Serial.println("Currently Landed");
  updateRelativeLocation();
  encodeRelativePacket();
  radio.tx(data, 5);
  delay(1000);
  return LANDED;
}

enum State testHandler(void) {
  uint32_t ts = millis();
  if (rxMode != 0 || byteMode != 5) {
    radio.TXradioinit(5);
    rxMode = 0;
    byteMode = 5;
  }
  //updateAbsoluteLocation();
  updateRelativeLocation();
  encodeRelativePacket();
  //encodeAbsolutePacket();
  if (SEND) Serial.println("Starting send");
  radio.tx(data, 5);
  //radio.tx(absData, 10);
  if (SEND) Serial.println("Done send");
  Serial.println(millis() - ts);
  return TEST;
}

uint32_t smoothedADCValue = 0;
float alpha =
  0.01; // More alpha = faster response (1.0 = no smoothing, 0.001 = lots)
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

  if (gps.location.age() > 100) {
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

void setup() {
  Serial.begin(115200);
  Wire.begin();
  imu.init();
  flash.init();
  pressureSensor.init();
  radio.RXradioinit(1);

  // startTimer(10); // In Hz
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  analogReadResolution(12);
  pinMode(A0, INPUT);

  Serial.println("Starting");
  if (FLASH) {
    while (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
      Serial.println("initialization failed");
      fastBlink();
    }
  }

  Serial1.begin(9600); // GPS connection
  Serial1.println("$PMTK251,38400*27"); // Set serial baud to 38400
  delay(300);
  Serial1.begin(38400);
  Serial1.println("$PMTK220,100*2F"); // Set update rate to 10HZ (Won't work at default baud)
  if (DEBUG) {
    Serial.print("initial state: ");
    Serial.println(curr_state);
  }
  rxMode = 1;
  byteMode = 1;
}

void loop() {
  switch (curr_state) {
  case SLEEP:
    curr_state = sleepHandler();
    break;
  case IDLE:
    curr_state = idleHandler();
    break;
  case ARMED:
    curr_state = armedHandler();
    break;
  case ACTIVE:
    curr_state = activeHandler();
    break;
  case LANDED:
    curr_state = landedHandler();
    break;
  case TEST:
    curr_state = testHandler();
    break;
  default:
    Serial.print("Error - Invalid State: ");
    Serial.println(curr_state);
  }

  if (millis() > (lastLogTime + 100)) {
    lastLogTime = millis();
    if (FLASH) logLineOfDataToSDCard();
  }
  updateADCReading();

  // Update GPS Parser
  while (Serial1.available()) {
    char c = Serial1.read();
    // Serial.write(c);
    gps.encode(c);
  }
}
