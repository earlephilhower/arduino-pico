Multicore Processing
====================

The RP2040 chip has 2 cores that can run independently of each other, sharing
peripherals and memory with each other.  Arduino code will normally execute
only on core 0, with the 2nd core sitting idle in a low power state.

By adding a ``setup1()`` and ``loop1()`` function to your sketch you can make
use of the second core.  Anything called from within the ``setup1()`` or
``loop1()`` routines will execute on the second core.

``setup()`` and ``setup1()`` will be called at the same time, and the ``loop()``
or ``loop1()`` will be started as soon as the core's ``setup()`` completes (i.e.
not necessarily simultaneously!).

See the ``Multicore.ino`` example in the ``rp2040`` example directory for a
quick introduction.

Pausing Cores
-------------

Sometimes an application needs to pause the other core on chip (i.e. it is
writing to flash or needs to stop processing while some other event occurs).

void rp2040.idleOtherCore()
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sends a message to stop the other core (i.e. when called from core 0 it
pauses core 1, and vice versa).  Waits for the other core to acknowledge
before returning.

The other core will have its interrupts disabled and be busy-waiting in
an RAM-based routine, so flash and other peripherals can be accessed.

**NOTE** If you idle core 0 too long, then the USB port can become frozen.
This is because core 0 manages the USB and needs to service IRQs in a
timely manner (which it can't do when idled).

void rp2040.resumeOtherCore()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Resumes processing in the other core, where it left off.


void rp2040.restartCore1()
~~~~~~~~~~~~~~~~~~~~~~~~~~

Hard resets Core1 from Core 0 and restarts its operation from ``setup1()``.

Communicating Between Cores
---------------------------

The RP2040 provides a hardware FIFO for communicating between cores, but it
is used exclusively for the idle/resume calls described above.  Instead, please
use the following functions to access a software-managed, multicore safe
FIFO.

void rp2040.fifo.push(uint32_t)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pushes a value to the other core.  Will block if the FIFO is full.

bool rp2040.fifo.push_nb(uint32_t)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Pushes a value to the other core.  If the FIFO is full, returns ``false``
immediately and doesn't block.  If the push is successful, returns ``true``.

uint32_t rp2040.fifo.pop()
~~~~~~~~~~~~~~~~~~~~~~~~~~

Reads a value from this core's FIFO.  Blocks until one is available.

bool rp2040.fifo.pop_nb(uint32_t \*dest)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Reads a value from this core's FIFO and places it in dest.  Will return
``true`` if successful, or ``false`` if the pop would block.

int rp2040.fifo.available()
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Returns the number of values available in this core's FIFO.
