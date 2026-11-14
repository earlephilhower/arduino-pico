Low Power (Sleep and Dormant)
=============================

The ``LowPower`` library puts the RP2040 or RP2350 into a low power state and wakes it up again from a timer or a
GPIO.  It is a thin Arduino wrapper over the Pico SDK ``pico_low_power`` library, plus the work the Arduino core
needs after a wake (restoring the CPU clock, restoring pin state).

.. code:: cpp

    #include <LowPower.h>

    void setup() {
        Serial1.begin(115200);
        pinMode(15, INPUT_PULLUP);
    }

    void loop() {
        Serial1.println("sleeping 5 seconds");
        Serial1.flush();
        LowPower.sleepFor(5000);                // light sleep, timer wake

        Serial1.println("dormant until GP15 goes low");
        Serial1.flush();
        LowPower.dormantUntilPin(15, FALLING);  // oscillators off, pin wake
    }

Modes
-----

**Sleep** gates most clocks.  The SDK's sleep leaves the oscillators and PLLs running, which is where most of the
sleep current goes.  On RP2040 and on the RP2350's ARM cores the library therefore runs the system from the crystal
and stops the system PLL and the ring oscillator around ``sleepFor`` and both forms of ``sleepUntilPin`` (about
1.3 ms of extra wake time).  What else is stopped depends on USB, and is decided at every sleep:

* ``NO_USB`` build: the USB PLL, the USB clock and the ADC clock are stopped too, and the USB pins D+ / D- are pulled
  down for the length of the sleep (the USB PHY's receivers float otherwise and draw about 0.25 mA each).
* USB build with a host using USB (connected and not suspended): the USB PLL and clock keep running, so the
  ``Serial`` port stays connected.
* USB build with no host: stopped as with ``NO_USB``; all clocks are back before the call returns, so a cable plugged
  in during a sleep enumerates normally.
* With Adafruit TinyUSB (``USE_TINYUSB``) USB is always treated as in use.  The RP2350's RISC-V cores use the SDK's
  sleep unchanged (no clock gating there).

Light sleep keeps the system timer, and therefore ``millis()``, running.

**Dormant** stops the oscillators and PLLs.  It can only be woken by a GPIO (or, with a suitable low-frequency
clock, a timer) and restarts more slowly (the crystal has to start again, about 73 ms on an RP2040).  USB is shut
down while dormant.

Every function returns ``0`` on success or a negative Pico SDK error code (``PICO_ERROR_*``, see ``pico/error.h``).

API
---

.. code:: cpp

    int sleepFor(uint32_t ms);
    int sleepUntilPin(pin_size_t pin, PinStatus mode);                       // LOW, HIGH, RISING, FALLING
    int sleepUntilPin(pin_size_t pin, PinStatus mode, uint32_t timeoutMs);   // pin or timeout
    int dormantUntilPin(pin_size_t pin, PinStatus mode);
    int dormantFor(uint32_t ms);
    void keepPins(uint64_t mask);
    void setPinRestore(bool restore);
    uint32_t lastSleepMs() const;
    WakeReason wakeReason() const;     // LowPower.WAKE_NONE, LowPower.WAKE_PIN, LowPower.WAKE_TIMER

``CHANGE`` is not supported as a wake condition (``PICO_ERROR_INVALID_ARG``).

+-------------------------+---------------------------+-----------------------------------------+
| Call                    | RP2040 (Pico, Pico W)     | RP2350 (Pico 2, Pico 2 W)               |
+=========================+===========================+=========================================+
| ``sleepFor``,           | yes                       | yes                                     |
| ``sleepUntilPin``       |                           |                                         |
| (with or without        |                           |                                         |
| timeout)                |                           |                                         |
+-------------------------+---------------------------+-----------------------------------------+
| ``dormantUntilPin``     | yes                       | yes                                     |
+-------------------------+---------------------------+-----------------------------------------+
| ``dormantFor``          | only with an external     | yes, using the always-on timer on the   |
|                         | clock, see below          | low-power oscillator                    |
+-------------------------+---------------------------+-----------------------------------------+
| Minimum ``dormantFor``  | 2000 ms                   | 10 ms                                   |
+-------------------------+---------------------------+-----------------------------------------+

Shorter ``dormantFor`` requests return ``PICO_ERROR_INVALID_ARG``.  The RP2040 has no always-on timer that survives
dormant, so ``dormantFor`` needs a 32 kHz-class clock fed into GPIO 20 or 22 and registered with
``low_power_set_external_clock_source()`` before the call; without it ``PICO_ERROR_PRECONDITION_NOT_MET`` is
returned.  ``dormantUntilPin`` needs no extra hardware.  The RP2350's switched-core power-down (the SDK's "P-state"
calls) is not wrapped by this library.

Pins
----

