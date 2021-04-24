Multicore Processing
====================

The RP2040 chip has 2 cores that can run independently of each other, sharing
peripherals and memory with each other.  Arduino code will normally execute
only on core 0, with the 2nd core sitting idle in a low power state.

By adding a ``setup1()`` and ``loop1()`` function to your sketch you can make
use of the second core.  Anything called from within the ``setup1()`` or
``loop1()`` routines will execute on the second core.

See the ``Multicore.ino`` example in the ``rp2040`` example directory for a
quick introduction.
