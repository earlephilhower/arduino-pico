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
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

FRAMEWORK_DIR = platform.get_package_dir("framework-arduinopico")
assert os.path.isdir(FRAMEWORK_DIR)

# todo  -DCFG_TUSB_MCU=OPT_MCU_RP2040 -DUSB_VID=0x2e8a -DUSB_PID=0x000a "-DUSB_MANUFACTURER=\"Raspberry Pi\"" "-DUSB_PRODUCT=\"Pico\""
# -std=gnu++17 -g -DSERIALUSB_PID=0x000a
# -DARDUINO_RASPBERRY_PI_PICO "-DBOARD_NAME=\"RASPBERRY_PI_PICO\""

# todo
# # Compile the boot stage 2 blob
# recipe.hooks.linking.prelink.2.pattern="{compiler.path}{compiler.S.cmd}" {compiler.c.elf.flags} {compiler.c.elf.extra_flags} -c "{runtime.platform.path}/boot2/{build.boot2}.S" "-I{runtime.platform.path}/pico-sdk/src/rp2040/hardware_regs/include/" "-I{runtime.platform.path}/pico-sdk/src/common/pico_binary_info/include" -o "{build.path}/boot2.o"

env.Append(
    ASFLAGS=env.get("CCFLAGS", [])[:],

    CCFLAGS=[
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
    ],

    CPPPATH=[
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040", "api", "deprecated"),
        os.path.join(FRAMEWORK_DIR, "cores", "rp2040",
                     "api", "deprecated-avr-comp")
    ],

    LINKFLAGS=[
        "@%s" % os.path.join(FRAMEWORK_DIR, "lib", "platform_wrap.txt"),
        "-u _printf_float",
        "-u _scanf_float"
    ],

    LIBSOURCE_DIRS=[os.path.join(FRAMEWORK_DIR, "libraries")],

    LIBPATH=[
        os.path.join(FRAMEWORK_DIR, "lib")
    ],

    # link lib/libpico.a
    LIBS=["pico"]
)


def configure_usb_flags(cpp_defines):
    if "USE_TINYUSB" in cpp_defines:
        env.Append(CPPPATH=[os.path.join(
            FRAMEWORK_DIR, "libraries", "Adafruit_TinyUSB_Arduino", "src", "arduino")])
    else:
        # standard Pico SDK USB stack used.
        env.Append(CPPPATH=[os.path.join(FRAMEWORK_DIR, "tools", "libpico")])

#
# Process configuration flags
#


cpp_defines = env.Flatten(env.get("CPPDEFINES", []))

configure_usb_flags(cpp_defines)

# TODO:
# dynamically generate linker script as the arduino core does
#"C:\\Users\\Max\\Desktop\\Programming Stuff\\arduino-1.8.13\\hardware\\pico\\rp2040/system/python3/python3"
# "C:\\Users\\Max\\Desktop\\Programming Stuff\\arduino-1.8.13\\hardware\\pico\\rp2040/tools/simplesub.py"
# --input "C:\\Users\\Max\\Desktop\\Programming Stuff\\arduino-1.8.13\\hardware\\pico\\rp2040/lib/memmap_default.ld"
# --out "C:\\Users\\Max\\AppData\\Local\\Temp\\arduino_build_520211/memmap_default.ld"
# --sub __FLASH_LENGTH__ 2093056
# --sub __EEPROM_START__ 270528512
# --sub __FS_START__ 270528512
# --sub __FS_END__ 270528512
# --sub __RAM_LENGTH__ 256k

# if no custom linker script is provided, we use the Arduino core's standard one.
if not board.get("build.ldscript", ""):
    env.Replace(LDSCRIPT_PATH=os.path.join(
        FRAMEWORK_DIR, "lib", "memmap_default.ld"))

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

env.Prepend(LIBS=libs)
