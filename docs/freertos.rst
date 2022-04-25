FreeRTOS
========

The SMP port of FreeRTOS is included with the core.  This allows complex task operations
and real preemptive multithreading in your sketches.  To enable FreeRTOS, simply add

.. code:: c++

    #include <FreeRTOS.h>

to your sketch and it will be included and enabled automatically.

FreeRTOS is configured with 8 priority levels (0 through 7) and a process for
``setup()/loop()``, ``setup1()/loop1()``, and the USB port will be created.  The task
quantum is 1 millisecond (i.e. 1,000 switches per second).

``setup()`` and ``loop()`` are assigned to only run on core 0, while ``setup1()`` and ``loop1()``
only run in core 1 in this mode, the same as the default multithreading mode.

You can launch and manage additional processes using the standard FreeRTOS routines.


``delay()`` and ``yield()`` free the CPU for other tasks, while ``delayMicroseconds()`` does not.

For full FreeRTOS documentation look at `FreeRTOS.org <https://freertos.org/index.html>`__
and `FreeRTOS SMP support <https://freertos.org/symmetric-multiprocessing-introduction.html>`__.
