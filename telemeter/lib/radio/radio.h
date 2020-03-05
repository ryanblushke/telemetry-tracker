#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <stdio.h>
#include <SPI.h>


const int slaveSelectPin = 8;

class Radio {
 public:
  bool TXradioinit(int byteLen);
  bool RXradioinit(int byteLen);
  void tx(byte data[], int dataLen);
  void rx(byte buffer[], byte len);
  bool writemasked(byte addr, byte data, byte mask);
  byte readbyte(byte addr);
  void writeFIFO(byte data[], int dataLen);
  void readFIFO(byte buffer[], byte num);
  int rssi();
  float snr();
  bool dataready();
};

#endif
