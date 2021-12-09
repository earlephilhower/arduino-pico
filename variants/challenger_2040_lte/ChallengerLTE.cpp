/*
    UBlox SARA helper class for the Challenger RP2040 LTE boards

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
#include <ChallengerLTE.h>

Challenger2040LTEClass::Challenger2040LTEClass() {
  pinMode(PIN_SARA_ON, OUTPUT);
  digitalWrite(PIN_SARA_ON, LOW);           // Output register must always be low
  pinMode(PIN_SARA_ON, INPUT_PULLUP);

  pinMode(PIN_SARA_RST, INPUT_PULLUP);      // Keep as input for now

  pinMode(PIN_SARA_PWR, OUTPUT);
  digitalWrite(PIN_SARA_PWR, LOW);          // No power to SARA yet
  serialPortConfigured = false;
}

// Do a HW reset by applying a low pulse to the reset line for 1mSec
bool Challenger2040LTEClass::doPowerOn() {
  bool ret;
  digitalWrite(PIN_SARA_PWR, HIGH);         // Make sure LDO is on
  delay(100);                               // let the power stabilize
  pinMode(PIN_SARA_ON, OUTPUT);             // Pull power on control low
  delay(150);                               // For 150mS
  pinMode(PIN_SARA_ON, INPUT_PULLUP);       // before releasing it again.
  delay(1000);                              // Now wait for 1 second
  SARA_SERIAL_PORT.begin(DEFAULT_SARA_BAUDRATE);
  serialPortConfigured = true;
  ret = isAlive();                          // Makie sure the modem is
                                            // up and running

  delay(250);                               // Allow for any extra characters
  // before flushing the input buffer
  while(SARA_SERIAL_PORT.available()) SARA_SERIAL_PORT.read();

  return ret;
}

// Checks to see if the modem responds to the "AT" poll command.
bool Challenger2040LTEClass::isAlive(uint32_t timeout) {
  SARA_SERIAL_PORT.setTimeout(100);
  SARA_SERIAL_PORT.println(F("AT"));
  String rdy = SARA_SERIAL_PORT.readStringUntil('\n');
  while(!rdy.startsWith(F("OK")) && --timeout) {
    SARA_SERIAL_PORT.println(F("AT"));
    rdy = SARA_SERIAL_PORT.readStringUntil('\n');
    //Serial.println(rdy);
  }
  SARA_SERIAL_PORT.setTimeout(1000);      // Restore serial timeout
  if (timeout)
    return true;
  return false;
}

// Return the current MNO profile
// Returns -1 if the serial port is not yet setup or the number of the current
// MNO profile setting from the modem.
int Challenger2040LTEClass::getMNOProfile() {
  if (!serialPortConfigured)
    return -1;
  SARA_SERIAL_PORT.println(F("AT+UMNOPROF?"));
  String resp = getResponse();
  return resp.substring(resp.indexOf("+UMNOPROF: ") + 11).toInt();
}

// Set a new MNO profile
// Returns false if the serial port is not yet setup
bool Challenger2040LTEClass::setMNOProfile(int profile) {
  if (!serialPortConfigured)
    return false;
  String cmd = "AT+UMNOPROF=" + String(profile) + ",1";
  SARA_SERIAL_PORT.println(cmd);

  if (!getResponse().endsWith("OK")) {
    return false;
  }
  return true;
}

// Disable power save features
bool Challenger2040LTEClass::enablePS(bool enable) {
  if (!serialPortConfigured)
    return false;
  if (enable)
    SARA_SERIAL_PORT.println(F("AT+CPSMS=1"));
  else
    SARA_SERIAL_PORT.println(F("AT+CPSMS=0"));

  if (!getResponse().endsWith("OK")) {
    return false;
  }
  return true;
}

// Get a response from SARA
// A default serial timeout of 2 seconds allow for reading really slow
// responses which should accommodate most replies. Replies are then trimmed
// from control characters and appended with a tab character as a separator.
//
String Challenger2040LTEClass::getResponse(int timeout) {
  SARA_SERIAL_PORT.setTimeout(2000);            // allow for really slow responses

  String resp = SARA_SERIAL_PORT.readStringUntil('\n');
  resp.trim();
  String acc = resp;
  while(resp.indexOf("OK") == -1 && resp.indexOf("ERROR") == -1 && --timeout) {
    resp = SARA_SERIAL_PORT.readStringUntil('\n');
    resp.trim();
    if (resp.length())
      acc += "\t" + resp;
  }
  return acc;
}

Challenger2040LTEClass Challenger2040LTE;
