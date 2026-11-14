// Part of the CompositeUSB example. Released to the public domain 2026 by K7MDL2
#include <Arduino.h>
#include "composite_config.h"
#include <CoreMutex.h>
#include <USB.h>
#include <tusb.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {
constexpr uint32_t sample_rate = 48000;
struct Function {
    uint8_t string = 0, interface = 255, in = 0, out = 0, auxiliary = 0;
} speaker, microphone, cdc, midi;
bool ready = false;
volatile uint32_t playback_bytes = 0, capture_bytes = 0, audio_errors = 0;
volatile uint32_t alternate_changes = 0;
uint32_t cdc_bytes = 0, midi_packets = 0;
bool muted[2][2] = {};
int16_t volume[2][2] = {};
int32_t gain[2] = {32767, 32767};
uint16_t phase = 0;
volatile bool tone_enabled = false;
bool tone_timed = false;
uint32_t tone_started = 0;
constexpr char sample_text[] = "Composite USB sample text: hello from Pico!\r\n";
size_t text_offset = sizeof(sample_text) - 1;
enum class MidiTest { Idle, NoteOn, NoteOff, CC };
MidiTest midi_test = MidiTest::Idle;
uint32_t text_started = 0, midi_started = 0, note_started = 0;
bool note_queued = false;
char response[1600] = {};
size_t response_offset = 0;

void reply(const char *text) {
    const size_t used = std::strlen(response);
    if (used + std::strlen(text) >= sizeof(response)) {
        panic("CLI response overflow");
    }
    std::strcpy(response + used, text);
}

void status() {
    char report[180];
    std::snprintf(report, sizeof(report),
                  "USB OUT=%lu IN=%lu CDC2=%lu MIDI=%lu ALT=%lu ERR=%lu TONE=%s\r\n",
                  static_cast<unsigned long>(playback_bytes), static_cast<unsigned long>(capture_bytes),
                  static_cast<unsigned long>(cdc_bytes), static_cast<unsigned long>(midi_packets),
                  static_cast<unsigned long>(alternate_changes), static_cast<unsigned long>(audio_errors),
                  tone_enabled ? "ON" : "OFF");
    reply(report);
}

void command(const char *line) {
    if (!std::strcmp(line, "\\?")) {
        reply("Commands (CDC0, 115200 baud; Enter to execute):\r\n"
              "\\text   Send sample text to CDC2. Open its terminal to see it.\r\n"
              "\\note   Send ch1 Note On 60 (middle C), velocity 100; Off after 500 ms.\r\n"
              "\\cc     Send ch1 CC21 value 64. View it in a PC MIDI monitor.\r\n"
              "\\tone   Send 1 kHz tone for 3 seconds through USB capture.\r\n"
              "        Enable PC input monitoring to hear it through PC speakers.\r\n"
              "\\stop   Stop the tone (capture continues with silence).\r\n"
              "\\status Show USB counters and tone state.\r\n"
              "\\?      Show this help.\r\n"
              "Open MIDI input before tests; disable MIDI thru/echo. USB queueing\r\n"
              "does not prove host receipt. Tone respects PC mute/volume controls.\r\n");
    } else if (!std::strcmp(line, "\\text")) {
        if (text_offset < sizeof(sample_text) - 1) {
            reply("Error: text test busy.\r\n");
        } else {
            text_offset = 0;
            text_started = millis();
            reply("Text test pending; open CDC2 within 5 seconds.\r\n");
        }
    } else if (!std::strcmp(line, "\\note") || !std::strcmp(line, "\\cc")) {
        if (midi_test != MidiTest::Idle) {
            reply("Error: MIDI test busy.\r\n");
        } else {
            midi_test = !std::strcmp(line, "\\note") ? MidiTest::NoteOn : MidiTest::CC;
            midi_started = millis();
            reply("MIDI test pending; open the PC MIDI input port.\r\n");
        }
    } else if (!std::strcmp(line, "\\tone")) {
        tone_started = millis();
        tone_timed = tone_enabled = true;
        reply("Tone ON for 3 seconds; monitor Composite test 1 kHz capture on PC.\r\n");
    } else if (!std::strcmp(line, "\\stop")) {
        tone_timed = tone_enabled = false;
        reply("Tone OFF; capture now sends silence.\r\n");
    } else if (!std::strcmp(line, "\\status")) {
        status();
    } else {
        reply("Error: unknown command. Type \\? and Enter for help.\r\n");
    }
}

