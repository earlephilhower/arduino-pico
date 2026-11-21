// Part of the WM8960USB example. Released to the public domain 2026 by K7MDL2
#include "composite_config.h"
#if WM8960USB_SUPPORTED

#if defined(ARDUINO_ARCH_RP2040)
#include "codec.h"
#include "wm8960_i2s.h"
#define wm8960_audio_debug_port() Serial
#include "wm8960_i2s_tx.pio.h"
#include "wm8960_i2s_rx.pio.h"
#include <PIOProgram.h>
#include <hardware/clocks.h>
#include <hardware/dma.h>
#include <hardware/sync.h>
#include <algorithm>
#include <cstring>


namespace wm8960_i2s {
namespace {
// 4096 frames (~85 ms) outlasts a ~47 ms flash erase/program stall.
constexpr uint32_t kFramesLog2 = 12;
constexpr uint32_t kFrames = 1u << kFramesLog2;
constexpr uint32_t kMask = kFrames - 1;
constexpr uint32_t kLead = 256;
// DMA may prefetch eight frames into the joined TX FIFO.
constexpr uint32_t kPrefetch = 16;
alignas(kFrames * sizeof(uint32_t)) uint32_t tx_ring[kFrames];
alignas(kFrames * sizeof(uint16_t)) uint16_t rx_ring[kFrames];
PIOProgram tx_program(&wm8960_i2s_tx_program);
PIOProgram rx_program(&wm8960_i2s_rx_program);
PIO tx_pio;
PIO rx_pio;
int tx_sm = -1;
int rx_sm = -1;
int tx_dma = -1;
int rx_dma = -1;
bool running = false;
uint32_t tx_head = 0;
uint32_t tx_previous = 0;
uint32_t rx_previous = 0;
uint32_t rx_tail = 0;
uint32_t tx_total = 0;
uint32_t rx_total = 0;
uint32_t last_tx_poll = 0;
uint32_t last_rx_poll = 0;
uint32_t ring_time_us = 0;
uint32_t tx_underruns = 0;
uint32_t max_gap_us = 0;
uint32_t window_gap_us = 0;
uint32_t flash_pauses = 0;
uint32_t flash_overruns = 0;
uint32_t gap_underruns = 0;
uint32_t lead_underruns = 0;
uint32_t last_underrun_core = 0;
volatile bool paused = false;
uint32_t last_lead_distance = 0;
bool last_lead_paused = false;
uint32_t last_lead_since_resume_us = 0;
uint32_t resume_us = 0;
uint32_t resume_position = 0;
uint32_t last_lead_head_from_resume = 0;

uint32_t tx_position() {
    return ((dma_channel_hw_addr(tx_dma)->read_addr -
             reinterpret_cast<uintptr_t>(tx_ring)) / sizeof(uint32_t)) & kMask;
}

uint32_t rx_position() {
    return ((dma_channel_hw_addr(rx_dma)->write_addr -
             reinterpret_cast<uintptr_t>(rx_ring)) / sizeof(uint16_t)) & kMask;
}

uint32_t poll_tx(bool expected_stall = false) {
    const uint32_t now = micros();
    const uint32_t position = tx_position();
    if (!expected_stall) {
        max_gap_us = std::max(max_gap_us, now - last_tx_poll);
        window_gap_us = std::max(window_gap_us, now - last_tx_poll);
    }
    tx_total += (position - tx_previous) & kMask;
    if (now - last_tx_poll >= ring_time_us) {
        if (expected_stall) {
            ++flash_overruns;
        } else {
            ++tx_underruns;
            ++gap_underruns;
            last_underrun_core = get_core_num();
        }
        std::memset(tx_ring, 0, sizeof(tx_ring));
        tx_head = (position + kPrefetch) & kMask;
    } else {
        // Erase consumed frames so a delayed renderer cannot replay old keying.
        for (uint32_t frame = tx_previous; frame != position;
                frame = (frame + 1) & kMask) {
            tx_ring[frame] = 0;
        }
        if (((tx_head - position) & kMask) > kLead) {
            if (!expected_stall) {
                ++tx_underruns;
                ++lead_underruns;
                last_underrun_core = get_core_num();
                last_lead_distance = (tx_head - position) & kMask;
                last_lead_paused = paused;
                last_lead_since_resume_us = now - resume_us;
                last_lead_head_from_resume = (tx_head - resume_position) & kMask;
            }
            tx_head = (position + kPrefetch) & kMask;
        }
    }
    tx_previous = position;
    last_tx_poll = now;
    return position;
}

uint32_t poll_rx() {
    const uint32_t now = micros();
    const uint32_t position = rx_position();
    rx_total += (position - rx_previous) & kMask;
    if (now - last_rx_poll >= ring_time_us) {
        rx_tail = position;
    }
    rx_previous = position;
    last_rx_poll = now;
    __dmb();
    return position;
}
}

bool begin(uint32_t sample_rate) {
    if (running || sample_rate == 0) {
        wm8960_audio_debug_port().println("[WM8960] invalid PIO audio startup state/rate");
        return false;
    }
    // Target the SDR's 64 cycles/frame without changing the existing system clock.
    const float divider = static_cast<float>(clock_get_hz(clk_sys)) /
                          (sample_rate * 64.0f);
    if (divider < 1.0f || divider >= 65536.0f) {
        wm8960_audio_debug_port().println("[WM8960] PIO clock divider out of range");
        return false;
    }
    tx_dma = dma_claim_unused_channel(false);
    rx_dma = dma_claim_unused_channel(false);
    if (tx_dma < 0 || rx_dma < 0) {
        if (tx_dma >= 0) {
            dma_channel_unclaim(tx_dma);
        }
        if (rx_dma >= 0) {
            dma_channel_unclaim(rx_dma);
        }
        wm8960_audio_debug_port().println("[WM8960] no free audio DMA channels");
        return false;
    }
    const int lowest = std::min({WM8960_I2S_BCLK_PIN, WM8960_I2S_DOUT_PIN,
                                 WM8960_I2S_DIN_PIN});
    const int highest = std::max({WM8960_I2S_LRCLK_PIN, WM8960_I2S_DOUT_PIN,
                                  WM8960_I2S_DIN_PIN});
    int tx_offset = 0;
    int rx_offset = 0;
    if (!tx_program.prepare(&tx_pio, &tx_sm, &tx_offset, lowest,
                            highest - lowest + 1) ||
            !rx_program.prepare(&rx_pio, &rx_sm, &rx_offset, lowest,
                                highest - lowest + 1)) {
        if (tx_sm >= 0) {
            pio_sm_unclaim(tx_pio, tx_sm);
        }
        dma_channel_unclaim(tx_dma);
        dma_channel_unclaim(rx_dma);
        wm8960_audio_debug_port().println("[WM8960] no free audio PIO state machines/program space");
        return false;
    }
    for (uint pin : {WM8960_I2S_BCLK_PIN, WM8960_I2S_LRCLK_PIN,
                     WM8960_I2S_DOUT_PIN
                    }) {
        pio_gpio_init(tx_pio, pin);
        // Soft edges: fast 3 MHz BCLK edges couple into the shared I2C bus.
        gpio_set_slew_rate(pin, GPIO_SLEW_RATE_SLOW);
        gpio_set_drive_strength(pin, GPIO_DRIVE_STRENGTH_2MA);
    }
    pio_gpio_init(rx_pio, WM8960_I2S_DIN_PIN);
    pio_sm_config tx = wm8960_i2s_tx_program_get_default_config(tx_offset);
    sm_config_set_out_pins(&tx, WM8960_I2S_DOUT_PIN, 1);
    sm_config_set_sideset_pins(&tx, WM8960_I2S_BCLK_PIN);
    sm_config_set_out_shift(&tx, false, true, 32);
    sm_config_set_fifo_join(&tx, PIO_FIFO_JOIN_TX);
    sm_config_set_clkdiv(&tx, divider);
    pio_sm_init(tx_pio, tx_sm, tx_offset, &tx);
    pio_sm_set_consecutive_pindirs(tx_pio, tx_sm, WM8960_I2S_BCLK_PIN, 2, true);
    pio_sm_set_consecutive_pindirs(tx_pio, tx_sm, WM8960_I2S_DOUT_PIN, 1, true);
    const uint32_t tx_mask =
        (3u << (WM8960_I2S_BCLK_PIN - pio_get_gpio_base(tx_pio))) |
        (1u << (WM8960_I2S_DOUT_PIN - pio_get_gpio_base(tx_pio)));
    pio_sm_set_pins_with_mask(tx_pio, tx_sm, 0, tx_mask);

    const uint bclk = WM8960_I2S_BCLK_PIN - pio_get_gpio_base(rx_pio);
    const uint lrclk = WM8960_I2S_LRCLK_PIN - pio_get_gpio_base(rx_pio);
    for (uint instruction : {0u, 1u, 2u, 3u, 5u, 7u}) {
        const uint pin = instruction < 2 ? lrclk : bclk;
        const bool high = instruction == 0 || instruction == 2 || instruction == 5;
        rx_pio->instr_mem[rx_offset + instruction] = pio_encode_wait_gpio(high, pin);
    }
    pio_sm_config rx = wm8960_i2s_rx_program_get_default_config(rx_offset);
    sm_config_set_in_pins(&rx, WM8960_I2S_DIN_PIN);
    sm_config_set_in_shift(&rx, false, true, 16);
    sm_config_set_fifo_join(&rx, PIO_FIFO_JOIN_RX);
    pio_sm_init(rx_pio, rx_sm, rx_offset, &rx);
    pio_sm_set_consecutive_pindirs(rx_pio, rx_sm, WM8960_I2S_DIN_PIN, 1, false);

    std::memset(tx_ring, 0, sizeof(tx_ring));
    std::memset(rx_ring, 0, sizeof(rx_ring));
    dma_channel_config tx_config = dma_channel_get_default_config(tx_dma);
    channel_config_set_transfer_data_size(&tx_config, DMA_SIZE_32);
    channel_config_set_high_priority(&tx_config, true);
    channel_config_set_read_increment(&tx_config, true);
    channel_config_set_write_increment(&tx_config, false);
    channel_config_set_dreq(&tx_config, pio_get_dreq(tx_pio, tx_sm, true));
    channel_config_set_ring(&tx_config, false, kFramesLog2 + 2);
    dma_channel_configure(tx_dma, &tx_config, &tx_pio->txf[tx_sm], tx_ring,
                          dma_encode_endless_transfer_count(), false);
    dma_channel_config rx_config = dma_channel_get_default_config(rx_dma);
    channel_config_set_transfer_data_size(&rx_config, DMA_SIZE_16);
    channel_config_set_high_priority(&rx_config, true);
    channel_config_set_read_increment(&rx_config, false);
    channel_config_set_write_increment(&rx_config, true);
    channel_config_set_dreq(&rx_config, pio_get_dreq(rx_pio, rx_sm, false));
    channel_config_set_ring(&rx_config, true, kFramesLog2 + 1);
    dma_channel_configure(rx_dma, &rx_config, rx_ring, &rx_pio->rxf[rx_sm],
                          dma_encode_endless_transfer_count(), false);
    dma_start_channel_mask(1u << tx_dma);
    // DMA may prefetch eight frames into the joined TX FIFO.
    tx_previous = tx_position();
    tx_head = (tx_previous + kLead) & kMask;
    rx_tail = rx_previous = 0;
    ring_time_us = static_cast<uint32_t>(1000000ULL * kFrames / sample_rate);
    last_tx_poll = last_rx_poll = micros();
    dma_start_channel_mask(1u << rx_dma);
    pio_sm_set_enabled(rx_pio, rx_sm, true);
    pio_sm_set_enabled(tx_pio, tx_sm, true);
    running = true;
#if defined(DEBUG_WM8960_AUDIO)
    wm8960_audio_debug_port().printf("[WM8960] ring DMA TX PIO%u/SM%d/DMA%d RX PIO%u/SM%d/DMA%d, divider %.6f\n",
                                     pio_get_index(tx_pio), tx_sm, tx_dma,
                                     pio_get_index(rx_pio), rx_sm, rx_dma, divider);
#endif
    return true;
}

int available_for_write() {
    if (!running) {
        return 0;
    }
    const uint32_t position = poll_tx(paused);
    if (paused) {
        return 0;
    }
    return (kLead - ((tx_head - position) & kMask)) * 4;
}

void pause_for_flash() {
    if (!running) {
        return;
    }
    const uint32_t interrupt_state = save_and_disable_interrupts();
    paused = true;
    ++flash_pauses;
    // Fade the queued lead to silence so the stall is a dropout, not a click.
    const uint32_t start = (poll_tx() + kPrefetch) & kMask;
    const uint32_t queued = (tx_head - start) & kMask;
    if (queued <= kLead) {
        for (uint32_t index = 0; index < queued; ++index) {
            uint32_t &frame = tx_ring[(start + index) & kMask];
            const int32_t scale = static_cast<int32_t>(queued - 1 - index);
            const int32_t left = static_cast<int16_t>(frame >> 16) * scale /
                                 static_cast<int32_t>(queued);
            const int32_t right = static_cast<int16_t>(frame & 0xffff) * scale /
                                  static_cast<int32_t>(queued);
            frame = (static_cast<uint32_t>(static_cast<uint16_t>(left)) << 16) |
                    static_cast<uint16_t>(right);
        }
    }
    __dmb();
    restore_interrupts(interrupt_state);
}

void resume_after_flash() {
    if (!running) {
        return;
    }
    const uint32_t interrupt_state = save_and_disable_interrupts();
    poll_tx(true);
    // Restart with a silent cushion; the first render may be ~1 ms away.
    const uint32_t position = tx_position();
    for (uint32_t index = 0; index < kLead / 2; ++index) {
        tx_ring[(position + index) & kMask] = 0;
    }
    tx_head = (position + kLead / 2) & kMask;
    resume_position = position;
    __dmb();
    paused = false;
    resume_us = micros();
    restore_interrupts(interrupt_state);
}

void lead_underrun_detail(uint32_t *distance, bool *was_paused,
                          uint32_t *since_resume_us,
                          uint32_t *head_from_resume) {
    *distance = last_lead_distance;
    *was_paused = last_lead_paused;
    *since_resume_us = last_lead_since_resume_us;
    *head_from_resume = last_lead_head_from_resume;
}

size_t write(const uint8_t *buffer, size_t bytes) {
    if (!running) {
        return 0;
    }
    const size_t frames = std::min(bytes / 4,
                                   static_cast<size_t>(available_for_write() / 4));
    for (size_t frame = 0; frame < frames; ++frame) {
        int16_t samples[2];
        std::memcpy(samples, buffer + frame * 4, sizeof(samples));
        tx_ring[tx_head] = (static_cast<uint32_t>(
                                static_cast<uint16_t>(samples[0])) << 16) |
                           static_cast<uint16_t>(samples[1]);
        tx_head = (tx_head + 1) & kMask;
    }
    __dmb();
    return frames * 4;
}

int available() {
    if (!running) {
        return 0;
    }
    return ((poll_rx() - rx_tail) & kMask) * 4;
}

size_t read(uint8_t *buffer, size_t bytes) {
    if (!running) {
        return 0;
    }
    const size_t frames = std::min(bytes / 4,
                                   static_cast<size_t>(available() / 4));
    for (size_t frame = 0; frame < frames; ++frame) {
        const uint16_t sample = rx_ring[rx_tail];
        const uint16_t stereo[] = {sample, sample};
        std::memcpy(buffer + frame * 4, stereo, sizeof(stereo));
        rx_tail = (rx_tail + 1) & kMask;
    }
    return frames * 4;
}

uint32_t received_frames() {
    return rx_total;
}
uint32_t transmitted_frames() {
    return tx_total;
}
uint32_t underruns() {
    return tx_underruns;
}
uint32_t flash_pause_count() {
    return flash_pauses;
}
uint32_t flash_overrun_count() {
    return flash_overruns;
}
uint32_t gap_underrun_count() {
    return gap_underruns;
}
uint32_t lead_underrun_count() {
    return lead_underruns;
}
uint32_t last_underrun_core_num() {
    return last_underrun_core;
}
uint32_t max_service_gap_us() {
    return max_gap_us;
}
uint32_t take_window_gap_us() {
    const uint32_t gap = window_gap_us;
    window_gap_us = 0;
    return gap;
}
uint32_t queued_frames() {
    if (!running) {
        return 0;
    }
    const uint32_t queued = (tx_head - tx_position()) & kMask;
    return queued <= kLead ? queued : 0;
}
}
#endif

#endif // WM8960USB_SUPPORTED