Before going dormant every GPIO is put into its lowest-leakage state (pulls off, input off, output disconnected),
except the wake pin, pins routed to a UART (so a TX line does not float), pins routed to I2C (parking them leaves the
controller unusable until ``Wire.end()`` / ``begin()``), and the pins you list with ``keepPins(mask)`` (bit *n* =
GPIO *n*).  The state of the parked pins is saved and **restored after the wake**; ``setPinRestore(false)`` leaves
them parked.  Light sleep never parks pins.

The wake pin must be configured by the sketch before the call (``pinMode(15, INPUT_PULLUP)`` for a button to
ground); on the RP2350 an untouched pad stays isolated from the input logic and never produces the event.

A pin routed to a UART can be the wake pin (``dormantUntilPin(1, FALLING)`` with ``Serial1`` on GP1; the start bit
is the edge).  With ``dormantUntilPin`` the byte that causes the wake is **lost** (the UART clock is stopped and the
byte is over before the clocks are back): send a dummy byte first, wait for the wake and then send the data.  With
``sleepUntilPin`` the wake byte is received normally.

Waking on a pin or a timeout
----------------------------

``sleepUntilPin(pin, mode, timeoutMs)`` ends at whichever comes first; ``LowPower.wakeReason()`` tells which:

.. code:: cpp

    int rc = LowPower.sleepUntilPin(15, FALLING, 60000);
    if (rc == 0 && LowPower.wakeReason() == LowPower.WAKE_PIN) {
        // the button / radio IRQ
    } else {
        // 60 s heartbeat
    }

* ``WAKE_PIN`` and ``WAKE_TIMER`` are also what the other calls report; ``WAKE_NONE`` before the first call and after
  a call that failed.  The names belong to the library object, so they cannot clash with constants of your own.
* With ``LOW`` or ``HIGH`` and the pin already in that state the call returns at once with ``WAKE_PIN``.  If the pin
  and the timeout coincide either reason may be reported (never both).  ``timeoutMs`` of 0 is invalid.
* Pins set up with ``attachInterrupt()`` keep working after every sleep and dormant call (the library does not
  touch the core's GPIO interrupt handler); do not use the wake pin itself for ``attachInterrupt()`` as well.

``millis()`` and ``micros()`` use the system timer, which **stops in dormant**: dormant time does not appear in them.
Light sleep keeps it running.  ``lastSleepMs()`` describes the last call: the *requested* time after a successful
``sleepFor`` / ``dormantFor``, the *measured* time after ``sleepUntilPin``, ``LowPower.SLEEP_UNKNOWN``
(``0xFFFFFFFF``) after ``dormantUntilPin`` (no time reference while dormant), and ``0`` after an error or before the
first call.

Current
-------

Whole-board current at VSYS, mean of INA226 results (0.1 ohm shunt, 25 uA steps, the board's regulator included, so
the chip alone uses less).  The INA226 zero reading was -0.075 mA: add that to the RP2040 Pico column.  The
Pico was measured at 133 MHz without USB, the Pico W at 133 MHz with a 3.7 V supply (a switching regulator draws more
current from a lower supply for the same power: compare in mW, not mA).  ``NO_USB``, nothing attached, WiFi never
started:

+--------------------------------------------+---------------+---------------+
| RP2040                                     | Pico          | Pico W        |
+============================================+===============+===============+
| running (delay loop)                       | 19.7 mA       | 26.1 mA       |
+--------------------------------------------+---------------+---------------+
| ``sleepFor``, SDK sleep (PLLs left on)     | 7.9 mA        |               |
+--------------------------------------------+---------------+---------------+
| ``sleepFor`` / ``sleepUntilPin``           | 0.8 mA        | 1.4 - 1.5 mA  |
+--------------------------------------------+---------------+---------------+
| same, USB build with no host               | 1.1 mA        | 1.8 mA        |
+--------------------------------------------+---------------+---------------+
| same, USB build with a host using USB      | 3.1 mA        |               |
+--------------------------------------------+---------------+---------------+
| ``dormantUntilPin``                        | 0.8 mA        | 1.2 mA        |
+--------------------------------------------+---------------+---------------+

Dormant uses 0.07 to 0.2 mA less than light sleep but costs about 18 uC more per wake (the crystal restart), so it
pays off for sleeps longer than roughly 0.1 to 0.3 s, at the price of a wake in about 73 ms instead of 4 to 6 ms.

+--------------------------------------------+------------------------------------+
| RP2350 (Pico 2, Pico 2 W), 150 MHz         | Board current                      |
+============================================+====================================+
| running (delay loop)                       | 15.6 mA (Pico 2), 17.0 mA (2 W)    |
+--------------------------------------------+------------------------------------+
| ``sleepFor``                               | 2.2 to 4.3 mA                      |
+--------------------------------------------+------------------------------------+
| ``sleepUntilPin``, with or without timeout | 1.6 to 4.7 mA                      |
+--------------------------------------------+------------------------------------+
| ``dormantUntilPin``, ``dormantFor``        | 3.3 to 3.9 mA                      |
+--------------------------------------------+------------------------------------+
| the SDK's switched-core power-down         | 0.15 mA (not wrapped by the        |
|                                            | library; the wake is a reboot)     |
+--------------------------------------------+------------------------------------+

