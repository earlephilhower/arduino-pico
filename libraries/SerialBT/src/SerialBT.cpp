/*
    Serial-over-Bluetooth for the Raspberry Pi Pico RP2040

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

#include "SerialBT.h"
#include <CoreMutex.h>
#define CCALLBACKNAME _SERIALBTCB
#include <ctocppcallback.h>

#define PACKETHANDLERCB(class, cbFcn) \
  (CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))


bool SerialBT_::setFIFOSize(size_t size) {
    if (!size || _running) {
        return false;
    }
    _fifoSize = size + 1; // Always 1 unused entry
    return true;
}

SerialBT_::SerialBT_() {
    mutex_init(&_mutex);
}

void SerialBT_::begin(unsigned long baud, uint16_t config) {
    if (_running) {
        end();
    }

    (void) baud;
    (void) config;

    _overflow = false;

    _queue = new uint8_t[_fifoSize];
    _writer = 0;
    _reader = 0;

    // register for HCI events
    _hci_event_callback_registration.callback = PACKETHANDLERCB(SerialBT_, packetHandler);
    hci_add_event_handler(&_hci_event_callback_registration);

    l2cap_init();

#ifdef ENABLE_BLE
    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
#endif

    rfcomm_init();
    rfcomm_register_service(PACKETHANDLERCB(SerialBT_, packetHandler), RFCOMM_SERVER_CHANNEL, 0xffff);  // reserved channel, mtu limited by l2cap

    // init SDP, create record for SPP and register with SDP
    sdp_init();
    bzero(_spp_service_buffer, sizeof(_spp_service_buffer));
    spp_create_sdp_record(_spp_service_buffer, 0x10001, RFCOMM_SERVER_CHANNEL, "PicoW Serial");
    sdp_register_service(_spp_service_buffer);

    gap_discoverable_control(1);
    gap_ssp_set_io_capability(SSP_IO_CAPABILITY_DISPLAY_YES_NO);
    if (!_name) {
        setName("PicoW Serial 00:00:00:00:00:00");
    }
    gap_set_local_name(_name);

    // Turn on!
    hci_power_control(HCI_POWER_ON);

    _running = true;
}

void SerialBT_::end() {
    if (!_running) {
        return;
    }
    _running = false;

    hci_power_control(HCI_POWER_OFF);
    __lockBluetooth();
    delete[] _queue;
    __unlockBluetooth();
}

int SerialBT_::peek() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return -1;
    }
    if (_writer != _reader) {
        return _queue[_reader];
    }
    return -1;
}

int SerialBT_::read() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return -1;
    }
    if (_writer != _reader) {
        auto ret = _queue[_reader];
        asm volatile("" ::: "memory"); // Ensure the value is read before advancing
        auto next_reader = (_reader + 1) % _fifoSize;
        asm volatile("" ::: "memory"); // Ensure the reader value is only written once, correctly
        _reader = next_reader;
        return ret;
    }
    return -1;
}

bool SerialBT_::overflow() {
    if (!_running) {
        return false;
    }

    __lockBluetooth();
    bool ovf = _overflow;
    _overflow = false;
    __unlockBluetooth();

    return ovf;
}

int SerialBT_::available() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return 0;
    }
    return (_fifoSize + _writer - _reader) % _fifoSize;
}

int SerialBT_::availableForWrite() {
    CoreMutex m(&_mutex);
    if (!_running || !m) {
        return 0;
    }
    return _connected ? 1 : 0;
}

void SerialBT_::flush() {
    // We always send blocking
}

size_t SerialBT_::write(uint8_t c) {
    return write(&c, 1);
}

size_t SerialBT_::write(const uint8_t *p, size_t len) {
    CoreMutex m(&_mutex);
    if (!_running || !m || !_connected || !len)  {
        return 0;
    }
    _writeBuff = p;
    _writeLen = len;
    __lockBluetooth();
    rfcomm_request_can_send_now_event(_channelID);
    __unlockBluetooth();
    while (_connected && _writeLen) {
        /* noop busy wait */
    }
    return len;
}

SerialBT_::operator bool() {
    return _running;
}

void SerialBT_::packetHandler(uint8_t type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    bd_addr_t event_addr;
    //uint8_t   rfcomm_channel_nr;
    //uint16_t  mtu;
    int i;

    switch (type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_PIN_CODE_REQUEST:
            //Serial.printf("Pin code request - using '0000'\n");
            hci_event_pin_code_request_get_bd_addr(packet, event_addr);
            gap_pin_code_response(event_addr, "0000");
            break;

        case HCI_EVENT_USER_CONFIRMATION_REQUEST:
            // ssp: inform about user confirmation request
            //Serial.printf("SSP User Confirmation Request with numeric value '%06" PRIu32 "'\n", little_endian_read_32(packet, 8));
            //Serial.printf("SSP User Confirmation Auto accept\n");
            break;

        case RFCOMM_EVENT_INCOMING_CONNECTION:
            rfcomm_event_incoming_connection_get_bd_addr(packet, event_addr);
            //rfcomm_channel_nr = rfcomm_event_incoming_connection_get_server_channel(packet);
            _channelID = rfcomm_event_incoming_connection_get_rfcomm_cid(packet);
            //Serial.printf("RFCOMM channel %u requested for %s\n", rfcomm_channel_nr, bd_addr_to_str(event_addr));
            rfcomm_accept_connection(_channelID);
            break;

        case RFCOMM_EVENT_CHANNEL_OPENED:
            if (rfcomm_event_channel_opened_get_status(packet)) {
                //Serial.printf("RFCOMM channel open failed, status 0x%02x\n", rfcomm_event_channel_opened_get_status(packet));
            } else {
                _channelID = rfcomm_event_channel_opened_get_rfcomm_cid(packet);
                //mtu = rfcomm_event_channel_opened_get_max_frame_size(packet);
                //Serial.printf("RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n", rfcomm_channel_id, mtu);
                _connected = true;
            }
            break;
        case RFCOMM_EVENT_CAN_SEND_NOW:
            rfcomm_send(_channelID, (uint8_t *)_writeBuff, _writeLen);
            _writeLen = 0;
            break;
        case RFCOMM_EVENT_CHANNEL_CLOSED:
            //Serial.printf("RFCOMM channel closed\n");
            _channelID = 0;
            _connected = false;
            break;

        default:
            break;
        }
        break;

    case RFCOMM_DATA_PACKET:
        for (i = 0; i < size; i++) {
            auto next_writer = _writer + 1;
            if (next_writer == _fifoSize) {
                next_writer = 0;
            }
            if (next_writer != _reader) {
                _queue[_writer] = packet[i];
                asm volatile("" ::: "memory"); // Ensure the queue is written before the written count advances
                // Avoid using division or mod because the HW divider could be in use
                _writer = next_writer;
            } else {
                _overflow = true;
            }
        }
        break;

    default:
        break;
    }
}

SerialBT_ SerialBT;
