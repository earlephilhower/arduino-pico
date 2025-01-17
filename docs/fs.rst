File Systems
============

The Arduino-Pico core supports using some of the onboard flash as a file
system, useful for storing configuration data, output strings, logging,
and more.  It also supports using SD cards as another (FAT32) filesystem,
with an API that's compatible with the onboard flash file system.


Flash Layout
------------

Even though file system is stored on the same flash chip as the program,
programming new sketch will not modify file system contents (or EEPROM
data).

The following diagram shows the flash layout used in Arduino-Pico:

::

    |----|---------------------|-------------|----|
    ^    ^                     ^             ^
    OTA  Sketch                File system   EEPROM

The file system size is configurable via the IDE menus, from 64k up to 15MB
(assuming you have an RP2040 board with that much flash)

**Note:** to use any of file system functions in the sketch, add the
following include to the sketch:

.. code:: cpp

    #include "LittleFS.h" // LittleFS is declared
    // #include <SDFS.h>
    // #include <SD.h>
    // #include <FatFS.h>


Compatible Filesystem APIs
--------------------------

LittleFS is an onboard filesystem that sets aside some program flash for
use as a filesystem without requiring any external hardware.

SDFS is a filesystem for SD cards, based on [SdFat 2.0](https://github.com/earlephilhower/ESP8266SdFat).
It supports FAT16 and FAT32 formatted cards, and requires an external
SD card reader.

SD is the Arduino-supported, somewhat old and limited SD card filesystem.
It is recommended to use SDFS for new applications instead of SD.

FatFS implements a wear-leveled, FTL-backed FAT filesystem in the onboard
flash which can be easily accessed over USB as a standard memory stick
via FatFSUSB.

All of these filesystems can open and manipulate ``File`` and ``Dir``
objects with the same code because the implement a common end-user
filesystem API.

FatFS File System Caveats and Warnings
--------------------------------------

The FAT filesystem is ubiquitous, but it is also around 50 years old and ill
suited to SPI flash memory due to having "hot spots" like the FAT copies that
are rewritten many times over.  SPI flash allows a high, but limited, number
of writes before losing the ability to write safely.  Applications like
data loggers where many writes occur could end up wearing out the SPI flash
sector that holds the FAT **years** before coming close to the write limits of
the data sectors.

To circumvent this issue, the FatFS implementation here uses a flash translation
layer (FTL) developed for SPI flash on embedded systems.  This allows for the
same LBA to be written over and over by the FAT filesystem, but use different
flash locations.  For more information see
[SPIFTL](https://github.com/earlephilhower/SPIFTL).  In this mode the Pico
flash appears as a normal, 512-byte sector drive to the FAT.

What this means, practically, is that about 5KB of RAM per megabyte of flash
is required for housekeeping.  Writes can also become very slow if most of the
flash LBA range is used (i.e. if the FAT drive is 99% full) due to the need
for garbage collection processes to move data around and preserve the flash
lifetime.

Alternatively, if an FTL is not desired or memory is tight, FatFS can use the
raw flash directly.  In this mode sectors are 4K in size and flash is mapped
1:1 to sectors, so things like the FAT table updates will all use the same
physical flash bits.  For low-utilization operations this may be fine, but if
significant writes are done (from the Pico or the PC host) this may wear out
portions of flash very quickly , rendering it unusable.

LittleFS File System Limitations
--------------------------------

The LittleFS implementation for the RP2040 supports filenames of up
to 254 characters + terminating zero (i.e. ``char filename[255]`` or
better ``char filename[LFS_NAME_MAX]`` ), and as many subdirectories
as space permits.

Filenames are assumed to be in the root directory if no initial "/" is
present.

Opening files in subdirectories requires specifying the complete path to
the file (i.e. ``LittleFS.open("/sub/dir/file.txt", "r");``).  Subdirectories
are automatically created when you attempt to create a file in a
subdirectory, and when the last file in a subdirectory is removed the
subdirectory itself is automatically deleted.

Uploading Files to the LittleFS File System
-------------------------------------------

*PicoLittleFS* is a tool which integrates into the Arduino IDE. It adds a
menu item to **Tools** menu for uploading the contents of sketch data
directory into a new LittleFS flash file system.

**IDE 1.x**

-  Download the tool: https://github.com/earlephilhower/arduino-pico-littlefs-plugin/releases
-  In your Arduino sketchbook directory, create ``tools`` directory if it doesn't exist yet.
-  Unpack the tool into ``tools`` directory (the path will look like ``<home_dir>/Arduino/tools/PicoLittleFS/tool/picolittlefs.jar``)
   If upgrading, overwrite the existing JAR file with the newer version.
-  Restart Arduino IDE.
-  Open a sketch (or create a new one and save it).
-  Go to sketch directory (choose Sketch > Show Sketch Folder).
-  Create a directory named ``data`` and any files you want in the file system there.
-  Make sure you have selected a board, port, and closed Serial Monitor.
-  Double check the Serial Monitor is closed.  Uploads will fail if the Serial Monitor has control of the serial port.
-  Select ``Tools > Pico LittleFS Data Upload``. This should start uploading the files into the flash file system.

**IDE 2.x**

-  Download the new tool: https://github.com/earlephilhower/arduino-littlefs-upload/releases
-  Exit the IDE, if running
-  Copy the VSIX file manually to (Linux/Mac) ``~/.arduinoIDE/plugins/`` (you may need to make this directory yourself beforehand) or to (Windows) ``C:\Users\<username>\.arduinoIDE\``
-  Restart the IDE
-  Double check the Serial Monitor is closed.  Uploads will fail if the Serial Monitor has control of the serial port.
-  Enter ``[Ctrl]`` + ``[Shift]`` + ``[P]`` to bring up the command palette, then select/type ``Upload LittleFS to Pico/ESP8266``

Downloading Files from a LittleFS System
----------------------------------------

Using ``gdb`` it is possible to dump the flash data making up the filesystem and then extract
it using the ``mklittlefs`` tool.  A working ``OpenOCD`` setup, DebugProbe, and ``gdb`` are required.
To download the raw filesystem, from within ``GDB`` run:

.. code::

    ^C (break)
    (gdb) dump binary memory littlefs.bin &_FS_start &_FS_end

It may take a few seconds as ``GDB`` reads out the flash to the file.  Once the raw file is downloaded it can be extracted using the ``mklittlefs`` tool from the BASH/Powershell/command line

.. code::

    $ <path-to-mklittlefs>/mklittlefs -u output-dir littlefs.bin
     Directory <output-dir> does not exists. Try to create it.
     gmon.out    > <output-dir>/gmon.out    size: 24518 Bytes
     gmon.bak    > <output-dir>/gmon.bak    size: 1 Bytes

The defaults built into ``mklittlefs`` should be appropriate for normal LittleFS filesystems built on the device or using the upload tool.

SD Library Information
----------------------
The included ``SD`` library is the Arduino standard one.  Please refer to
the [Arduino SD reference](https://www.arduino.cc/en/reference/SD) for
more information.

Using Second SPI port for SD
----------------------------
The ``SD`` library ``begin()`` has been modified to allow you to use the
second SPI port, ``SPI1``.  Just use the following call in place of
``SD.begin(cspin)``

.. code:: cpp

    SD.begin(cspin, SPI1);

Enabling SDIO operation for SD
------------------------------
SDIO support is available thanks to SdFat implementing a PIO-based SDIO controller.
This mode can significantly increase IO performance to SD cards but it requires that
all 4 DAT0..DAT3 lines to be wired to the Pico (most SD breakout boards only provide
1-but SPI mode of operation).

To enable SDIO mode, simply specify the SD_CLK, SD_CMD, and SD_DAT0 GPIO pins.  The clock
and command pins can be any GPIO (not limited to legal SPI pins).  The DAT0 pin can be any
GPIO with remaining DAT1...3 pins consecutively connected.

..code:: cpp

    SD.begin(RP_CLK_GPIO, RP_CMD_GPIO, RP_DAT0_GPIO);

No other changes are required in the application to take advantage of this high
performance mode.

Using VFS (Virtual File System) for POSIX support
-------------------------------------------------
The ``VFS`` library enables sketches to use standard POSIX file I/O operations using
standard ``FILE *`` operations.  Include the ``VFS`` library in your application and
add a call to map the ``VFS.root()`` to your filesystem.  I.e.:

.. code:: cpp

    #include <VFS.h>
    #include <LittleFS.h>

    void setup() {
      LittleFS.begin();
      VFS.root(LittleFS);
      FILE *fp = fopen("/thisfilelivesonflash.txt", "w");
      fprintf(fp, "Hello!\n");
      fclose(fp);
    }

Multiple filesystems can be ``VFS.map()`` into the VFS namespace under different directory
names.  For example, the following will make files on ``/sd`` reside on an external\
SD card and files on ``/lfs`` live in internal flash.

.. code:: cpp

    #include <VFS.h>
    #include <LittleFS.h>
    #include <SDFS.h>

    void setup() {
      LittleFS.begin();
      SDFS.begin();
      VFS.map("/lfs", LittleFS);
      VFS.map("/sd", SDFS);
      FILE *onSD = fopen("/sd/thislivesonsd.txt", "wb");
      ....
    }

See the examples in the ``VFS`` library for more information.



File system object (LittleFS/SD/SDFS/FatFS)
-------------------------------------------

setConfig
~~~~~~~~~

.. code:: cpp

    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);

    SDFSConfig c2;
    c2.setCSPin(12);
    SDFS.setConfig(c2);

    FatFSConfig c3;
    c3.setUseFTL(false); // Directly access flash memory
    c3.setDirEntries(256); // We need 256 root directory entries on a format()
    c3.setFATCopies(1); // Only 1 FAT to save 4K of space and extra writes
    FatFS.setConfig(c3);
    FatFS.format(); // Format using these settings, erasing everything

This method allows you to configure the parameters of a filesystem
before mounting.  All filesystems have their own ``*Config`` (i.e.
``SDFSConfig`` or ``LittleFSConfig`` with their custom set of options.
All filesystems allow explicitly enabling/disabling formatting when
mounts fail.  If you do not call this ``setConfig`` method before
perforing ``begin()``, you will get the filesystem's default
behavior and configuration. By default, LittleFS and FatFS will autoformat the
filesystem if it cannot mount it, while SDFS will not.  FatFS will also use
the built-in FTL to support 512 byte sectors and higher write lifetime.

begin
~~~~~

.. code:: cpp

    SDFS.begin()
    or LittleFS.begin()

This method mounts file system. It must be called before any
other FS APIs are used. Returns *true* if file system was mounted
successfully, false otherwise.

Note that LittleFS will automatically format the filesystem
if one is not detected.  This is configurable via ``setConfig``.

end
~~~

.. code:: cpp

    SDFS.end()
    or LittleFS.end()

This method unmounts the file system.

format
~~~~~~

.. code:: cpp

    SDFS.format()
    or LittleFS.format()

Formats the file system. May be called either before or after calling
``begin``. Returns *true* if formatting was successful.

open
~~~~

.. code:: cpp

    SDFS.open(path, mode)
    or LittleFS.open(path, mode)

Opens a file. ``path`` should be an absolute path starting with a slash
(e.g. ``/dir/filename.txt``). ``mode`` is a string specifying access
mode. It can be one of "r", "w", "a", "r+", "w+", "a+". The meaning of these
modes is the same as for the ``fopen`` C function.

::

       r      Open text file for reading.  The stream is positioned at the
              beginning of the file.

       r+     Open for reading and writing.  The stream is positioned at the
              beginning of the file.

       w      Truncate file to zero length or create text file for writing.
              The stream is positioned at the beginning of the file.

       w+     Open for reading and writing.  The file is created if it does
              not exist, otherwise it is truncated.  The stream is
              positioned at the beginning of the file.

       a      Open for appending (writing at end of file).  The file is
              created if it does not exist.  The stream is positioned at the
              end of the file.

       a+     Open for reading and appending (writing at end of file).  The
              file is created if it does not exist.  The initial file
              position for reading is at the beginning of the file, but
              output is always appended to the end of the file.

Returns *File* object. To check whether the file was opened
successfully, use the boolean operator.

.. code:: cpp

    File f = LittleFS.open("/f.txt", "w");
    if (!f) {
        Serial.println("file open failed");
    }

exists
~~~~~~

.. code:: cpp

    SDFS.exists(path)
    or LittleFS.exists(path)

Returns *true* if a file with given path exists, *false* otherwise.

mkdir
~~~~~

.. code:: cpp

    SDFS.mkdir(path)
    or LittleFS.mkdir(path)

Returns *true* if the directory creation succeeded, *false* otherwise.

rmdir
~~~~~

.. code:: cpp

    SDFS.rmdir(path)
    or LittleFS.rmdir(path)

Returns *true* if the directory was successfully removed, *false* otherwise.


openDir
~~~~~~~

.. code:: cpp

    SDFS.openDir(path)
    or LittleFS.openDir(path)

Opens a directory given its absolute path. Returns a *Dir* object.

remove
~~~~~~

.. code:: cpp

    SDFS.remove(path)
    or LittleFS.remove(path)

Deletes the file given its absolute path. Returns *true* if file was
deleted successfully.

rename
~~~~~~

.. code:: cpp

    SDFS.rename(pathFrom, pathTo)
    or LittleFS.rename(pathFrom, pathTo)

Renames file from ``pathFrom`` to ``pathTo``. Paths must be absolute.
Returns *true* if file was renamed successfully.

info
~~~~

.. code:: cpp

    FSInfo fs_info;
    or LittleFS.info(fs_info);

Fills `FSInfo structure <#filesystem-information-structure>`__ with
information about the file system. Returns ``true`` if successful,
``false`` otherwise. ``ìnfo()`` has been updated to support filesystems 
greater than 4GB and ``FSInfo64`` and ``info64()`` have been discarded.

Filesystem information structure
--------------------------------

.. code:: cpp

    struct FSInfo {
        uint64_t totalBytes;
        uint64_t usedBytes;
        size_t blockSize;
        size_t pageSize;
        size_t maxOpenFiles;
        size_t maxPathLength;
    };

This is the structure which may be filled using FS::info method. -
``totalBytes`` — total size of useful data on the file system -
``usedBytes`` — number of bytes used by files - ``blockSize`` — filesystem
block size - ``pageSize`` — filesystem logical page size - ``maxOpenFiles``
— max number of files which may be open simultaneously -
``maxPathLength`` — max file name length (including one byte for zero
termination)

setTimeCallback(time_t (\*cb)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: cpp

    time_t myTimeCallback() {
        return 1455451200; // UNIX timestamp
    }
    void setup () {
        LittleFS.setTimeCallback(myTimeCallback);
        ...
        // Any files will now be made with Pris' incept date
    }


The SD, SDFS, and LittleFS filesystems support a file timestamp, updated when the file is
opened for writing.  By default, the Pico will use the internal time returned from
``time(NULL)`` (i.e. local time, not UTC, to conform to the existing FAT filesystem), but this
can be overridden to GMT or any other standard you'd like by using ``setTimeCallback()``.
If your app sets the system time using NTP before file operations, then
you should not need to use this function.  However, if you need to set a specific time
for a file, or the system clock isn't correct and you need to read the time from an external
RTC or use a fixed time, this call allows you do to so.

In general use, with a functioning ``time()`` call, user applications should not need
to use this function.

Directory object (Dir)
----------------------

The purpose of *Dir* object is to iterate over files inside a directory.
It provides multiple access methods.

The following example shows how it should be used:

.. code:: cpp

    Dir dir = LittleFS.openDir("/data");
    // or Dir dir = LittleFS.openDir("/data");
    while (dir.next()) {
        Serial.print(dir.fileName());
        if(dir.fileSize()) {
            File f = dir.openFile("r");
            Serial.println(f.size());
        }
    }

next
~~~~

Returns true while there are files in the directory to
iterate over. It must be called before calling ``fileName()``, ``fileSize()``,
and ``openFile()`` functions.

fileName
~~~~~~~~~

Returns the name of the current file pointed to
by the internal iterator.

fileSize
~~~~~~~~

Returns the size of the current file pointed to
by the internal iterator.

fileTime
~~~~~~~~

Returns the time_t write time of the current file pointed
to by the internal iterator.

fileCreationTime
~~~~~~~~~~~~~~~~
Returns the time_t creation time of the current file
pointed to by the internal iterator.

isFile
~~~~~~

Returns *true* if the current file pointed to by
the internal iterator is a File.

isDirectory
~~~~~~~~~~~

Returns *true* if the current file pointed to by
the internal iterator is a Directory.

openFile
~~~~~~~~

This method takes *mode* argument which has the same meaning as
for ``SDFS/LittleFS.open()`` function.

rewind
~~~~~~

Resets the internal pointer to the start of the directory.

setTimeCallback(time_t (\*cb)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sets the time callback for any files accessed from this Dir object via openNextFile.
Note that the SD and SDFS filesystems only support a filesystem-wide callback and
calls to  ``Dir::setTimeCallback`` may produce unexpected behavior.

File object
-----------

``SDFS/LittleFS.open()`` and ``dir.openFile()`` functions return a *File* object.
This object supports all the functions of *Stream*, so you can use
``readBytes``, ``findUntil``, ``parseInt``, ``println``, and all other
*Stream* methods.

There are also some functions which are specific to *File* object.

seek
~~~~

.. code:: cpp

    file.seek(offset, mode)

This function behaves like ``fseek`` C function. Depending on the value
of ``mode``, it moves current position in a file as follows:

-  if ``mode`` is ``SeekSet``, position is set to ``offset`` bytes from
   the beginning.
-  if ``mode`` is ``SeekCur``, current position is moved by ``offset``
   bytes.
-  if ``mode`` is ``SeekEnd``, position is set to ``offset`` bytes from
   the end of the file.

Returns *true* if position was set successfully.

position
~~~~~~~~

.. code:: cpp

    file.position()

Returns the current position inside the file, in bytes.

size
~~~~

.. code:: cpp

    file.size()

Returns file size, in bytes.

name
~~~~

.. code:: cpp

    String name = file.name();

Returns short (no-path) file name, as ``const char*``. Convert it to *String* for
storage.

fullName
~~~~~~~~

.. code:: cpp

    // Filesystem:
    //   testdir/
    //           file1
    Dir d = LittleFS.openDir("testdir/");
    File f = d.openFile("r");
    // f.name() == "file1", f.fullName() == "testdir/file1"

Returns the full path file name as a ``const char*``.

getLastWrite
~~~~~~~~~~~~

Returns the file last write time, and only valid for files opened in read-only
mode.  If a file is opened for writing, the returned time may be indeterminate.

getCreationTime
~~~~~~~~~~~~~~~

Returns the file creation time, if available.

isFile
~~~~~~

.. code:: cpp

    bool amIAFile = file.isFile();

Returns *true* if this File points to a real file.

isDirectory
~~~~~~~~~~~

.. code:: cpp

    bool amIADir = file.isDir();

Returns *true* if this File points to a directory (used for emulation
of the SD.* interfaces with the ``openNextFile`` method).

close
~~~~~

.. code:: cpp

    file.close()

Close the file. No other operations should be performed on *File* object
after ``close`` function was called.

openNextFile  (compatibility method, not recommended for new code)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: cpp

    File root = LittleFS.open("/");
    File file1 = root.openNextFile();
    File file2 = root.openNextFile();

Opens the next file in the directory pointed to by the File.  Only valid
when ``File.isDirectory() == true``.

rewindDirectory  (compatibility method, not recommended for new code)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: cpp

    File root = LittleFS.open("/");
    File file1 = root.openNextFile();
    file1.close();
    root.rewindDirectory();
    file1 = root.openNextFile(); // Opens first file in dir again

Resets the ``openNextFile`` pointer to the top of the directory.  Only
valid when ``File.isDirectory() == true``.

setTimeCallback(time_t (\*cb)(void))
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sets the time callback for this specific file.  Note that the SD and
SDFS filesystems only support a filesystem-wide callback and calls to
``Dir::setTimeCallback`` may produce unexpected behavior.
