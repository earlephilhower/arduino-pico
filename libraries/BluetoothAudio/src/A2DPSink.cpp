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

#include "A2DPSink.h"
#include <functional>


#define CCALLBACKNAME _A2DPSINKCB
#include <ctocppcallback.h>

#define PACKETHANDLERCB(class, cbFcn) \
  (CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(CCALLBACKNAME<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))

#define L2CAPPACKETHANDLERCB(class, cbFcn) \
  (CCALLBACKNAME<void(uint8_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), \
   static_cast<void (*)(uint8_t, uint8_t *, uint16_t)>(CCALLBACKNAME<void(uint8_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))

#define PCMDECODERCB(class, cbFcn) \
  (CCALLBACKNAME<void(int16_t*, int, int, int, void*), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5), \
   static_cast<void (*)(int16_t *, int, int, int, void *)>(CCALLBACKNAME<void(int16_t *, int, int, int, void *), __COUNTER__ - 1>::callback))

// Based off of the BlueKitchen A2DP sink demo
/*
    Copyright (C) 2023 BlueKitchen GmbH

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

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "btstack.h"
#include "btstack_resample.h"
#include "btstack_ring_buffer.h"


bool A2DPSink::begin() {
    if (_running || !_consumer) {
        return false;
    }

    // init protocols
    l2cap_init();
    sdp_init();
#ifdef ENABLE_BLE
    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
#endif

    // Init profiles
    a2dp_sink_init();
    avrcp_init();
    avrcp_controller_init();
    avrcp_target_init();


    // Configure A2DP Sink
    a2dp_sink_register_packet_handler(PACKETHANDLERCB(A2DPSink, a2dp_sink_packet_handler));
    a2dp_sink_register_media_handler(L2CAPPACKETHANDLERCB(A2DPSink, handle_l2cap_media_data_packet));
    a2dp_sink_stream_endpoint_t * stream_endpoint = &a2dp_sink_stream_endpoint;
    avdtp_stream_endpoint_t * local_stream_endpoint = a2dp_sink_create_stream_endpoint(AVDTP_AUDIO,
            AVDTP_CODEC_SBC, media_sbc_codec_capabilities, sizeof(media_sbc_codec_capabilities),
            stream_endpoint->media_sbc_codec_configuration, sizeof(stream_endpoint->media_sbc_codec_configuration));
    if (!local_stream_endpoint) {
        DEBUGV("A2DP Source: not enough memory to create local stream endpoint\n");
        return false;
    }
    // - Store stream enpoint's SEP ID, as it is used by A2DP API to identify the stream endpoint
    stream_endpoint->a2dp_local_seid = avdtp_local_seid(local_stream_endpoint);


    // Configure AVRCP Controller + Target
    avrcp_register_packet_handler(PACKETHANDLERCB(A2DPSink, avrcp_packet_handler));
    avrcp_controller_register_packet_handler(PACKETHANDLERCB(A2DPSink, avrcp_controller_packet_handler));
    avrcp_target_register_packet_handler(PACKETHANDLERCB(A2DPSink, avrcp_target_packet_handler));


    // Configure SDP

    // - Create and register A2DP Sink service record
    memset(sdp_avdtp_sink_service_buffer, 0, sizeof(sdp_avdtp_sink_service_buffer));
    a2dp_sink_create_sdp_record(sdp_avdtp_sink_service_buffer, sdp_create_service_record_handle(),
                                AVDTP_SINK_FEATURE_MASK_HEADPHONE, NULL, NULL);
    sdp_register_service(sdp_avdtp_sink_service_buffer);

    // - Create AVRCP Controller service record and register it with SDP. We send Category 1 commands to the media player, e.g. play/pause
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features = 1 << AVRCP_CONTROLLER_SUPPORTED_FEATURE_CATEGORY_PLAYER_OR_RECORDER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer, sdp_create_service_record_handle(),
                                       controller_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    // - Create and register A2DP Sink service record
    //   -  We receive Category 2 commands from the media player, e.g. volume up/down
    memset(sdp_avrcp_target_service_buffer, 0, sizeof(sdp_avrcp_target_service_buffer));
    uint16_t target_supported_features = 1 << AVRCP_TARGET_SUPPORTED_FEATURE_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer,
                                   sdp_create_service_record_handle(), target_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    // - Create and register Device ID (PnP) service record
    memset(device_id_sdp_service_buffer, 0, sizeof(device_id_sdp_service_buffer));
    device_id_create_sdp_record(device_id_sdp_service_buffer,
                                sdp_create_service_record_handle(), DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);


    // Configure GAP - discovery / connection

    // - Set local name with a template Bluetooth address, that will be automatically
    //   replaced with an actual address once it is available, i.e. when BTstack boots
    //   up and starts talking to a Bluetooth module.
    if (!_name) {
        setName("PicoW A2DP 00:00:00:00:00:00");
    }
    gap_set_local_name(_name);

    // - Allow to show up in Bluetooth inquiry
    gap_discoverable_control(1);

    // - Set Class of Device - Service Class: Audio, Major Device Class: Audio, Minor: Loudspeaker
    gap_set_class_of_device(0x200414);

    // - Allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE);

    // - Allow for role switch on outgoing connections
    //   - This allows A2DP Source, e.g. smartphone, to become master when we re-connect to it.
    gap_set_allow_role_switch(true);


    // Register for HCI events
    //  hci_event_callback_registration.callback = &hci_packet_handler;
    //  hci_add_event_handler(&hci_event_callback_registration);
    _hci.install();
    _running = true;
    _hci.begin();

    return true;
}

bool A2DPSink::disconnect() {
    BluetoothLock b;
    if (_connected) {
        a2dp_sink_disconnect(a2dp_sink_a2dp_connection.a2dp_cid);
    }
    if (!_running || !_connected) {
        return false;
    }
    _connected = false;
    return true;
}

void A2DPSink::clearPairing() {
    BluetoothLock b;
    if (_connected) {
        a2dp_sink_disconnect(a2dp_sink_a2dp_connection.a2dp_cid);
    }
    gap_delete_all_link_keys();
}


void A2DPSink::playback_handler(int16_t * buffer, uint16_t num_audio_frames) {

    // called from lower-layer but guaranteed to be on main thread
    if (sbc_frame_size == 0) {
        memset(buffer, 0, num_audio_frames * BYTES_PER_FRAME);
        return;
    }

    // first fill from resampled audio
    uint32_t bytes_read;
    btstack_ring_buffer_read(&decoded_audio_ring_buffer, (uint8_t *) buffer, num_audio_frames * BYTES_PER_FRAME, &bytes_read);
    buffer          += bytes_read / NUM_CHANNELS;
    num_audio_frames   -= bytes_read / BYTES_PER_FRAME;

    // then start decoding sbc frames using request_* globals
    request_buffer = buffer;
    request_frames = num_audio_frames;
    while (request_frames && btstack_ring_buffer_bytes_available(&sbc_frame_ring_buffer) >= sbc_frame_size) {
        // decode frame
        uint8_t sbc_frame[MAX_SBC_FRAME_SIZE];
        btstack_ring_buffer_read(&sbc_frame_ring_buffer, sbc_frame, sbc_frame_size, &bytes_read);
        btstack_sbc_decoder_process_data(&state, 0, sbc_frame, sbc_frame_size);
    }
    while (request_frames) {
        *(request_buffer++) = 0;
        *(request_buffer++) = 0;
        request_frames--;
    }

}

void A2DPSink::handle_pcm_data(int16_t * data, int num_audio_frames, int num_channels, int sample_rate, void * context) {
    UNUSED(sample_rate);
    UNUSED(context);
    UNUSED(num_channels);   // must be stereo == 2

    // resample into request buffer - add some additional space for resampling
    uint32_t resampled_frames = btstack_resample_block(&resample_instance, data, num_audio_frames, output_buffer);

    // store data in btstack_audio buffer first
    int frames_to_copy = btstack_min(resampled_frames, request_frames);
    memcpy(request_buffer, output_buffer, frames_to_copy * BYTES_PER_FRAME);
    request_frames  -= frames_to_copy;
    request_buffer  += frames_to_copy * NUM_CHANNELS;

    // and rest in ring buffer
    int frames_to_store = resampled_frames - frames_to_copy;
    if (frames_to_store) {
        int status = btstack_ring_buffer_write(&decoded_audio_ring_buffer, (uint8_t *)&output_buffer[frames_to_copy * NUM_CHANNELS], frames_to_store * BYTES_PER_FRAME);
        if (status) {
            DEBUGV("Error storing samples in PCM ring buffer!!!\n");
        }
    }

}

int A2DPSink::media_processing_init(BluetoothMediaCodecConfigurationSBC * configuration) {
    if (media_initialized) {
        return 0;
    }
    btstack_sbc_decoder_init(&state, mode, PCMDECODERCB(A2DPSink, handle_pcm_data), NULL);

    btstack_ring_buffer_init(&sbc_frame_ring_buffer, sbc_frame_storage, sizeof(sbc_frame_storage));
    btstack_ring_buffer_init(&decoded_audio_ring_buffer, decoded_audio_storage, sizeof(decoded_audio_storage));
    btstack_resample_init(&resample_instance, configuration->num_channels);

    // setup audio playback
    _consumer->init(NUM_CHANNELS, configuration->sampling_frequency, this);

    audio_stream_started = 0;
    media_initialized = 1;
    return 0;
}

void A2DPSink::media_processing_start(void) {
    if (!media_initialized) {
        return;
    }

    // setup audio playback
    _consumer->startStream();
    audio_stream_started = 1;
}

void A2DPSink::media_processing_pause(void) {
    if (!media_initialized) {
        return;
    }

    // stop audio playback
    audio_stream_started = 0;
    _consumer->stopStream();

    // discard pending data
    btstack_ring_buffer_reset(&decoded_audio_ring_buffer);
    btstack_ring_buffer_reset(&sbc_frame_ring_buffer);
}

void A2DPSink::media_processing_close(void) {
    if (!media_initialized) {
        return;
    }

    media_initialized = 0;
    audio_stream_started = 0;
    sbc_frame_size = 0;

    // stop audio playback
    _consumer->close();
}

/*  @section Handle Media Data Packet

    @text Here the audio data, are received through the handle_l2cap_media_data_packet callback.
    Currently, only the SBC media codec is supported. Hence, the media data consists of the media packet header and the SBC packet.
    The SBC frame will be stored in a ring buffer for later processing (instead of decoding it to PCM right away which would require a much larger buffer).
    If the audio stream wasn't started already and there are enough SBC frames in the ring buffer, start playback.
*/

