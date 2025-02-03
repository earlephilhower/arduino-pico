# PlatformIO PSRam Pimoroni Pico 2 (2350) Demo

This shows the PlatformIO config and a simple helper struct to use the PSRAM instead of the SRAM.

Include the relevant entries from the platformio.ini file. Then import the PsramAllocator file and use it like so:

```cpp

// Before
// std::vector<uint16_t> pageBuffer;

// After
static std::vector<uint16_t, PsramAllocator<uint16_t>> pageBuffer;

pageBuffer.size(SCREEN_WIDTH * SCREEN_HEIGHT);
```