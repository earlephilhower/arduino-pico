Using this core with PlatformIO
===============================

What is PlatformIO? 
-------------------

`PlatformIO <https://platformio.org/>`__  is a free, open-source build-tool written in Python, which also integrates into VSCode code as an extension.

PlatformIO significantly simplifies writing embedded software by offering a unified build system, yet being able to create project files for many different IDEs, including VSCode, Eclipse, CLion, etc. 
Through this, PlatformIO can offer extensive features such as IntelliSense (autocomplete), debugging, unit testing etc., which not available in the standard Arduino IDE.

The Arduino IDE experience:

.. image:: images/the_arduinoide_experience.png

The PlatformIO experience:

.. image:: images/the_platformio_experience.png

Refer to the general documentation at https://docs.platformio.org/.

Especially useful is the `Getting started with VSCode + PlatformIO <https://docs.platformio.org/en/latest/integration/ide/vscode.html#installation>`_, `CLI reference <https://docs.platformio.org/en/latest/core/index.html>`_ and the `platformio.ini options <https://docs.platformio.org/en/latest/projectconf/index.html>`_ page.

Hereafter it is assumed that you have a basic understanding of PlatformIO in regards to project creation, project file structure and building and uploading PlatformIO projects, through reading the above pages.

Current state of development
----------------------------

At the time of writing, PlatformIO integration for this core is a work-in-progress and not yet merged into mainline PlatformIO. This is subject to change once `this pull request <https://github.com/platformio/platform-raspberrypi/pull/36>`_ is merged.

If you want to use the PlatformIO integration right now, make sure you first create a standard Raspberry Pi Pico + Arduino project within PlatformIO. 
This will give you a project with the ``platformio.ini`` 

.. code:: ini

    [env:pico]
    platform = raspberrypi
    board = pico
    framework = arduino

Here, you need to change the `platform` to take advantage of the features described hereunder and switch to the new core.

.. code:: ini

    [env:pico]
    platform = https://github.com/maxgerhardt/platform-raspberrypi.git
    board = pico
    framework = arduino
    board_build.core = earlephilhower
    
When the support for this core has been merged into mainline PlatformIO, this notice will be removed and a standard `platformio.ini` as shown above will work as a base.

Deprecation warnings
---------------------

Previous versions of this documentation told users to inject the framework and toolchain package into the project by using

.. code:: ini

    ; note that download link for toolchain is specific for OS. see https://github.com/earlephilhower/pico-quick-toolchain/releases.
    platform_packages = 
        maxgerhardt/framework-arduinopico@https://github.com/earlephilhower/arduino-pico.git
        maxgerhardt/toolchain-pico@https://github.com/earlephilhower/pico-quick-toolchain/releases/download/1.3.1-a/x86_64-w64-mingw32.arm-none-eabi-7855b0c.210706.zip

This is now **deprecated** and should not be done anymore. Users should delete these ``platform_packages`` lines and update the platform integration by issuing the command

.. code:: bash

    pio pkg update -g -p https://github.com/maxgerhardt/platform-raspberrypi.git

in the `PlatformIO CLI <https://docs.platformio.org/en/latest/integration/ide/vscode.html#platformio-core-cli>`_. The same can be achieved by using the VSCode PIO Home -> Platforms -> Updates GUI.

The toolchain, which was also renamed to ``toolchain-rp2040-earlephilhower`` is downloaded automatically from the registry. The same goes for the ``framework-arduinopico`` toolchain package, which points directly to the Arduino-Pico Github repository.
However, users can still select a custom fork or branch of the core if desired so, as detailed in a chapter below.

Selecting the new core
----------------------

Prerequisite for using this core is to tell PlatformIO to switch to it.
There will be board definition files where the Earle-Philhower core will
be the default since it's a board that only exists in this core (and not
the other https://github.com/arduino/ArduinoCore-mbed). To switch boards
for which this is not the default core (which are only
``board = pico`` and ``board = nanorp2040connect``), the directive

.. code:: ini

    board_build.core = earlephilhower

must be added to the ``platformio.ini``. This controls the `core
switching
logic <https://github.com/maxgerhardt/platform-raspberrypi/blob/77e0d3a29d1dbf00fd3ec3271104e3bf4820869c/builder/frameworks/arduino/arduino.py#L27-L32>`__.

