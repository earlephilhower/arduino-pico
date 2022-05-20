**ATTENTION**  
Please be aware that this library was copied from the original Arduino AVR FreeRTOS library and may still contain files from that library that have no meaning in the context of this package, i.e. the licensing and code of conduct file! Each file retains the license and copyright from where it was taken (FreeRTOS repository or AVR FreeRTOS library, respectively.)

______

This is a copy of the RP2040 port of the FreeRTOS SMP branch, packaged as an Arduino library.

It has been created to provide access to FreeRTOS capabilities, with full compatibility to the Arduino environment.
It does this by keeping hands off almost everything, and only touching the minimum of hardware to be successful.

## General

FreeRTOS has a multitude of configuration options, which can be specified from within the FreeRTOSConfig.h file.
To keep commonality with all of the Arduino hardware options, some sensible defaults have been selected.

System ticks are 1ms, which means that Tasks can only be scheduled to run up to 1000 times per second.

Stack for the `loop()` function has been set at 256 bytes. This can be configured by adjusting the `configMINIMAL_STACK_SIZE` parameter. If you have stack overflow issues, just increase it.
Users should prefer to allocate larger structures, arrays, or buffers using `pvPortMalloc()`, rather than defining them locally on the stack.

Memory for the heap is allocated by the normal `malloc()` function, wrapped by `pvPortMalloc()`.
This option has been selected because it is automatically adjusted to use the capabilities of each device.
Other heap allocation schemes are supported by FreeRTOS, and they can used with additional configuration.

## Errors

* Stack Overflow: If any stack (for the `loop()` or) for any Task overflows, there will be a slow LED blink, with 4 second cycle.
* Heap Overflow: If any Task tries to allocate memory and that allocation fails, there will be a fast LED blink, with 100 millisecond cycle.

## Files & Configuration

* `RP2040_FreeRTOS.h` : Must always be `#include` first. It references other configuration files, and sets defaults where necessary.
* `FreeRTOSConfig.h` : Contains a multitude of API and environment configurations.
* `variantHooks.cpp` : Contains the RP2040 specific configurations for this port of FreeRTOS.
* `heap_3.c` : Contains the heap allocation scheme based on `malloc()`. Other schemes are available, but depend on user configuration for specific MCU choice.

## Sources / Credits ✨

This library is built on the efforts of the authors of the FreeRTOS SMP port for the RP2040, using the original Arduino FreeRTOS library as a template for the library package and the Arduino specific behavior.