void A2DPSink::handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet, uint16_t size) {
    UNUSED(seid);
    int pos = 0;

    avdtp_media_packet_header_t media_header;
    if (!read_media_data_header(packet, size, &pos, &media_header)) {
        return;
    }

    avdtp_sbc_codec_header_t sbc_header;
    if (!read_sbc_header(packet, size, &pos, &sbc_header)) {
        return;
    }

    int packet_length = size - pos;
    uint8_t *packet_begin = packet + pos;

    // store sbc frame size for buffer management
    sbc_frame_size = packet_length / sbc_header.num_frames;
    int status = btstack_ring_buffer_write(&sbc_frame_ring_buffer, packet_begin, packet_length);
    if (status != ERROR_CODE_SUCCESS) {
        DEBUGV("Error storing samples in SBC ring buffer!!!\n");
    }

    // decide on audio sync drift based on number of sbc frames in queue
    int sbc_frames_in_buffer = btstack_ring_buffer_bytes_available(&sbc_frame_ring_buffer) / sbc_frame_size;

    uint32_t resampling_factor;

    // nominal factor (fixed-point 2^16) and compensation offset
    uint32_t nominal_factor = 0x10000;
    uint32_t compensation   = 0x00100;

    if (sbc_frames_in_buffer < OPTIMAL_FRAMES_MIN) {
        resampling_factor = nominal_factor - compensation;    // stretch samples
    } else if (sbc_frames_in_buffer <= OPTIMAL_FRAMES_MAX) {
        resampling_factor = nominal_factor;                   // nothing to do
    } else {
        resampling_factor = nominal_factor + compensation;    // compress samples
    }

    btstack_resample_set_factor(&resample_instance, resampling_factor);

    // start stream if enough frames buffered
    if (!audio_stream_started && sbc_frames_in_buffer >= OPTIMAL_FRAMES_MIN) {
        media_processing_start();
    }
}

