# lwIP_WINC1500 library 

RP2040 Arduino driver for Arduino WiFi101 shield and Adafruit WINC1500 shields and modules.

The Adafruit learning system has instruction for wiring the WINC1500 modules and the shields should work well with Adafruit Metro RP2040.

The driver wraps the Atmel driver for ATWIC1500 chip and is based on code from the Arduino WiFi101 library. 

## Configuration

Pins are configured with defines. To add the defines, modify boards.txt or create boards.local.txt next to boards.txt and add `<board>.build.extra_flags`.

The WiFi library uses this driver if `WINC1501_SPI` is defined. The value of `WINC1501_SPI` is the SPI interface to use. The driver will use default pins of the SPI interface.

Defaults for additional pins are as for the shields: RESET 5, INTN 7 and CS 10. The pins can be changed with defines in `<board>.build.extra_flags`

Example of complete settings:

```
rpipico.build.extra_flags=-DWINC1501_SPI=SPI -DWINC1501_RESET_PIN=D5 -DWINC1501_INTN_PIN=D7 -DWINC1501_SPI_CS_PIN=D10 
```