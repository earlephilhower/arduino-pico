#ifndef __generic_common_h__
#define __generic_common_h__

#include <Arduino.h>

static const pin_size_t D0 = 0;
static const pin_size_t D1 = 1;
static const pin_size_t D2 = 2;
static const pin_size_t D3 = 3;
static const pin_size_t D4 = 4;
static const pin_size_t D5 = 5;
static const pin_size_t D6 = 6;
static const pin_size_t D7 = 7;
static const pin_size_t D8 = 8;
static const pin_size_t D9 = 9;
static const pin_size_t D10 = 10;
static const pin_size_t D11 = 11;
static const pin_size_t D12 = 12;
static const pin_size_t D13 = 13;
static const pin_size_t D14 = 14;
static const pin_size_t D15 = 15;
static const pin_size_t D16 = 16;
static const pin_size_t D17 = 17;
static const pin_size_t D18 = 18;
static const pin_size_t D19 = 19;
static const pin_size_t D20 = 20;
static const pin_size_t D21 = 21;
static const pin_size_t D22 = 22;
static const pin_size_t D23 = 23;
static const pin_size_t D24 = 24;
static const pin_size_t D25 = 25;
static const pin_size_t D26 = 26;
static const pin_size_t D27 = 27;
static const pin_size_t D28 = 28;
static const pin_size_t D29 = 29;

static const pin_size_t A0 = 26;
static const pin_size_t A1 = 27;
static const pin_size_t A2 = 28;
static const pin_size_t A3 = 29;


#ifndef PIN_SPI_SS
#define PIN_SPI_SS   (1)
#endif
#ifndef PIN_SPI_MOSI
#define PIN_SPI_MOSI (3)
#endif
#ifndef PIN_SPI_MISO
#define PIN_SPI_MISO (0)
#endif
#ifndef PIN_SPI_SCK
#define PIN_SPI_SCK  (2)
#endif

static const pin_size_t SS = PIN_SPI_SS;
static const pin_size_t MOSI = PIN_SPI_MOSI;
static const pin_size_t MISO = PIN_SPI_MISO;
static const pin_size_t SCK = PIN_SPI_SCK;


#endif