int A2DPSink::read_sbc_header(uint8_t * packet, int size, int * offset, avdtp_sbc_codec_header_t * sbc_header) {
    int sbc_header_len = 12; // without crc
    int pos = *offset;

    if (size - pos < sbc_header_len) {
        DEBUGV("Not enough data to read SBC header, expected %d, received %d\n", sbc_header_len, size - pos);
        return 0;
    }

    sbc_header->fragmentation = get_bit16(packet[pos], 7);
    sbc_header->starting_packet = get_bit16(packet[pos], 6);
    sbc_header->last_packet = get_bit16(packet[pos], 5);
    sbc_header->num_frames = packet[pos] & 0x0f;
    pos++;
    *offset = pos;
    return 1;
}

int A2DPSink::read_media_data_header(uint8_t *packet, int size, int *offset, avdtp_media_packet_header_t *media_header) {
    int media_header_len = 12; // without crc
    int pos = *offset;

    if (size - pos < media_header_len) {
        DEBUGV("Not enough data to read media packet header, expected %d, received %d\n", media_header_len, size - pos);
        return 0;
    }

    media_header->version = packet[pos] & 0x03;
    media_header->padding = get_bit16(packet[pos], 2);
    media_header->extension = get_bit16(packet[pos], 3);
    media_header->csrc_count = (packet[pos] >> 4) & 0x0F;
    pos++;

    media_header->marker = get_bit16(packet[pos], 0);
    media_header->payload_type  = (packet[pos] >> 1) & 0x7F;
    pos++;

    media_header->sequence_number = big_endian_read_16(packet, pos);
    pos += 2;

    media_header->timestamp = big_endian_read_32(packet, pos);
    pos += 4;

    media_header->synchronization_source = big_endian_read_32(packet, pos);
    pos += 4;
    *offset = pos;
    return 1;
}

