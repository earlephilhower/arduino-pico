# Copyright 2021-present Maximilian Gerhardt <maximilian.gerhardt@rub.de>
# TinyUSB ignore snippet from https://github.com/episource/
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os, re, sys
from SCons.Script import DefaultEnvironment, Builder, AlwaysBuild

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()
chip = board.get("build.mcu")
upload_protocol = env.subst("$UPLOAD_PROTOCOL") or "picotool"
ram_size = int(board.get("upload.maximum_ram_size"))
psram_len = int(board.get("upload.psram_length", "0"))

FRAMEWORK_DIR = platform.get_package_dir("framework-arduinopico")
assert os.path.isdir(FRAMEWORK_DIR)

# read includes from this file to add them into CPPPATH later for good IDE intellisense
# will use original -iprefix <prefix> @<file> for compilation though.
includes_file = os.path.join(FRAMEWORK_DIR, "lib", chip, "platform_inc.txt")
file_lines = []
includes = []
with open(includes_file, "r") as fp:
    file_lines = fp.readlines()
includes_file = os.path.join(FRAMEWORK_DIR, "lib", "core_inc.txt")
with open(includes_file, "r") as fp:
    file_lines = file_lines + fp.readlines()
for l in file_lines:
    path = l.strip().replace("-iwithprefixbefore/", "").replace("/", os.sep)
    # emulate -iprefix <framework path>.
    path = os.path.join(FRAMEWORK_DIR, path)
    # prevent non-existent paths from being added
    if os.path.isdir(path):
        includes.append(path)

def is_pio_build():
	from SCons.Script import COMMAND_LINE_TARGETS
	return all([x not in COMMAND_LINE_TARGETS for x in ["idedata", "_idedata", "__idedata"]])

# get all activated macros
flatten_cppdefines = env.Flatten(env['CPPDEFINES'])

#
# Exceptions
#
stdcpp_lib = None
if "PIO_FRAMEWORK_ARDUINO_ENABLE_EXCEPTIONS" in flatten_cppdefines:
    env.Append(
        CXXFLAGS=["-fexceptions"]
    )
    stdcpp_lib = "stdc++-exc"
else:
    env.Append(
        CXXFLAGS=["-fno-exceptions"]
    )
    stdcpp_lib = "stdc++"

#
# RTTI
#
# standard is -fno-rtti flag. If special macro is present, don't 
# add that flag.
if not "PIO_FRAMEWORK_ARDUINO_ENABLE_RTTI" in flatten_cppdefines:
    env.Append(
        CXXFLAGS=["-fno-rtti"]
    )

# update progsize expression to also check for bootloader.
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.boot2|\.text|\.data|\.rodata|\.text.align|\.ARM.exidx)\s+(\d+).*"
)

# pico support library depends on ipv6 enable/disable
libpico = File(os.path.join(FRAMEWORK_DIR, "lib", chip, "libpico.a"))
if "PIO_FRAMEWORK_ARDUINO_ENABLE_BLUETOOTH" in flatten_cppdefines:
    if "PIO_FRAMEWORK_ARDUINO_ENABLE_IPV6" in flatten_cppdefines:
        libpicow = File(os.path.join(FRAMEWORK_DIR, "lib", chip, "libipv4-ipv6-bt.a"))
    else:
        libpicow = File(os.path.join(FRAMEWORK_DIR, "lib", chip, "libipv4-bt.a"))
    env.Append(
        CPPDEFINES=[
            ("ENABLE_CLASSIC", 1),
            ("ENABLE_BLE", 1)
        ]
    )
elif "PIO_FRAMEWORK_ARDUINO_ENABLE_IPV6" in flatten_cppdefines:
    libpicow = File(os.path.join(FRAMEWORK_DIR, "lib", chip, "libipv4-ipv6.a"))
else:
    libpicow = File(os.path.join(FRAMEWORK_DIR, "lib", chip, "libipv4.a"))

env.Append(
    ASFLAGS=env.get("CCFLAGS", [])[:],
)

