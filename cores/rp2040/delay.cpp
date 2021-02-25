#include "../../pico-sdk/src/common/pico_base/include/pico.h"
#include "../../pico-sdk/src/common/pico_time/include/pico/time.h"

extern "C" void delay( unsigned long ms )
{
	if (!ms) {
		return;
	}

	sleep_ms(ms);
}

extern "C" void delayMicroseconds( unsigned int usec )
{
	if (!usec) {
		return;
	}
	sleep_us(usec);
}

