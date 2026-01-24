// Simple debug option

#pragma once

#if !defined(DEBUG_RP2040_PORT) || !defined(DEBUG_RP2040_BLE)
#define DEBUGBLE(...) do { } while(0)
#else
#define DEBUGBLE(fmt, ...) do { DEBUG_RP2040_PORT.printf(fmt, ## __VA_ARGS__); DEBUG_RP2040_PORT.flush(); } while (0)
#endif

