/*
    Board init for the Challenger RP2040 NFC

    Copyright (c) 2022 P. Oldberg <pontus@ilabs.se>

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

/**
    Setup control pins for the NFC chip.
*/
void initVariant() {
    // Initialize the interrupt pin to be an input.
    // Setting it to an interrupt and connecting a call back is up to the app.
    pinMode(PIN_PN7150_IRQ_B, INPUT);

    // Initialize the reset pin to an output and hold the device in reset.
    // It is up to the application to release it.
    pinMode(PIN_PN7150_RST_B, OUTPUT);
    digitalWrite(PIN_PN7150_RST_B, LOW);
}
