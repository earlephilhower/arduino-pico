/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include "/home/earle/src/pico/pico-sdk/src/rp2_common/hardware_gpio/include/hardware/gpio.h"
#include "/home/earle/src/pico/pico-sdk/src/rp2_common/hardware_pwm/include/hardware/pwm.h"
#include "/home/earle/src/pico/pico-sdk/src/rp2_common/hardware_clocks/include/hardware/clocks.h"
#include "/home/earle/src/pico/pico-sdk/src/rp2_common/hardware_pll/include/hardware/pll.h"
#include "/home/earle/src/pico/pico-sdk/src/rp2_common/hardware_clocks/include/hardware/clocks.h"

extern "C" {

static int32_t analogScale = 255;
static uint32_t analogMap = 0;
static uint16_t analogFreq = 1000;
static bool pwmInitted = false;

void analogWriteFreq(uint32_t freq) {
  if (freq == analogFreq) {
    return;
  }
  if (freq < 100) {
    analogFreq = 100;
  } else if (freq > 60000) {
    analogFreq = 60000;
  } else {
    analogFreq = freq;
  }
  pwmInitted = false;
}

void analogWriteRange(uint32_t range) {
  if (range == analogScale) {
    return;
  }
  if ((range >= 15) && (range <= 65535)) {
    analogScale = range;
    pwmInitted = false;
  }
}

void analogWriteResolution(int res) {
  if ((res >= 4) && (res <= 16)) {
    analogWriteRange((1 << res) - 1);
  }
}

void analogWrite(pin_size_t pin, int val) {
  if (!pwmInitted) {
    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv( &c, clock_get_hz(clk_sys) / 1.0 * (analogScale * analogFreq) );
    pwm_config_set_wrap( &c, analogScale );
    for (int i=0; i<30; i++) {
      pwm_init(pwm_gpio_to_slice_num(i), &c, true);
    }
    pwmInitted = true;
  }

  if (val < 0) {
    val = 0;
  } else if (val > analogScale) {
    val = analogScale;
  }

  gpio_set_function(pin, GPIO_FUNC_PWM);
  pwm_set_gpio_level(pin, val);
}

}
