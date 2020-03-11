// Code copy-pasted from my bin sensor project. Currently hardcoded for 7 byte messages, and tx only.
// Need to add flexable messsage lengths, rx code, and the ability to change data rates after initialization


#include "radio.h"

// uint32_t ts = 0;

byte Radio::readbyte(byte addr) { // Reads one byte at address
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(addr); // send address, for reading
  byte resp = SPI.transfer(0x00); // transfer zeros, to read register
  digitalWrite(slaveSelectPin, HIGH); // end transaction
  return resp;
}

// writes mask bits of data to address
bool Radio::writemasked(byte addr, byte data, byte mask) {
  byte readdata = readbyte(addr) & ~mask;
  byte writedata = data & mask;
  digitalWrite(slaveSelectPin, LOW); // Start new transaction
  SPI.transfer(addr | (1 << 7)); // send address, in write mode
  SPI.transfer(readdata | writedata);//send data
  digitalWrite(slaveSelectPin, HIGH);//end transaction
  byte resp = readbyte(addr);
  return resp == (readdata | writedata);
}

bool Radio::TXradioinit(int byteLen) {
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
  bool good = true;
  // RegOpMode (0x01) - Mode=000(SLEEP)
  good &= writemasked(0x01, 0b00000000, 0b00000111);
  // RegOpMode (0x01) - LongRangeMode=1, AccessSharedReg=0, LowFrequencyModeOn=0
  good &= writemasked(0x01, 0b10000000, 0b11001000);
  // RegFrMsb (0x06) - Frf(23:16)=0xE4
  good &= writemasked(0x06, 0xE4, 0xFF);
  // RegFrMid (0x07) - Frf(15:8)=0xC0
  good &= writemasked(0x07, 0xC0, 0xFF);
  // RegFrLsb (0x08) - Frf(7:0)=0x00
  good &= writemasked(0x08, 0x00, 0xFF);
  // RegOpMode (0x01) - Mode=001(STDBY)
  good &= writemasked(0x01, 0b00000001, 0b00000111);
  // RegPaConfig (0x09) - PaSelect=1(PA_BOOST), OutputPower=1111
  good &= writemasked(0x09, 0b10001111, 0b10001111);
  // RegOcp (0x0B) - OcpOn=0
  good &= writemasked(0x0B, 0b00000000, 0b00100000);
  // RegModemConfig1 (0x1D) - Bw=500kHz, CR=4/8, ImplicitHeaderModeOn=1
  good &= writemasked(0x1D, 0b10011001, 0b11111111);
  // RegModemConfig2 (0x1E) - SF=9, RxPayloadCrcOn=1, SymbTimeoutMSB=0
  good &= writemasked(0x1E, 0b10010100, 0b11111111);
  // RegModemConfig3 (0x26) - LowDataRateOptimize=0, AgcAutoOn=1
  // TODO: May need to enable LowDataRateOptimize
  good &= writemasked(0x26, 0b00000100, 0b00001100);
  // RegFifoTxBaseAddr (0x0E) - FifoTxBaseAddr=0x00
  good &= writemasked(0x0E, 0b00000000, 0b11111111);
  // RegPayloadLength (0x22) - PayloadLength=byteLen
  good &= writemasked(0x22, byteLen, 0xFF);
  return good;
}

bool Radio::RXradioinit(int byteLen) {
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
  bool good = true;
  // RegOpMode (0x01) - Mode=000(SLEEP)
  good &= writemasked(0x01, 0b00000000, 0b00000111);
  // RegOpMode (0x01) - LongRangeMode=1, AccessSharedReg=0, LowFrequencyModeOn=0
  good &= writemasked(0x01, 0b10000000, 0b11001000);
  // RegFrMsb (0x06) - Frf(23:16)=0xE4
  good &= writemasked(0x06, 0xE4, 0xFF);
  // RegFrMid (0x07) - Frf(15:8)=0xC0
  good &= writemasked(0x07, 0xC0, 0xFF);
  // RegFrLsb (0x08) - Frf(7:0)=0x00
  good &= writemasked(0x08, 0x00, 0xFF);
  // RegOpMode (0x01) - Mode=101(RXCONTINUOUS)
  good &= writemasked(0x01, 0b00000101, 0b00000111);
  // RegPaConfig (0x09) - PaSelect=1(PA_BOOST), OutputPower=1111
  good &= writemasked(0x09, 0b10001111, 0b10001111);
  // RegOcp (0x0B) - OcpOn=0
  good &= writemasked(0x0B, 0b00000000, 0b00100000);
  // RegModemConfig1 (0x1D) - Bw=500kHz, CR=4/8, ImplicitHeaderModeOn=1
  good &= writemasked(0x1D, 0b10011001, 0b11111111);
  // RegModemConfig2 (0x1E) - SF=9, RxPayloadCrcOn=1, SymbTimeoutMSB=0
  good &= writemasked(0x1E, 0b10010100, 0b11111111);
  // RegModemConfig3 (0x26) - LowDataRateOptimize=0, AgcAutoOn=1
  // TODO: May need to enable LowDataRateOptimize
  good &= writemasked(0x26, 0b00000100, 0b00001100);
  // RegFifoRxBaseAddr (0x0F) - FifoRxBaseAddr=0x00
  good &= writemasked(0x0F, 0x00, 0xFF);
  // RegPayloadLength (0x22) - PayloadLength=byteLen
  good &= writemasked(0x22, byteLen, 0xFF);
  return good;
}

