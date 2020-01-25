#include <Arduino.h>
#include <Wire.h>
#include <cmath>

#include "flash.h"

#define PAGE_SIZE 256
#define MIN_ERASE_SIZE 4096

const int PIN_CS = 5;

void Flash::init() {
    pinMode(PIN_CS, OUTPUT);
	digitalWrite(PIN_CS, HIGH); // Disable
}

void Flash::writeArray(uint8_t data[], uint8_t length, uint16_t baseaddr){
	for(uint8_t i = 0; i < length; i++)
		writeByte(baseaddr + i, data[i]);
}

void Flash::readArray(uint8_t data[], uint8_t length, uint16_t baseaddr){
	for(uint8_t i = 0; i < length; i++)
		data[i] = readByte(baseaddr + i);
}