When using Arduino-Pico-only boards like ``board = rpipico`` or ``board = adafruit_feather``, this is not needed.

Flash size
----------

Controlled via specifying the size allocated for the filesystem.
Available sketch size is calculated accordingly by using (as in
``makeboards.py``) that number and the (constant) EEPROM size (4096
bytes) and the total flash size as known to PlatformIO via the board
definition file. The expression on the right can involve "b","k","m"
(bytes/kilobytes/megabytes) and floating point numbers. This makes it
actually more flexible than in the Arduino IDE where there is a finite
list of choices. Calculations happen in `the
platform <https://github.com/maxgerhardt/platform-raspberrypi/blob/77e0d3a29d1dbf00fd3ec3271104e3bf4820869c/builder/main.py#L118-L184>`__.

.. code:: ini

    ; in reference to a board = pico config (2MB flash)
    ; Flash Size: 2MB (Sketch: 1MB, FS:1MB)
    board_build.filesystem_size = 1m
    ; Flash Size: 2MB (No FS)
    board_build.filesystem_size = 0m
    ; Flash Size: 2MB (Sketch: 0.5MB, FS:1.5MB)
    board_build.filesystem_size = 1.5m

CPU Speed
---------

As for all other PlatformIO platforms, the ``f_cpu`` macro value (which
is passed to the core) can be changed as
`documented <https://docs.platformio.org/en/latest/boards/raspberrypi/pico.html#configuration>`__

.. code:: ini

    ; 133MHz
    board_build.f_cpu = 133000000L

Debug Port
----------

Via
`build_flags <https://docs.platformio.org/en/latest/projectconf/section_env_build.html#build-flags>`__
as done for many other cores
(`example <https://docs.platformio.org/en/latest/platforms/ststm32.html#configuration>`__).

.. code:: ini

    ; Debug Port: Serial
    build_flags = -DDEBUG_RP2040_PORT=Serial
    ; Debug Port: Serial 1
    build_flags = -DDEBUG_RP2040_PORT=Serial1
    ; Debug Port: Serial 2
    build_flags = -DDEBUG_RP2040_PORT=Serial2

Debug Level
-----------

Done again by directly adding the needed `build
flags <https://github.com/earlephilhower/arduino-pico/blob/05356da2c5552413a442f742e209c6fa92823666/boards.txt#L104-L114>`__.
When wanting to define multiple build flags, they must be accumulated in
either a sing line or a newline-separated expression.

.. code:: ini

    ; Debug level: Core
    build_flags = -DDEBUG_RP2040_CORE
    ; Debug level: SPI
    build_flags = -DDEBUG_RP2040_SPI
    ; Debug level: Wire
    build_flags = -DDEBUG_RP2040_WIRE
    ; Debug level: All
    build_flags = -DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE
    ; Debug level: NDEBUG
    build_flags = -DNDEBUG

    ; example: Debug port on serial 2 and all debug output
    build_flags = -DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE -DDEBUG_RP2040_PORT=Serial2
    ; equivalent to above
    build_flags = 
       -DDEBUG_RP2040_WIRE
       -DDEBUG_RP2040_SPI
       -DDEBUG_RP2040_CORE
       -DDEBUG_RP2040_PORT=Serial2

C++ Exceptions
--------------

Exceptions are disabled by default. To enable them, use

.. code:: ini

    ; Enable Exceptions
    build_flags = -DPIO_FRAMEWORK_ARDUINO_ENABLE_EXCEPTIONS

Stack Protector
---------------

To enable GCC's stack protection feature, use

.. code:: ini

    ; Enable Stack Protector
    build_flags = -fstack-protector


RTTI
----

RTTI (run-time type information) is disabled by default. To enable it, use

.. code:: ini

    ; Enable RTTI
    build_flags = -DPIO_FRAMEWORK_ARDUINO_ENABLE_RTTI

USB Stack
---------

Not specifying any special build flags regarding this gives one the
default Pico SDK USB stack. To change it, add

.. code:: ini

    ; Adafruit TinyUSB
    build_flags = -DUSE_TINYUSB
    ; No USB stack
    build_flags = -DPIO_FRAMEWORK_ARDUINO_NO_USB

