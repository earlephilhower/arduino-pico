/*
    MouseBLE.h

    Copyright (c) 2015, Arduino LLC
    Original code (pre-library): Copyright (c) 2011, Peter Barrett

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

#ifndef MOUSEBLE_h
#define MOUSEBLE_h

#include <HID_Mouse.h>

class MouseBLE_ : public HID_Mouse {
private:
    bool _running;

public:
    MouseBLE_(bool absolute = false);
    void begin(const char *localName = nullptr, const char *hidName = nullptr);
    void end(void);
    virtual void move(int x, int y, signed char wheel = 0) override;
    void setBattery(int lvl);
    void setAbsolute(bool absolute = true);
};
extern MouseBLE_ MouseBLE;

#endif