void A2DPSink::avrcp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    uint16_t local_cid;
    uint8_t  status;
    bd_addr_t address;

    a2dp_sink_avrcp_connection_t * connection = &a2dp_sink_avrcp_connection;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) {
        return;
    }
    switch (packet[2]) {
    case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED: {
        local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
        status = avrcp_subevent_connection_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("AVRCP: Connection failed, status 0x%02x\n", status);
            connection->avrcp_cid = 0;
            return;
        }

        connection->avrcp_cid = local_cid;
        avrcp_subevent_connection_established_get_bd_addr(packet, address);
        DEBUGV("AVRCP: Connected to %s, cid 0x%02x\n", bd_addr_to_str(address), connection->avrcp_cid);

        avrcp_target_support_event(connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
        avrcp_target_support_event(connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_BATT_STATUS_CHANGED);
        avrcp_target_battery_status_changed(connection->avrcp_cid, battery_status);

        // query supported events:
        avrcp_controller_get_supported_events(connection->avrcp_cid);
        return;
    }

    case AVRCP_SUBEVENT_CONNECTION_RELEASED:
        DEBUGV("AVRCP: Channel released: cid 0x%02x\n", avrcp_subevent_connection_released_get_avrcp_cid(packet));
        connection->avrcp_cid = 0;
        connection->notifications_supported_by_target = 0;
        return;
    default:
        break;
    }
}

