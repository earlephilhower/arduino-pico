# Copyright 2021-present Maximilian Gerhardt <maximilian.gerhardt@rub.de>
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
upload_protocol = env.subst("$UPLOAD_PROTOCOL") or "picotool"
ram_size = board.get("upload.maximum_ram_size")

FRAMEWORK_DIR = platform.get_package_dir("framework-arduinopico")
assert os.path.isdir(FRAMEWORK_DIR)

# update progsize expression to also check for bootloader.
env.Replace(
    SIZEPROGREGEXP=r"^(?:\.boot2|\.text|\.data|\.rodata|\.text.align|\.ARM.exidx)\s+(\d+).*"
)

env.Append(
    ASFLAGS=env.get("CCFLAGS", [])[:],

    CCFLAGS=[
        "-Werror=return-type",
        "-march=armv6-m",
        "-mcpu=cortex-m0plus",
        "-mthumb",
        "-ffunction-sections",
        "-fdata-sections",
        "-iprefix" + os.path.join(FRAMEWORK_DIR),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "platform_inc.txt")
    ],

    CFLAGS=[
        "-std=gnu17"
    ],

    CXXFLAGS=[
        "-std=gnu++17",
        "-fno-exceptions",
        "-fno-rtti",
    ],

    CPPDEFINES=[
        ("ARDUINO", 10810),
        "ARDUINO_ARCH_RP2040",
        ("F_CPU", "$BOARD_F_CPU"),
        ("BOARD_NAME", '\\"%s\\"' % env.subst("$BOARD")),
    ],

    CPPPATH=[
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040", "api", "deprecated"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040",
                     "api", "deprecated-avr-comp")
    ],

    LINKFLAGS=[
        "-march=armv6-m",
        "-mcpu=cortex-m0plus",
        "-mthumb",
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "platform_wrap.txt"),
        "-u_printf_float",
        "-u_scanf_float",
        # no cross-reference table, heavily spams the output
        # "-Wl,--cref",
        "-Wl,--check-sections",
        "-Wl,--gc-sections",
        "-Wl,--unresolved-symbols=report-all",
        "-Wl,--warn-common"
    ],

    LIBSOURCE_DIRS=[os.path.join(FRAMEWORK_DIR, "libraries")],

    LIBPATH=[
        os.path.join(FRAMEWORK_DIR, "lib")
    ],

    # link lib/libpico.a
    LIBS=["pico", "m", "c", "stdc++", "c"]
)


def configure_usb_flags(cpp_defines):
    global ram_size
    if "USE_TINYUSB" in cpp_defines:
        env.Append(CPPPATH=[os.path.join(
            FRAMEWORK_DIR, "libraries", "Adafruit_TinyUSB_Arduino", "src", "arduino")])
    elif "PIO_FRAMEWORK_ARDUINO_NO_USB" in cpp_defines:
        env.Append(
            CPPPATH=[os.path.join(FRAMEWORK_DIR, "tools", "libpico")],
            CPPDEFINES=[
                "NO_USB",
                "DISABLE_USB_SERIAL" 
            ]
        )
        # do not further add more USB flags or update sizes. no USB used.
        return
    else:
        # standard Pico SDK USB stack used.
        env.Append(CPPPATH=[os.path.join(FRAMEWORK_DIR, "tools", "libpico")])
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
    elif upload_protocol == "picodebug":
        vidtouse = '0x1209'
        pidtouse = '0x2488'
        ram_size = 240 * 1024

    env.Append(CPPDEFINES=[
        ("CFG_TUSB_MCU", "OPT_MCU_RP2040"),
        ("USB_VID", usb_vid),
        ("USB_PID", usb_pid),
        ("USB_MANUFACTURER", '\\"%s\\"' % usb_manufacturer),
        ("USB_PRODUCT", '\\"%s\\"' % usb_product),
        ("SERIALUSB_PID", usb_pid),
        ("USBD_MAX_POWER_MA", 250)
    ])

    # use vidtouse and pidtouse 
    # for USB PID/VID autodetection
    hw_ids = board.get("build.hwids", [["0x2E8A", "0x00C0"]])
    hw_ids[0][0] = vidtouse
    hw_ids[0][1] = pidtouse
    board.update("build.hwids", hw_ids)
    board.update("upload.maximum_ram_size", ram_size)

#
# Process configuration flags
#
cpp_defines = env.Flatten(env.get("CPPDEFINES", []))

configure_usb_flags(cpp_defines)

# info about the filesystem is already parsed by the platform's main.py 
# script. We can just use the info here
 
linkerscript_cmd = env.Command(
    os.path.join("$BUILD_DIR", "memmap_default.ld"),  # $TARGET
    os.path.join(FRAMEWORK_DIR, "lib", "memmap_default.ld"),  # $SOURCE
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
    ]), "Generating linkerscript $BUILD_DIR/memmap_default.ld")
)

# if no custom linker script is provided, we use the command that we prepared to generate one.
if not board.get("build.ldscript", ""):
    # execute fetch filesystem info stored in env to alawys have that info ready
    env["fetch_fs_size"](env)
    env.Depends("$BUILD_DIR/${PROGNAME}.elf", linkerscript_cmd)
    env.Replace(LDSCRIPT_PATH=os.path.join("$BUILD_DIR", "memmap_default.ld"))

libs = []

variant = board.get("build.arduino.earlephilhower.variant", board.get("build.variant", None))

if variant is not None:
    env.Append(CPPPATH=[
        os.path.join(FRAMEWORK_DIR, "variants", variant)
    ])

    libs.append(
        env.BuildLibrary(
            os.path.join("$BUILD_DIR", "FrameworkArduinoVariant"),
            os.path.join(FRAMEWORK_DIR, "variants", variant)))

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
    os.path.join(FRAMEWORK_DIR, "boot2"),
    "-<*> +<%s>" % bootloader_src_file,
)
# Add include flags for all .S assembly file builds
env.Append(
    ASFLAGS=[
        "-I", os.path.join(FRAMEWORK_DIR, "pico-sdk", "src",
                           "rp2040", "hardware_regs", "include"),
        "-I", os.path.join(FRAMEWORK_DIR, "pico-sdk", "src",
                           "common", "pico_binary_info", "include")
    ]
)

env.Prepend(LIBS=libs)
