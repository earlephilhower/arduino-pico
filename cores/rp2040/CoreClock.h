/*
    Core clock configuration helper

    Copyright (c) 2026 Earle F. Philhower, III <earlephilhower@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Apply the F_CPU clock/VREG/PSRAM-timing setup on top of the SDK default clocks.
// Does nothing when F_CPU is the SDK default.  Safe to call repeatedly.
void __applyCoreClockConfig();

#ifdef __cplusplus
}
#endif
