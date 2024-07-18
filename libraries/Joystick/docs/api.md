# Joystick library

## Methods

### `Joystick.begin()`

Must be called before starting using the Joystick emulation. To end control, use `Joystick.end()`.

#### Syntax 

```
Joystick.begin()
```

#### Parameters

None.

#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  // Initialize the Joystick library when button is pressed
  if (digitalRead(2) == LOW) {
    Joystick.begin();
  }
}
```

#### See also


* [Joystick.button()](#joystickbutton)
* [Joystick.end()](#joystickend)
* [Joystick.send_now()](#joysticksend_now)
* [Joystick.position()](#joystickposition)
* [Joystick.X()](#joystickx)
* [Joystick.hat()](#joystickhat)
* [Joystick.use8bit()](#joystickuse8bit)
* [Joystick.useManualSend()](#joystickuseManualSend)

### `Joystick.button()`

Updates a button of the USB joystick.

#### Syntax 

```
Joystick.button(1,true);
delay(250);
Joystick.button(1,false);
```

#### Parameters

* `button`: number of Joystick button, which status should be changed
* `val`: state of button, `true` for pressed, `false` for released
  
#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
  Joystick.begin();
}

void loop() {
  uint8_t i = 1; //counter for the button number 1-32
  if (digitalRead(2) == LOW) { //if button is pressed
    Joystick.button(i,true); //"press" the Joystick button
	delay(250); //wait for 0.25s
	Joystick.button(i,false); //"release" the Joystick button
	i = i + 1; //increment & use next Joystick button number
	if(i > 32) i = 1; //we have 32 buttons available, wrap-around
  }
}
```

#### Notes

* Up to 32 buttons are available, numbered as button 1 to button 32.
* If manual_send is active, call `Joystick.send_now()` to send an update to the host.

#### See also

