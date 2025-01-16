/*
    A2DP Sink (Bluetooth audio receiver)

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
#include "BluetoothAudioConsumer.h"
#include "BluetoothMediaConfigurationSBC.h"

#include "btstack.h"
#include "btstack_resample.h"
#include "btstack_ring_buffer.h"

class A2DPSink {
public:
    A2DPSink() {
        _title[0] = 0;
        _artist[0] = 0;
        _album[0] = 0;
        _genre[0] = 0;
    }

    bool setName(const char *name) {
        if (_running) {
            return false;
        }
        free(_name);
        _name = strdup(name);
        return true;
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

    void onTrackChanged(void (*cb)(void *), void *cbData = nullptr) {
        _trackChangedCB = cb;
        _trackChangedData = cbData;
    }

    typedef enum { STOPPED, PLAYING, PAUSED } PlaybackStatus;
    void onPlaybackStatus(void (*cb)(void *, PlaybackStatus), void *cbData = nullptr) {
        _playbackStatusCB = cb;
        _playbackStatusData = cbData;
    }

    const uint8_t *getSourceAddress() {
        if (!_connected) {
            return nullptr;
        } else {
            return _sourceAddress;
        }
    }

    void setConsumer(BluetoothAudioConsumer_ *c) {
        _consumer = c;
    }

    bool begin();
    bool disconnect();
    void clearPairing();

    void playback_handler(int16_t * buffer, uint16_t num_audio_frames);

    void play() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_play(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void stop() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_stop(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void pause() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_pause(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void fastForward() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_fast_forward(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void rewind() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_rewind(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void forward() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_forward(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void backward() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_backward(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void volumeUp() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_volume_up(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void volumeDown() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_volume_down(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    void mute() {
        BluetoothLock b;
        if (_connected) {
            avrcp_controller_mute(a2dp_sink_avrcp_connection.avrcp_cid);
        }
    }

    const char *trackTitle() {
        return _title;
    }

    const char *trackArtist() {
        return _artist;
    }

    const char *trackAlbum() {
        return _album;
    }

    const char *trackGenre() {
        return _genre;
    }

private:
    void handle_pcm_data(int16_t * data, int num_audio_frames, int num_channels, int sample_rate, void * context);

    int media_processing_init(BluetoothMediaCodecConfigurationSBC * configuration);
    void media_processing_start();
    void media_processing_pause();
    void media_processing_close();

    void handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet, uint16_t size);
    int read_sbc_header(uint8_t * packet, int size, int * offset, avdtp_sbc_codec_header_t * sbc_header);
    int read_media_data_header(uint8_t *packet, int size, int *offset, avdtp_media_packet_header_t *media_header);
    void avrcp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void a2dp_sink_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

    BluetoothHCI _hci;
    BluetoothAudioConsumer_ *_consumer = nullptr;
    bool _running = false;
    bool _connected = false;

    // Callbacks
    void (*_avrcpCB)(void *, avrcp_operation_id_t, int) = nullptr;
    void *_avrcpData;
    void (*_batteryCB)(void *, avrcp_battery_status_t) = nullptr;
    void *_batteryData;
    void (*_volumeCB)(void *, int) = nullptr;
    void *_volumeData;
    void (*_connectCB)(void *, bool) = nullptr;
    void *_connectData;
    void (*_playbackStatusCB)(void *, PlaybackStatus) = nullptr;
    void *_playbackStatusData;
    void (*_trackChangedCB)(void *) = nullptr;
    void *_trackChangedData;

    char *_name = nullptr;
    uint8_t _sourceAddress[6];

    enum { NUM_CHANNELS = 2, BYTES_PER_FRAME = (2 * NUM_CHANNELS), MAX_SBC_FRAME_SIZE = 120 };

    uint8_t  sdp_avdtp_sink_service_buffer[150];
    uint8_t  sdp_avrcp_target_service_buffer[150];
    uint8_t  sdp_avrcp_controller_service_buffer[200];
    uint8_t  device_id_sdp_service_buffer[100];

    // we support all configurations with bitpool 2-53
    const uint8_t media_sbc_codec_capabilities[4] = {
        0xFF,//(AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
        0xFF,//(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
        2, 53
    };

    // SBC Decoder for WAV file or live playback
    btstack_sbc_decoder_state_t state;
    btstack_sbc_mode_t mode = SBC_MODE_STANDARD;

    // ring buffer for SBC Frames
    // below 30: add samples, 30-40: fine, above 40: drop samples

    enum { OPTIMAL_FRAMES_MIN = 60, OPTIMAL_FRAMES_MAX = 80, ADDITIONAL_FRAMES = 30 };
    uint8_t sbc_frame_storage[(OPTIMAL_FRAMES_MAX + ADDITIONAL_FRAMES) * MAX_SBC_FRAME_SIZE];
    btstack_ring_buffer_t sbc_frame_ring_buffer;
    unsigned int sbc_frame_size;

    // overflow buffer for not fully used sbc frames, with additional frames for resampling
    uint8_t decoded_audio_storage[(128 + 16) * BYTES_PER_FRAME];
    btstack_ring_buffer_t decoded_audio_ring_buffer;

    int media_initialized = 0;
    int audio_stream_started;

    btstack_resample_t resample_instance;

    // temp storage of lower-layer request for audio samples
    int16_t * request_buffer;
    int       request_frames;

    // sink state
    int volume_percentage = 0;
    avrcp_battery_status_t battery_status = AVRCP_BATTERY_STATUS_WARNING;

    typedef enum {
        STREAM_STATE_CLOSED,
        STREAM_STATE_OPEN,
        STREAM_STATE_PLAYING,
        STREAM_STATE_PAUSED,
    } stream_state_t;

    typedef struct {
        uint8_t  a2dp_local_seid;
        uint8_t  media_sbc_codec_configuration[4];
    } a2dp_sink_stream_endpoint_t;
    a2dp_sink_stream_endpoint_t a2dp_sink_stream_endpoint;

    typedef struct {
        bd_addr_t addr;
        uint16_t  a2dp_cid;
        uint8_t   a2dp_local_seid;
        stream_state_t stream_state;
        BluetoothMediaCodecConfigurationSBC sbc_configuration;
    } a2dp_sink_a2dp_connection_t;
    a2dp_sink_a2dp_connection_t a2dp_sink_a2dp_connection;

    typedef struct {
        bd_addr_t addr;
        uint16_t  avrcp_cid;
        bool playing;
        uint16_t notifications_supported_by_target;
    } a2dp_sink_avrcp_connection_t;
    a2dp_sink_avrcp_connection_t a2dp_sink_avrcp_connection;

    int16_t output_buffer[(128 + 16) * NUM_CHANNELS]; // 16 * 8 * 2

    char _title[64];
    char _artist[64];
    char _album[64];
    char _genre[32];
};
