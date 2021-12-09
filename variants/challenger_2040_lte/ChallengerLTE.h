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
#pragma once

#define DEFAULT_SARA_BAUDRATE      115200

class Challenger2040LTEClass {
  public:
    Challenger2040LTEClass();
    bool doPowerOn();
    bool isAlive(uint32_t timeout = 50);
    int getMNOProfile();
    bool setMNOProfile(int profile);
    bool enablePS(bool enable = true);
    String getResponse(int timeout = 5);

  private:
    bool serialPortConfigured;
};

extern Challenger2040LTEClass Challenger2040LTE;
