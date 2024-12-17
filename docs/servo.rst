Servo Library
=============

A hardware-based servo controller is provided using the ``Servo`` library.
It utilizes the PIO state machines and generates the appropriate servo
control pulses, glitch-free and jitter-free (within crystal limits).

Up to 8 Servos can be controlled in parallel assuming no other tasks
require the use of a PIO machine.

See the Arduino standard
`Servo documentation <https://www.arduino.cc/reference/en/libraries/servo/>`_
for detailed usage instructions.  There is also an included ``sweep`` example.

Pulse Width Defaults
--------------------
The defaults in the Servo library are conservatively set to avoid damage in the case of over-driving. The pulse widths individual servos, especially the no-name or clones, occasionally need tweaking.

You can set the min and max servo pulse width in the attach command, with default values used in most Arduino cores of 540/2400:
```myServo.attach(D3, 540, 2400)```
