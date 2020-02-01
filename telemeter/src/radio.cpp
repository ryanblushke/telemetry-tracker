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




// Here's the receiver code. It's written in python, but it's basically doing the same stuff


def writeaddr(addr):  # puts address into write mode
	return addr | (1 << 7)
	
def writemasked(addr, data, mask, ignoreerror=False):  # writes mask bits of data to address
	global spi
	resp = spi.xfer2([addr, 0x00])
	readdata = resp[1] & ~mask
	writedata = data & mask
	spi.xfer2([writeaddr(addr), readdata | writedata])
	newbyte = readbyte(addr)
	if newbyte != (readdata | writedata) and not ignoreerror:
		template = "Failed SPI Write: address=0x%X, initaldata=%s, writedata=%s, readdata=%s"
		print(template % (addr, "{0:b}".format(resp[1]), "{0:b}".format((readdata | writedata)), "{0:b}".format(newbyte)))
		return False
	else:
		return True
		
def readbyte(addr):  # Reads one byte at address
	data = spi.xfer2([addr, 0x00])
	return data[1]
	
def readfifo(num):  # Read num of bytes from Fifo buffer
	data = spi.xfer2([0x00] * (num+1))  # First byte sets address 0, rest are just zeros
	return data[1:]  # remove first byte

def init():
	spi.open(0, 0)  # assign device 0, CE0
	writemasked(0x01, 0b00000000, 0b00000111)  # Set to sleep Mode
	writemasked(0x01, 0b10001000, 0b11001000)  # Set to LORA Mode, sharedReg off, Low freq mode
	writemasked(0x01, 0b00000101, 0b00000111)  # Set to Recieve Continous Mode
	writemasked(0x09, 0b10001111, 0b10001111)  # Set PA_BOOST, set OutputPower to 1111
	writemasked(0x4d, 0b00000111, 0b00000111)  # Enable High power output mode
	writemasked(0x0b, 0b00000000, 0b00100000)  # Disable Overcurrent protection
	writemasked(0x1D, 0b01111000, 0b11111111)  # ModemConfig1 - Bw=125khz, CR=4/8, exp header
	# writemasked(0x1D, 0b01111001, 0b11111111)  # ModemConfig1 - Bw=125khz, CR=4/8, inp header
	writemasked(0x1E, 0b11000000, 0b11111111)  # ModemConfig2 - Sf=4096, single packet, CRC on, TimeoutMSB=0
	# writemasked(0x1E, 0b11000100, 0b11111111)  # ModemConfig2 - Sf=4096, single packet, CRC on, TimeoutMSB=0
	# writemasked(0x1E, 0b10000100, 0b11111111)  # ModemConfig2 - Sf=256, single packet, CRC on, TimeoutMSB=0
	# writemasked(0x26, 0b00001100, 0b00001100)  # ModemConfig3 - set AGC on, Lowdatarateoptimise on
	writemasked(0x26, 0b00000100, 0b00001100)  # ModemConfig3 - set AGC on, Lowdatarateoptimise off
	# LDR optimise needs to be off, or it fails to send messages longer then ~4 bytes, despite the datasheet
	writemasked(0x26, 0x00, 0xFF)  # Lora data pointer
	writemasked(0x0F, 0x00, 0xFF)  # Set RxBase Address
	# writemasked(0x22, 9, 0xFF)  # Set payload length - don't need this, unless using implicit header
	print("Done Radio Init")

def dataready():  # Return true when message is ready
	stat = readbyte(0x12)
	if 0b01000000 != stat & 0b01000000:  # bit 6 is RxDone, wait until this is true
		return False  # exit if bit not 1
	# print("Stat:", "{0:b}".format(stat))
	if 0b00100000 == stat & 0b00100000:  # bit 5 is PayloadCrcError
		writemasked(0x12, 0xFF, 0xFF, ignoreerror=True)  # Clear the flags
		print("E")
		return False  # exit if bit 1
	# we now have a message with valid CRC
	writemasked(0x12, 0xFF, 0xFF, ignoreerror=True)  # Clear the flags
	return True
	
def recieve():  # Return 7 byte message
	writemasked(0x0D, readbyte(0x10), 0xFF)  # Set SPI FIFO Address to location of last packet
	# print("Start of packet", readbyte(0x10))
	# writemasked(0x0D, 0, 0xFF)  # Set SPI FIFO Address to start of FIFO
	data = readfifo(7)
	# print("End of last packet", readbyte(0x25))
	# print("Number of bytes recieved", readbyte(0x13))
	# data = data[4:]  # Remove four byte header added by radiohead
	return data
	
def rssi():
	raw = readbyte(0x1A)
	return -137 + raw
	
def snr():
	raw = readbyte(0x19)
	raw = raw & 0b01111111
	return raw / 4