void service_cli() {
    if (tone_timed && millis() - tone_started >= 3000) {
        tone_timed = tone_enabled = false;
        reply("Tone complete.\r\n");
    }
    if (response[response_offset]) {
        if (Serial) {
            const int space = Serial.availableForWrite();
            if (space > 0) {
                const size_t count = std::min(std::strlen(response + response_offset),
                                              static_cast<size_t>(std::min(space, 64)));
                response_offset += Serial.write(
                                       reinterpret_cast<const uint8_t *>(response + response_offset), count);
            }
        }
        if (!response[response_offset]) {
            response_offset = response[0] = 0;
        }
        return;
    }
    static char line[32] = {};
    static size_t length = 0;
    static bool overflow = false;
    for (unsigned i = 0; i < 32 && Serial.available(); ++i) {
        const int c = Serial.read();
        if (c == '\r' || c == '\n') {
            if (overflow) {
                reply("Error: command too long. Type \\? for help.\r\n");
            } else if (length) {
                line[length] = 0;
                command(line);
            }
            length = 0;
            overflow = false;
            if (response[0]) {
                break;
            }
        } else if (c == 8 || c == 127) {
            if (length) {
                --length;
            }
        } else if (c >= 32 && c <= 126 && !overflow) {
            if (length + 1 < sizeof(line)) {
                line[length++] = static_cast<char>(c);
            } else {
                overflow = true;
            }
        }
    }
}

void update_gain(unsigned function) {
    gain[function] = muted[function][0] || muted[function][1] ? 0 :
                     static_cast<int32_t>(32767 * std::pow(10.0,
                         (volume[function][0] + volume[function][1]) / (256.0 * 20)));
}

void descriptor(int number, uint8_t *destination, int length, void *parameter) {
    auto &f = *static_cast<Function *>(parameter);
    f.interface = static_cast<uint8_t>(number);
    if (&f == &speaker) {
        const uint8_t bytes[] = {TUD_AUDIO_SPEAKER_MONO_FB_DESCRIPTOR(
                                     f.interface, f.string, 2, 16, f.out, 98, f.auxiliary, 4)
                                };
        if (length != sizeof(bytes)) {
            panic("Speaker descriptor length mismatch");
        }
        std::memcpy(destination, bytes, sizeof(bytes));
    } else if (&f == &microphone) {
        const uint8_t bytes[] = {TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR(
                                     f.interface, f.string, 2, 16, f.in, 98)
                                };
        if (length != sizeof(bytes)) {
            panic("Mic descriptor length mismatch");
        }
        std::memcpy(destination, bytes, sizeof(bytes));
    } else if (&f == &cdc) {
        const uint8_t bytes[] = {TUD_CDC_DESCRIPTOR(
                                     f.interface, f.string, f.auxiliary, 8, f.out, f.in, 64)
                                };
        if (length != sizeof(bytes)) {
            panic("CDC descriptor length mismatch");
        }
        std::memcpy(destination, bytes, sizeof(bytes));
    } else {
        const uint8_t bytes[] = {TUD_MIDI_DESCRIPTOR(f.interface, f.string, f.out, f.in, 64)};
        if (length != sizeof(bytes)) {
            panic("MIDI descriptor length mismatch");
        }
        std::memcpy(destination, bytes, sizeof(bytes));
    }
}

