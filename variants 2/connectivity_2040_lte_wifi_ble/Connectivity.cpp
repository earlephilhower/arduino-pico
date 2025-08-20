/*
    UBlox SARA/ESP helper class for the Connectivity RP2040 LTE/WIFI/BLE board

    Copyright (c) 2024 P. Oldberg <pontus@ilabs.se>

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
#include <Connectivity.h>

iLabsConnectivityClass::iLabsConnectivityClass(HardwareSerial* espSerial, HardwareSerial* modemSerial) {
    _espSerial = espSerial;
    _modemSerial = modemSerial;
    /* SARA pins */
    pinMode(PIN_SARA_ON, OUTPUT);
    digitalWrite(PIN_SARA_ON, LOW);           // Output register must always be low
    pinMode(PIN_SARA_ON, INPUT_PULLUP);

    pinMode(PIN_SARA_RST, INPUT_PULLUP);      // Keep as input for now

    pinMode(PIN_SARA_PWR, OUTPUT);
    digitalWrite(PIN_SARA_PWR, LOW);          // No power to SARA yet

    pinMode(PIN_SARA_DTR, OUTPUT);
    digitalWrite(PIN_SARA_PWR, HIGH);         // Make sure DTR is low on the R412M

    modemSerialPortConfigured = false;

    /* ESP Pins  */
    pinMode(PIN_ESP_RST, OUTPUT);
    digitalWrite(PIN_ESP_RST, LOW);           // Hold ESP in reset
    pinMode(PIN_ESP_MODE, OUTPUT);
    digitalWrite(PIN_ESP_MODE, HIGH);         // Prepare for normal start
    espSerialPortConfigured = false;
}

// Do a HW reset by applying a low pulse to the reset line for 1mSec
bool iLabsConnectivityClass::doModemPowerOn() {
    bool ret;
    digitalWrite(PIN_SARA_PWR, HIGH);         // Make sure LDO is on
    delay(100);                               // let the power stabilize
    pinMode(PIN_SARA_ON, OUTPUT);             // Pull power on control low
    delay(150);                               // For 150mS
    pinMode(PIN_SARA_ON, INPUT_PULLUP);       // before releasing it again.
    delay(1000);                              // Now wait for 1 second

    SARA_SERIAL_PORT.setRTS(PIN_SERIAL2_RTS); // Enable hardware handshaking
    SARA_SERIAL_PORT.setCTS(PIN_SERIAL2_CTS);
    SARA_SERIAL_PORT.begin(DEFAULT_SARA_BAUDRATE);
    modemSerialPortConfigured = true;
    ret = isModemAlive();                     // Make sure the modem is up and running

    delay(250);                               // Allow for any extra characters
    // before flushing the input buffer
    while (SARA_SERIAL_PORT.available()) {
        SARA_SERIAL_PORT.read();
    }

    return ret;
}

// Checks to see if the modem responds to the "AT" poll command.
bool iLabsConnectivityClass::isModemAlive(uint32_t timeout) {
    SARA_SERIAL_PORT.setTimeout(100);
    SARA_SERIAL_PORT.println(F("AT"));
    String rdy = SARA_SERIAL_PORT.readStringUntil('\n');
    while (!rdy.startsWith(F("OK")) && --timeout) {
        SARA_SERIAL_PORT.println(F("AT"));
        rdy = SARA_SERIAL_PORT.readStringUntil('\n');
        //Serial.println(rdy);
    }
    SARA_SERIAL_PORT.setTimeout(1000);      // Restore serial timeout
    if (timeout) {
        return true;
    }
    return false;
}

// Return the current MNO profile
// Returns -1 if the serial port is not yet setup or the number of the current
// MNO profile setting from the modem.
int iLabsConnectivityClass::getModemMNOProfile() {
    if (!modemSerialPortConfigured) {
        return -1;
    }
    SARA_SERIAL_PORT.println(F("AT+UMNOPROF?"));
    String resp = getModemResponse();
    return resp.substring(resp.indexOf("+UMNOPROF: ") + 11).toInt();
}

// Set a new MNO profile
// Returns false if the serial port is not yet setup or if an error
// is detected during the communication with the modem.
// This call is synchronous and will wait for the modem to start after
// the soft restart which takes about 10 seconds.
bool iLabsConnectivityClass::setModemMNOProfile(int profile) {
    if (!modemSerialPortConfigured) {
        return false;
    }

    // Disconnect from the network
    SARA_SERIAL_PORT.println("AT+COPS=2");
    if (!getModemResponse().endsWith("OK")) {
        return false;
    }

    String cmd = "AT+UMNOPROF=" + String(profile) + ",1";
    SARA_SERIAL_PORT.println(cmd);
    if (!getModemResponse().endsWith("OK")) {
        return false;
    }

    // Restart the modem to apply the new MNO profile
    SARA_SERIAL_PORT.println("AT+CFUN=15");
    if (!getModemResponse().endsWith("OK")) {
        return false;
    }

    return isModemAlive(15000);
}

