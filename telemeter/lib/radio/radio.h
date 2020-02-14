#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <stdio.h>
#include <SPI.h>


const int slaveSelectPin = 8;

class Radio {
public:
    bool TXradioinit(int byteLen);
    bool RXradioinit();
    void tx(byte data[], int dataLen);
    byte rx();
    bool writemasked(byte addr, byte data, byte mask);
    byte readbyte(byte addr);
    void writeFIFO(byte data[], int dataLen);
    byte readFIFO(int num);
    byte rssi();
    byte snr();
    bool dataready();
};

#endif