void Radio::tx(byte data[], int dataLen) {
  bool good = true;
  writemasked(0x01, 0b00000010, 0b00000111);  // Set to FSTX Mode
  delay(3);
  //TODO: Set fails safes for when good == FALSE
  //lcd.print(good);
  // for (int i = 0; i < dataLen; i++) {
  //   Serial.print(data[i], HEX);
  //   Serial.print(", ");
  // }
  // Serial.println("");
  good &= writemasked(0x0D, 0b00000000,
                      0b11111111); //set FifoPtrAddr to FifoTxPtrBase (0x00)
  writeFIFO(data, dataLen);//Write PayloadLength bytes to the FIFO (RegFifo)
  good &= writemasked(0x01, 0b00000011, 0b00000111); //set mode to TX
  byte stat = 0;
  // ts = millis();
  do {
    stat = readbyte(0x12);
    // bit 3 is TxDone, wait until this is true
  } while (0b00001000 != (stat & 0b00001000));
  // Serial.print("tx: ");
  // Serial.println(millis() - ts);
  // Serial.println("Trasmission Done.");
  writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
}

// TODO: DEBUG THIS FUNCTION. I BELIEVE THERE WILL BE ISSUES HERE
void Radio::rx(byte buffer[], byte len) { // Return 7 byte message
  writemasked(0x0D, readbyte(0x10),
              0xFF);  // Set SPI FIFO Address to location of last packet
  // print("Start of packet", readbyte(0x10))
  // writemasked(0x0D, 0, 0xFF)  # Set SPI FIFO Address to start of FIFO
  readFIFO(buffer, len);
  // print("End of last packet", readbyte(0x25))
  // print("Number of bytes recieved", readbyte(0x13))
  // data = data[4:]  # Remove four byte header added by radiohead
}

void Radio::writeFIFO(byte data[],
                      int dataLen) { // writes data to FIFO register
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(0b10000000);//send address, for writing
  for (int i = 0; i < dataLen; i++) {
    SPI.transfer(data[i]); //send data
  }
  digitalWrite(slaveSelectPin, HIGH);//end transaction
}

void Radio::readFIFO(byte buffer[],
                     byte num) { // Read num of bytes from Fifo buffer
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(0b00000000); //send FIFO address, for reading
  for (int i = 0; i < num; i++) {
    buffer[i] = SPI.transfer(0x00);//transfer zeros, to read register
  }
  digitalWrite(slaveSelectPin, HIGH);//end transaction
}

bool Radio::dataready() { // Return true when message is ready
  byte stat = readbyte(0x12);
  if (0b01000000 != (stat &
                     0b01000000)) { // bit 6 is RxDone, wait until this is true
    return false; // if bit not 1;
  }
  // Serial.print("rx: ");
  // Serial.println(millis() - ts);
  // ts = millis();
  // print("Stat:", "{0:b}".format(stat))
  if (0b00100000 == (stat & 0b00100000)) { // bit 5 is PayloadCrcError
    writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
    Serial.println("Error!");
    return false;
  }  // exit if bit 1
  // we now have a message with valid CRC
  writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
  return true;
}

int Radio::rssi() {
  int8_t snr = (int8_t)readbyte(0x19) / 4;
  int rssi = (int)readbyte(0x1A);
  // Adjust the RSSI, datasheet page 87
  if (snr < 0)
    rssi = rssi + snr;
  else
    rssi = rssi * 16 / 15;
  rssi -= 157;
  return rssi;
}

float Radio::snr() {
  int8_t snr = (int8_t)readbyte(0x19);
  return (snr / 4.0);
}
