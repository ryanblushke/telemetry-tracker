// Main file for receiver build

#include "receiver.h"
#include "radio.h"

#define DEBUG true

Radio radio;

void fastBlink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
}


void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    while (!Serial) {;} // Wait for serial channel to open


    Serial.println("Starting");
    radio.RXradioinit();
    Serial.println("Starting");

}


void loop() {
    if (radio.dataready()){
        byte buffer[256];
        radio.rx(buffer, 256);
        for (int i = 0; i < 256; i++){
            Serial.print(buffer[i], HEX);
        }
        Serial.println();
        Serial.print("RSSI: ");
        Serial.println(radio.rssi());
        Serial.print("SNR: ");
        Serial.println(radio.snr());
    }
}
