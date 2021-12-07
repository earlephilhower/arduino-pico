/*
    ESP8285 helper class for the Challenger RP2040 WiFi boards

    Copyright (c) 2021 P. Oldberg <pontus@ilabs.se>

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

Challenger2040WiFiClass::Challenger2040WiFiClass() {
  pinMode(PIN_ESP8285_RST, OUTPUT);
  digitalWrite(PIN_ESP8285_RST, LOW);       // Hold ESP8285 in reset
  pinMode(PIN_ESP8285_MODE, OUTPUT);
  digitalWrite(PIN_ESP8285_MODE, HIGH);     // Prepare for normal start
}

// Do a HW reset by applying a low pulse to the reset line for 1mSec
void Challenger2040WiFiClass::doHWReset() {
  digitalWrite(PIN_ESP8285_RST, LOW);       // Hold ESP8285 in reset
  delay(1);
  digitalWrite(PIN_ESP8285_RST, HIGH);      // Release ESP8285 reset
}

// Set the mode flag high to indicate normal run operation and do a HW
// reset.
void Challenger2040WiFiClass::runReset() {  // Prepare ESP8285 for normal op
  digitalWrite(PIN_ESP8285_MODE, HIGH);     // Prepare for normal start
  doHWReset();
}

// Set the mode flag low to indicate flash operation and do a HW
// reset.
void Challenger2040WiFiClass::flashReset() { // Prepare ESP8285 for flashing
  digitalWrite(PIN_ESP8285_MODE, LOW);       // Prepare for normal start
  doHWReset();
}

// Wait for the modem to reply with a "ready" prompt. This can be done
// after a sw or hw reset have been performed to ensure that the AT
// interpreter is up and running.
bool Challenger2040WiFiClass::waitForReady() {
  int timeout = 20;                         // Aprox max 2 sec

  Serial2.begin(DEFAULT_ESP8285_BAUDRATE);
  Serial2.setTimeout(100);
  String rdy = Serial2.readStringUntil('\n');
  while(!rdy.startsWith("ready") && timeout--) {
    rdy = Serial2.readStringUntil('\n');
  }
  Serial2.setTimeout(1000);  // Reset default timeout to 1000
  if (timeout)
    return true;
  return false;
}

// Reset the ESP8285 and wait for the "ready" prompt to be returned.
bool Challenger2040WiFiClass::reset() {
  runReset();
  return waitForReady();
}

// Checks to see if the modem responds to the "AT" poll command.
bool Challenger2040WiFiClass::isAlive() {
  int timeout = 5;

  Serial2.setTimeout(250);
  Serial2.println(F("AT"));
  String rdy = Serial2.readStringUntil('\n');
  while(!rdy.startsWith(F("OK")) && timeout--) {
    rdy = Serial2.readStringUntil('\n');
  }
  Serial2.setTimeout(1000);

  if (timeout)
    return true;
  return false;
}

// Change the baud rate of the ESP8285 as well as the local UART.
// No checking is done on the input baud rate so the user must know what
// baud rates are valid. The function ends by checking if the ESP8285 is
// reachable by doing an "AT" poll.
bool Challenger2040WiFiClass::changeBaudRate(int baud) {
  Serial2.print(F("AT+UART_CUR="));
  Serial2.print(baud);
  Serial2.println(F(",8,1,0,0"));
  delay(100);
  Serial2.end();
  Serial2.begin(baud);
  return isAlive();
}

Challenger2040WiFiClass Challenger2040WiFi;
