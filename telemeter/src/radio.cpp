// Code copy-pasted from my bin sensor project. Currently hardcoded for 7 byte messages, and tx only.
// Need to add flexable messsage lengths, rx code, and the ability to change data rates after initialization


#include <SPI.h>

const int slaveSelectPin = 10;


bool radioinit(){
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
  bool good = true;
  good &= writemasked(0x01, B00000000, B00000111);  // Set to sleep Mode
  good &= writemasked(0x01, B10001000, B11001000);  // Set to LORA Mode, sharedReg off, Low freq mode
  good &= writemasked(0x01, B00000001, B00000111);  // Set to Standby Mode

  good &= writemasked(0x09, B10001111, B10001111);  // Set PA_BOOST, set OutputPower to 1111
  good &= writemasked(0x4d, B00000111, B00000111);  // Enable High power output
  good &= writemasked(0x0b, B00000000, B00100000);  // Disable Overcurrent protection

  good &= writemasked(0x1D, B01111000, B11111111);  // ModemConfig1 - Bw=125khz, CR=4/8, exp header
  
  good &= writemasked(0x1E, B11000100, B11111111);  // ModemConfig2 - Sf=4096, single packet, CRC on, TimeoutMSB=0

  good &= writemasked(0x26, B00000100, B00001100);  // ModemConfig3 - set AGC on, Lowdatarateoptimise off
  
  good &= writemasked(0x0E, B00000000, B11111111);  //FIFOTxBaseAddress to 0x00

  good &= writemasked(0x22, 7, 0xFF);  // Set payload length
  return good;
}


void tx(byte data[7]){//send 7 bytes of data
  bool good = true;
  writemasked(0x01, B00000010, B00000111);  // Set to FSTX Mode
  delay(3);
  //lcd.print(good);
  good &= writemasked(0x0D, B00000000, B11111111); //set FifoPtrAddr to FifoTxPtrBase (0x00)
  writeFIFO(data);//Write PayloadLength bytes to the FIFO (RegFifo)
  good &= writemasked(0x01, B00000011, B00000111); //set mode to TX
  byte stat = 0;
  do {
    stat = readbyte(0x12);
    // bit 3 is TxDone, wait until this is true
  } while (B00001000 != (stat & B00001000));
  writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
}


bool writemasked(byte addr, byte data, byte mask){  // writes mask bits of data to address
  byte readdata = readbyte(addr) & ~mask;
  byte writedata = data & mask;
  digitalWrite(slaveSelectPin, LOW); // Start new transaction
  SPI.transfer(addr | (1 << 7)); // send address, in write mode
  SPI.transfer(readdata | writedata);//send data
  digitalWrite(slaveSelectPin, HIGH);//end transaction
  byte resp = readbyte(addr);
  return resp == (readdata | writedata);
}


byte readbyte(byte addr){  // Reads one byte at address
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(addr);//send address, for reading
  byte resp = SPI.transfer(0x00);//transfer zeros, to read register
  digitalWrite(slaveSelectPin, HIGH);//end transaction
  return resp;
}

void writeFIFO(byte data[7]){  // writes data to FIFO register
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(B10000000);//send address, for writing
  for (int i = 0; i < 7; i++){
    SPI.transfer(data[i]); //send data
  }
  digitalWrite(slaveSelectPin, HIGH);//end transaction
}
