#include <Arduino.h>

extern void setup();
extern void loop();

extern "C" {

int main() {
	Serial.begin();
	setup();
	while (1) loop();
	return 0;
}

}