if chip == "rp2040":
    env.Append(
        CPPDEFINES=[
            ("ARDUINO", 10810),
            "ARDUINO_ARCH_RP2040",
            ("F_CPU", "$BOARD_F_CPU"),
            ("BOARD_NAME", '\\"%s\\"' % env.subst("$BOARD")),
            "ARM_MATH_CM0_FAMILY",
            "ARM_MATH_CM0_PLUS",
            "TARGET_RP2040",
            ("PICO_RP2040", "1"),
            # at this point, the main.py builder script hasn't updated upload.maximum_size yet,
            # so it's the original value for the full flash.
            ("PICO_FLASH_SIZE_BYTES", board.get("upload.maximum_size"))
        ]
    )
elif (chip == "rp2350") or (chip == "rp2350-riscv"):
    env.Append(
        CPPDEFINES=[
            ("ARDUINO", 10810),
            "ARDUINO_ARCH_RP2040",
            ("F_CPU", "$BOARD_F_CPU"),
            ("BOARD_NAME", '\\"%s\\"' % env.subst("$BOARD")),
            ("CFG_TUSB_MCU", "OPT_MCU_RP2040"),
            ("CFG_TUSB_OS", "OPT_OS_PICO"),
            ("LIB_BOOT_STAGE2_HEADERS", "1"),
            ("LIB_PICO_ATOMIC", "1"),
            ("LIB_PICO_BIT_OPS", "1"),
            ("LIB_PICO_BIT_OPS_PICO", "1"),
            ("LIB_PICO_CLIB_INTERFACE", "1"),
            ("LIB_PICO_CRT0", "1"),
            ("LIB_PICO_CXX_OPTIONS", "1"),
            ("LIB_PICO_DIVIDER", "1"),
            ("LIB_PICO_DIVIDER_COMPILER", "1"),
            ("LIB_PICO_DOUBLE", "1"),
            ("LIB_PICO_FIX_RP2040_USB_DEVICE_ENUMERATION", "1"),
            ("LIB_PICO_FLOAT", "1"),
            ("LIB_PICO_INT64_OPS", "1"),
            ("LIB_PICO_INT64_OPS_COMPILER", "1"),
            ("LIB_PICO_MEM_OPS", "1"),
            ("LIB_PICO_MEM_OPS_COMPILER", "1"),
            ("LIB_PICO_NEWLIB_INTERFACE", "1"),
            ("LIB_PICO_PLATFORM", "1"),
            ("LIB_PICO_PLATFORM_COMPILER", "1"),
            ("LIB_PICO_PLATFORM_PANIC", "1"),
            ("LIB_PICO_PLATFORM_SECTIONS", "1"),
            ("LIB_PICO_RUNTIME", "1"),
            ("LIB_PICO_RUNTIME_INIT", "1"),
            ("LIB_PICO_STANDARD_BINARY_INFO", "1"),
            ("LIB_PICO_STANDARD_LINK", "1"),
            ("LIB_PICO_SYNC", "1"),
            ("LIB_PICO_SYNC_CRITICAL_SECTION", "1"),
            ("LIB_PICO_SYNC_MUTEX", "1"),
            ("LIB_PICO_SYNC_SEM", "1"),
            ("LIB_PICO_TIME", "1"),
            ("LIB_PICO_TIME_ADAPTER", "1"),
            ("LIB_PICO_UNIQUE_ID", "1"),
            ("LIB_PICO_UTIL", "1"),
            ("LIB_TINYUSB_BOARD", "1"),
            ("LIB_TINYUSB_DEVICE", "1"),
            ("PICO_32BIT", "1"),
            ("PICO_BOARD", '\\"pico2\\"'),
            ("PICO_BUILD", "1"),
            ("PICO_COPY_TO_RAM", "0"),
            ("PICO_CXX_ENABLE_EXCEPTIONS", "0"),
            ("PICO_NO_FLASH", "0"),
            ("PICO_NO_HARDWARE", "0"),
            ("PICO_ON_DEVICE", "1"),
            ("PICO_RP2040_USB_DEVICE_ENUMERATION_FIX", "1"),
            ("PICO_RP2040_USB_DEVICE_UFRAME_FIX", "1"),
            ("PICO_RP2350", "1"),
            ("PICO_USE_BLOCKED_RAM", "0"),
            "TARGET_RP2350",
            ("PICO_RP2350", "1"),
            # at this point, the main.py builder script hasn't updated upload.maximum_size yet,
            # so it's the original value for the full flash.
            ("PICO_FLASH_SIZE_BYTES", board.get("upload.maximum_size"))
        ]
    )
    if chip == "rp2350":
        env.Append(
            CPPDEFINES=[
                ("LIB_PICO_DOUBLE_PICO", "1"),
                ("LIB_PICO_FLOAT_PICO", "1"),
                ("LIB_PICO_FLOAT_PICO_VFP", "1")
            ]
        )
    elif chip == "rp2350-riscv":
        env.Append(
            CPPDEFINES=[
                ("LIB_PICO_DOUBLE_COMPILER", "1"),
                ("LIB_PICO_FLOAT_COMPILER", "1"),
                ("PICO_RISCV", "1")
            ]
        )

