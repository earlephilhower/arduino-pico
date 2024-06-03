/*
    A1DP Source (Bluetooth audio sender)

    Copyright (c) 2024 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#include <Arduino.h>

class BluetoothMediaCodecConfigurationSBC {
public:
    uint8_t  reconfigure;
    uint8_t  num_channels;
    uint16_t sampling_frequency;
    uint8_t  block_length;
    uint8_t  subbands;
    uint8_t  min_bitpool_value;
    uint8_t  max_bitpool_value;
    btstack_sbc_channel_mode_t      channel_mode;
    btstack_sbc_allocation_method_t allocation_method;

    void dump() {
        DEBUGV("    - num_channels: %d\n", num_channels);
        DEBUGV("    - sampling_frequency: %d\n", sampling_frequency);
        DEBUGV("    - channel_mode: %d\n", channel_mode);
        DEBUGV("    - block_length: %d\n", block_length);
        DEBUGV("    - subbands: %d\n", subbands);
        DEBUGV("    - allocation_method: %d\n", allocation_method);
        DEBUGV("    - bitpool_value [%d, %d] \n", min_bitpool_value, max_bitpool_value);
        DEBUGV("\n");
    }
};
