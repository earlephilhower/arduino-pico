FreeRTOS
========

The SMP port of FreeRTOS is included with the core.  This allows complex task operations
and real preemptive multithreading in your sketches.  To enable FreeRTOS, simply add

.. code:: c++

    #include <FreeRTOS.h>

to your sketch and it will be included and enabled automatically.

FreeRTOS is configured with 8 priority levels (0 through 7) and a process for
``setup()/loop()``, ``setup1()/loop1()``, and the USB port will be created.  You can
launch additional processes using the standard FreeRTOS routines.

``setup()`` and ``loop()`` are assigned to only run on core 0, while ``setup1()`` and ``loop1()``
only run in core 1 in this mode, the same as the default multithreading mode.

For full FreeRTOS documentation look at `FreeRTOS.org <https://freertos.org/index.html>`__
and `FreeRTOS SMP support <https://freertos.org/symmetric-multiprocessing-introduction.html>`__.
