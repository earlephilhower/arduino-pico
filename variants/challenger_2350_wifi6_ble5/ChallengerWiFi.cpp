/*
    ESP8285/ESP32C3 helper class for the Challenger RP2040 WiFi enabled boards

    Copyright (c) 2021,2022 P. Oldberg <pontus@ilabs.se>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include <ChallengerWiFi.h>

Challenger2040WiFiClass::Challenger2040WiFiClass(HardwareSerial* serial) {
    _serial = serial;

    pinMode(PIN_ESP_RST, OUTPUT);
    digitalWrite(PIN_ESP_RST, LOW);       // Hold ESP in reset
    pinMode(PIN_ESP_MODE, OUTPUT);
    digitalWrite(PIN_ESP_MODE, HIGH);     // Prepare for normal start
}

// Do a HW reset by applying a low pulse to the reset line for 1mSec
void Challenger2040WiFiClass::doHWReset() {
    digitalWrite(PIN_ESP_RST, LOW);       // Hold ESP in reset
    delay(1);
    digitalWrite(PIN_ESP_RST, HIGH);      // Release ESP reset
}

// Set the mode flag high to indicate normal run operation and do a HW
// reset.
void Challenger2040WiFiClass::runReset() {  // Prepare ESP for normal op
    digitalWrite(PIN_ESP_MODE, HIGH);     // Prepare for normal start
    doHWReset();
}

// Set the mode flag low to indicate flash operation and do a HW
// reset.
void Challenger2040WiFiClass::flashReset() { // Prepare ESP for flashing
    digitalWrite(PIN_ESP_MODE, LOW);       // Prepare for normal start
    doHWReset();
}

// Wait for the modem to reply with a "ready" prompt. This can be done
// after a sw or hw reset have been performed to ensure that the AT
// interpreter is up and running.
bool Challenger2040WiFiClass::waitForReady() {
    int timeout = 20;                         // Aprox max 2 sec

    _serial->setTimeout(100);
    String rdy = _serial->readStringUntil('\n');
    while (!rdy.startsWith("ready") && timeout--) {
        rdy = _serial->readStringUntil('\n');
    }
    _serial->setTimeout(1000);  // Reset default timeout to 1000
    if (timeout) {
        return true;
    }
    return false;
}

// Reset the ESP and wait for the "ready" prompt to be returned.
bool Challenger2040WiFiClass::reset() {
    runReset();
    _serial->begin(DEFAULT_ESP_BAUDRATE);
    return waitForReady();
}

// Checks to see if the modem responds to the "AT" poll command.
bool Challenger2040WiFiClass::isAlive() {
    int timeout = 100;

    _serial->setTimeout(250);
    _serial->println(F("AT"));
    String rdy = _serial->readStringUntil('\n');
    while (!rdy.startsWith(F("OK")) && timeout--) {
        _serial->println(F("AT"));
        rdy = _serial->readStringUntil('\n');
    }
    _serial->setTimeout(1000);

    if (timeout) {
        return true;
    }
    return false;
}

// Change the baud rate of the ESP device as well as the local UART.
// No checking is done on the input baud rate so the user must know what
// baud rates are valid. The function ends by checking if the ESP is
// reachable by doing an "AT" poll.
bool Challenger2040WiFiClass::changeBaudRate(int baud) {
    _serial->print(F("AT+UART_CUR="));
    _serial->print(baud);
    _serial->println(F(",8,1,0,0"));
    delay(100);
    _serial->end();
    _serial->begin(baud);
    return isAlive();
}

// This method should be called id the builtin object isn't needed any more
// It basically just releases the UART pins for other use.
void Challenger2040WiFiClass::release() {
    _serial->end();
}

// We can assign a new hardware serial port to accommodate the ESP device here.
// The function will release the previously used serial port and assign the
// new port. The ESP will be left in a reset state ready to start normal
// operation when exiting the function.
// This function is useful for when using the PIO serial ports to communicate
// with the ESP instead of the built in hardware serial port.
void Challenger2040WiFiClass::setSerial(HardwareSerial* serial) {

    release();
    _serial = serial;

    pinMode(PIN_ESP_RST, OUTPUT);
    digitalWrite(PIN_ESP_RST, LOW);       // Hold ESP in reset
    pinMode(PIN_ESP_MODE, OUTPUT);
    digitalWrite(PIN_ESP_MODE, HIGH);     // Prepare for normal start
}

// Return the current serial object
HardwareSerial* Challenger2040WiFiClass::getSerial() {
    return _serial;
}

Challenger2040WiFiClass Challenger2040WiFi;
