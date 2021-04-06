Libraries Ported/Optimized for the RP2040
=========================================

Most Arduino libraries that work on modern 32-bit CPU based Arduino boards
will run fine using Arduino-Pico.

The following libraries have undergone additional porting and optimizations
specifically for the RP2040 and you should consider using them instead of
the generic versions available in the Library Manager

* `Adafruit GFX Library <https://github.com/Bodmer/Adafruit-GFX-Library>`_ by @Bodmer, 2-20x faster than the standard version on the Pico
* `Adafruit ILI9341 Library <https://github.com/Bodmer/Adafruit_ILI9341>`_ again by @Bodmer
* `ESP8266Audio <https://github.com/earlephilhower/ESP8266Audio>`_ ported to use the included ``I2S`` library