if chip == "rp2040":
    toolopts = ["-march=armv6-m", "-mcpu=cortex-m0plus", "-mthumb"]
elif chip == "rp2350":
    toolopts = ["-mcpu=cortex-m33", "-mthumb", "-march=armv8-m.main+fp+dsp", "-mfloat-abi=softfp", "-mcmse"]
elif chip == "rp2350-riscv":
    toolopts = ["-march=rv32imac_zicsr_zifencei_zba_zbb_zbs_zbkb", "-mabi=ilp32"]

env.Append(
    CCFLAGS=[
        "-Os", # Optimize for size by default
        "-Werror=return-type",
        "-Wno-psabi",
        "-ffunction-sections",
        "-fdata-sections",
        # -iprefix etc. added later if in build mode
    ] + toolopts,

    CFLAGS=[
        "-std=gnu17"
    ],

    CXXFLAGS=[
        "-std=gnu++17",
    ],

    CPPDEFINES=[
        # SdFat definitions required for SDFS
        ("FILE_COPY_CONSTRUCTOR_SELECT", "FILE_COPY_CONSTRUCTOR_PUBLIC"),
        ("USE_UTF8_LONG_NAMES", "1"),
        ("DISABLE_FS_H_WARNING", "1")
    ],

    CPPPATH=[
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040", "api", "deprecated"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040",
                     "api", "deprecated-avr-comp")
    ],

    LINKFLAGS=[
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", chip, "platform_wrap.txt"),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "core_wrap.txt"),
        # no cross-reference table, heavily spams the output
        # "-Wl,--cref",
        "-Wl,--no-warn-rwx-segments",
        "-Wl,--check-sections",
        "-Wl,--gc-sections",
        "-Wl,--unresolved-symbols=report-all",
        "-Wl,--warn-common",
        "-Wl,--undefined=runtime_init_install_ram_vector_table",
        "-Wl,--undefined=__pre_init_runtime_init_clocks",
        "-Wl,--undefined=__pre_init_runtime_init_bootrom_reset",
        "-Wl,--undefined=__pre_init_runtime_init_early_resets",
        "-Wl,--undefined=__pre_init_runtime_init_usb_power_down",
        "-Wl,--undefined=__pre_init_runtime_init_clocks",
        "-Wl,--undefined=__pre_init_runtime_init_post_clock_resets",
        "-Wl,--undefined=__pre_init_runtime_init_spin_locks_reset",
        "-Wl,--undefined=__pre_init_runtime_init_boot_locks_reset",
        "-Wl,--undefined=__pre_init_runtime_init_bootrom_locking_enable",
        "-Wl,--undefined=__pre_init_runtime_init_mutex",
        "-Wl,--undefined=__pre_init_runtime_init_default_alarm_pool",
        "-Wl,--undefined=__pre_init_first_per_core_initializer",
        "-Wl,--undefined=__pre_init_runtime_init_per_core_bootrom_reset"
        "-Wl,--undefined=__pre_init_runtime_init_per_core_h3_irq_registers",
        "-Wl,--undefined=__pre_init_runtime_init_per_core_irq_priorities"
    ] + toolopts,

    LIBSOURCE_DIRS=[os.path.join(FRAMEWORK_DIR, "libraries")],

    # do **NOT** Add lib to LIBPATH, otherwise 
    # erroneous libstdc++.a will be found that crashes! 
    #LIBPATH=[
    #    os.path.join(FRAMEWORK_DIR, "lib")
    #],

    # link lib/libpico.a by full path, ignore libstdc++
    LIBS=[
        File(os.path.join(FRAMEWORK_DIR, "lib", chip, "ota.o")),
        libpico,
        libpicow,
        File(os.path.join(FRAMEWORK_DIR, "lib", chip, "libbearssl.a")),
        "m", "c", stdcpp_lib, "c"]
)


