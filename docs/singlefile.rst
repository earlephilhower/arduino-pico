SingleFileDrive
===============

USB drive mode is supported through the ``SingleFileDrive`` class which
allows the Pico to emulate a FAT-formatted USB stick while preserving the
onboard LittleFS filesystem.  A single file can be exported this way without
needing to use FAT as the onboard filesystem (FAT is not appropriate for
flash-based devices without complicated wear leveling because of the update
frequency of the FAT tables).

This emulation is very simple and only allows for the reading of the single
file, and deleting it.

Callbacks, Interrupt Safety, and File Operations
------------------------------------------------

The ``SingleFileDrive`` library allows your application to get a callback
when a PC attempts to mount or unmount the Pico as a drive.  Your app can
also get a callback if the user attempts to delete the file (but your
sketch does not actually need to delete the file, it's up to you).

Note that when the USB drive is mounted by a PC it is not safe for your
main sketch to make changes to the LittleFS filesystem or the file being
exported.  So, normally, your ``onPlug`` callback will set a flag letting
your application know not to touch the filesystem, with the ``onUnplug``
callback clearing this flag.

Also, because the USB port can be connected at any time, it is important
to disable interrupts using ``noInterrupts()`` before writing to a file
you will be exporting (and restoring them with ``interrupts()`` afterwards).
It is also important to ``close()`` the file after each update, or the
on-flash version the ``SingleFileDrive`` will attempt to export may not be
up to date causing issues later on.

See the included ``DataLoggerUSB`` sketch for an example of working with
these limitations.

Using SingleFileDrive
---------------------

Implementing the drive requires including the header file, starting LittleFS,
defining your callbacks, and telling the library what file to export.  No
polling or other calls are required outside of your ``setup()``.  (Note that
the callback routines allow for a parameter to be passed to them, but in most
cases this can be safely ignored.)

.. code:: cpp

        #include <LittleFS.h>
        #include <SingleFileDrive.h>

        void myPlugCB(uint32_t data) {
            // Tell my app not to write to flash, we're connected
        }

        void myUnplugCB(uint32_t data) {
            // I can start writing to flash again
        }

        void myDeleteDB(uint32_t data) {
            // Maybe LittleFS.remove("myfile.txt")?  or do nothing
        }

        void setup() {
            LittleFS.begin();
            singleFileDrive.onPlug(myPlugCB);
            singleFileDrive.onUnplug(myUnplugCB);
            singleFileDrive.onDelete(myDeleteCB);
            singleFileDrive.begin("littlefsfile.csv", "Data Recorder.csv");
            // ... rest of setup ...
        }

        void loop() {
            // Take some measurements, delay, etc.
            if (okay-to-write) {
                noInterrupts();
                File f = LittleFS.open("littlefsfile.csv", "a");
                f.printf("%d,%d,%d\n", data1, data2, data3);
                f.close();
                interrupts();
            }
        }
