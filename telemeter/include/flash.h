#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
 
class Flash {
private:
public:
    void init();
	uint8_t readByte(uint16_t addr);
	void writeByte(uint16_t addr, uint8_t data);
	void writeArray(uint8_t data[], uint8_t length, uint16_t baseaddr);
	void readArray(uint8_t data[], uint8_t length, uint16_t baseaddr);
};
 
#endif