int audio_function(tusb_control_request_t const *request) {
    const uint8_t interface = TU_U16_LOW(request->wIndex);
    return interface == speaker.interface ? 0 : interface == microphone.interface ? 1 : -1;
}
}

void composite_begin() {
    USB.disconnect();
    speaker.out = USB.registerEndpointOut();
    speaker.auxiliary = USB.registerEndpointIn();
    microphone.in = USB.registerEndpointIn();
    cdc.auxiliary = USB.registerEndpointIn();
    cdc.in = USB.registerEndpointIn();
    cdc.out = USB.registerEndpointOut();
    midi.in = USB.registerEndpointIn();
    midi.out = USB.registerEndpointOut();
    if (!speaker.out || !speaker.auxiliary || !microphone.in || !cdc.auxiliary ||
            !cdc.in || !cdc.out || !midi.in || !midi.out) {
        USB.connect();
        panic("Composite USB endpoint allocation failed");
    }
    speaker.string = USB.registerString("Composite test playback");
    microphone.string = USB.registerString("Composite test 1 kHz capture");
    cdc.string = USB.registerString("Composite test CDC2");
    midi.string = USB.registerString("Composite test MIDI");
    USB.registerInterface(2, descriptor, &speaker, CFG_TUD_AUDIO_FUNC_1_DESC_LEN, 10, 0);
    USB.registerInterface(2, descriptor, &microphone, CFG_TUD_AUDIO_FUNC_2_DESC_LEN, 11, 0);
    USB.registerInterface(2, descriptor, &cdc, TUD_CDC_DESC_LEN, 20, 0);
    USB.registerInterface(2, descriptor, &midi, TUD_MIDI_DESC_LEN, 30, 0);
    ready = true;
    USB.connect();
    reply("Composite USB ready. Type \\? and Enter for commands.\r\n");
}

void composite_service() {
    if (!ready) {
        return;
    }
    service_cli();
    {
        CoreMutex lock(&USB.mutex, false);
        static uint8_t packet[4];
        static bool pending = false;
        if (lock && !tud_mounted()) {
            pending = false;
            if (midi_test != MidiTest::Idle || text_offset < sizeof(sample_text) - 1) {
                midi_test = MidiTest::Idle;
                note_queued = false;
                text_offset = sizeof(sample_text) - 1;
                reply("Error: USB disconnected; pending tests cancelled.\r\n");
            }
        }
        if (lock && tud_mounted()) {
            if (text_offset < sizeof(sample_text) - 1) {
                if (tud_cdc_n_connected(1)) {
                    const size_t count = std::min(sizeof(sample_text) - 1 - text_offset,
                                                  static_cast<size_t>(tud_cdc_n_write_available(1)));
                    const size_t written = tud_cdc_n_write(1, sample_text + text_offset, count);
                    text_offset += written;
                    cdc_bytes += written;
                    if (text_offset == sizeof(sample_text) - 1) {
                        reply("Sample text queued on CDC2.\r\n");
                    }
                }
                if (text_offset < sizeof(sample_text) - 1 && millis() - text_started >= 5000) {
                    text_offset = sizeof(sample_text) - 1;
                    reply("Error: CDC2 text test timed out; open CDC2 and retry.\r\n");
                }
            }
            if (tud_cdc_n_connected(1) && tud_cdc_n_available(1) &&
                    tud_cdc_n_write_available(1)) {
                const int input = tud_cdc_n_read_char(1);
                if (input >= 0) {
                    const uint8_t byte = static_cast<uint8_t>(input);
                    if (tud_cdc_n_write(1, &byte, 1) == 1) {
                        ++cdc_bytes;
                    } else {
                        ++audio_errors;
                    }
                }
            }
            tud_cdc_n_write_flush(1);
            if (!pending && tud_midi_available()) {
                pending = tud_midi_packet_read(packet);
            }
            if (pending && tud_midi_packet_write(packet)) {
                ++midi_packets;
                pending = false;
            }
            if (midi_test != MidiTest::Idle) {
                if (midi_test == MidiTest::NoteOn || midi_test == MidiTest::CC) {
                    const uint8_t test[4] = {
                        static_cast<uint8_t>(midi_test == MidiTest::NoteOn ? 0x09 : 0x0B),
                        static_cast<uint8_t>(midi_test == MidiTest::NoteOn ? 0x90 : 0xB0),
                        static_cast<uint8_t>(midi_test == MidiTest::NoteOn ? 60 : 21),
                        static_cast<uint8_t>(midi_test == MidiTest::NoteOn ? 100 : 64)
                    };
                    if (tud_midi_packet_write(test)) {
                        ++midi_packets;
                        if (midi_test == MidiTest::NoteOn) {
                            midi_test = MidiTest::NoteOff;
                            note_queued = true;
                            note_started = millis();
                            reply("Note On queued: ch1, note60, velocity100.\r\n");
                        } else {
                            midi_test = MidiTest::Idle;
                            reply("CC queued: ch1, CC21, value64.\r\n");
                        }
                    } else if (millis() - midi_started >= 5000) {
                        midi_test = MidiTest::Idle;
                        reply("Error: MIDI test timed out before queueing.\r\n");
                    }
                } else if (millis() - note_started >= 500) {
                    const uint8_t off[4] = {0x08, 0x80, 60, 0};
                    if (tud_midi_packet_write(off)) {
                        ++midi_packets;
                        midi_test = MidiTest::Idle;
                        note_queued = false;
                        reply("Note Off queued: ch1, note60.\r\n");
                    } else if (note_queued && millis() - note_started >= 5500) {
                        note_queued = false;
                        reply("Error: Note Off blocked; retrying until accepted or USB disconnects.\r\n");
                    }
                }
            }
        }
    }
    yield();
}

