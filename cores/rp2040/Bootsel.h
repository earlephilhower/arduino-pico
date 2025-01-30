/*
    Simple BOOTSEL reader object

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

/**
    @brief Wrapper class for polling the BOOTSEL button
*/
class __Bootsel {
public:
    __Bootsel() { }
    /**
        @brief Get state of the BOOTSEL pin

        @returns True if BOOTSEL pushed
    */
    operator bool();
};

/**
    @brief BOOTSEL accessor instance
*/
extern __Bootsel BOOTSEL;
