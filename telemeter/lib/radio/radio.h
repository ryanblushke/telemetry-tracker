#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

const int slaveSelectPin = 8;

class Radio {
public:
    bool TXradioinit(int byteLen);
    void tx(byte data[], int dataLen);
    bool writemasked(byte addr, byte data, byte mask);
    byte readbyte(byte addr);
    void writeFIFO(byte data[], int dataLen);
};

#endif
