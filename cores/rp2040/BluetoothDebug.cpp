/*
    Enable BTStack debugging to a Print-able object

    Copyright (c) 2023 Earle F. Philhower, III <earlephilhower@yahoo.com>

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

#if defined(ENABLE_CLASSIC) || defined(ENABLE_BLE)
#include <Arduino.h>
#include <btstack.h>
#include <hci_dump.h>

static Print *_print;

// Taken from hci_dump_embedded_stdout.c

/*
    Copyright (C) 2014 BlueKitchen GmbH

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the names of
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
    4. Any redistribution, use, or modification is done solely for
      personal benefit and not for any commercial purpose or for
      monetary gain.

    THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
    ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
    GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
    THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Please inquire about commercial licensing options at
    contact@bluekitchen-gmbh.com

*/


static inline char char_for_high_nibble(int value) {
    return char_for_nibble((value >> 4) & 0x0f);
}

static inline char char_for_low_nibble(int value) {
    return char_for_nibble(value & 0x0f);
}


static void Xprintf_hexdump(const void * data, int size) {
    char buffer[4];
    buffer[2] = ' ';
    buffer[3] =  0;
    const uint8_t * ptr = (const uint8_t *) data;
    while (size > 0) {
        uint8_t byte = *ptr++;
        buffer[0] = char_for_high_nibble(byte);
        buffer[1] = char_for_low_nibble(byte);
        _print->printf("%s", buffer);
        size--;
    }
    printf("\n");
}



static char log_message_buffer[256];

static void hci_dump_embedded_stdout_timestamp(void) {
    uint32_t time_ms = btstack_run_loop_get_time_ms();
    int      seconds = time_ms / 1000u;
    int      minutes = seconds / 60;
    unsigned int hours = minutes / 60;

    uint16_t p_ms      = time_ms - (seconds * 1000u);
    uint16_t p_seconds = seconds - (minutes * 60);
    uint16_t p_minutes = minutes - (hours   * 60u);
    _print->printf("[%02u:%02u:%02u.%03u] ", hours, p_minutes, p_seconds, p_ms);
}

static void hci_dump_embedded_stdout_packet(uint8_t packet_type, uint8_t in, uint8_t * packet, uint16_t len) {
    switch (packet_type) {
    case HCI_COMMAND_DATA_PACKET:
        _print->printf("CMD => ");
        break;
    case HCI_EVENT_PACKET:
        _print->printf("EVT <= ");
        break;
    case HCI_ACL_DATA_PACKET:
        if (in) {
            _print->printf("ACL <= ");
        } else {
            _print->printf("ACL => ");
        }
        break;
    case HCI_SCO_DATA_PACKET:
        if (in) {
            _print->printf("SCO <= ");
        } else {
            _print->printf("SCO => ");
        }
        break;
    case HCI_ISO_DATA_PACKET:
        if (in) {
            _print->printf("ISO <= ");
        } else {
            _print->printf("ISO => ");
        }
        break;
    case LOG_MESSAGE_PACKET:
        _print->printf("LOG -- %s\n", (char*) packet);
        return;
    default:
        return;
    }
    Xprintf_hexdump(packet, len);
}

static void hci_dump_embedded_stdout_log_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len) {
    hci_dump_embedded_stdout_timestamp();
    hci_dump_embedded_stdout_packet(packet_type, in, packet, len);
}

static void hci_dump_embedded_stdout_log_message(int log_level, const char * format, va_list argptr) {
    UNUSED(log_level);
    int len = vsnprintf(log_message_buffer, sizeof(log_message_buffer), format, argptr);
    hci_dump_embedded_stdout_log_packet(LOG_MESSAGE_PACKET, 0, (uint8_t*) log_message_buffer, len);
}

void __EnableBluetoothDebug(Print &print) {
    static const hci_dump_t hci_dump_instance = {
        // void (*reset)(void);
        NULL,
        // void (*log_packet)(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len);
        &hci_dump_embedded_stdout_log_packet,
        // void (*log_message)(int log_level, const char * format, va_list argptr);
        &hci_dump_embedded_stdout_log_message,
    };
    _print = &print;
    hci_dump_init(&hci_dump_instance);
}

#endif
