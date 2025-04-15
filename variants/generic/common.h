#pragma once

#include <stdint.h>

#define PINS_COUNT          (30u)
#define NUM_DIGITAL_PINS    (30u)
#define NUM_ANALOG_INPUTS   (4u)
#define NUM_ANALOG_OUTPUTS  (0u)
#define ADC_RESOLUTION      (12u)
#define WIRE_INTERFACES_COUNT (WIRE_HOWMANY)

#ifdef PIN_LED
#define LED_BUILTIN PIN_LED
#endif

#ifdef __PIN_D0
static const uint8_t D0 = __PIN_D0;
#else
static const uint8_t D0 = (0u);
#endif
#ifdef __PIN_D1
static const uint8_t D1 = __PIN_D1;
#else
static const uint8_t D1 = (1u);
#endif
#ifdef __PIN_D2
static const uint8_t D2 = __PIN_D2;
#else
static const uint8_t D2 = (2u);
#endif
#ifdef __PIN_D3
static const uint8_t D3 = __PIN_D3;
#else
static const uint8_t D3 = (3u);
#endif
#ifdef __PIN_D4
static const uint8_t D4 = __PIN_D4;
#else
static const uint8_t D4 = (4u);
#endif
#ifdef __PIN_D5
static const uint8_t D5 = __PIN_D5;
#else
static const uint8_t D5 = (5u);
#endif
#ifdef __PIN_D6
static const uint8_t D6 = __PIN_D6;
#else
static const uint8_t D6 = (6u);
#endif
#ifdef __PIN_D7
static const uint8_t D7 = __PIN_D7;
#else
static const uint8_t D7 = (7u);
#endif
#ifdef __PIN_D8
static const uint8_t D8 = __PIN_D8;
#else
static const uint8_t D8 = (8u);
#endif
#ifdef __PIN_D9
static const uint8_t D9 = __PIN_D9;
#else
static const uint8_t D9 = (9u);
#endif
#ifdef __PIN_D10
static const uint8_t D10 = __PIN_D10;
#else
static const uint8_t D10 = (10u);
#endif
#ifdef __PIN_D11
static const uint8_t D11 = __PIN_D11;
#else
static const uint8_t D11 = (11u);
#endif
#ifdef __PIN_D12
static const uint8_t D12 = __PIN_D12;
#else
static const uint8_t D12 = (12u);
#endif
#ifdef __PIN_D13
static const uint8_t D13 = __PIN_D13;
#else
static const uint8_t D13 = (13u);
#endif
#ifdef __PIN_D14
static const uint8_t D14 = __PIN_D14;
#else
static const uint8_t D14 = (14u);
#endif
#ifdef __PIN_D15
static const uint8_t D15 = __PIN_D15;
#else
static const uint8_t D15 = (15u);
#endif
#ifdef __PIN_D16
static const uint8_t D16 = __PIN_D16;
#else
static const uint8_t D16 = (16u);
#endif
#ifdef __PIN_D17
static const uint8_t D17 = __PIN_D17;
#else
static const uint8_t D17 = (17u);
#endif
#ifdef __PIN_D18
static const uint8_t D18 = __PIN_D18;
#else
static const uint8_t D18 = (18u);
#endif
#ifdef __PIN_D19
static const uint8_t D19 = __PIN_D19;
#else
static const uint8_t D19 = (19u);
#endif
#ifdef __PIN_D20
static const uint8_t D20 = __PIN_D20;
#else
static const uint8_t D20 = (20u);
#endif
#ifdef __PIN_D21
static const uint8_t D21 = __PIN_D21;
#else
static const uint8_t D21 = (21u);
#endif
#ifdef __PIN_D22
static const uint8_t D22 = __PIN_D22;
#else
static const uint8_t D22 = (22u);
#endif
#ifdef __PIN_D23
static const uint8_t D23 = __PIN_D23;
#else
static const uint8_t D23 = (23u);
#endif
#ifdef __PIN_D24
static const uint8_t D24 = __PIN_D24;
#else
static const uint8_t D24 = (24u);
#endif
#ifdef __PIN_D25
static const uint8_t D25 = __PIN_D25;
#else
static const uint8_t D25 = (25u);
#endif
#ifdef __PIN_D26
static const uint8_t D26 = __PIN_D26;
#else
static const uint8_t D26 = (26u);
#endif
#ifdef __PIN_D27
static const uint8_t D27 = __PIN_D27;
#else
static const uint8_t D27 = (27u);
#endif
#ifdef __PIN_D28
static const uint8_t D28 = __PIN_D28;
#else
static const uint8_t D28 = (28u);
#endif
#ifdef __PIN_D29
static const uint8_t D29 = __PIN_D29;
#else
static const uint8_t D29 = (29u);
#endif