Note that the special "No USB" setting is also supported, through the
shortcut-define ``PIO_FRAMEWORK_ARDUINO_NO_USB``.


Selecting a different core version
----------------------------------

If you wish to use a different version of the core, e.g., the latest git
``master`` version, you can use a
`platform_packages <https://docs.platformio.org/en/latest/projectconf/section_env_platform.html#platform-packages>`__
directive to do so. Simply specify that the framework package
(``framework-arduinopico``) comes from a different source.

.. code:: ini

    platform_packages =
       framework-arduinopico@https://github.com/earlephilhower/arduino-pico.git#master

Whereas the ``#master`` can also be replaced by a ``#branchname`` or a
``#commithash``. If left out, it will pull the default branch, which is ``master``.

The ``file://`` and ``symlink://`` pseudo-protocols can also be used instead of ``https://`` to point to a
local copy of the core (with e.g. some modifications) on disk (`see documentation <https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_install.html?#local-folder>`_).

Note that this can only be done for versions that have the PlatformIO
builder script it in, so versions before 1.9.2 are not supported.

Examples 
--------

The following example ``platformio.ini`` can be used for a Raspberry Pi Pico
and 0.5MByte filesystem. 

.. code:: ini

    [env:pico]
    platform = https://github.com/maxgerhardt/platform-raspberrypi.git
    board = pico
    framework = arduino
    ; board can use both Arduino cores -- we select Arduino-Pico here
    board_build.core = earlephilhower
    board_build.filesystem_size = 0.5m


The initial project structure should be generated just creating a new
project for the Pico and the Arduino framework, after which the
auto-generated ``platformio.ini`` can be adapted per above.

Debugging
---------

With recent updates to the toolchain and OpenOCD, debugging firmwares is also possible.

To specify the debugging adapter, use ``debug_tool`` (`documentation <https://docs.platformio.org/en/latest/projectconf/section_env_debug.html#debug-tool>`_). Supported values are:

* ``picoprobe``
* ``cmsis-dap``
* ``jlink``
* ``raspberrypi-swd``

These values can also be used in ``upload_protocol`` if you want PlatformIO to upload the regular firmware through this method, which you likely want.

Especially the PicoProbe method is convenient when you have two Raspberry Pi Pico boards. One of them can be flashed with the PicoProbe firmware (`documentation <https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html#debugging-using-another-raspberry-pi-pico>`_) and is then connected to the target Raspberry Pi Pico board (see `documentation <https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf>`_ chapter "Picoprobe Wiring"). Remember that on Windows, you have to use `Zadig <https://zadig.akeo.ie/>`_ to also load "WinUSB" drivers for the "Picoprobe (Interface 2)" device so that OpenOCD can speak to it.

With that set up, debugging can be started via the left debugging sidebar and works nicely: Setup breakpoints, inspect the value of variables in the code, step through the code line by line. When a breakpoint is hit or execution is halted, you can even see the execution state both Cortex-M0+ cores of the RP2040.

.. image:: images/pio_debugging.png

For further information on customizing debug options, like the initial breakpoint or debugging / SWD speed, consult `the documentation <https://docs.platformio.org/en/latest/projectconf/section_env_debug.html>`_.

Filesystem Uploading
--------------------

For the Arduino IDE, `a plugin <https://github.com/earlephilhower/arduino-pico#uploading-filesystem-images>`_ is available that enables a data folder to be packed as a LittleFS filesystem binary and uploaded to the Pico.

This functionality is also built-in in the PlatformIO integration. Open the `project tasks <https://docs.platformio.org/en/latest/integration/ide/vscode.html#project-tasks>`_ and expand the "Platform" tasks: 

.. image:: images/pio_fs_upload.png

The files you want to upload should be placed in a folder called ``data`` inside the project. This can be customized `if needed <https://docs.platformio.org/en/latest/projectconf/section_platformio.html#data-dir>`_.

The task "Build Filesystem Image" will take all files in the data directory and create a ``littlefs.bin`` file from it using the ``mklittlefs`` tool.

The task "Upload Filesystem Image" will upload the filesystem image to the Pico via the specified ``upload_protocol``. 
