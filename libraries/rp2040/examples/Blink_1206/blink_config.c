#include <Arduino.h>
#include "blink_config.h"

// These variables are stored in a separate file in order to trigger earlephilhower/arduino-pico#1206

int led = LED_BUILTIN;
long blink_time = 500;
