// Main file for telemeter build

#include "telemeter.h"

#define DEBUG true
#define FLASH false

byte data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
double altitude_abs = 0;
int16_t altitude_rel = 0;
int16_t GPS_lat_rel = 0;
int16_t GPS_lng_rel = 0;
double GPS_lat_abs = 0;
double GPS_lng_abs = 0;
int counter = 0;
const int chipSelect = 8;
SdFat sd;
SdFile file;
TinyGPSPlus gps;


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

void get_location(){
  altitude_rel = gps.altitude.meters() - altitude_abs;
  //Rounding error from .009 * 100000. Goes to 899.
  GPS_lat_rel = (GPS_lat_abs - gps.location.lat()) * 100000;
  GPS_lng_rel = (GPS_lng_abs - gps.location.lng()) * 100000;
  if (DEBUG) {
    altitude_rel = 900;
    GPS_lat_rel = 900;
    GPS_lng_rel = 900;
    Serial.print("Alititude: ");
    Serial.println(altitude_rel);
    Serial.print("GPS_lng_rel: ");
    Serial.println(GPS_lng_rel);
    Serial.print("GPS_lat_rel: ");
    Serial.println(GPS_lat_rel);
  }
}

//Payload first 3 bits header, next 11 bits for lat (first bit sign),
// next 11 bits for long (first bit sign), next 11 bits altitude (first bit signed)
void set_payload(){
  uint8_t GPS_lat_rel_top;
  uint8_t GPS_lat_rel_bot;
  uint8_t GPS_lng_rel_top;
  uint8_t GPS_lng_rel_mid;
  uint8_t GPS_lng_rel_bot;
  uint8_t altitude_rel_top;
  uint8_t altitude_rel_bot;
  if(GPS_lat_rel < 0){
    if(DEBUG) Serial.print("In set_payload: GPS_lat_rel: ");
    if(DEBUG) Serial.println(GPS_lat_rel);
    GPS_lat_rel = GPS_lat_rel * -1;
    GPS_lat_rel_top = (GPS_lat_rel & 0x03C0) >> 6;
    GPS_lat_rel_bot = (GPS_lat_rel & 0x003F);
    data[0] &= 0xE0;
    data[0] |= GPS_lat_rel_top;
    data[0] |= 0x10;
    data[1] = (GPS_lat_rel_bot << 2);
    if(DEBUG) Serial.println(data[0], HEX);
  }
  else{
    GPS_lat_rel_top = (GPS_lat_rel & 0x03C0) >> 6;
    GPS_lat_rel_bot = (GPS_lat_rel & 0x003F);
    data[0] &= 0xE0;
    data[0] |= GPS_lat_rel_top;
    data[0] &= ~0x10;
    data[1] = (GPS_lat_rel_bot << 2);
  }
  if(GPS_lng_rel < 0){
    GPS_lng_rel = GPS_lng_rel * -1;
    GPS_lng_rel_top = (GPS_lng_rel & 0x0200) >> 9;
    GPS_lng_rel_mid = (GPS_lng_rel & 0x01FE) >> 1;
    GPS_lng_rel_bot = (GPS_lng_rel & 0x0001);
    data[1] |= 0x02;
    data[1] &= ~0x01;
    data[1] |= GPS_lng_rel_top;
    data[2] = GPS_lng_rel_mid;
    data[3] = GPS_lng_rel_bot;
  }
  else{
    GPS_lng_rel_top = (GPS_lng_rel & 0x0200) >> 9;
    GPS_lng_rel_mid = (GPS_lng_rel & 0x01FE) >> 1;
    GPS_lng_rel_bot = (GPS_lng_rel & 0x0001);
    data[1] &= ~0x03;
    data[1] |= GPS_lng_rel_top;
    data[2] = GPS_lng_rel_mid;
    data[3] = GPS_lng_rel_bot;
  }
  if(altitude_rel < 0){
    altitude_rel = altitude_rel * -1;
    altitude_rel_top = (altitude_rel & 0x3F0) >> 4;
    altitude_rel_bot = (altitude_rel & 0x00F);
    data[3] &= ~0x7F;
    data[3] |= 0x40;
    data[3] |= altitude_rel_top;
    data[4] = altitude_rel_bot << 4;
  }
  else{
    altitude_rel_top = (altitude_rel & 0x3F0) >> 4;
    altitude_rel_bot = (altitude_rel & 0x00F);
    data[3] &= ~0x7F;
    data[3] |= altitude_rel_top;
    data[4] = altitude_rel_bot << 4;
  }
  //TODO: SET BATTERYVOLTAGE IN LAST 4 BITS
}