# expand with read includes for IDE, but use -iprefix command for actual building
if not is_pio_build():
    env.Append(CPPPATH=includes)
else:
    env.Append(CCFLAGS=[
        "-iprefix" + os.path.join(FRAMEWORK_DIR),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", chip, "platform_inc.txt"),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "core_inc.txt")
    ])
    env.Append(ASFLAGS=[
        "-iprefix" + os.path.join(FRAMEWORK_DIR),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", chip, "platform_inc.txt"),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "core_inc.txt")
    ] + toolopts)

def configure_usb_flags(cpp_defines):
    if "USE_TINYUSB" in cpp_defines:
        env.Append(CPPPATH=[os.path.join(
            FRAMEWORK_DIR, "libraries", "Adafruit_TinyUSB_Arduino", "src", "arduino")])
        # automatically build with lib_archive = no to make weak linking work, needed for TinyUSB
        env_section = "env:" + env["PIOENV"]
        platform.config.set(env_section, "lib_archive", False)
    elif "PIO_FRAMEWORK_ARDUINO_NO_USB" in cpp_defines:
        env.Append(
            CPPDEFINES=[
                "NO_USB",
                "DISABLE_USB_SERIAL" 
            ]
        )
        # do not further add more USB flags or update sizes. no USB used.
        return
    else:
        # standard Pico SDK USB stack used, will get include path later on
        pass
    # in any case, add standard flags
    # preferably use USB information from arduino.earlephilhower section,
    # but fallback to sensible values derived from other parts otherwise.
    usb_pid = board.get("build.arduino.earlephilhower.usb_pid",
                        board.get("build.hwids", [[0, 0]])[0][1])
    usb_vid = board.get("build.arduino.earlephilhower.usb_vid",
                        board.get("build.hwids", [[0, 0]])[0][0])
    usb_manufacturer = board.get(
        "build.arduino.earlephilhower.usb_manufacturer", board.get("vendor", "Raspberry Pi"))
    usb_product = board.get(
        "build.arduino.earlephilhower.usb_product", board.get("name", "Pico"))

    # Copy logic from makeboards.py. 
    # Depending on whether a certain upload / debug method is used, change
    # the PID/VID.
    # https://github.com/earlephilhower/arduino-pico/blob/master/tools/makeboards.py
    vidtouse = usb_vid
    pidtouse = usb_pid
    if upload_protocol == "picoprobe": 
        pidtouse = '0x0004'

    env.Append(CPPDEFINES=[
        ("CFG_TUSB_MCU", "OPT_MCU_RP2040"),
        # used by TinyUSB stack
        ("USB_VID", usb_vid),
        ("USB_PID", usb_pid),
        # Used by native USB stack
        ("USBD_VID", usb_vid),
        ("USBD_PID", usb_pid),
        # Used by both stacks
        ("USB_MANUFACTURER", '\\"%s\\"' % usb_manufacturer),
        ("USB_PRODUCT", '\\"%s\\"' % usb_product)
    ])

    if "USBD_MAX_POWER_MA" not in env.Flatten(env.get("CPPDEFINES", [])):
        env.Append(CPPDEFINES=[("USBD_MAX_POWER_MA", 500)])
        print("Warning: Undefined USBD_MAX_OWER_MA, assuming 500mA")

    # use vidtouse and pidtouse 
    # for USB PID/VID autodetection
    hw_ids = board.get("build.hwids", [["0x2E8A", "0x00C0"]])
    hw_ids[0][0] = vidtouse
    hw_ids[0][1] = pidtouse
    board.update("build.hwids", hw_ids)

def configure_network_flags(cpp_defines):
    env.Append(CPPDEFINES=[
        ("PICO_CYW43_ARCH_THREADSAFE_BACKGROUND", 1),
        ("CYW43_LWIP", 1),
        ("CYW43_PIO_CLOCK_DIV_DYNAMIC", 1),
        ("LWIP_IPV4", 1),
        ("LWIP_IGMP", 1),
        ("LWIP_CHECKSUM_CTRL_PER_NETIF", 1)
    ])
    if "PIO_FRAMEWORK_ARDUINO_ENABLE_IPV6" in cpp_defines:
        env.Append(CPPDEFINES=[("LWIP_IPV6", 1)])
    else:
        env.Append(CPPDEFINES=[("LWIP_IPV6", 0)])

