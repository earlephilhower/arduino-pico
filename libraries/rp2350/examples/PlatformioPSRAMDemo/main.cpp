#include <Arduino.h>
#include <vector>
#include "PsramAllocator.h"

// A vector that uses your PsramAllocator, so its buffer is in PSRAM.
static std::vector<int, PsramAllocator<int>> psramVector;

void setup()
{
	Serial.begin(115200);
	delay(5000);

	Serial.println("PSRAM Allocator Demo Start");

	// Show how much PSRAM was detected
	Serial.print("Detected PSRAM size: ");
	Serial.println(rp2040.getPSRAMSize());

	// Reserve space in the PSRAM vector
	psramVector.resize(100);
	
    // Write some values
	for (size_t i = 0; i < psramVector.size(); i++)
	{
		psramVector[i] = i * 2; // e.g. fill with even numbers
	}

	Serial.println("Data written to psramVector in external PSRAM.");

	delay(1000);

    // Read them back
	Serial.println("Reading back the first 10 elements:");
	for (size_t i = 0; i < 10 && i < psramVector.size(); i++)
	{
		Serial.print("psramVector[");
		Serial.print(i);
		Serial.print("] = ");
		Serial.println(psramVector[i]);
	}
}

void loop()
{
}