// Disable power save features
bool iLabsConnectivityClass::enableModemPS(bool enable) {
    if (!modemSerialPortConfigured) {
        return false;
    }
    if (enable) {
        SARA_SERIAL_PORT.println(F("AT+CPSMS=1"));
    } else {
        SARA_SERIAL_PORT.println(F("AT+CPSMS=0"));
    }

    if (!getModemResponse().endsWith("OK")) {
        return false;
    }
    return true;
}

// Get a response from SARA
// A default serial timeout of 2 seconds allow for reading really slow
// responses which should accommodate most replies. Replies are then trimmed
// from control characters and appended with a tab character as a separator.
//
String iLabsConnectivityClass::getModemResponse(int timeout) {
    SARA_SERIAL_PORT.setTimeout(2000);            // allow for really slow responses

    String resp = SARA_SERIAL_PORT.readStringUntil('\n');
    resp.trim();
    String acc = resp;
    while (resp.indexOf("OK") == -1 && resp.indexOf("ERROR") == -1 && --timeout) {
        resp = SARA_SERIAL_PORT.readStringUntil('\n');
        resp.trim();
        if (resp.length()) {
            acc += "\t" + resp;
        }
    }
    return acc;
}

// Do a HW reset by applying a low pulse to the reset line for 1mSec
void iLabsConnectivityClass::doEspHWReset() {
    digitalWrite(PIN_ESP_RST, LOW);       // Hold ESP in reset
    delay(1);
    digitalWrite(PIN_ESP_RST, HIGH);      // Release ESP reset
}

// Set the mode flag high to indicate normal run operation and do a HW
// reset.
void iLabsConnectivityClass::runEspReset() {  // Prepare ESP for normal op
    digitalWrite(PIN_ESP_MODE, HIGH);     // Prepare for normal start
    doEspHWReset();
}

// Set the mode flag low to indicate flash operation and do a HW
// reset.
void iLabsConnectivityClass::flashEspReset() { // Prepare ESP for flashing
    digitalWrite(PIN_ESP_MODE, LOW);       // Prepare for normal start
    doEspHWReset();
}

// Wait for the modem to reply with a "ready" prompt. This can be done
// after a sw or hw reset have been performed to ensure that the AT
// interpreter is up and running.
bool iLabsConnectivityClass::waitForEspReady() {
    int timeout = 20;                         // Approx max 2 sec

    _espSerial->setTimeout(100);
    String rdy = _espSerial->readStringUntil('\n');
    while (!rdy.startsWith("ready") && timeout--) {
        rdy = _espSerial->readStringUntil('\n');
    }
    _espSerial->setTimeout(1000);  // Reset default timeout to 1000
    if (timeout) {
        return true;
    }
    return false;
}

// Reset the ESP and wait for the "ready" prompt to be returned.
bool iLabsConnectivityClass::resetEsp() {
    runEspReset();
    _espSerial->begin(DEFAULT_ESP_BAUDRATE);
    return waitForEspReady();
}

// Checks to see if the modem responds to the "AT" poll command.
bool iLabsConnectivityClass::isEspAlive() {
    int timeout = 100;

    _espSerial->setTimeout(250);
    _espSerial->println(F("AT"));
    String rdy = _espSerial->readStringUntil('\n');
    while (!rdy.startsWith(F("OK")) && timeout--) {
        _espSerial->println(F("AT"));
        rdy = _espSerial->readStringUntil('\n');
    }
    _espSerial->setTimeout(1000);

    if (timeout) {
        return true;
    }
    return false;
}

// Change the baud rate of the ESP device as well as the local UART.
// No checking is done on the input baud rate so the user must know what
// baud rates are valid. The function ends by checking if the ESP is
// reachable by doing an "AT" poll.
bool iLabsConnectivityClass::changeEspBaudRate(int baud) {
    _espSerial->print(F("AT+UART_CUR="));
    _espSerial->print(baud);
    _espSerial->println(F(",8,1,0,0"));
    delay(100);
    _espSerial->end();
    _espSerial->begin(baud);
    return isEspAlive();
}

// This method should be called if the builtin object isn't needed any more
// It basically just releases the UART pins for other use.
void iLabsConnectivityClass::releaseEsp() {
    _espSerial->end();
}

// We can assign a new hardware serial port to accommodate the ESP device here.
// The function will release the previously used serial port and assign the
// new port. The ESP will be left in a reset state ready to start normal
// operation when exiting the function.
// This function is useful for when using the PIO serial ports to communicate
// with the ESP instead of the built in hardware serial port.
void iLabsConnectivityClass::setEspSerial(HardwareSerial* serial) {

    releaseEsp();
    _espSerial = serial;

    pinMode(PIN_ESP_RST, OUTPUT);
    digitalWrite(PIN_ESP_RST, LOW);       // Hold ESP in reset
    pinMode(PIN_ESP_MODE, OUTPUT);
    digitalWrite(PIN_ESP_MODE, HIGH);     // Prepare for normal start
}

// Return the current serial object
HardwareSerial* iLabsConnectivityClass::getEspSerial() {
    return _espSerial;
}

iLabsConnectivityClass iLabsConnectivity;
