// Part of the WM8960USB example. Released to the public domain 2026 by K7MDL2
#ifndef SAMPLE_CODEC_H
#define SAMPLE_CODEC_H

#include <Arduino.h>

// Pico GPIO numbers, not physical header pin numbers.
#define WM8960_I2S_BCLK_PIN 18
#define WM8960_I2S_LRCLK_PIN 19
#define WM8960_I2S_DOUT_PIN 6
#define WM8960_I2S_DIN_PIN 7
#define WM8960_I2C_SDA_PIN 20
#define WM8960_I2C_SCL_PIN 21
// External oscillator on the codec module. No Pico-generated MCLK in this sample.
#define WM8960_MCLK_HZ 24000000UL

bool codec_begin();
#endif
