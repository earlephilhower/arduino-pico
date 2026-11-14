// Part of the WM8960USB example. Released to the public domain 2026 by K7MDL2
#ifndef WM8960_I2S_H
#define WM8960_I2S_H

#include <Arduino.h>

namespace wm8960_i2s {
bool begin(uint32_t sample_rate);
int available();
size_t read(uint8_t *buffer, size_t bytes);
int available_for_write();
size_t write(const uint8_t *buffer, size_t bytes);
uint32_t received_frames();
uint32_t transmitted_frames();
uint32_t underruns();
uint32_t max_service_gap_us();
uint32_t take_window_gap_us();
uint32_t queued_frames();
void pause_for_flash();
void resume_after_flash();
uint32_t flash_pause_count();
uint32_t flash_overrun_count();
uint32_t gap_underrun_count();
uint32_t lead_underrun_count();
uint32_t last_underrun_core_num();
void lead_underrun_detail(uint32_t *distance, bool *was_paused,
                          uint32_t *since_resume_us,
                          uint32_t *head_from_resume);
}

#endif
