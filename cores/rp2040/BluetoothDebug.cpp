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

static void _log_packet(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len) {
    if (!_print) {
        return;
    }
    _print->printf("[BT @%lu] ", millis());

    switch (packet_type) {
    case HCI_COMMAND_DATA_PACKET:
        _print->printf("CMD => ");
        break;
    case HCI_EVENT_PACKET:
        _print->printf("EVT <= ");
        break;
    case HCI_ACL_DATA_PACKET:
        _print->printf("ACL %s ", in ? "<=" : "=>");
        break;
    case HCI_SCO_DATA_PACKET:
        _print->printf("SCO %s ", in ? "<=" : "=>");
        break;
    case HCI_ISO_DATA_PACKET:
        _print->printf("ISO %s ", in ? "<=" : "=>");
        break;
    case LOG_MESSAGE_PACKET:
        _print->printf("LOG -- %s\n", (char*) packet);
        return;
    default:
        _print->printf("UNK(%x) %s ", packet_type, in ? "<=" : "=>");
        break;
    }

    for (uint16_t i = 0; i < len; i++) {
        _print->printf("%02X ", packet[i]);
    }
    _print->printf("\n");
}

static void _log_message(int log_level, const char * format, va_list argptr) {
    (void)log_level;
    char log_message_buffer[HCI_DUMP_MAX_MESSAGE_LEN];
    if (!_print) {
        return;
    }
    vsnprintf(log_message_buffer, sizeof(log_message_buffer), format, argptr);
    _print->printf("[BT @%lu] LOG -- %s\n", millis(), log_message_buffer);
}


static const hci_dump_t hci_dump_instance = {
    NULL,
    _log_packet,
    _log_message
};


void __EnableBluetoothDebug(Print &print) {
    _print = &print;
    hci_dump_init(&hci_dump_instance);
}

#endif
