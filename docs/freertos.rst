FreeRTOS SMP
============

The SMP (multicore) port of FreeRTOS is included with the core.  This allows complex
task operations and real preemptive multithreading in your sketches.  While the
``setup1`` and ``loop1`` way of multitasking is simplest for most folks, FreeRTOS
is much more powerful.

Enabling FreeRTOS
-----------------

To enable FreeRTOS, simply add

.. code:: c++

    #include <FreeRTOS.h>

to your sketch and it will be included and enabled automatically.

Configuration and Predefined Tasks
----------------------------------

FreeRTOS is configured with 8 priority levels (0 through 7) and a process for
``setup()/loop()``, ``setup1()/loop1()``, and the USB port will be created.  The task
quantum is 1 millisecond (i.e. 1,000 switches per second).

``setup()`` and ``loop()`` are assigned to only run on core 0, while ``setup1()`` and ``loop1()``
only run in core 1 in this mode, the same as the default multithreading mode.

You can launch and manage additional processes using the standard FreeRTOS routines.

``delay()`` and ``yield()`` free the CPU for other tasks, while ``delayMicroseconds()`` does not.

Caveats
-------

While the core now supports FreeRTOS, most (probably all) Arduino libraries were not written
to support preemptive multithreading.  This means that all calls to a particular library should
be made from a single task.

In particular, the ``LittleFS`` and ``SDFS`` libraries can not be called from different
threads.  Do all ``File`` operations from a single thread or else undefined behavior
(aka strange crashes or data corruption) can occur.

More Information
----------------

For full FreeRTOS documentation look at `FreeRTOS.org <https://freertos.org/index.html>`__
and `FreeRTOS SMP support <https://freertos.org/symmetric-multiprocessing-introduction.html>`__.
