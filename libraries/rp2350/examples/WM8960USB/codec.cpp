// Part of the WM8960USB example. Released to the public domain 2026 by K7MDL2
#include "composite_config.h"
#if WM8960USB_SUPPORTED

#include "codec.h"
#include <Wire.h>

namespace {
bool write_register(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(0x1A);
    Wire.write(static_cast<uint8_t>((reg << 1) | ((value >> 8) & 1)));
    Wire.write(static_cast<uint8_t>(value));
    const uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("WM8960 I2C error %u at register 0x%02X\r\n", error, reg);
        return false;
    }
    return true;
}

bool write_table(const uint16_t (*table)[2], size_t count) {
    for (size_t i = 0; i < count; ++i)
        if (!write_register(table[i][0], table[i][1])) {
            return false;
        }
    return true;
}
}

bool codec_begin() {
    static_assert(WM8960_I2S_LRCLK_PIN == WM8960_I2S_BCLK_PIN + 1,
                  "LRCLK must be BCLK+1");
    static_assert((WM8960_I2C_SDA_PIN & 3) == 0 &&
                  WM8960_I2C_SCL_PIN == WM8960_I2C_SDA_PIN + 1,
                  "This sample uses Wire/I2C0 with consecutive SDA/SCL pins");
    Wire.setSDA(WM8960_I2C_SDA_PIN);
    Wire.setSCL(WM8960_I2C_SCL_PIN);
    Wire.begin();
    Wire.setClock(100000);
    Wire.setTimeout(10);
    const uint16_t reset[][2] = {{0x0F, 0}, {0x05, 0x008}, {0x1C, 0x09C}, {0x19, 0x080}};
    if (!write_table(reset, sizeof(reset) / sizeof(reset[0]))) {
        return false;
    }
    delay(100);
#if WM8960_MCLK_HZ == 24000000UL
    // External 24 MHz -> PLL 24.576 MHz -> SYSCLK /2 = 12.288 MHz.
    const uint16_t pll[][2] = {
        {0x04, 0}, {0x1A, 0}, {0x35, 0x031}, {0x36, 0x026},
        {0x37, 0x0E9}, {0x34, 0x038}, {0x1A, 1}
    };
    if (!write_table(pll, sizeof(pll) / sizeof(pll[0]))) {
        return false;
    }
    delay(250);
    constexpr uint16_t clock = 0x005, power = 0x1F9;
#elif WM8960_MCLK_HZ == 12288000UL
    constexpr uint16_t clock = 0, power = 0x1F8;
#else
#error "Use an external 24 MHz or 12.288 MHz MCLK"
#endif
    const uint16_t output[][2] = {
        {0x19, 0x0C0}, {0x1C, 0x008}, {0x19, 0x140}, {0x04, clock},
        {0x07, 0x002}, // 16-bit I2S, codec slave; Pico PIO supplies BCLK/LRCLK.
        {0x0A, 0x0FF}, {0x0B, 0x1FF}, {0x22, 0x100}, {0x25, 0x100},
        {0x2F, 0x03C}, {0x31, 0x0C0}, {0x1A, power},
        {0x02, 0x078}, {0x03, 0x178}, {0x28, 0x078}, {0x29, 0x178},
        {0x20, 0x108}, {0x21, 0x108}, {0x00, 23}, {0x01, 0x100 | 23},
        {0x11, 0x07B}, // ALC off for a deterministic demonstration.
        {0x15, 0x0C3}, {0x16, 0x1C3}, {0x2F, 0x03C}, {0x19, 0x17E}
    };
    if (!write_table(output, sizeof(output) / sizeof(output[0]))) {
        return false;
    }
    delay(100);
    return write_register(0x05, 0); // Unmute DAC only after clock and output setup.
}

#endif // WM8960USB_SUPPORTED
