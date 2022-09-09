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

    |---------------------|-------------|----|
    ^                     ^             ^
    Sketch                File system   EEPROM

The file system size is configurable via the IDE menus, rom 64k up to 15MB
(assuming you have an RP2040 boad with that much flash)

**Note:** to use any of file system functions in the sketch, add the
following include to the sketch:

.. code:: cpp

    #include "LittleFS.h" // LittleFS is declared
    // #include <SDFS.h>
    // #include <SD.h>


Compatible Filesystem APIs
--------------------------

LittleFS is an onboard filesystem that sets asidesome program flash for
use as a filesystem without requiring any external hardware.

SDFS is a filesystem for SD cards, based on [SdFat 2.0](https://github.com/earlephilhower/ESP8266SdFat).
It supports FAT16 and FAT32 formatted cards, and requires an external
SD card reader.

SD is the Arduino supported, somewhat old and limited SD card filesystem.
It is recommended to use SDFS for new applications instead of SD.

All three of these filesystems can open and manipulate ``File`` and ``Dir``
objects with the same code because the implement a common end-user
filesystem API.

LittleFS File System Limitations
--------------------------------

The LittleFS implementation for the RP2040 supports filenames of up
to 31 characters + terminating zero (i.e. ``char filename[32]``), and
as many subdirectories as space permits.

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

-  Download the tool: https://github.com/earlephilhower/arduino-pico-littlefs-plugin/releases
-  In your Arduino sketchbook directory, create ``tools`` directory if
   it doesn't exist yet.
-  Unpack the tool into ``tools`` directory (the path will look like
   ``<home_dir>/Arduino/tools/PicoLittleFS/tool/picolittlefs.jar``)
   If upgrading, overwrite the existing JAR file with the newer version.
-  Restart Arduino IDE.
-  Open a sketch (or create a new one and save it).
-  Go to sketch directory (choose Sketch > Show Sketch Folder).
-  Create a directory named ``data`` and any files you want in the file
   system there.
-  Make sure you have selected a board, port, and closed Serial Monitor.
-  Double check theSerial Monitor is closed.  Uploads will fail if the Serial
   Monitor has control of the serial port.
-  Select ``Tools > Pico LittleFS Data Upload``. This should start
   uploading the files into the flash file system.

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


File system object (LittleFS/SD/SDFS)
-------------------------------------

setConfig
~~~~~~~~~

.. code:: cpp

    LittleFSConfig cfg;
    cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);

    SDFSConfig c2;
    c2.setCSPin(12);
    SDFS.setConfig(c2);

This method allows you to configure the parameters of a filesystem
before mounting.  All filesystems have their own ``*Config`` (i.e.
``SDFSConfig`` or ``LittleFSConfig`` with their custom set of options.
All filesystems allow explicitly enabling/disabling formatting when
mounts fail.  If you do not call this ``setConfig`` method before
perforing ``begin()``, you will get the filesystem's default
behavior and configuration. By default, SPIFFS will autoformat the
filesystem if it cannot mount it, while SDFS will not.

begin
~~~~~

.. code:: cpp

    SDFS.begin()
    or LittleFS.begin()

This method mounts file system. It must be called before any
other FS APIs are used. Returns *true* if file system was mounted
successfully, false otherwise.  With no options it will format SPIFFS
if it is unable to mount it on the first try.

Note that LittleFS will automatically format the filesystem
if one is not detected.  This is configurable via ``setConfig``

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
mode. It can be one of "r", "w", "a", "r+", "w+", "a+". Meaning of these
modes is the same as for ``fopen`` C function.

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
Please note the previous discussion on the difference in behavior between
LittleFS and SPIFFS for this call.

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

info  **DEPRECATED**
~~~~~~~~~~~~~~~~~~~~

.. code:: cpp

    FSInfo fs_info;
    or LittleFS.info(fs_info);

Fills `FSInfo structure <#filesystem-information-structure>`__ with
information about the file system. Returns ``true`` if successful,
``false`` otherwise.  Because this cannot report information about
filesystemd greater than 4MB, don't use it in new code.  Use ``info64``
instead which uses 64-bit fields for filesystem sizes.

Filesystem information structure
--------------------------------

.. code:: cpp

    struct FSInfo {
        size_t totalBytes;
        size_t usedBytes;
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

info64
~~~~~~

.. code:: cpp

    FSInfo64 fsinfo;
    SDFS.info(fsinfo);
    or LittleFS(fsinfo);

Performs the same operation as ``info`` but allows for reporting greater than
4GB for filesystem size/used/etc.  Should be used with the SD and SDFS
filesystems since most SD cards today are greater than 4GB in size.

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

openNextFile  (compatibiity method, not recommended for new code)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code:: cpp

    File root = LittleFS.open("/");
    File file1 = root.openNextFile();
    File file2 = root.openNextFile();

Opens the next file in the directory pointed to by the File.  Only valid
when ``File.isDirectory() == true``.

rewindDirectory  (compatibiity method, not recommended for new code)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
