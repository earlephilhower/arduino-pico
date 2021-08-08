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
