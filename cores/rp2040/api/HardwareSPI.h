/*
  Copyright (c) 2018 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include "Common.h"
#include <inttypes.h>
#include "Stream.h"

#define SPI_HAS_TRANSACTION

namespace arduino {

typedef enum {
  SPI_MODE0 = 0,
  SPI_MODE1 = 1,
  SPI_MODE2 = 2,
  SPI_MODE3 = 3,
} SPIMode;


class SPISettings {
  public:
  SPISettings(uint32_t clock, BitOrder bitOrder, SPIMode dataMode) {
    if (__builtin_constant_p(clock)) {
      init_AlwaysInline(clock, bitOrder, dataMode);
    } else {
      init_MightInline(clock, bitOrder, dataMode);
    }
  }

  SPISettings(uint32_t clock, BitOrder bitOrder, int dataMode) {
    if (__builtin_constant_p(clock)) {
      init_AlwaysInline(clock, bitOrder, (SPIMode)dataMode);
    } else {
      init_MightInline(clock, bitOrder, (SPIMode)dataMode);
    }
  }

  // Default speed set to 4MHz, SPI mode set to MODE 0 and Bit order set to MSB first.
  SPISettings() { init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0); }

  bool operator==(const SPISettings& rhs) const
  {
    if ((this->clockFreq == rhs.clockFreq) &&
        (this->bitOrder == rhs.bitOrder) &&
        (this->dataMode == rhs.dataMode)) {
      return true;
    }
    return false;
  }

  bool operator!=(const SPISettings& rhs) const
  {
    return !(*this == rhs);
  }

  uint32_t getClockFreq() const {
    return clockFreq;
  }
  SPIMode getDataMode() const {
    return dataMode;
  }
  BitOrder getBitOrder() const {
    return (bitOrder);
  }

  private:
  void init_MightInline(uint32_t clock, BitOrder bitOrder, SPIMode dataMode) {
    init_AlwaysInline(clock, bitOrder, dataMode);
  }

  // Core developer MUST use an helper function in beginTransaction() to use this data
  void init_AlwaysInline(uint32_t clock, BitOrder bitOrder, SPIMode dataMode) __attribute__((__always_inline__)) {
    this->clockFreq = clock;
    this->dataMode = dataMode;
    this->bitOrder = bitOrder;
  }

  uint32_t clockFreq;
  SPIMode dataMode;
  BitOrder bitOrder;

  friend class HardwareSPI;
};

const SPISettings DEFAULT_SPI_SETTINGS = SPISettings();

class HardwareSPI
{
  public:
    virtual ~HardwareSPI() { }

    virtual uint8_t transfer(uint8_t data) = 0;
    virtual uint16_t transfer16(uint16_t data) = 0;
    virtual void transfer(void *buf, size_t count) = 0;

    // Transaction Functions
    virtual void usingInterrupt(int interruptNumber) = 0;
    virtual void notUsingInterrupt(int interruptNumber) = 0;
    virtual void beginTransaction(SPISettings settings) = 0;
    virtual void endTransaction(void) = 0;

    // SPI Configuration methods
    virtual void attachInterrupt() = 0;
    virtual void detachInterrupt() = 0;

    virtual void begin() = 0;
    virtual void end() = 0;
};

// Alias SPIClass to HardwareSPI since it's already the defacto standard for SPI classe name
typedef HardwareSPI SPIClass;

}