void A2DPSink::avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    // helper to print c strings
    uint8_t avrcp_subevent_value[256];
    uint8_t play_status;
    uint8_t event_id;

    a2dp_sink_avrcp_connection_t * avrcp_connection = &a2dp_sink_avrcp_connection;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) {
        return;
    }
    if (avrcp_connection->avrcp_cid == 0) {
        return;
    }

    memset(avrcp_subevent_value, 0, sizeof(avrcp_subevent_value));
    switch (packet[2]) {
    case AVRCP_SUBEVENT_GET_CAPABILITY_EVENT_ID:
        avrcp_connection->notifications_supported_by_target |= (1 << avrcp_subevent_get_capability_event_id_get_event_id(packet));
        break;
    case AVRCP_SUBEVENT_GET_CAPABILITY_EVENT_ID_DONE:

        DEBUGV("AVRCP Controller: supported notifications by target:\n");
        for (event_id = (uint8_t) AVRCP_NOTIFICATION_EVENT_FIRST_INDEX; event_id < (uint8_t) AVRCP_NOTIFICATION_EVENT_LAST_INDEX; event_id++) {
            DEBUGV("   - [%s] %s\n",
                   (avrcp_connection->notifications_supported_by_target & (1 << event_id)) != 0 ? "X" : " ",
                   avrcp_notification2str((avrcp_notification_event_id_t)event_id));
        }
        DEBUGV("\n\n");

        // automatically enable notifications
        avrcp_controller_enable_notification(avrcp_connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED);
        avrcp_controller_enable_notification(avrcp_connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED);
        avrcp_controller_enable_notification(avrcp_connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
        break;

    case AVRCP_SUBEVENT_NOTIFICATION_STATE:
        event_id = (avrcp_notification_event_id_t)avrcp_subevent_notification_state_get_event_id(packet);
        DEBUGV("AVRCP Controller: %s notification registered\n", avrcp_notification2str((avrcp_notification_event_id_t)event_id));
        break;

    case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_POS_CHANGED:
        DEBUGV("AVRCP Controller: Playback position changed, position %d ms\n", (unsigned int) avrcp_subevent_notification_playback_pos_changed_get_playback_position_ms(packet));
        break;
    case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_STATUS_CHANGED:
        DEBUGV("AVRCP Controller: Playback status changed %s\n", avrcp_play_status2str(avrcp_subevent_notification_playback_status_changed_get_play_status(packet)));
        play_status = avrcp_subevent_notification_playback_status_changed_get_play_status(packet);
        switch (play_status) {
        case AVRCP_PLAYBACK_STATUS_PLAYING:
            avrcp_connection->playing = true;
            break;
        default:
            avrcp_connection->playing = false;
            break;
        }
        if (_playbackStatusCB) {
            PlaybackStatus status;
            switch (play_status) {
            case AVRCP_PLAYBACK_STATUS_PLAYING:
                status = PLAYING;
                break;
            case AVRCP_PLAYBACK_STATUS_PAUSED:
                status = PAUSED;
                break;
            default:
                status = STOPPED;
                break;
            }
            _playbackStatusCB(_playbackStatusData, status);
        }
        break;

    case AVRCP_SUBEVENT_NOTIFICATION_NOW_PLAYING_CONTENT_CHANGED:
        DEBUGV("AVRCP Controller: Playing content changed\n");
        break;

    case AVRCP_SUBEVENT_NOTIFICATION_TRACK_CHANGED:
        _title[0] = 0;
        _artist[0] = 0;
        _album[0] = 0;
        _genre[0] = 0;
        avrcp_controller_get_now_playing_info(avrcp_connection->avrcp_cid);
        if (_trackChangedCB) {
            _trackChangedCB(_trackChangedData);
        }
        DEBUGV("AVRCP Controller: Track changed\n");
        break;

    case AVRCP_SUBEVENT_NOTIFICATION_AVAILABLE_PLAYERS_CHANGED:
        DEBUGV("AVRCP Controller: Available Players Changed\n");
        break;

    case AVRCP_SUBEVENT_SHUFFLE_AND_REPEAT_MODE: {
        uint8_t shuffle_mode = avrcp_subevent_shuffle_and_repeat_mode_get_shuffle_mode(packet);
        uint8_t repeat_mode  = avrcp_subevent_shuffle_and_repeat_mode_get_repeat_mode(packet);
        (void) shuffle_mode;
        (void) repeat_mode;
        DEBUGV("AVRCP Controller: %s, %s\n", avrcp_shuffle2str(shuffle_mode), avrcp_repeat2str(repeat_mode));
        break;
    }
    case AVRCP_SUBEVENT_NOW_PLAYING_TRACK_INFO:
        DEBUGV("AVRCP Controller: Track %d\n", avrcp_subevent_now_playing_track_info_get_track(packet));
        break;

    case AVRCP_SUBEVENT_NOW_PLAYING_TOTAL_TRACKS_INFO:
        DEBUGV("AVRCP Controller: Total Tracks %d\n", avrcp_subevent_now_playing_total_tracks_info_get_total_tracks(packet));
        break;

    case AVRCP_SUBEVENT_NOW_PLAYING_TITLE_INFO:
        if (avrcp_subevent_now_playing_title_info_get_value_len(packet) > 0) {
            memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_title_info_get_value(packet), avrcp_subevent_now_playing_title_info_get_value_len(packet));
            strncpy(_title, (char *)avrcp_subevent_value, sizeof(_title));
            _title[sizeof(_title) - 1] = 0;
            DEBUGV("AVRCP Controller: Title %s\n", avrcp_subevent_value);
        }
        break;

    case AVRCP_SUBEVENT_NOW_PLAYING_ARTIST_INFO:
        if (avrcp_subevent_now_playing_artist_info_get_value_len(packet) > 0) {
            memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_artist_info_get_value(packet), avrcp_subevent_now_playing_artist_info_get_value_len(packet));
            strncpy(_artist, (char *)avrcp_subevent_value, sizeof(_artist));
            _artist[sizeof(_artist) - 1] = 0;
            DEBUGV("AVRCP Controller: Artist %s\n", avrcp_subevent_value);
        }
        break;

    case AVRCP_SUBEVENT_NOW_PLAYING_ALBUM_INFO:
        if (avrcp_subevent_now_playing_album_info_get_value_len(packet) > 0) {
            memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_album_info_get_value(packet), avrcp_subevent_now_playing_album_info_get_value_len(packet));
            strncpy(_album, (char *)avrcp_subevent_value, sizeof(_album));
            _album[sizeof(_album) - 1] = 0;
            DEBUGV("AVRCP Controller: Album %s\n", avrcp_subevent_value);
        }
        break;

    case AVRCP_SUBEVENT_NOW_PLAYING_GENRE_INFO:
        if (avrcp_subevent_now_playing_genre_info_get_value_len(packet) > 0) {
            memcpy(avrcp_subevent_value, avrcp_subevent_now_playing_genre_info_get_value(packet), avrcp_subevent_now_playing_genre_info_get_value_len(packet));
            strncpy(_genre, (char *)avrcp_subevent_value, sizeof(_genre));
            _genre[sizeof(_genre) - 1] = 0;
            DEBUGV("AVRCP Controller: Genre %s\n", avrcp_subevent_value);
        }
        break;

    case AVRCP_SUBEVENT_PLAY_STATUS:
        DEBUGV("AVRCP Controller: Song length %" PRIu32 " ms, Song position %" PRIu32 " ms, Play status %s\n",
               avrcp_subevent_play_status_get_song_length(packet),
               avrcp_subevent_play_status_get_song_position(packet),
               avrcp_play_status2str(avrcp_subevent_play_status_get_play_status(packet)));
        break;

    case AVRCP_SUBEVENT_OPERATION_COMPLETE:
        DEBUGV("AVRCP Controller: %s complete\n", avrcp_operation2str(avrcp_subevent_operation_complete_get_operation_id(packet)));
        break;

    case AVRCP_SUBEVENT_OPERATION_START:
        DEBUGV("AVRCP Controller: %s start\n", avrcp_operation2str(avrcp_subevent_operation_start_get_operation_id(packet)));
        break;

    case AVRCP_SUBEVENT_NOTIFICATION_EVENT_TRACK_REACHED_END:
        DEBUGV("AVRCP Controller: Track reached end\n");
        break;

    case AVRCP_SUBEVENT_PLAYER_APPLICATION_VALUE_RESPONSE:
        DEBUGV("AVRCP Controller: Set Player App Value %s\n", avrcp_ctype2str(avrcp_subevent_player_application_value_response_get_command_type(packet)));
        break;

    default:
        break;
    }
}

