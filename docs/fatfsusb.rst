FatFSUSB
========

When the onboard flash memory is used as a ``FatFS`` filesystem, the
``FatFSUSB`` can be used to allow exporting it to a PC as a standard
memory stick.  The PC can then access, add, and remove files as if the
Pico was a USB memory stick, and upon ejection the Pico can access
any new files just as if it made them itself.

(Note, if you are using LittleFS then you need to use ``SingleFileDrive``
to export a single file, not this class, because the PC does not
understand the LittleFS disk format.)

Callbacks, Interrupt Safety, and File Operations
------------------------------------------------

The ``FatFSUSB`` library allows your application to get a callback
when a PC attempts to mount or unmount the Pico as a FAT drive.

When the drive is being used by the Pico (i.e. any ``File`` is open for
read or write, the ``FatFS`` is not ``end()`` -ed and still mounted,
etc.) the host may not access it.  Conversely, while the host PC is
connected to the drive no ``FatFS`` access by the Pico is allowed.

Your ``driveReady`` callback will be called when the PC attempts to mount
the drive.  If you have any files open, then this callback can report back
that the drive is not yet ready.  When you complete file processing, the PC
can re-attempt to mount the drive and your callback can return ``true`` .

The ``onPlug`` callback will generally ``FatFS.end()`` and set a
global flag letting your application know not to touch the filesystem until
the flag is cleared by the ``onUnplug`` callback (which will also do a
``FatFS.begin()`` call).

Failing to close all files **and** ``FatFS.end()`` before granting the
PC access to flash memory will result in corruption.  FAT does not allow multiple
writers to access the same drive.  Even mounting and only reading files from
the PC may cause hidden writes (things like access time, etc.) which would
also cause corruption.

See the included ``Listfiles-USB`` sketch for an example of working with
these limitations.
