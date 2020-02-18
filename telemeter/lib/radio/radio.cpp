// Code copy-pasted from my bin sensor project. Currently hardcoded for 7 byte messages, and tx only.
// Need to add flexable messsage lengths, rx code, and the ability to change data rates after initialization


#include "radio.h"

bool Radio::writemasked(byte addr, byte data, byte mask){  // writes mask bits of data to address
  byte readdata = readbyte(addr) & ~mask;
  byte writedata = data & mask;
  digitalWrite(slaveSelectPin, LOW); // Start new transaction
  SPI.transfer(addr | (1 << 7)); // send address, in write mode
  SPI.transfer(readdata | writedata);//send data
  digitalWrite(slaveSelectPin, HIGH);//end transaction
  byte resp = readbyte(addr);
  return resp == (readdata | writedata);
}

bool Radio::TXradioinit(int bitLen){
  pinMode(slaveSelectPin, OUTPUT);
  
  SPI.begin();
  bool good = true;
  good &= writemasked(0x01, 0b00000000, 0b00000111);  // Set to sleep Mode
  good &= writemasked(0x01, 0b10001000, 0b11001000);  // Set to LORA Mode, sharedReg off, Low freq mode
  good &= writemasked(0x01, 0b00000001, 0b00000111);  // Set to Standby Mode

  good &= writemasked(0x09, 0b10001111, 0b10001111);  // Set PA_BOOST, set OutputPower to 1111
  good &= writemasked(0x4d, 0b00000111, 0b00000111);  // Enable High power output
  good &= writemasked(0x0b, 0b00000000, 0b00100000);  // Disable Overcurrent protection

	// good &= writemasked(0x1D, 0b10011000, 0b11111111);  // ModemConfig1 - Bw=500khz, CR=4/8, exp header
  good &= writemasked(0x1D, 0b01111000, 0b11111111);  // ModemConfig1 - Bw=125khz, CR=4/8, exp header

  good &= writemasked(0x1E, 0b11000100, 0b11111111);  // ModemConfig2 - Sf=4096, single packet, CRC on, TimeoutMSB=0

  good &= writemasked(0x26, 0b00000100, 0b00001100);  // ModemConfig3 - set AGC on, Lowdatarateoptimise off

  good &= writemasked(0x0E, 0b00000000, 0b11111111);  //FIFOTxBaseAddress to 0x00

  good &= writemasked(0x22, bitLen, 0xFF);  // Set payload length
  return good;
}

bool Radio::RXradioinit(){
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
  bool good = true;
	//good &= writemasked(0x01, 0x00, 0x07);  // Set to sleep Mode
	good &= writemasked(0x01, 0b10001000, 0b11001000);  // Set to LORA Mode, sharedReg off, Low freq mode
	good &= writemasked(0x01, 0b00000101, 0b00000111);  // Set to Recieve Continous Mode
	good &= writemasked(0x09, 0b10001111, 0b10001111);  // Set PA_BOOST, set OutputPower to 1111
	good &= writemasked(0x4d, 0b00000111, 0b00000111);  // Enable High power output mode
	good &= writemasked(0x0b, 0b00000000, 0b00100000);  // Disable Overcurrent protection
	good &= writemasked(0x1D, 0b01111000, 0b11111111);  // ModemConfig1 - Bw=125khz, CR=4/8, exp header
  // good &= writemasked(0x1D, 0b10011000, 0b11111111);  // ModemConfig1 - Bw=500khz, CR=4/8, exp header

	// writemasked(0x1D, 0b01111001, 0b11111111)  # ModemConfig1 - Bw=125khz, CR=4/8, inp header
	good &= writemasked(0x1E, 0b11000000, 0b11111111);  // ModemConfig2 - Sf=4096,  , CRC on, TimeoutMSB=0
	// writemasked(0x1E, 0b11000100, 0b11111111)  # ModemConfig2 - Sf=4096, single packet, CRC on, TimeoutMSB=0
  // writemasked(0x1E, 0b10000100, 0b11111111)  # ModemConfig2 - Sf=256, single packet, CRC on, TimeoutMSB=0
	// writemasked(0x26, 0b00001100, 0b00001100)  # ModemConfig3 - set AGC on, Lowdatarateoptimise on
	good &= writemasked(0x26, 0b00000100, 0b00001100);  // ModemConfig3 - set AGC on, Lowdatarateoptimise off
	// LDR optimise needs to be off, or it fails to send messages longer then ~4 bytes, despite the datasheet
	good &= writemasked(0x26, 0x00, 0xFF);  // Lora data pointer
	good &= writemasked(0x0F, 0x00, 0xFF);  // Set RxBase Address
	// writemasked(0x22, 6, 0xFF);  // Set payload length - don't need this, unless using implicit header
  return good;
}

