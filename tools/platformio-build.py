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

import os
from SCons.Script import DefaultEnvironment, Builder, AlwaysBuild

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

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
        "-fno-exceptions",
        "-fno-rtti",
        "-iprefix" + os.path.join(FRAMEWORK_DIR),
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "platform_inc.txt")
    ],

    CFLAGS=[
        "-std=gnu17"
    ],

    CXXFLAGS=[
        "-std=gnu++17"
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
        #"-Wl,--cref",
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
    if "USE_TINYUSB" in cpp_defines:
        env.Append(CPPPATH=[os.path.join(
            FRAMEWORK_DIR, "libraries", "Adafruit_TinyUSB_Arduino", "src", "arduino")])
    else:
        # standard Pico SDK USB stack used.
        env.Append(CPPPATH=[os.path.join(FRAMEWORK_DIR, "tools", "libpico")])
    # in any case, add standard flags
    env.Append(CPPDEFINES=[
        ("CFG_TUSB_MCU", "OPT_MCU_RP2040"),
        ("USB_VID", board.get("build.hwids", [[0, 0]])[0][0]),
#        ("USB_PID", "0x000a"),
        ("USB_PID", board.get("build.hwids", [[0, 0]])[0][1]),
        ("USB_MANUFACTURER", '\\"%s\\"' % board.get("vendor", "Raspberry Pi")),
        # ToDo: Add info to board manifest
        ("USB_PRODUCT", '\\"%s\\"' % board.get("build.usb_product", "Pico")),
        ("SERIALUSB_PID", board.get("build.hwids", [[0, 0]])[0][1])
#        ("SERIALUSB_PID", "0x000a")
    ])

#
# Process configuration flags
#


cpp_defines = env.Flatten(env.get("CPPDEFINES", []))

configure_usb_flags(cpp_defines)

# ToDo: Figure out how we can get the values of __FLASH_LENGTH__ etc 
# and replace hardcoded values below.
def get_sketch_partition_info(env):
    pass

linkerscript_cmd = env.Command(
    os.path.join("$BUILD_DIR", "memmap_default.ld"), # $TARGET
    os.path.join(FRAMEWORK_DIR, "lib", "memmap_default.ld") , # $SOURCE 
    env.VerboseAction(" ".join([
        '"$PYTHONEXE" "%s"' % os.path.join(FRAMEWORK_DIR, "tools", "simplesub.py"),
        "--input", "$SOURCE",
        "--out", "$TARGET",
        "--sub", "__FLASH_LENGTH__", "2093056",
        "--sub", "__EEPROM_START__", "270528512",
        "--sub", "__FS_START__", "270528512",
        "--sub", "__FS_END__", "270528512",
        "--sub", "__RAM_LENGTH__", "256k",
    ]), "Generating linkerscript $BUILD_DIR/memmap_default.ld")
)

# if no custom linker script is provided, we use the command that we prepared to generate one.
if not board.get("build.ldscript", ""):
    env.Depends("$BUILD_DIR/${PROGNAME}.elf", linkerscript_cmd)
    env.Replace(LDSCRIPT_PATH=os.path.join("$BUILD_DIR", "memmap_default.ld"))

libs = []

if "build.variant" in board:
    env.Append(CPPPATH=[
        os.path.join(FRAMEWORK_DIR, "variants", board.get("build.variant"))
    ])

    libs.append(
        env.BuildLibrary(
            os.path.join("$BUILD_DIR", "FrameworkArduinoVariant"),
            os.path.join(FRAMEWORK_DIR, "variants", board.get("build.variant"))))

libs.append(
    env.BuildLibrary(
        os.path.join("$BUILD_DIR", "FrameworkArduino"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040")))

bootloader_src_file = board.get(
    "build.arduino.boot2_source", "boot2_generic_03h_2_padded_checksum.S")

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