void A2DPSink::avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) {
        return;
    }

    uint8_t volume;
    char const * button_state;
    (void) button_state;
    avrcp_operation_id_t operation_id;

    switch (packet[2]) {
    case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
        volume = avrcp_subevent_notification_volume_changed_get_absolute_volume(packet);
        volume_percentage = volume * 100 / 127;
        DEBUGV("AVRCP Target    : Volume set to %d%% (%d)\n", volume_percentage, volume);
        _consumer->setVolume(volume);
        if (_volumeCB) {
            _volumeCB(_volumeData, volume);
        }
        break;

    case AVRCP_SUBEVENT_OPERATION:
        operation_id = (avrcp_operation_id_t)avrcp_subevent_operation_get_operation_id(packet);
        button_state = avrcp_subevent_operation_get_button_pressed(packet) > 0 ? "PRESS" : "RELEASE";
        DEBUGV("AVRCP Target: operation %s (%s)\n", avrcp_operation2str(operation_id), button_state);
        if (_avrcpCB) {
            _avrcpCB(_avrcpData, operation_id, avrcp_subevent_operation_get_button_pressed(packet) > 0);
        }
        switch (operation_id) {
        case AVRCP_OPERATION_ID_VOLUME_UP:
            DEBUGV("AVRCP Target    : VOLUME UP (%s)\n", button_state);
            break;
        case AVRCP_OPERATION_ID_VOLUME_DOWN:
            DEBUGV("AVRCP Target    : VOLUME DOWN (%s)\n", button_state);
            break;
        default:
            return;
        }
        break;
    default:
        DEBUGV("AVRCP Target    : Event 0x%02x is not parsed\n", packet[2]);
        break;
    }
}

