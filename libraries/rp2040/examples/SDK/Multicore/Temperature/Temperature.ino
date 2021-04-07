/*
  Temperature

  This sketch demonstrates the data exchange between the two processing cores
  in the RP2040. A sketch normally runs on Core 0, but Core 1 can also be
  employed and the two cores can communicate with each other.
  
  This example is a slightly modified version from the excellent tutorial by
  Learn Embedded Systems here:

  https://www.youtube.com/watch?v=aIFElaK14V4

  That tutorial uses Visual Studio, so the parts about setting up the environment
  using cmake etc. can be skipped!

  Watch that tutorial for a explanation of how this sketch works. In summary,
  Core 0 uses the analogue to digital converter (ADC) in the Pico to read the internal
  temperature and sends this data to Core 1 which calculates the temperature of the
  chip and sends it to the USB serial interface.
  
  The sketch uses function calls from the Software Development Kit (SDK) provided
  by the Raspberry Pi foundation. This SDK is built into the Arduino IDE RP2040
  board package already, so does not need to be installed separately.
  The Application Programming Interface (API) documentation details the functions
  used:
  
  https://raspberrypi.github.io/pico-sdk-doxygen/

*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/adc.h"


// Core 1 interrupt Handler
void core1_interrupt_handler() {
  // Receive Raw Value, Convert and Print Temperature Value
  while (multicore_fifo_rvalid()) {
    // Read the raw value from the FIFO
    uint32_t raw = multicore_fifo_pop_blocking();

    // Calculate the temperature
    const float conversion_factor = 3.3f / (1 << 12);
    float result = raw * conversion_factor;
    float temp = 27 - (result - 0.706) / 0.001721;

    // Print to serial port
    Serial.printf("Temp = %f C\n", temp);
  }

  // Clear interrupt and return to core 1 main code infinite loop
  multicore_fifo_clear_irq();
}

// Core 1 Main Code
void core1_entry() {
  // Start serial support
  Serial.begin(115200);

  // Configure Core 1 Interrupt
  multicore_fifo_clear_irq();
  irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
  irq_set_enabled(SIO_IRQ_PROC1, true);

  // Infinite while Loop to wait for interrupt
  while (1) {
    tight_loop_contents(); // Does nothing - for possible future debug use
    __wfi;                 // Enter low power mode and wait for interrupt
  }
}


// Core 0
void setup() {
  // Start core 1 - Do this before any interrupt configuration
  multicore_launch_core1(core1_entry);

  // Configure the ADC
  adc_init();

  // Enable on board temp sensor and select the ADC channel
  adc_set_temp_sensor_enabled(true);
  adc_select_input(4);
}

void loop() {
  // Read the ADC channel
  uint32_t raw = adc_read();

  // Send raw data to the FIFO to be received by Core 1
  multicore_fifo_push_blocking(raw);

  // Sleep for 1 second
  sleep_ms(1000);
}
