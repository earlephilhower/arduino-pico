# SimpleMDNS Library for Arduino-Pico

This library implements a very basic MDNS responder (xxx.local) for the
Pico to enable things like local name resolution and Arduino IDE OTA
connections.  It uses the LWIP-provided simplified MDNS application.

Unlike LEAmDNS, this library only supports very simple configurations.
They should be sufficient for OTA and name resolution and simple web
servers, but for more complicated needs please use LEAmDNS.

The benefit of this simplicity is that it is low code and has no runtime
memory allocations.  This means it can be run under FreeRTOS (which LEAmDNS
does not presently support).

This should be a drop-in replacement, just replace `#include <LEAmDNS.h>`
with `#include <SimpleMDNS.h>`.

Be sure to `MDNS.begin()` after enabling WiFi/Ethernet.begin().

-Earle F. Philhower, III
 <earlephilhower@yahoo.com>
