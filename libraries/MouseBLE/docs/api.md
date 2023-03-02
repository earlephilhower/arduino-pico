# Mouse library

## Methods

### `Mouse.begin()`

Begins emulating the mouse connected to a computer. `begin()` must be called before controlling the computer. To end control, use `Mouse.end()`.

#### Syntax 

```
Mouse.begin()
```

#### Parameters

None.

#### Returns

None.

#### Example

```
#include <Mouse.h>

void setup() {
  pinMode(2, INPUT);
}

void loop() {
  // Initialize the Mouse library when button is pressed
  if (digitalRead(2) == HIGH) {
    Mouse.begin();
  }
}
```

#### See also

* [Mouse.click()](#mouseclick)
* [Mouse.end()](#mouseend)
* [Mouse.move()](#mousemove)
* [Mouse.press()](#mousepress)
* [Mouse.release()](#mouserelease)
* [Mouse.isPressed()](#mouseispressed)

### `Mouse.click()`

Sends a momentary click to the computer at the location of the cursor. This is the same as pressing and immediately releasing the mouse button.

`Mouse.click()` defaults to the left mouse button.

#### Syntax 

```
Mouse.click()
Mouse.click(button)
```

#### Parameters

* `button`: which mouse button to press (MOUSE_LEFT, MOUSE_RIGHT or MOUSE_MIDDLE, default is MOUSE_LEFT).
  
#### Returns

None.

#### Example

```
#include <Mouse.h>

void setup() {
  pinMode(2, INPUT);
  // Initialize the Mouse library
  Mouse.begin();
}

void loop() {
  // If the button is pressed, send a left mouse click
  if (digitalRead(2) == HIGH) {
    Mouse.click();
  }
}
```

#### Notes and warnings

When you use the `Mouse.click()` command, the Arduino takes over your mouse! Make sure you have control before you use the command. A pushbutton to toggle the mouse control state is effective.

#### See also

* [Mouse.begin()](#mousebegin)
* [Mouse.end()](#mouseend)
* [Mouse.move()](#mousemove)
* [Mouse.press()](#mousepress)
* [Mouse.release()](#mouserelease)
* [Mouse.isPressed()](#mouseispressed)
  
### `Mouse.end()`

Stops emulating the mouse connected to a computer. To start control, use `Mouse.begin()`.

#### Syntax 

```
Mouse.end()
```

#### Parameters

None.

#### Returns

None.

#### Example

```
#include <Mouse.h>

void setup() {
  pinMode(2, INPUT);
  // Initiate the Mouse library
  Mouse.begin();
}

void loop() {
  // If the button is pressed, send a left mouse click
  if (digitalRead(2) == HIGH) {
    Mouse.click();
    // Then end the Mouse emulation
    Mouse.end();
  }
}
```

#### See also

* [Mouse.begin()](#mousebegin)
* [Mouse.click()](#mouseclick)
* [Mouse.move()](#mousemove)
* [Mouse.press()](#mousepress)
* [Mouse.release()](#mouserelease)
* [Mouse.isPressed()](#mouseispressed)

### `Mouse.move()`

Moves the cursor on a connected computer. The motion onscreen is always relative to the cursor’s current location. Before using `Mouse.move()` you must call `Mouse.begin()`.

#### Syntax 

```
Mouse.move(xVal, yVal, wheel)
```

#### Parameters

* `xVal`: amount to move along the x-axis. Allowed data types: signed char.
* `yVal`: amount to move along the y-axis. Allowed data types: signed char.
* `wheel`: amount to move scroll wheel. Allowed data types: signed char.

#### Returns

None.

#### Example

```
#include <Mouse.h>

const int xAxis = A1;         // Analog sensor for X axis
const int yAxis = A2;         // Analog sensor for Y axis

int range = 12;               // Output range of X or Y movement
int responseDelay = 2;        // Response delay of the mouse, in ms
int threshold = range / 4;    // Resting threshold
int center = range / 2;       // Resting position value
int minima[] = {1023, 1023};  // Actual analogRead minima for (x, y)
int maxima[] = {0, 0};        // Actual analogRead maxima for (x, y)
int axis[] = {xAxis, yAxis};  // Pin numbers for (x, y)
int mouseReading[2];          // Final mouse readings for (x, y)

void setup() {
  // Initialize the Mouse library
  Mouse.begin();
}

void loop() {
  // Read and scale the two axes
  int xReading = readAxis(0);
  int yReading = readAxis(1);

  // Move the mouse
  Mouse.move(xReading, yReading, 0);
  delay(responseDelay);
}

/*
  Reads an axis (0 or 1 for x or y) and scales the
  analog input range to a range from 0 to <range>
*/
int readAxis(int axisNumber) {
  int distance = 0; // Distance from center of the output range

  // Read the analog input
  int reading = analogRead(axis[axisNumber]);

  // Of the current reading exceeds the max or min for this axis, reset the max or min
  if (reading < minima[axisNumber]) {
    minima[axisNumber] = reading;
  }
  if (reading > maxima[axisNumber]) {
    maxima[axisNumber] = reading;
  }

  // Map the reading from the analog input range to the output range
  reading = map(reading, minima[axisNumber], maxima[axisNumber], 0, range);

  // If the output reading is outside from the rest position threshold,  use it
  if (abs(reading - center) > threshold) {
    distance = (reading - center);
  }

  // The Y axis needs to be inverted in order to map the movement correctly
  if (axisNumber == 1) {
    distance = -distance;
  }

  // Return the distance for this axis
  return distance;
}
```

#### Notes and warnings

When you use the `Mouse.move()` command, the Arduino takes over your mouse! Make sure you have control before you use the command. A pushbutton to toggle the mouse control state is effective.

#### See also

* [Mouse.begin()](#mousebegin)
* [Mouse.click()](#mouseclick)
* [Mouse.end()](#mouseend)
* [Mouse.press()](#mousepress)
* [Mouse.release()](#mouserelease)
* [Mouse.isPressed()](#mouseispressed)

### `Mouse.press()`

Sends a button press to a connected computer. A press is the equivalent of clicking and continuously holding the mouse button. A press is cancelled with `Mouse.release()`. Before using `Mouse.press()`, you need to start communication with `Mouse.begin()`. `Mouse.press()` defaults to a left button press.

#### Syntax 

```
Mouse.press()
Mouse.press(button)
```

#### Parameters

* `button`: which mouse button to press (MOUSE_LEFT, MOUSE_RIGHT or MOUSE_MIDDLE, default is MOUSE_LEFT).

#### Returns

None.

#### Example

```
#include <Mouse.h>

void setup() {
  // The switch that will initiate the Mouse press
  pinMode(2, INPUT);
  // The switch that will terminate the Mouse press
  pinMode(3, INPUT);
  // Initialize the Mouse library
  Mouse.begin();
}

void loop() {
  // If the switch attached to pin 2 is closed, press and hold the left mouse button
  if (digitalRead(2) == HIGH) {
    Mouse.press();
  }
  // If the switch attached to pin 3 is closed, release the left mouse button
  if (digitalRead(3) == HIGH) {
    Mouse.release();
  }
}
```

#### Notes and warnings

When you use the `Mouse.press()` command, the Arduino takes over your mouse! Make sure you have control before you use the command. A pushbutton to toggle the mouse control state is effective.

#### See also

* [Mouse.begin()](#mousebegin)
* [Mouse.click()](#mouseclick)
* [Mouse.end()](#mouseend)
* [Mouse.move()](#mousemove)
* [Mouse.release()](#mouserelease)
* [Mouse.isPressed()](#mouseispressed)

### `Mouse.release()`

Sends a message that a previously pressed button (invoked through `Mouse.press()`) is released. `Mouse.release()` defaults to the left button.

#### Syntax 

```
Mouse.press()
Mouse.press(button)
```

#### Parameters

* `button`: which mouse button was released (MOUSE_LEFT, MOUSE_RIGHT or MOUSE_MIDDLE, default is MOUSE_LEFT).

#### Returns

None.

#### Example

```
#include <Mouse.h>

void setup() {
  // The switch that will initiate the Mouse press
  pinMode(2, INPUT);
  // The switch that will terminate the Mouse press
  pinMode(3, INPUT);
  // Initialize the Mouse library
  Mouse.begin();
}

void loop() {
  // If the switch attached to pin 2 is closed, press and hold the left mouse button
  if (digitalRead(2) == HIGH) {
    Mouse.press();
  }
  // If the switch attached to pin 3 is closed, release the left mouse button
  if (digitalRead(3) == HIGH) {
    Mouse.release();
  }
}
```

#### Notes and warnings

When you use the `Mouse.release()` command, the Arduino takes over your mouse! Make sure you have control before you use the command. A pushbutton to toggle the mouse control state is effective.

#### See also

* [Mouse.begin()](#mousebegin)
* [Mouse.click()](#mouseclick)
* [Mouse.end()](#mouseend)
* [Mouse.move()](#mousemove)
* [Mouse.press()](#mousepress)
* [Mouse.isPressed()](#mouseispressed)

### `Mouse.isPressed()`

Checks the current status of all mouse buttons, and reports if any are pressed or not. By default, it checks the status of the left mouse button.

#### Syntax 

```
Mouse.isPressed();
Mouse.isPressed(button);
```

#### Parameters

* `button`: which mouse button was released (MOUSE_LEFT, MOUSE_RIGHT or MOUSE_MIDDLE, default is MOUSE_LEFT).

#### Returns

1 if a button was pressed, 0 if a not.

#### Example

```
#include <Mouse.h>

void setup() {
  // The switch that will initiate the Mouse press
  pinMode(2, INPUT);
  // The switch that will terminate the Mouse press
  pinMode(3, INPUT);
  // Start serial communication with the computer
  Serial.begin(9600);
  // Initialize the Mouse library
  Mouse.begin();
}

void loop() {
  // A variable for checking the button's state
  int mouseState = 0;
  // If the switch attached to pin 2 is closed, press and hold the left mouse button and save the state ina  variable
  if (digitalRead(2) == HIGH) {
    Mouse.press();
    mouseState = Mouse.isPressed();
  }
  // If the switch attached to pin 3 is closed, release the left mouse button and save the state in a variable
  if (digitalRead(3) == HIGH) {
    Mouse.release();
    mouseState = Mouse.isPressed();
  }
  // Print out the current mouse button state
  Serial.println(mouseState);
  delay(10);
}
```

#### See also

* [Mouse.begin()](#mousebegin)
* [Mouse.click()](#mouseclick)
* [Mouse.end()](#mouseend)
* [Mouse.move()](#mousemove)
* [Mouse.press()](#mousepress)
* [Mouse.release()](#mouserelease)