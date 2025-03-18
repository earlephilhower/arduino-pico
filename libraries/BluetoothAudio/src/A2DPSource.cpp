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

// Based heavily off of the a2dp_source example from BlueKitchen BTStack

/*
    Copyright (C) 2016 BlueKitchen GmbH

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


#include <Arduino.h>
#include "A2DPSource.h"

#define CCALLBACKNAME _A2DPSOURCECB
#include <ctocppcallback.h>

#define PACKETHANDLERCB(class, cbFcn) \
  (_A2DPSOURCECB<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), \
   static_cast<btstack_packet_handler_t>(_A2DPSOURCECB<void(uint8_t, uint16_t, uint8_t*, uint16_t), __COUNTER__ - 1>::callback))

#define TIMEOUTHANDLERCB(class, cbFcn) \
  (_A2DPSOURCECB<void(btstack_timer_source_t *), __COUNTER__>::func = std::bind(&class::cbFcn, this, std::placeholders::_1), \
   static_cast<void(*)(btstack_timer_source_t*)>(_A2DPSOURCECB<void(btstack_timer_source_t*), __COUNTER__ - 1>::callback))


bool A2DPSource::begin() {
    if (_running) {
        return false;
    }

    _pcmBuffer = (int16_t *)malloc(_pcmBufferSize * sizeof(int16_t));
    if (!_pcmBuffer) {
        DEBUGV("A2DPSource: OOM for pcm buffer\n");
        return false;
    }
    _pcmWriter = 0;
    _pcmReader = 0;

    // Request role change on reconnecting headset to always use them in slave mode
    hci_set_master_slave_policy(0);
    // enabled EIR
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    l2cap_init();

#ifdef ENABLE_BLE
    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
#endif

    // Initialize  A2DP Source
    a2dp_source_init();
    a2dp_source_register_packet_handler(PACKETHANDLERCB(A2DPSource, a2dp_source_packet_handler));

    // Create stream endpoint
    avdtp_stream_endpoint_t * local_stream_endpoint = a2dp_source_create_stream_endpoint(AVDTP_AUDIO, AVDTP_CODEC_SBC, media_sbc_codec_capabilities, sizeof(media_sbc_codec_capabilities), media_sbc_codec_configuration, sizeof(media_sbc_codec_configuration));
    if (!local_stream_endpoint) {
        DEBUGV("A2DP Source: not enough memory to create local stream endpoint\n");
        return false;
    }

    // Store stream endpoint's SEP ID, as it is used by A2DP API to identify the stream endpoint
    media_tracker.local_seid = avdtp_local_seid(local_stream_endpoint);
    avdtp_source_register_delay_reporting_category(media_tracker.local_seid);

    // Initialize AVRCP Service
    avrcp_init();
    avrcp_register_packet_handler(PACKETHANDLERCB(A2DPSource, avrcp_packet_handler));
    // Initialize AVRCP Target
    avrcp_target_init();
    avrcp_target_register_packet_handler(PACKETHANDLERCB(A2DPSource, avrcp_target_packet_handler));

    // Initialize AVRCP Controller
    avrcp_controller_init();
    avrcp_controller_register_packet_handler(PACKETHANDLERCB(A2DPSource, avrcp_controller_packet_handler));

    // Initialize SDP,
    sdp_init();

    // Create A2DP Source service record and register it with SDP
    memset(sdp_a2dp_source_service_buffer, 0, sizeof(sdp_a2dp_source_service_buffer));
    a2dp_source_create_sdp_record(sdp_a2dp_source_service_buffer, 0x10001, AVDTP_SOURCE_FEATURE_MASK_PLAYER, NULL, NULL);
    sdp_register_service(sdp_a2dp_source_service_buffer);

    // Create AVRCP Target service record and register it with SDP. We receive Category 1 commands from the headphone, e.g. play/pause
    memset(sdp_avrcp_target_service_buffer, 0, sizeof(sdp_avrcp_target_service_buffer));
    uint16_t supported_features = AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
#ifdef AVRCP_BROWSING_ENABLED
    supported_features |= AVRCP_FEATURE_MASK_BROWSING;
#endif
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, 0x10002, supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    // Create AVRCP Controller service record and register it with SDP. We send Category 2 commands to the headphone, e.g. volume up/down
    memset(sdp_avrcp_controller_service_buffer, 0, sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features = AVRCP_FEATURE_MASK_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer, 0x10003, controller_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    // Register Device ID (PnP) service SDP record
    memset(device_id_sdp_service_buffer, 0, sizeof(device_id_sdp_service_buffer));
    device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10004, DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);

    // Set local name with a template Bluetooth address, that will be automatically
    // replaced with a actual address once it is available, i.e. when BTstack boots
    // up and starts talking to a Bluetooth module.
    if (!_name) {
        setName("PicoW A2DP 00:00:00:00:00:00");
    }
    gap_set_local_name(_name);
    gap_discoverable_control(1);
    gap_set_class_of_device(0x200408);

    // Register for HCI events.
    _hci.install();
    _running = true;
    _hci.begin();
    return true;
}

bool A2DPSource::connect(const uint8_t *addr) {
    if (!_running) {
        return false;
    }
    while (!_hci.running()) {
        delay(10);
    }
    uint8_t a[6];
    if (addr) {
        memcpy(a, addr, sizeof(a));
        auto ret = !a2dp_source_establish_stream(a, &media_tracker.a2dp_cid);
        return ret;
    } else {
        clearPairing();
        auto l = scan();
        for (auto e : l) {
            DEBUGV("Scan connecting %s at %s ... ", e.name(), e.addressString());
            memcpy(a, e.address(), sizeof(a));
            if (!a2dp_source_establish_stream(a, &media_tracker.a2dp_cid)) {
                DEBUGV("Connection established\n");
                return true;
            }
            DEBUGV("Failed\n");
        }
        return false;
    }
}
bool A2DPSource::disconnect() {
    BluetoothLock b;
    if (_connected) {
        a2dp_source_disconnect(media_tracker.a2dp_cid);
    }
    if (!_running || !_connected) {
        return false;
    }
    _connected = false;
    return true;
}

void A2DPSource::clearPairing() {
    BluetoothLock b;
    if (_connected) {
        a2dp_source_disconnect(media_tracker.a2dp_cid);
    }
    gap_delete_all_link_keys();
}

// from Print (see notes on write() methods below)
size_t A2DPSource::write(const uint8_t *buffer, size_t size) {
    BluetoothLock b;

    size = std::min((size_t)availableForWrite(), size);

    size_t count = 0;
    size /= sizeof(int16_t); // Convert size to samples

    // First copy from writer to either end of
    uint32_t start = _pcmWriter;
    uint32_t end = _pcmReader > _pcmWriter ? _pcmReader : _pcmBufferSize;
    if (end - start > size) {
        end = start + size;
    }
    memcpy(_pcmBuffer + start, buffer, (end - start) * sizeof(int16_t));
    count += (end - start) * sizeof(int16_t);
    size -= end - start;
    _pcmWriter += end - start;
    _pcmWriter %= _pcmBufferSize;
    // Possibly wraparound to 0 if still left
    if (size) {
        start = 0;
        end = _pcmReader;
        if (end - start > size) {
            end = size;
        }
        memcpy(_pcmBuffer + start, buffer, (end - start) * sizeof(int16_t));
        count += (end - start) * sizeof(int16_t);
        size -= end - start;
        _pcmWriter += end - start;
        _pcmWriter %= _pcmBufferSize;
    }
    return count;
}

int A2DPSource::availableForWrite() {
    BluetoothLock b;
    int avail = 0;
    if (_pcmWriter == _pcmReader) {
        avail =  _pcmBufferSize - 1;
    } else if (_pcmReader > _pcmWriter) {
        avail =  _pcmReader - _pcmWriter - 1;
    } else {
        avail = _pcmBufferSize - _pcmWriter + _pcmReader - 1;
    }
    avail *= sizeof(int16_t); // Convert samples to bytes
    return avail;
}

void A2DPSource::a2dp_timer_start(a2dp_media_sending_context_t * context) {
    context->max_media_payload_size = btstack_min(a2dp_max_media_payload_size(context->a2dp_cid, context->local_seid), SBC_STORAGE_SIZE);
    context->sbc_storage_count = 0;
    context->sbc_ready_to_send = 0;
    context->streaming = 1;
    btstack_run_loop_remove_timer(&context->audio_timer);
    btstack_run_loop_set_timer_handler(&context->audio_timer, TIMEOUTHANDLERCB(A2DPSource, a2dp_audio_timeout_handler));
    btstack_run_loop_set_timer_context(&context->audio_timer, context);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
}

void A2DPSource::a2dp_timer_stop(a2dp_media_sending_context_t * context) {
    context->time_audio_data_sent = 0;
    context->acc_num_missed_samples = 0;
    context->samples_ready = 0;
    context->streaming = 1;
    context->sbc_storage_count = 0;
    context->sbc_ready_to_send = 0;
    btstack_run_loop_remove_timer(&context->audio_timer);
}

int A2DPSource::a2dp_fill_sbc_audio_buffer(a2dp_media_sending_context_t * context) {
    // perform sbc encoding
    int total_num_bytes_read = 0;
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    while (context->samples_ready >= num_audio_samples_per_sbc_buffer
            && (context->max_media_payload_size - context->sbc_storage_count) >= btstack_sbc_encoder_sbc_buffer_length()) {

        if ((_pcmWriter / 256) != (_pcmReader / 256)) {
            btstack_sbc_encoder_process_data(_pcmBuffer + _pcmReader);
            asm volatile("" ::: "memory"); // Ensure the data is processed before advancing
            auto next_reader = (_pcmReader + 256) % _pcmBufferSize;
            asm volatile("" ::: "memory"); // Ensure the reader value is only written once, correctly
            _pcmReader = next_reader;
        } else {
            _underflow = true;
            // Just keep sending old data
            btstack_sbc_encoder_process_data(_pcmBuffer + _pcmReader);
        }

        uint16_t sbc_frame_size = btstack_sbc_encoder_sbc_buffer_length();
        uint8_t *sbc_frame = btstack_sbc_encoder_sbc_buffer();

        total_num_bytes_read += num_audio_samples_per_sbc_buffer;
        // first byte in sbc storage contains sbc media header
        memcpy(&context->sbc_storage[1 + context->sbc_storage_count], sbc_frame, sbc_frame_size);
        context->sbc_storage_count += sbc_frame_size;
        context->samples_ready -= num_audio_samples_per_sbc_buffer;
    }
    return total_num_bytes_read;
}

void A2DPSource::a2dp_audio_timeout_handler(btstack_timer_source_t * timer) {
    a2dp_media_sending_context_t * context = (a2dp_media_sending_context_t *) btstack_run_loop_get_timer_context(timer);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
    uint32_t now = btstack_run_loop_get_time_ms();

    uint32_t update_period_ms = AUDIO_TIMEOUT_MS;
    if (context->time_audio_data_sent > 0) {
        update_period_ms = now - context->time_audio_data_sent;
    }

    uint32_t num_samples = (update_period_ms * _frequency) / 1000;
    context->acc_num_missed_samples += (update_period_ms * _frequency) % 1000;

    while (context->acc_num_missed_samples >= 1000) {
        num_samples++;
        context->acc_num_missed_samples -= 1000;
    }
    context->time_audio_data_sent = now;
    context->samples_ready += num_samples;

    if (context->sbc_ready_to_send) {
        return;
    }

    a2dp_fill_sbc_audio_buffer(context);

    if ((context->sbc_storage_count + btstack_sbc_encoder_sbc_buffer_length()) > context->max_media_payload_size) {
        // schedule sending
        context->sbc_ready_to_send = 1;
        a2dp_source_stream_endpoint_request_can_send_now(context->a2dp_cid, context->local_seid);
    }
}

void A2DPSource::a2dp_send_media_packet() {
    int num_bytes_in_frame = btstack_sbc_encoder_sbc_buffer_length();
    int bytes_in_storage = media_tracker.sbc_storage_count;
    uint8_t num_sbc_frames = bytes_in_storage / num_bytes_in_frame;
    // Prepend SBC Header
    media_tracker.sbc_storage[0] = num_sbc_frames;  // (fragmentation << 7) | (starting_packet << 6) | (last_packet << 5) | num_frames;
    a2dp_source_stream_send_media_payload_rtp(media_tracker.a2dp_cid, media_tracker.local_seid, 0,
            media_tracker.rtp_timestamp,
            media_tracker.sbc_storage, bytes_in_storage + 1);

    // update rtp_timestamp
    unsigned int num_audio_samples_per_sbc_buffer = btstack_sbc_encoder_num_audio_frames();
    media_tracker.rtp_timestamp += num_sbc_frames * num_audio_samples_per_sbc_buffer;

    media_tracker.sbc_storage_count = 0;
    media_tracker.sbc_ready_to_send = 0;
    if (_transmitCB) {
        _transmitCB(_transmitData);
    }
}

void A2DPSource::a2dp_source_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    uint8_t status;
    uint8_t local_seid;
    bd_addr_t address;
    uint16_t cid;

    avdtp_channel_mode_t channel_mode;
    uint8_t allocation_method;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META) {
        return;
    }

    switch (hci_event_a2dp_meta_get_subevent_code(packet)) {
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_ESTABLISHED:
        a2dp_subevent_signaling_connection_established_get_bd_addr(packet, address);
        cid = a2dp_subevent_signaling_connection_established_get_a2dp_cid(packet);
        status = a2dp_subevent_signaling_connection_established_get_status(packet);

        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("A2DP Source: Connection failed, status 0x%02x, cid 0x%02x, a2dp_cid 0x%02x \n", status, cid, media_tracker.a2dp_cid);
            media_tracker.a2dp_cid = 0;
            break;
        }
        media_tracker.a2dp_cid = cid;
        media_tracker.volume = 32;
        memcpy(_sinkAddress, address, sizeof(_sinkAddress));
        DEBUGV("A2DP Source: Connected to address %s, a2dp cid 0x%02x, local seid 0x%02x.\n", bd_addr_to_str(address), media_tracker.a2dp_cid, media_tracker.local_seid);
        break;

    case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        cid  = avdtp_subevent_signaling_media_codec_sbc_configuration_get_avdtp_cid(packet);
        if (cid != media_tracker.a2dp_cid) {
            return;
        }

        media_tracker.remote_seid = a2dp_subevent_signaling_media_codec_sbc_configuration_get_remote_seid(packet);

        sbc_configuration.reconfigure = a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(packet);
        sbc_configuration.num_channels = a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(packet);
        sbc_configuration.sampling_frequency = a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(packet);
        sbc_configuration.block_length = a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(packet);
        sbc_configuration.subbands = a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(packet);
        sbc_configuration.min_bitpool_value = a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(packet);
        sbc_configuration.max_bitpool_value = a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(packet);

        channel_mode = (avdtp_channel_mode_t) a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(packet);
        allocation_method = a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(packet);

        DEBUGV("A2DP Source: Received SBC codec configuration, sampling frequency %u, a2dp_cid 0x%02x, local seid 0x%02x, remote seid 0x%02x.\n",
               sbc_configuration.sampling_frequency, cid,
               a2dp_subevent_signaling_media_codec_sbc_configuration_get_local_seid(packet),
               a2dp_subevent_signaling_media_codec_sbc_configuration_get_remote_seid(packet));

        // Adapt Bluetooth spec definition to SBC Encoder expected input
        sbc_configuration.allocation_method = (btstack_sbc_allocation_method_t)(allocation_method - 1);
        switch (channel_mode) {
        case AVDTP_CHANNEL_MODE_JOINT_STEREO:
            sbc_configuration.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
            break;
        case AVDTP_CHANNEL_MODE_STEREO:
            sbc_configuration.channel_mode = SBC_CHANNEL_MODE_STEREO;
            break;
        case AVDTP_CHANNEL_MODE_DUAL_CHANNEL:
            sbc_configuration.channel_mode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
            break;
        case AVDTP_CHANNEL_MODE_MONO:
            sbc_configuration.channel_mode = SBC_CHANNEL_MODE_MONO;
            break;
        default:
            btstack_assert(false);
            break;
        }
        sbc_configuration.dump();

        btstack_sbc_encoder_init(&sbc_encoder_state, SBC_MODE_STANDARD,
                                 sbc_configuration.block_length, sbc_configuration.subbands,
                                 sbc_configuration.allocation_method, sbc_configuration.sampling_frequency,
                                 sbc_configuration.max_bitpool_value,
                                 sbc_configuration.channel_mode);
        break;
    }

    case A2DP_SUBEVENT_SIGNALING_DELAY_REPORTING_CAPABILITY:
        DEBUGV("A2DP Source: remote supports delay report, remote seid %d\n",
               avdtp_subevent_signaling_delay_reporting_capability_get_remote_seid(packet));
        break;
    case A2DP_SUBEVENT_SIGNALING_CAPABILITIES_DONE:
        DEBUGV("A2DP Source: All capabilities reported, remote seid %d\n",
               avdtp_subevent_signaling_capabilities_done_get_remote_seid(packet));
        break;

    case A2DP_SUBEVENT_SIGNALING_DELAY_REPORT:
        DEBUGV("A2DP Source: Received delay report of %d.%0d ms, local seid %d\n",
               avdtp_subevent_signaling_delay_report_get_delay_100us(packet) / 10, avdtp_subevent_signaling_delay_report_get_delay_100us(packet) % 10,
               avdtp_subevent_signaling_delay_report_get_local_seid(packet));
        break;

    case A2DP_SUBEVENT_STREAM_ESTABLISHED:
        a2dp_subevent_stream_established_get_bd_addr(packet, address);
        status = a2dp_subevent_stream_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("A2DP Source: Stream failed, status 0x%02x.\n", status);
            break;
        }

        local_seid = a2dp_subevent_stream_established_get_local_seid(packet);
        cid = a2dp_subevent_stream_established_get_a2dp_cid(packet);
        (void) local_seid;
        DEBUGV("A2DP Source: Stream established a2dp_cid 0x%02x, local_seid 0x%02x, remote_seid 0x%02x\n", cid, local_seid, a2dp_subevent_stream_established_get_remote_seid(packet));

        media_tracker.stream_opened = 1;
        status = a2dp_source_start_stream(media_tracker.a2dp_cid, media_tracker.local_seid);
        break;

    case A2DP_SUBEVENT_STREAM_RECONFIGURED:
        status = a2dp_subevent_stream_reconfigured_get_status(packet);
        local_seid = a2dp_subevent_stream_reconfigured_get_local_seid(packet);
        cid = a2dp_subevent_stream_reconfigured_get_a2dp_cid(packet);

        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("A2DP Source: Stream reconfiguration failed, status 0x%02x\n", status);
            break;
        }

        DEBUGV("A2DP Source: Stream reconfigured a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
        status = a2dp_source_start_stream(media_tracker.a2dp_cid, media_tracker.local_seid);
        break;

    case A2DP_SUBEVENT_STREAM_STARTED:
        local_seid = a2dp_subevent_stream_started_get_local_seid(packet);
        cid = a2dp_subevent_stream_started_get_a2dp_cid(packet);

        play_info.status = AVRCP_PLAYBACK_STATUS_PLAYING;
        if (media_tracker.avrcp_cid) {
            avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, &_currentTrack, _tracks);
            avrcp_target_set_playback_status(media_tracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PLAYING);
        }
        a2dp_timer_start(&media_tracker);
        DEBUGV("A2DP Source: Stream started, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);
        _connected = true;
        if (_connectCB) {
            _connectCB(_connectData, true);
        }
        break;

    case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
        local_seid = a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(packet);
        cid = a2dp_subevent_signaling_media_codec_sbc_configuration_get_a2dp_cid(packet);
        a2dp_send_media_packet();
        break;

    case A2DP_SUBEVENT_STREAM_SUSPENDED:
        local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
        cid = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);

        play_info.status = AVRCP_PLAYBACK_STATUS_PAUSED;
        if (media_tracker.avrcp_cid) {
            avrcp_target_set_playback_status(media_tracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_PAUSED);
        }
        DEBUGV("A2DP Source: Stream paused, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);

        a2dp_timer_stop(&media_tracker);
        break;

    case A2DP_SUBEVENT_STREAM_RELEASED:
        play_info.status = AVRCP_PLAYBACK_STATUS_STOPPED;
        cid = a2dp_subevent_stream_released_get_a2dp_cid(packet);
        local_seid = a2dp_subevent_stream_released_get_local_seid(packet);

        DEBUGV("A2DP Source: Stream released, a2dp_cid 0x%02x, local_seid 0x%02x\n", cid, local_seid);

        if (cid == media_tracker.a2dp_cid) {
            media_tracker.stream_opened = 0;
            DEBUGV("A2DP Source: Stream released.\n");
        }
        if (media_tracker.avrcp_cid) {
            avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, NULL, _tracks);
            avrcp_target_set_playback_status(media_tracker.avrcp_cid, AVRCP_PLAYBACK_STATUS_STOPPED);
        }
        a2dp_timer_stop(&media_tracker);
        break;
    case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        cid = a2dp_subevent_signaling_connection_released_get_a2dp_cid(packet);
        if (cid == media_tracker.a2dp_cid) {
            media_tracker.avrcp_cid = 0;
            media_tracker.a2dp_cid = 0;
            DEBUGV("A2DP Source: Signaling released.\n\n");
            _connected = false;
            if (_connectCB) {
                _connectCB(_connectData, false);
            }
        }
        break;
    default:
        break;
    }
}

void A2DPSource::avrcp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    bd_addr_t event_addr;
    uint16_t local_cid;
    uint8_t  status = ERROR_CODE_SUCCESS;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) {
        return;
    }

    switch (packet[2]) {
    case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED:
        local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
        status = avrcp_subevent_connection_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            DEBUGV("AVRCP: Connection failed, local cid 0x%02x, status 0x%02x\n", local_cid, status);
            return;
        }
        media_tracker.avrcp_cid = local_cid;
        avrcp_subevent_connection_established_get_bd_addr(packet, event_addr);

        DEBUGV("AVRCP: Channel to %s successfully opened, avrcp_cid 0x%02x\n", bd_addr_to_str(event_addr), media_tracker.avrcp_cid);

        avrcp_target_support_event(media_tracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED);
        avrcp_target_support_event(media_tracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
        avrcp_target_support_event(media_tracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED);
        avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, NULL, _tracks);

        DEBUGV("Enable Volume Change notification\n");
        avrcp_controller_enable_notification(media_tracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
        DEBUGV("Enable Battery Status Change notification\n");
        avrcp_controller_enable_notification(media_tracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_BATT_STATUS_CHANGED);
        return;

    case AVRCP_SUBEVENT_CONNECTION_RELEASED:
        DEBUGV("AVRCP Target: Disconnected, avrcp_cid 0x%02x\n", avrcp_subevent_connection_released_get_avrcp_cid(packet));
        media_tracker.avrcp_cid = 0;
        return;
    default:
        break;
    }

    if (status != ERROR_CODE_SUCCESS) {
        DEBUGV("Responding to event 0x%02x failed, status 0x%02x\n", packet[2], status);
    }
}

void A2DPSource::avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    uint8_t  status = ERROR_CODE_SUCCESS;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) {
        return;
    }

    bool button_pressed;
    char const * button_state;
    avrcp_operation_id_t operation_id;

    switch (packet[2]) {
    case AVRCP_SUBEVENT_PLAY_STATUS_QUERY:
        status = avrcp_target_play_status(media_tracker.avrcp_cid, play_info.song_length_ms, play_info.song_position_ms, play_info.status);
        break;
    // case AVRCP_SUBEVENT_NOW_PLAYING_INFO_QUERY:
    //     status = avrcp_target_now_playing_info(avrcp_cid);
    //     break;
    case AVRCP_SUBEVENT_OPERATION:
        operation_id = (avrcp_operation_id_t)avrcp_subevent_operation_get_operation_id(packet);
        button_pressed = avrcp_subevent_operation_get_button_pressed(packet) > 0;
        button_state = button_pressed ? "PRESS" : "RELEASE";
        (void) button_state;
        DEBUGV("AVRCP Target: operation %s (%s)\n", avrcp_operation2str(operation_id), button_state);
        if (_avrcpCB) {
            _avrcpCB(_avrcpData, operation_id, button_pressed);
        }
        if (!button_pressed) {
            break;
        }
        switch (operation_id) {
        case AVRCP_OPERATION_ID_PLAY:
            status = a2dp_source_start_stream(media_tracker.a2dp_cid, media_tracker.local_seid);
            break;
        case AVRCP_OPERATION_ID_PAUSE:
            status = a2dp_source_pause_stream(media_tracker.a2dp_cid, media_tracker.local_seid);
            break;
        case AVRCP_OPERATION_ID_STOP:
            status = a2dp_source_disconnect(media_tracker.a2dp_cid);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (status != ERROR_CODE_SUCCESS) {
        DEBUGV("Responding to event 0x%02x failed, status 0x%02x\n", packet[2], status);
    }
}

void A2DPSource::avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) {
        return;
    }
    if (!media_tracker.avrcp_cid) {
        return;
    }

    switch (packet[2]) {
    case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
        DEBUGV("AVRCP Controller: Notification Absolute Volume %d %%\n", avrcp_subevent_notification_volume_changed_get_absolute_volume(packet) * 100 / 127);
        if (_volumeCB) {
            _volumeCB(_volumeData, avrcp_subevent_notification_volume_changed_get_absolute_volume(packet) * 100 / 127);
        }
        break;
    case AVRCP_SUBEVENT_NOTIFICATION_EVENT_BATT_STATUS_CHANGED:
        // see avrcp_battery_status_t
        DEBUGV("AVRCP Controller: Notification Battery Status 0x%02x\n", avrcp_subevent_notification_event_batt_status_changed_get_battery_status(packet));
        if (_batteryCB) {
            _batteryCB(_batteryData, (avrcp_battery_status_t)avrcp_subevent_notification_event_batt_status_changed_get_battery_status(packet));
        }
        break;
    case AVRCP_SUBEVENT_NOTIFICATION_STATE:
        DEBUGV("AVRCP Controller: Notification %s - %s\n",
               avrcp_event2str(avrcp_subevent_notification_state_get_event_id(packet)),
               avrcp_subevent_notification_state_get_enabled(packet) != 0 ? "enabled" : "disabled");
        break;
    default:
        break;
    }
}
