#include <stdint.h>

#define _OTA_WRITE 1
#define _OTA_VERIFY 1

typedef struct {
    uint32_t command;
    union {
        struct {
            char filename[64];
            uint32_t fileOffset;
            uint32_t fileLength;
            uint32_t flashAddress;   // Normally XIP_BASE
        } write;
    };
} commandEntry;

// Must fit within 4K page
typedef struct {
    uint8_t sign[8]; // "Pico OTA Format\0"

    // LittleFS partition information
    uint8_t *_start;
    uint32_t _blockSize;
    uint32_t _size;

    // List of operations
    uint32_t count;
    commandEntry cmd[8];

    uint32_t crc32; // CRC32 over just the contents of this struct, up until just before this value
} OTACmdPage;

static const OTACmdPage *_ota_command_rom = (const OTACmdPage*) 0x10004000;