On the RP2350 prefer light sleep: dormant is no lower (the regulator only enters its low-power mode in a power-state
change).  The light sleep floor is steady within one sleep but differs from sleep to sleep in steps of about 0.6 mA
(the cause is not known; a plain Pico 2 shows the same, so it is not the CYW43), and the first sleep after a reset
can cost about 1.7 mA more.  The RISC-V cores have no clock gating: light sleep draws about 6 to 9.5 mA.

Caveats
-------

* **RP2350-E9**: a GPIO input with its internal pull-down enabled can latch high.  The library's pin parking clears it,
  and so does a wake pin on an idle-high line.  A wake pin that relies on the internal pull-down and a RISING edge
  (a button to 3.3 V) can stay high after the button is released and then not wake again: use an external
  pull-down, or wake on FALLING with a pull-up.
* **WiFi on a Pico W / Pico 2 W.**  A station that is connected before the sleep is still connected after it (the
  library keeps the CYW43 pins through dormant and ``sleepFor`` holds the WiFi driver's lock until the clocks are
  back); the cost is the chip: about 48 mA asleep while connected.  ``WiFi.end()`` leaves the network but does not
  power the chip off (about 19 mA); with WiFi never started the board draws what a board without WiFi does.
* **USB.**  ``sleepFor`` does not affect USB.  Dormant disconnects it: the board re-attaches about 0.6 s after the
  wake, the host sees the port disappear, and output printed just before or in the first second after the wake can be
  lost.  Do not call ``Serial.end()`` / ``begin()`` after a wake.  Log anything important after a dormant wake on a
  UART.  Early RP2040 chips (B0/B1) can hit a rare panic in the SDK's USB enumeration workaround (RP2040-E5) when the
  host re-enumerates after a dormant wake (about 1 in 100 wakes on one board); builds without USB (``NO_USB``) and
  RP2040 B2 / RP2350 chips are not affected.
* **Clocks.**  The SDK wakes into its default clocks; the library re-applies the core's ``F_CPU`` setting (and the
  matching core voltage and PSRAM timing) so peripherals configured before the sleep keep working.  The UART clock is
  gated while sleeping: call ``Serial1.flush()`` before the call and do not expect to receive data during sleep.
* **Peripherals** configured before the call keep working afterwards without ``begin()``: UART, SPI, I2C,
  ``SerialPIO``, PIO programs (``tone()``), ADC, and PWM (``analogWrite`` stops while dormant and resumes at the same
  frequency and duty).
* **Bluetooth (BLE) on a Pico W / Pico 2 W.**  Advertising and an established connection survive every sleep and dormant
  call: tested on a Pico 2 W with a laptop connected, the link stayed up through ``sleepFor`` of 1, 3 and 10 s,
  ``sleepUntilPin`` and ``dormantUntilPin``, and reads and notifications worked right after each wake (the link layer runs
  in the CYW43, so even a sleep longer than the 9.6 s supervision timeout the laptop chose did not drop it).  Like WiFi it needs the driver's
  lock held around the sleep, which the library does for every call.
* **FreeRTOS: not supported.**  In a build with ``Tools->Operating System->FreeRTOS SMP`` every sleep and dormant call returns
  ``PICO_ERROR_NOT_PERMITTED`` and does nothing.  FreeRTOS owns the tick (SysTick), the second core and the WiFi driver's
  tasks, which the calls change underneath it.  Tested on a Pico 2 W: with plain tasks light sleep worked, but the FreeRTOS
  tick does not run asleep (tick counts, ``vTaskDelay()`` and timeouts do not include the sleep); with ``setup1()`` /
  ``loop1()`` defined or a connected WiFi chip light sleep hung; after a dormant wake the tick never ran again.  Use
  ``vTaskDelay()`` or the SDK's own sleep with FreeRTOS.
* **Both cores.**  With ``setup1()`` / ``loop1()`` the library idles the other core around the call and resumes it
  after the wake; the lowest currents need single-core sketches.
* **Flashing a dormant chip.**  A dormant chip cannot be reached over SWD and an upload fails.  Reset it through the
  RUN pin, use BOOTSEL, or wake it first; keep a startup delay at the start of ``setup()`` so a board can always be
  re-flashed.

Wake latency (RP2040 at 133 MHz, from the start of a UART wake byte to the first output): ``dormantUntilPin`` about
73 ms, ``sleepUntilPin`` about 4 to 6 ms, ``sleepFor`` and the timeout form return at the requested time (about 1 ms late).

Examples
--------

* ``SleepForSerial1`` - sleep 5 seconds in a loop, reporting over ``Serial1``.
* ``SleepPinOrTimeout`` - light sleep until a pin or a timeout.
* ``DormantUntilPin`` - go dormant until GP15 is pulled low.
