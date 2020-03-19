#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <stdio.h>
#include <SPI.h>


const int slaveSelectPin = 8;

enum Mode {
  TX = 0,
  RX = 1
};

class Radio {
 public:
  byte readbyte(byte addr);
  bool writemasked(byte addr, byte data, byte mask);
  bool TXradioinit(int byteLen);
  bool RXradioinit(int byteLen);
  void tx(byte data[], int dataLen);
  void rx(byte buffer[], byte len);
  void writeFIFO(byte data[], int dataLen);
  void readFIFO(byte buffer[], byte num);
  int rssi();
  float snr();
  bool dataready();
  void initialize(Mode mode, byte len);
private:
  Mode lastMode;
  byte lastLength;
};

#endif
