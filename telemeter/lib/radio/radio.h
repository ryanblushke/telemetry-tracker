/*
    telemetry-tracker is a telemetry tracker system for model rockets/airplanes.

    Copyright (C) 2020 Alex McNabb, Ryan Blushke, Torban Peterson, Scott Seidle

    This file is part of telemetry-tracker.

    telemetry-tracker is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    telemetry-tracker is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with telemetry-tracker.  If not, see <https://www.gnu.org/licenses/>.
 */
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
