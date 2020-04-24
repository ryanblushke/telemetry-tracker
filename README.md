# telemetry-tracker
Telemetry tracker system for model rockets/airplanes.

# Development Environment
This project was designed around the Adafruit Feather M0 with RFM95 LoRa Radio: https://www.adafruit.com/product/3178

This repository uses platformio to compile and run the specified program. See https://docs.platformio.org/en/latest/core/index.html for more information.

# Usage
## GUI
### Dependencies
```
python3 -m pip install PyQt5 pyserial qrcode PyQtChart image
```
### Running the GUI
You must provide a serial port for the GUI to connect to.
```
cd interface/
python3 interface.py /dev/cu.usbmodem12345
```
## Telemeter
### Dependencies
```
platformio lib install SdFat
platformio lib install TinyGpsPlus
```
### Running the Telemeter
The telemeter code must be compiled and uploaded to the board on the specified serial port.
```
cd telemeter/
platformio run --target upload --environment telemeter --upload-port /dev/cu.usbmodem12345
```
## Receiver
### Running the Receiver
The receiver code must be compiled and uploaded to the board on the specified serial port.
```
cd telemeter/
platformio run --target upload --environment receiver --upload-port /dev/cu.usbmodem12345
```
# Contact
Ryan Blushke
ryan.blushke@gmail.com