void A2DPSink::a2dp_sink_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    uint8_t status;

    uint8_t allocation_method;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META) {
        return;
    }

    a2dp_sink_a2dp_connection_t * a2dp_conn = &a2dp_sink_a2dp_connection;

    switch (packet[2]) {
    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_OTHER_CONFIGURATION:
        DEBUGV("A2DP  Sink      : Received non SBC codec - not implemented\n");
        break;
    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        DEBUGV("A2DP  Sink      : Received SBC codec configuration\n");
        a2dp_conn->sbc_configuration.reconfigure = a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(packet);
        a2dp_conn->sbc_configuration.num_channels = a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(packet);
        a2dp_conn->sbc_configuration.sampling_frequency = a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(packet);
        a2dp_conn->sbc_configuration.block_length = a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(packet);
        a2dp_conn->sbc_configuration.subbands = a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(packet);
        a2dp_conn->sbc_configuration.min_bitpool_value = a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(packet);
        a2dp_conn->sbc_configuration.max_bitpool_value = a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(packet);

        allocation_method = a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(packet);

        // Adapt Bluetooth spec definition to SBC Encoder expected input
        a2dp_conn->sbc_configuration.allocation_method = (btstack_sbc_allocation_method_t)(allocation_method - 1);

        switch (a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(packet)) {
        case AVDTP_CHANNEL_MODE_JOINT_STEREO:
            a2dp_conn->sbc_configuration.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
            break;
        case AVDTP_CHANNEL_MODE_STEREO:
            a2dp_conn->sbc_configuration.channel_mode = SBC_CHANNEL_MODE_STEREO;
            break;
        case AVDTP_CHANNEL_MODE_DUAL_CHANNEL:
            a2dp_conn->sbc_configuration.channel_mode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
            break;
        case AVDTP_CHANNEL_MODE_MONO:
            a2dp_conn->sbc_configuration.channel_mode = SBC_CHANNEL_MODE_MONO;
            break;
        default:
            btstack_assert(false);
            break;
        }
        a2dp_conn->sbc_configuration.dump();
        break;
    }

    case A2DP_SUBEVENT_STREAM_ESTABLISHED:
        status = a2dp_subevent_stream_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("A2DP  Sink      : Streaming connection failed, status 0x%02x\n", status);
            break;
        }

        a2dp_subevent_stream_established_get_bd_addr(packet, a2dp_conn->addr);
        a2dp_conn->a2dp_cid = a2dp_subevent_stream_established_get_a2dp_cid(packet);
        a2dp_conn->a2dp_local_seid = a2dp_subevent_stream_established_get_local_seid(packet);
        a2dp_conn->stream_state = STREAM_STATE_OPEN;

        DEBUGV("A2DP  Sink      : Streaming connection is established, address %s, cid 0x%02x, local seid %d\n",
               bd_addr_to_str(a2dp_conn->addr), a2dp_conn->a2dp_cid, a2dp_conn->a2dp_local_seid);
        memcpy(_sourceAddress, a2dp_conn->addr, sizeof(_sourceAddress));

        _connected = true;
        if (_connectCB) {
            _connectCB(_connectData, true);
        }
        break;

    case A2DP_SUBEVENT_STREAM_STARTED:
        DEBUGV("A2DP  Sink      : Stream started\n");
        a2dp_conn->stream_state = STREAM_STATE_PLAYING;
        if (a2dp_conn->sbc_configuration.reconfigure) {
            media_processing_close();
        }
        // prepare media processing
        media_processing_init(&a2dp_conn->sbc_configuration);
        // audio stream is started when buffer reaches minimal level
        break;

    case A2DP_SUBEVENT_STREAM_SUSPENDED:
        DEBUGV("A2DP  Sink      : Stream paused\n");
        a2dp_conn->stream_state = STREAM_STATE_PAUSED;
        media_processing_pause();
        break;

    case A2DP_SUBEVENT_STREAM_RELEASED:
        DEBUGV("A2DP  Sink      : Stream released\n");
        a2dp_conn->stream_state = STREAM_STATE_CLOSED;
        media_processing_close();
        break;

    case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        DEBUGV("A2DP  Sink      : Signaling connection released\n");
        a2dp_conn->a2dp_cid = 0;
        media_processing_close();
        _connected = false;
        if (_connectCB) {
            _connectCB(_connectData, false);
        }
        break;

    default:
        break;
    }
}