enum State {
  SLEEP = 0,
  IDLE = 1,
  ARMED = 2,
  ACTIVE = 3,
  LANDED = 4,
  DATA_TRANSFER = 5,
  TEST = 6
};

enum State curr_state = TEST;


enum State sleep_handler(void) {
  return SLEEP;
}

enum State idle_handler(void) {
  // send data only when you receive data:
  if (Serial.available() > 0) {
    // read the incoming byte:
    /*incomingByte = Serial.read();
    if(incomingByte != -1) {
      if(DEBUG) {
        Serial.print("Recieved: ");
        Serial.println(incomingByte);
      }
      if(incomingByte == "Armed") {
        //TODO: Send on serial the state of armed.
        return ARMED;
      }
    }*/
  }
  return IDLE;
}

enum State armed_handler(void) {
  //TODO: Obtain absolute position

  //TODO: Set to CONFIGMODE, do self test, set to AMG mode?
  imu.queryData();
  if ((imu.AX < -20 || imu.AX > 20) || (imu.AY < -20 || imu.AY > 20)
      || (imu.AZ < -20 || imu.AZ > 20)) {
    return ACTIVE;
  } else {
    return ARMED;
  }
}

enum State active_handler(void) {
  get_location();
  set_payload();
  if (DEBUG) Serial.println("Starting send");
  radio.tx(data, 5);
  if (DEBUG) Serial.println("Done send");
  imu.queryData();
  //TODO: Add loggings
  if (DEBUG) Serial.print("Counter:");
  if (DEBUG) Serial.println(counter);
  if ((imu.AX > -20 && imu.AX < 20) && (imu.AY > -20 && imu.AY < 20)
      && (imu.AZ > -20 && imu.AZ < 20)) {
    counter++;
    if (counter == 1000) {
      if (DEBUG) Serial.println("Switched state to LANDED");
      counter = 0;
      return LANDED;
    }
  } else {
    if (DEBUG) Serial.println("COUNTER RESET");
    counter = 0;
  }
  return ACTIVE;
}

enum State landed_handler(void) {
  get_location();
  set_payload();
  radio.tx(data, 5);
  delay(1000);
  return LANDED;
}

enum State data_transfer_handler(void) {
  return DATA_TRANSFER;
}

enum State test_handler(void) {
  get_location();
  set_payload();
  if (DEBUG) Serial.println("Starting send");
  radio.tx(data, 5);
  if (DEBUG) Serial.println("Done send");
  return TEST;
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  imu.init();
  flash.init();
  pressureSensor.init();
  radio.TXradioinit(5);

  // startTimer(10); // In Hz
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // while (!Serial) {;} // Wait for serial channel to open

  analogReadResolution(12);
  pinMode(A0, INPUT);
  REG_ADC_CTRLB = 0x0110; // PRESCALER=0x1, RESSEL=0x1 - 16 bit result
  REG_ADC_AVGCTRL = 0x0A; // Accumulate 1024 samples per read

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

  // Serial.print(gps.satellites.value());
  // Serial.print(",");
  // Serial.print(gps.hdop.hdop(), 1);
  // Serial.print(",");
  // Serial.print(gps.location.age());
  // Serial.print(",");
  // Serial.println(gps.location.lat(), 7);

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

uint32_t lastLogTime = 0;

void loop() {
  if (DEBUG) {
    Serial.print("Current state: ");
    Serial.println(curr_state);
  }

  switch (curr_state) {
  case SLEEP:
    curr_state = sleep_handler();
    break;
  case IDLE:
    curr_state = idle_handler();
    break;
  case ARMED:
    curr_state = armed_handler();
    break;
  case ACTIVE:
    curr_state = active_handler();
    break;
  case LANDED:
    curr_state = landed_handler();
    break;
  case DATA_TRANSFER:
    curr_state = data_transfer_handler();
    break;
  case TEST:
    curr_state = test_handler();
    break;
  default:
    if (DEBUG) {
      Serial.print("Error - Invalid states: ");
      Serial.println(curr_state);
    }
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