#if defined(PICO_RP2040) || (defined(PICO_RP2350) && PICO_RP2350A)

#ifdef __PIN_A0
static const uint8_t A0 = __PIN_A0;
#else
static const uint8_t A0 = (26u);
#endif

#ifdef __PIN_A1
static const uint8_t A1 = __PIN_A1;
#else
static const uint8_t A1 = (27u);
#endif

#ifdef __PIN_A2
static const uint8_t A2 = __PIN_A2;
#else
static const uint8_t A2 = (28u);
#endif

#ifdef __PIN_A3
static const uint8_t A3 = __PIN_A3;
#else
static const uint8_t A3 = (29u);
#endif

#elif defined(PICO_RP2350) && !PICO_RP2350A // RP2350B

#ifdef __PIN_D30
static const uint8_t D30 = __PIN_D30;
#else
static const uint8_t D30 = (30u);
#endif
#ifdef __PIN_D31
static const uint8_t D31 = __PIN_D31;
#else
static const uint8_t D31 = (31u);
#endif
#ifdef __PIN_D32
static const uint8_t D32 = __PIN_D32;
#else
static const uint8_t D32 = (32u);
#endif
#ifdef __PIN_D33
static const uint8_t D33 = __PIN_D33;
#else
static const uint8_t D33 = (33u);
#endif
#ifdef __PIN_D34
static const uint8_t D34 = __PIN_D34;
#else
static const uint8_t D34 = (34u);
#endif
#ifdef __PIN_D35
static const uint8_t D35 = __PIN_D35;
#else
static const uint8_t D35 = (35u);
#endif
#ifdef __PIN_D36
static const uint8_t D36 = __PIN_D36;
#else
static const uint8_t D36 = (36u);
#endif
#ifdef __PIN_D37
static const uint8_t D37 = __PIN_D37;
#else
static const uint8_t D37 = (37u);
#endif
#ifdef __PIN_D38
static const uint8_t D38 = __PIN_D38;
#else
static const uint8_t D38 = (38u);
#endif
#ifdef __PIN_D39
static const uint8_t D39 = __PIN_D39;
#else
static const uint8_t D39 = (39u);
#endif
#ifdef __PIN_D40
static const uint8_t D40 = __PIN_D40;
#else
static const uint8_t D40 = (40u);
#endif
#ifdef __PIN_D41
static const uint8_t D41 = __PIN_D41;
#else
static const uint8_t D41 = (41u);
#endif
#ifdef __PIN_D42
static const uint8_t D42 = __PIN_D42;
#else
static const uint8_t D42 = (42u);
#endif
#ifdef __PIN_D43
static const uint8_t D43 = __PIN_D43;
#else
static const uint8_t D43 = (43u);
#endif
#ifdef __PIN_D44
static const uint8_t D44 = __PIN_D44;
#else
static const uint8_t D44 = (44u);
#endif
#ifdef __PIN_D45
static const uint8_t D45 = __PIN_D45;
#else
static const uint8_t D45 = (45u);
#endif
#ifdef __PIN_D46
static const uint8_t D46 = __PIN_D46;
#else
static const uint8_t D46 = (46u);
#endif
#ifdef __PIN_D47
static const uint8_t D47 = __PIN_D47;
#else
static const uint8_t D47 = (47u);
#endif

#ifdef __PIN_A0
static const uint8_t A0 = __PIN_A0;
#else
static const uint8_t A0 = (40u);
#endif

#ifdef __PIN_A1
static const uint8_t A1 = __PIN_A1;
#else
static const uint8_t A1 = (41u);
#endif

#ifdef __PIN_A2
static const uint8_t A2 = __PIN_A2;
#else
static const uint8_t A2 = (42u);
#endif

#ifdef __PIN_A3
static const uint8_t A3 = __PIN_A3;
#else
static const uint8_t A3 = (43u);
#endif

#ifdef __PIN_A4
static const uint8_t A4 = __PIN_A4;
#else
static const uint8_t A4 = (44u);
#endif

#ifdef __PIN_A5
static const uint8_t A5 = __PIN_A5;
#else
static const uint8_t A5 = (45u);
#endif

#ifdef __PIN_A6
static const uint8_t A6 = __PIN_A6;
#else
static const uint8_t A6 = (46u);
#endif

#ifdef __PIN_A7
static const uint8_t A7 = __PIN_A7;
#else
static const uint8_t A7 = (47u);
#endif

#endif


static const uint8_t SS = PIN_SPI0_SS;
static const uint8_t MOSI = PIN_SPI0_MOSI;
static const uint8_t MISO = PIN_SPI0_MISO;
static const uint8_t SCK = PIN_SPI0_SCK;

static const uint8_t SDA = PIN_WIRE0_SDA;
static const uint8_t SCL = PIN_WIRE0_SCL;