extern "C" {
    bool tud_audio_set_itf_cb(uint8_t, tusb_control_request_t const *) {
        ++alternate_changes;
        return true;
    }

    bool tud_audio_set_req_entity_cb(uint8_t, tusb_control_request_t const *request,
                                     uint8_t *buffer) {
        const int function = audio_function(request);
        if (function < 0) {
            return false;
        }
        const unsigned entity = TU_U16_HIGH(request->wIndex);
        const unsigned channel = TU_U16_LOW(request->wValue);
        const unsigned selector = TU_U16_HIGH(request->wValue);
        if (entity == 2 && channel < 2 && request->bRequest == AUDIO_CS_REQ_CUR) {
            if (selector == AUDIO_FU_CTRL_MUTE && request->wLength == 1) {
                muted[function][channel] = buffer[0] != 0;
            } else if (selector == AUDIO_FU_CTRL_VOLUME && request->wLength == 2) {
                int16_t requested;
                std::memcpy(&requested, buffer, sizeof(requested));
                if (requested < -50 * 256 || requested > 0) {
                    return false;
                }
                volume[function][channel] = requested;
            } else {
                return false;
            }
            update_gain(function);
            return true;
        }
        // The microphone macro advertises a programmable clock; only 48 kHz is supported.
        if (entity == 4 && selector == AUDIO_CS_CTRL_SAM_FREQ &&
                request->bRequest == AUDIO_CS_REQ_CUR && request->wLength == 4) {
            uint32_t requested;
            std::memcpy(&requested, buffer, sizeof(requested));
            return requested == sample_rate;
        }
        return false;
    }

    bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const *request) {
        const int function = audio_function(request);
        if (function < 0) {
            return false;
        }
        const unsigned entity = TU_U16_HIGH(request->wIndex);
        const unsigned channel = TU_U16_LOW(request->wValue);
        const unsigned selector = TU_U16_HIGH(request->wValue);
        if (entity == 1 && function == 1 && selector == AUDIO_TE_CTRL_CONNECTOR &&
                request->bRequest == AUDIO_CS_REQ_CUR) {
            audio_desc_channel_cluster_t result = {};
            result.bNrChannels = 1;
            return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
        }
        if (entity == 2 && channel < 2) {
            if (selector == AUDIO_FU_CTRL_MUTE && request->bRequest == AUDIO_CS_REQ_CUR) {
                audio_control_cur_1_t result = {};
                result.bCur = muted[function][channel];
                return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
            }
            if (selector == AUDIO_FU_CTRL_VOLUME && request->bRequest == AUDIO_CS_REQ_CUR) {
                audio_control_cur_2_t result = {};
                result.bCur = volume[function][channel];
                return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
            }
            if (selector == AUDIO_FU_CTRL_VOLUME && request->bRequest == AUDIO_CS_REQ_RANGE) {
                audio_control_range_2_n_t(1) result = {};
                result.wNumSubRanges = 1;
                result.subrange[0].bMin = -50 * 256;
                result.subrange[0].bMax = 0;
                result.subrange[0].bRes = 256;
                return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
            }
        }
        if (entity == 4 && selector == AUDIO_CS_CTRL_SAM_FREQ) {
            if (request->bRequest == AUDIO_CS_REQ_CUR) {
                audio_control_cur_4_t result = {};
                result.bCur = sample_rate;
                return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
            }
            if (request->bRequest == AUDIO_CS_REQ_RANGE) {
                audio_control_range_4_n_t(1) result = {};
                result.wNumSubRanges = 1;
                result.subrange[0].bMin = result.subrange[0].bMax = sample_rate;
                return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
            }
        }
        if (entity == 4 && selector == AUDIO_CS_CTRL_CLK_VALID && request->bRequest == AUDIO_CS_REQ_CUR) {
            audio_control_cur_1_t result = {};
            result.bCur = 1;
            return tud_audio_buffer_and_schedule_control_xfer(rhport, request, &result, sizeof(result));
        }
        return false;
    }

    bool tud_audio_rx_done_post_read_cb(uint8_t, uint16_t, uint8_t function,
                                        uint8_t, uint8_t) {
        if (function != 0) {
            return true;
        }
        int16_t samples[49];
        const uint16_t read = tud_audio_n_read(0, samples, sizeof(samples));
        playback_bytes += read;
        // A null sink follows USB deadlines, so nominal feedback is sufficient here.
        if (!tud_audio_n_fb_set(0, 48UL << 16)) {
            ++audio_errors;
        }
        return true;
    }

    bool tud_audio_tx_done_pre_load_cb(uint8_t, uint8_t function, uint8_t, uint8_t) {
        if (function != 1) {
            return true;
        }
        auto *fifo = tud_audio_n_get_ep_in_ff(1);
        const unsigned queued = fifo ? tu_fifo_count(fifo) / 2 : 0;
        const unsigned target = CFG_TUD_AUDIO_FUNC_2_EP_IN_SW_BUF_SZ / 4;
        const unsigned frames = queued < target ? target - queued : 0;
        int16_t samples[target];
        for (unsigned i = 0; i < frames; ++i) {
            samples[i] = tone_enabled ?
                         static_cast<int16_t>((phase < 24 ? 4096 : -4096) * gain[1] / 32767) : 0;
            phase = (phase + 1) % 48;
        }
        const uint16_t bytes = static_cast<uint16_t>(frames * 2);
        const uint16_t written = tud_audio_n_write(1, samples, bytes);
        capture_bytes += written;
        if (written != bytes) {
            ++audio_errors;
        }
        return true;
    }

    void tud_audio_feedback_params_cb(uint8_t, uint8_t, audio_feedback_params_t *parameters) {
        parameters->method = AUDIO_FEEDBACK_METHOD_DISABLED;
        parameters->sample_freq = sample_rate;
    }
}
