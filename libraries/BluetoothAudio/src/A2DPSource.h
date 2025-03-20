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
#include <BluetoothHCI.h>
#include <BluetoothLock.h>
#include "BluetoothMediaConfigurationSBC.h"
#include <AudioOutputBase.h>
#include <functional>
#include <list>
#include <memory>


class A2DPSource : public Stream, public AudioOutputBase {
public:
    A2DPSource() {
    }

    virtual ~A2DPSource() { }

    virtual bool setFrequency(int rate) override {
        if (_running || ((rate != 44100) && (rate != 48000))) {
            return false;
        }
        _frequency = rate;
        return true;
    }

    bool setName(const char *name) {
        if (_running) {
            return false;
        }
        free(_name);
        _name = strdup(name);
        return true;
    }

    virtual bool setBitsPerSample(int bps) override {
        return bps == 16;
    }
    virtual bool setStereo(bool stereo = true) override {
        return stereo;
    }

    virtual void onTransmit(void (*cb)(void *), void *cbData = nullptr) override {
        _transmitCB = cb;
        _transmitData = cbData;
    }

    void onAVRCP(void (*cb)(void *, avrcp_operation_id_t, int), void *cbData = nullptr) {
        _avrcpCB = cb;
        _avrcpData = cbData;
    }

    void onBattery(void (*cb)(void *, avrcp_battery_status_t), void *cbData = nullptr) {
        _batteryCB = cb;
        _batteryData = cbData;
    }

    void onVolume(void (*cb)(void *, int), void *cbData = nullptr) {
        _volumeCB = cb;
        _volumeData = cbData;
    }

    void onConnect(void (*cb)(void *, bool), void *cbData = nullptr) {
        _connectCB = cb;
        _connectData = cbData;
    }

    bool setBufferSize(size_t size) {
        if (_running || (size & 127) || (size < 1024)) {
            return false;
        }
        _pcmBufferSize = size;
        return true;
    }

    virtual bool setBuffers(size_t buffers, size_t bufferWords, int32_t silenceSample = 0) override {
        return setBufferSize(buffers * bufferWords * sizeof(int32_t));
    }

    virtual bool getUnderflow() override {
        BluetoothLock b;
        if (!_running) {
            return false;
        }
        auto ret = _underflow;
        _underflow = false;
        return ret;
    }

    const uint8_t *getSinkAddress() {
        if (!_connected) {
            return nullptr;
        } else {
            return _sinkAddress;
        }
    }

    virtual bool begin() override;
    virtual bool end() override {
        return false; // We can't actually stop bluetooth on this device
    }

    std::vector<BTDeviceInfo> scan(uint32_t mask = BluetoothHCI::speaker_cod, int scanTimeSec = 5, bool async = false) {
        return _hci.scan(mask, scanTimeSec, async);
    }

    bool scanAsyncDone() {
        return _hci.scanAsyncDone();
    }

    std::vector<BTDeviceInfo> scanAsyncResult() {
        return _hci.scanAsyncResult();
    }

    bool connect(const uint8_t *addr = nullptr);

    bool connected() {
        return _connected;
    }

    bool disconnect();
    void clearPairing();

    // from Stream
    virtual int available() override {
        return 0; // Unreadable, this is output only
    }

    virtual int read() override {
        return 0;
    }

    virtual int peek() override {
        return 0;
    }

    virtual void flush() override {
    }

    // from Print (see notes on write() methods below)
    // Writes only full samples (size must be divisible by sample size in bytes)
    virtual size_t write(const uint8_t *buffer, size_t size) override;
    virtual int availableForWrite() override;

    virtual size_t write(uint8_t s) override {
        (void) s;
        return 0; // Never any reason to write 8-bit data, make it always fail.
    }


private:
    BluetoothHCI _hci;
    bool _running = false;
    bool _connected = false;

    char *_name = nullptr;
    int _frequency = 44100;

    // Callbacks
    void (*_transmitCB)(void *) = nullptr;
    void *_transmitData;
    void (*_avrcpCB)(void *, avrcp_operation_id_t, int) = nullptr;
    void *_avrcpData;
    void (*_batteryCB)(void *, avrcp_battery_status_t) = nullptr;
    void *_batteryData;
    void (*_volumeCB)(void *, int) = nullptr;
    void *_volumeData;
    void (*_connectCB)(void *, bool) = nullptr;
    void *_connectData;

    const uint8_t media_sbc_codec_capabilities[4] = {
        (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
        0xFF,//(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
        2, 53
    };
    uint8_t media_sbc_codec_configuration[4];

    enum {SBC_STORAGE_SIZE = 1030, AUDIO_TIMEOUT_MS = 5};
    typedef struct {
        uint16_t a2dp_cid;
        uint8_t  local_seid;
        uint8_t  remote_seid;
        uint8_t  stream_opened;
        uint16_t avrcp_cid;

        uint32_t time_audio_data_sent; // ms
        uint32_t acc_num_missed_samples;
        uint32_t samples_ready;
        btstack_timer_source_t audio_timer;
        uint8_t  streaming;
        int      max_media_payload_size;
        uint32_t rtp_timestamp;

        uint8_t  sbc_storage[SBC_STORAGE_SIZE];
        uint16_t sbc_storage_count;
        uint8_t  sbc_ready_to_send;

        uint8_t volume;
    } a2dp_media_sending_context_t;

    uint8_t sdp_a2dp_source_service_buffer[150];
    uint8_t sdp_avrcp_target_service_buffer[200];
    uint8_t sdp_avrcp_controller_service_buffer[200];
    uint8_t device_id_sdp_service_buffer[100];

    BluetoothMediaCodecConfigurationSBC sbc_configuration;
    btstack_sbc_encoder_state_t sbc_encoder_state;

    a2dp_media_sending_context_t media_tracker;

    typedef struct {
        uint8_t track_id[8];
        uint32_t song_length_ms;
        avrcp_playback_status_t status;
        uint32_t song_position_ms; // 0xFFFFFFFF if not supported
    } avrcp_play_status_info_t;
    avrcp_play_status_info_t play_info;

    void a2dp_timer_start(a2dp_media_sending_context_t * context);
    void a2dp_timer_stop(a2dp_media_sending_context_t * context);

    int16_t *_pcmBuffer = nullptr;
    size_t _pcmBufferSize = 4096; // Multiple of 128 required
    uint32_t _pcmWriter;
    uint32_t _pcmReader;
    bool _underflow;

    int a2dp_fill_sbc_audio_buffer(a2dp_media_sending_context_t * context);

    void a2dp_audio_timeout_handler(btstack_timer_source_t * timer);

    void a2dp_send_media_packet();

    int _tracks = 1;
    avrcp_track_t _currentTrack;
    uint8_t _sinkAddress[6];

    void a2dp_source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

    void avrcp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
};
