/*
 * I2S Master library for the Raspberry Pi Pico RP2040
 *
 * Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#include <Arduino.h>
#include "pico/audio_i2s.h"

class I2SClass : public Stream
{
public:
  I2SClass();

  // Only 16 bitsPerSample are allowed by the PIO code.  Only write, no read.
  bool begin(long sampleRate, pin_size_t sck = 26, /* lrclk is sck+1 */ pin_size_t data = 28);
  void end();

  // from Stream
  virtual int available() override { return -1; }
  virtual int read() override { return -1; }
  virtual int peek() override { return -1; }
  virtual void flush() override;

  // from Print (see notes on write() methods below)
  virtual size_t write(uint8_t) override;
  virtual size_t write(const uint8_t *buffer, size_t size) override;
  virtual int availableForWrite() override;

  // Write a single L:R sample to the I2S device.  Blocking until write succeeds
  size_t write(int16_t);
  // Write up to size samples to the I2S device.  Non-blocking, will write
  // from 0...size samples and return that count.  Be sure your app handles
  // partial writes (i.e. by yield()ing and then retrying to write the
  // remaining data.
  size_t write(const void *buffer, size_t lrsamples);

  // Note that these callback are called from **INTERRUPT CONTEXT** and hence
  // must be both stored in IRAM and not perform anything that's not legal in
  // an interrupt
  //void onTransmit(void(*)(void)); -- Not yet implemented, need to edit pico-extra to get callback
  //void onReceive(void(*)(void)); -- no I2S input yet

private:
    int _bps;
    bool _running;

    // Audio format/pool config
    audio_format_t _audio_format;
    audio_buffer_format_t _producer_format;
    audio_i2s_config_t _config;

    // We manage our own buffer pool here...
    audio_buffer_pool_t *_pool;
    audio_buffer_t *_curBuff;

    // Support for ::write(x) on 16b quantities
    uint32_t _writtenData;
    bool _writtenHalf;
};

extern I2SClass I2S;
