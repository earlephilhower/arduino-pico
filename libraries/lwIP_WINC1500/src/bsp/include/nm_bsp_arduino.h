/**
 *
 * \file
 *
 * \brief This module contains NMC1500 BSP APIs definitions.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef _NM_BSP_ARDUINO_H_
#define _NM_BSP_ARDUINO_H_

#include <stdint.h>

#include <Arduino.h>

/*
 * Arduino variants may redefine those pins.
 * If no pins are specified the following defaults are used:
 *  WINC1501_RESET_PIN   - pin 5
 *  WINC1501_INTN_PIN    - pin 7
 *  WINC1501_CHIP_EN_PIN - not connected (tied to VCC)
 */
#if !defined(WINC1501_RESET_PIN)
  #define WINC1501_RESET_PIN  5
#endif
#if !defined(WINC1501_INTN_PIN)
  #define WINC1501_INTN_PIN   7
#endif
#if !defined(WINC1501_SPI_CS_PIN)
  #define WINC1501_SPI_CS_PIN 10
#endif
#if !defined(WINC1501_CHIP_EN_PIN)
  #define WINC1501_CHIP_EN_PIN -1
#endif

extern int8_t gi8Winc1501CsPin;
extern int8_t gi8Winc1501ResetPin;
extern int8_t gi8Winc1501IntnPin;
extern int8_t gi8Winc1501ChipEnPin;

#endif /* _NM_BSP_ARDUINO_H_ */
