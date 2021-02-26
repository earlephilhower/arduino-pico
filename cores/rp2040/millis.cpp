#include "../../pico-sdk/src/common/pico_time/include/pico/time.h"

extern "C" unsigned long millis() {
    return to_ms_since_boot(get_absolute_time());
}