* [Joystick.send_now()](#joysticksend_now)
* [Joystick.useManualSend()](#joystickuseManualSend)
* [Joystick.X()](#joystickx)
* [Joystick.hat()](#joystickhat)

  
### `Joystick.end()`

Stops emulating the Joystick connected to a computer. To start control, use `Joystick.begin()`.

#### Syntax 

```
Joystick.end()
```

#### Parameters

None.

#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
  // Initiate the Joystick library
  Joystick.begin();
}

void loop() {
  // If the button is pressed, send a button 1 press / release
  if (digitalRead(2) == LOW) {
    Joystick.button(1,true);
	delay(250);
	Joystick.button(1,false);
    // Then end the Joystick emulation
    Joystick.end();
  }
}
```

#### See also

* [Joystick.begin()](#joystickbegin)

### `Joystick.use8bit()`

Switch axis value range between 10bit and 8bit.
* Default: 10bit, range for an axis from 0 to 1023
* 8bit mode: range from -127 to 127.


#### Syntax 

```
Joystick.use8bit(true)
```

#### Parameters

* `mode`: true, if values from -127/127 are used. False to use a range from 0 to 1023.

#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
  Joystick.begin();
}

void loop() {
  //send middle position in default 10bit mode
  Joystick.position(512,512);
  delay(250);
  //enable 8bit mode
  Joystick.use8bit(true);
  //send middle position in 8bit mode
  Joystick.position(0,0);
  delay(250);
  
  //send maximum left in 10bit mode
  Joystick.use8bit(false);
  Joystick.position(0,0);
  delay(250);
  //enable 8bit mode
  Joystick.use8bit(true);
  //send left position in 8bit mode
  Joystick.position(-127,-127);
}
```

#### See also

* [Joystick.position()](#joystickposition)
* [Joystick.X()](#joystickx)
* [Joystick.Y()](#joysticky)
* [Joystick.Z()](#joystickz)
* [Joystick.Zrotate()](#joystickrotate)
* [Joystick.slider()](#joystickslider)
* [Joystick.sliderLeft()](#joysticksliderleft)
* [Joystick.sliderRight()](#joysticksliderright)


### `Joystick.use10bit()`
### `Joystick.use16bit()`

Set axis value range to 10-bit (0...1024) or 16-bit (-32767...32767).

### `Joystick.useManualSend()`

To fully control transmitting the USB-HID reports, enable manual sending.
If disabled, each call to a function updating the Joystick status (buttons, all axis, hat)
will send a HID report. If you update in a loop, the time between updates (at least 1ms) is too short and something might be not transmitted correctly.
If enabled, update all your axis values, buttons, hat and then send one report via `Joystick.send_now()`.

#### Syntax 

```
Joystick.useManualSend(true)
```

#### Parameters

* `mode`: false is sending report each Joystick update, true enables manual sending via send_now().

#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
  Joystick.begin();
}

void loop() {
  if (digitalRead(2) == LOW) {
      // send data in 4 different reports
	  Joystick.button(1,true);
	  Joystick.button(2,true);
	  Joystick.button(3,true);
	  Joystick.button(4,true);

	  //enable manual send
	  Joystick.useManualSend(true);

	  //send same data in one report
	  Joystick.button(1,false);
	  Joystick.button(2,false);
	  Joystick.button(3,false);
	  Joystick.button(4,false);
	  Joystick.send_now();
  }
}
```

#### See also

* [Joystick.send_now()](#joysticksend_now)



### `Joystick.send_now()`

Send a HID report now. Used together with manual sending, see `Joystick.useManualSend()`.

#### Syntax 

```
Joystick.send_now()
```

#### Parameters

None.

#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
  Joystick.begin();
  //enable manual sending in setup
  Joystick.useManualSend(true);
}

void loop() {
  if (digitalRead(2) == LOW) {
      // update all buttons, but nothing is sent to the host
	  for(uint8_t i = 1; i<=32; i++) Joystick.button(i,true);
	  Joystick.X(256);
	  Joystick.sliderLeft(0);
	  
	  //now send in one HID report
	  Joystick.send_now();
  }
}
```

#### See also

* [Joystick.useManualSend()](#joystickusemanualsend)


### `Joystick.X()`

Update X axis.
__Note:__ If [manual send](#joystickusemanualsend) is active, the value is sent to the host only after calling [send_now](#joysticksend_now).
__Note:__ If in 10bit mode (default), the parameter is interpreted from 0 to 1023.
In 8bit mode from -127 to 127. The internal resolution is always 8bit. Change setting with [use8bit](#joystickuse8bit).

#### Syntax 

```
Joystick.X(0)
```

#### Parameters

* `val`: value from 0 to 1023 (default) or -127 to 127 (8bit mode)

#### Returns

None.

#### Example

```
#include <Joystick.h>

void setup() {
  pinMode(2, INPUT_PULLUP);
  Joystick.begin();
}

void loop() {
  if (digitalRead(2) == LOW) {
	  Joystick.X(256);
	  delay(500);
	  Joystick.X(512);
  }
}
```

#### See also

* [Joystick.Y()](#joysticky)
* [Joystick.Z()](#joystickz)
* [Joystick.Zrotate()](#joystickrotate)
* [Joystick.slider()](#joystickslider)
* [Joystick.sliderLeft()](#joysticksliderleft)
* [Joystick.sliderRight()](#joysticksliderright)
* [Joystick.send_now()](#joysticksend_now)
* [Joystick.position()](#joystickposition)
* [Joystick.hat()](#joystickhat)
* [Joystick.use8bit()](#joystickuse8bit)
* [Joystick.useManualSend()](#joystickuseManualSend)

### `Joystick.Y()`

Update Y axis. Please refer to [Joystick.X()](#joystickx).

### `Joystick.Z()`

Update Z axis. Please refer to [Joystick.X()](#joystickx).

### `Joystick.Zrotate()`

Update Z rotate axis. Please refer to [Joystick.X()](#joystickx).

### `Joystick.sliderLeft()`

Left slider value. Please refer to [Joystick.X()](#joystickx).

### `Joystick.sliderRight()`

Right slider value. Please refer to [Joystick.X()](#joystickx).

### `Joystick.slider()`

Same as [Joystick.sliderLeft()](#joysticksliderleft).

### `Joystick.position()`

Sets X and Y axis in one call. If autosending is active, one report is generated. Please refer to [Joystick.X()](#joystickx).

#### Syntax 

```
Joystick.position(512,512)
```

#### Parameters

* `X`: value from 0 to 1023 (default) or -127 to 127 (8bit mode); for X axis
* `Y`: value from 0 to 1023 (default) or -127 to 127 (8bit mode); for Y axis

#### See also

* [Joystick.X()](#joystickx)
* [Joystick.Y()](#joysticky)

### `Joystick.hat()`

Set the hat value to selection angle or rest position.

#### Syntax 

```
Joystick.hat(-1), // released/rest position
```

#### Parameters

* `angle` Angle value from 0-360 degrees or -1 for released resting position. Mapped to 8 different directions.