#
# Process configuration flags
#
cpp_defines = env.Flatten(env.get("CPPDEFINES", []))

# Ignore TinyUSB automatically if not active without requiring ldf_mode = chain+
if not "USE_TINYUSB" in cpp_defines:
    env_section = "env:" + env["PIOENV"]
    ignored_libs = platform.config.get(
            env_section, "lib_ignore", []
        )
    if not "Adafruit TinyUSB Library" in ignored_libs:
        ignored_libs.append("Adafruit TinyUSB Library")
    platform.config.set(
            env_section, "lib_ignore", ignored_libs
        )
# configure USB stuff
configure_usb_flags(cpp_defines)
configure_network_flags(cpp_defines)

# ensure LWIP headers are in path after any TINYUSB distributed versions, also PicoSDK USB path headers
env.Append(CPPPATH=[os.path.join(FRAMEWORK_DIR, "include")])

# info about the filesystem is already parsed by the platform's main.py 
# script. We can just use the info here
 
linkerscript_cmd = env.Command(
    os.path.join("$BUILD_DIR", "memmap_default.ld"),  # $TARGET
    os.path.join(FRAMEWORK_DIR, "lib", chip, "memmap_default.ld"),  # $SOURCE
    env.VerboseAction(" ".join([
        '"$PYTHONEXE" "%s"' % os.path.join(
            FRAMEWORK_DIR, "tools", "simplesub.py"),
        "--input", "$SOURCE",
        "--out", "$TARGET",
        "--sub", "__FLASH_LENGTH__", "$PICO_FLASH_LENGTH",
        "--sub", "__EEPROM_START__", "$PICO_EEPROM_START",
        "--sub", "__FS_START__", "$FS_START",
        "--sub", "__FS_END__", "$FS_END",
        "--sub", "__RAM_LENGTH__", "%dk" % (ram_size // 1024),
        "--sub", "__PSRAM_LENGTH__", "%d" % (psram_len)
    ]), "Generating linkerscript $BUILD_DIR/memmap_default.ld")
)

# if no custom linker script is provided, we use the command that we prepared to generate one.
if not board.get("build.ldscript", ""):
    # execute fetch filesystem info stored in env to always have that info ready
    env["__fetch_fs_size"](env)
    env.Depends("$BUILD_DIR/${PROGNAME}.elf", linkerscript_cmd)
    env.Replace(LDSCRIPT_PATH=os.path.join("$BUILD_DIR", "memmap_default.ld"))

libs = []

variant = board.get("build.arduino.earlephilhower.variant", board.get("build.variant", ""))

# The following 3 lines are adapted from espressif/arduino-esp32, also licensed under apache2
variants_dir = os.path.join(FRAMEWORK_DIR, "variants")

if "build.variants_dir" in board:
    variants_dir = os.path.join("$PROJECT_DIR", board.get("build.variants_dir"))

if variant != "":
    env.Append(CPPPATH=[
        os.path.join(variants_dir, variant)
    ])

    env.Append(CPPDEFINES=[
        ("ARDUINO_VARIANT", '\\"' + variant + '\\"'),
    ])


    # link variant's source files as object files into the binary.
    # otherwise weak function overriding won't work in the linking stage.
    env.BuildSources(
        os.path.join("$BUILD_DIR", "FrameworkArduinoVariant"),
        os.path.join(variants_dir, variant))

libs.append(
    env.BuildLibrary(
        os.path.join("$BUILD_DIR", "FrameworkArduino"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040")))

bootloader_src_file = board.get(
    "build.arduino.earlephilhower.boot2_source", "boot2_generic_03h_2_padded_checksum.S")

# Add bootloader file (boot2.o)
# Only build the needed .S file, exclude all others via src_filter.
env.BuildSources(
    os.path.join("$BUILD_DIR", "FrameworkArduinoBootloader"),
    os.path.join(FRAMEWORK_DIR, "boot2", chip),
    "-<*> +<%s>" % bootloader_src_file,
)
# Add include flags for all .S assembly file builds
env.Append(
    ASFLAGS=[
        "-I", os.path.join(FRAMEWORK_DIR, "pico-sdk", "src",
                           chip, "hardware_regs", "include"),
        "-I", os.path.join(FRAMEWORK_DIR, "pico-sdk", "src",
                           "common", "pico_binary_info", "include")
    ]
)

env.Prepend(LIBS=libs)