void Radio::tx(byte data[], int dataLen){
  bool good = true;
  writemasked(0x01, 0b00000010, 0b00000111);  // Set to FSTX Mode
  delay(3);
  //TODO: Set fails safes for when good == FALSE
  //lcd.print(good);
  Serial.println(data[0]);
  good &= writemasked(0x0D, 0b00000000, 0b11111111); //set FifoPtrAddr to FifoTxPtrBase (0x00)
  writeFIFO(data, dataLen);//Write PayloadLength bytes to the FIFO (RegFifo)
  Serial.println("1");
  good &= writemasked(0x01, 0b00000011, 0b00000111); //set mode to TX
  byte stat = 0;
  uint32_t ts = millis();
  do {
    stat = readbyte(0x12);
    // bit 3 is TxDone, wait until this is true
  } while (0b00001000 != (stat & 0b00001000));
  Serial.println(millis() - ts);
  Serial.println("Trasmission Done.");
  writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
}
// TODO: DEBUG THIS FUNCTION. I BELIEVE THERE WILL BE ISSUES HERE
void Radio::rx(byte buffer[], byte len){  // Return 7 byte message
	writemasked(0x0D, readbyte(0x10), 0xFF);  // Set SPI FIFO Address to location of last packet
	// print("Start of packet", readbyte(0x10))
	// writemasked(0x0D, 0, 0xFF)  # Set SPI FIFO Address to start of FIFO
	readFIFO(buffer, len);
	// print("End of last packet", readbyte(0x25))
	// print("Number of bytes recieved", readbyte(0x13))
	// data = data[4:]  # Remove four byte header added by radiohead
}

byte Radio::readbyte(byte addr){  // Reads one byte at address
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(addr);//send address, for reading
  byte resp = SPI.transfer(0x00);//transfer zeros, to read register
  digitalWrite(slaveSelectPin, HIGH);//end transaction
  return resp;
}

void Radio::writeFIFO(byte data[], int dataLen){  // writes data to FIFO register
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(0b10000000);//send address, for writing
  for (int i = 0; i < dataLen; i++){
    SPI.transfer(data[i]); //send data
  }
  digitalWrite(slaveSelectPin, HIGH);//end transaction
}

void Radio::readFIFO(byte buffer[], byte num){  // Read num of bytes from Fifo buffer
  digitalWrite(slaveSelectPin, LOW); // Start transaction
  SPI.transfer(0b00000000); //send FIFO address, for reading
  for (int i = 0; i < num; i++){
    buffer[i] = SPI.transfer(0x00);//transfer zeros, to read register
  }
  digitalWrite(slaveSelectPin, HIGH);//end transaction
}

bool Radio::dataready(){  // Return true when message is ready
	byte stat = readbyte(0x12);
	if (0b01000000 != (stat & 0b01000000)){ // bit 6 is RxDone, wait until this is true
    return false; // if bit not 1;
  }
	// print("Stat:", "{0:b}".format(stat))
	if (0b00100000 == (stat & 0b00100000)){  // bit 5 is PayloadCrcError
		writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
    Serial.println("Error!");
		return false;
  }  // exit if bit 1
	// we now have a message with valid CRC
	writemasked(0x12, 0xFF, 0xFF);  // Clear the flags
	return true;
}

int Radio::rssi(){
  byte raw = readbyte(0x1A);
  return (-137 + raw);
}

float Radio::snr(){
	byte raw = readbyte(0x19);
	raw = raw & 0b01111111;
	return (raw / 4.0);
}
