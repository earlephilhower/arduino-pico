#include "Arduino.h"
#include "/home/earle/src/pico/pico-sdk/src/rp2_common/hardware_gpio/include/hardware/gpio.h"
#ifdef __cplusplus
 extern "C" {
#endif

void pinMode( pin_size_t ulPin, PinMode ulMode )
{
	switch (ulMode) {
		case INPUT:
			gpio_init(ulPin);
			gpio_set_dir(ulPin, false);
			break;
		case INPUT_PULLUP:
			gpio_init(ulPin);
			gpio_set_dir(ulPin, false);
			gpio_pull_up(ulPin);
			break;
		case INPUT_PULLDOWN:
			gpio_init(ulPin);
			gpio_set_dir(ulPin, false);
			gpio_pull_down(ulPin);
			break;
		case OUTPUT:
			gpio_init(ulPin);
			gpio_set_dir(ulPin, true);
			break;
		default:
			// Error
			break;
	}
}

void digitalWrite( pin_size_t ulPin, PinStatus ulVal )
{
	if (!gpio_is_dir_out(ulPin)) {
		if (ulVal == LOW) {
			gpio_pull_down(ulPin);
		} else {
			gpio_pull_up(ulPin);
		}
	} else {
		gpio_put(ulPin, ulVal == LOW ? 0 : 1);
	}
}

PinStatus digitalRead( pin_size_t ulPin )
{
	return gpio_get(ulPin) ? HIGH : LOW;
}

#ifdef __cplusplus
}
#endif
