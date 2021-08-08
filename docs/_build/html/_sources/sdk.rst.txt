Using the Raspberry Pi Pico SDK (PICO-SDK)
==========================================

Included SDK
------------
A complete copy of the `Raspberry Pi Pico SDK <https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf>`_
is included with the Arduino core, and all functions in the core are available
inside the standard link libraries.

For simple programs wishing to call these functions, simply include the
appropriate header as shown below

.. code:: cpp

        #include "pico/stdlib.h"
        
        void setup() {
            const uint LED_PIN = 25;
            gpio_init(LED_PIN);
            gpio_set_dir(LED_PIN, GPIO_OUT);
            while (true) {
                gpio_put(LED_PIN, 1);
                sleep_ms(250);
                gpio_put(LED_PIN, 0);
                sleep_ms(250);
            }
        }
        void loop() {}

**Note:**  When you call SDK functions in your own app, the core and
libraries are not aware of any changes to the Pico you perform.  So,
you may break the functionality of certain libraries in doing so.

Multicore (CORE1) Processing
----------------------------
**Warning:**  While you may spawn multicore applications on CORE1
using the SDK, the Arduino core may have issues running properly with
them.  In particular, anything involving flash writes (i.e. EEPROM,
filesystems) will probably crash due to CORE1 attempting to read from
flash while CORE0 is writing to it.

PIOASM (Compiling for the PIO processors)
-----------------------------------------
A precompiled version of the PIOASM tool is included in the download
package and can be run from the CLI.

There is also a fully online version of PIOASM that runs in a web
browser without any CLI required, thanks to @jake653: https://wokwi.com/tools/pioasm
(GitHub source: https://github.com/wokwi/pioasm-wasm)

There is also Docker code available for the tool at:
https://github.com/kahara/pioasm-docker
