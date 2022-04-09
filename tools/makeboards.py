#!/usr/bin/env python3
import os
import sys

def BuildFlashMenu(name, flashsize, fssizelist):
    for fssize in fssizelist:
        if fssize == 0:
            fssizename = "no FS"
        elif fssize < 1024 * 1024:
            fssizename = "Sketch: %dKB, FS: %dKB" % ((flashsize - fssize) / 1024, fssize / 1024)
        else:
            fssizename = "Sketch: %dMB, FS: %dMB" % ((flashsize - fssize) / (1024 * 1024), fssize / (1024 * 1024))
        mn="%d_%d" % (flashsize, fssize)
        print("%s.menu.flash.%s=%dMB (%s)" % (name, mn, flashsize / (1024 * 1024), fssizename))
        print("%s.menu.flash.%s.upload.maximum_size=%d" % (name, mn, flashsize - 4096 - fssize))
        print("%s.menu.flash.%s.build.flash_length=%d" % (name, mn, flashsize - 4096 - fssize))
        print("%s.menu.flash.%s.build.eeprom_start=%d" % (name, mn, int("0x10000000",0) + flashsize - 4096))
        print("%s.menu.flash.%s.build.fs_start=%d" % (name, mn, int("0x10000000",0) + flashsize - 4096 - fssize))
        print("%s.menu.flash.%s.build.fs_end=%d" % (name, mn, int("0x10000000",0) + flashsize - 4096))

def BuildDebugPort(name):
    print("%s.menu.dbgport.Disabled=Disabled" % (name))
    print("%s.menu.dbgport.Disabled.build.debug_port=" % (name))
    for p in ["Serial", "Serial1", "Serial2"]:
        print("%s.menu.dbgport.%s=%s" % (name, p, p))
        print("%s.menu.dbgport.%s.build.debug_port=-DDEBUG_RP2040_PORT=%s" % (name, p, p))

def BuildDebugLevel(name):
    for l in [ ("None", ""), ("Core", "-DDEBUG_RP2040_CORE"), ("SPI", "-DDEBUG_RP2040_SPI"), ("Wire", "-DDEBUG_RP2040_WIRE"),
               ("All", "-DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE"), ("NDEBUG", "-DNDEBUG") ]:
        print("%s.menu.dbglvl.%s=%s" % (name, l[0], l[0]))
        print("%s.menu.dbglvl.%s.build.debug_level=%s" % (name, l[0], l[1]))

def BuildFreq(name):
    for f in [ 125, 50, 100, 133, 150, 175, 200, 225, 250, 275, 300]:
        warn = ""
        if f > 133: warn = " (Overclock)"
        print("%s.menu.freq.%s=%s MHz%s" % (name, f, f, warn))
        print("%s.menu.freq.%s.build.f_cpu=%dL" % (name, f, f * 1000000))

def BuildOptimize(name):
    for l in [ ("Small", "Small", "-Os", " (standard)"), ("Optimize", "Optimize", "-O", ""), ("Optimize2", "Optimize More", "-O2", ""),
               ("Optimize3", "Optimize Even More", "-O3", ""), ("Fast", "Fast", "-Ofast", " (maybe slower)"), ("Debug", "Debug", "-Og", "") ]:
        print("%s.menu.opt.%s=%s (%s)%s" % (name, l[0], l[1], l[2], l[3]))
        print("%s.menu.opt.%s.build.flags.optimize=%s" % (name, l[0], l[2]))

def BuildRTTI(name):
    print("%s.menu.rtti.Disabled=Disabled" % (name))
    print("%s.menu.rtti.Disabled.build.flags.rtti=-fno-rtti" % (name))
    print("%s.menu.rtti.Enabled=Enabled" % (name))
    print("%s.menu.rtti.Enabled.build.flags.rtti=" % (name))

def BuildBoot(name):
    for l in [ ("Generic SPI /2", "boot2_generic_03h_2_padded_checksum"),  ("Generic SPI /4", "boot2_generic_03h_4_padded_checksum"),
            ("IS25LP080 QSPI /2", "boot2_is25lp080_2_padded_checksum"), ("IS25LP080 QSPI /4", "boot2_is25lp080_4_padded_checksum"),
            ("W25Q080 QSPI /2", "boot2_w25q080_2_padded_checksum"), ("W25Q080 QSPI /4", "boot2_w25q080_4_padded_checksum"),
            ("W25X10CL QSPI /2", "boot2_w25x10cl_2_padded_checksum"), ("W25X10CL QSPI /4", "boot2_w25x10cl_4_padded_checksum") ]:
        print("%s.menu.boot2.%s=%s" % (name, l[1], l[0]))
        print("%s.menu.boot2.%s.build.boot2=%s" % (name, l[1], l[1]))

def BuildUSBStack(name):
    print("%s.menu.usbstack.picosdk=Pico SDK" % (name))
    print('%s.menu.usbstack.picosdk.build.usbstack_flags="-I{runtime.platform.path}/tools/libpico"' % (name))
    print("%s.menu.usbstack.tinyusb=Adafruit TinyUSB" % (name))
    print('%s.menu.usbstack.tinyusb.build.usbstack_flags=-DUSE_TINYUSB "-I{runtime.platform.path}/libraries/Adafruit_TinyUSB_Arduino/src/arduino"' % (name))

def BuildFreeRTOS(name):
    print("%s.menu.freertos.Disabled=Disabled" % (name))
    print("%s.menu.freertos.Disabled.build.flags.freertos=" % (name))
    print("%s.menu.freertos.Enabled=Enabled" % (name))
    print("%s.menu.freertos.Enabled.build.flags.freertos=-DUSE_FREERTOS" % (name))


def BuildWithoutUSBStack(name):
    print("%s.menu.usbstack.nousb=No USB" % (name))
    print('%s.menu.usbstack.nousb.build.usbstack_flags="-DNO_USB -DDISABLE_USB_SERIAL -I{runtime.platform.path}/tools/libpico"' % (name))

def BuildHeader(name, vendor_name, product_name, vidtouse, pidtouse, vid, pid, pwr, boarddefine, variant, uploadtool, flashsize, ramsize, boot2):
    prettyname = vendor_name + " " + product_name
    print()
    print("# -----------------------------------")
    print("# %s" % (prettyname))
    print("# -----------------------------------")
    print("%s.name=%s" % (name, prettyname))
    print("%s.vid.0=%s" % (name, vidtouse))
    print("%s.pid.0=%s" % (name, pidtouse))
    print("%s.build.usbpid=-DSERIALUSB_PID=%s" % (name, pid))
    print("%s.build.usbpwr=-DUSBD_MAX_POWER_MA=%s" % (name, pwr))
    print("%s.build.board=%s" % (name, boarddefine))
    print("%s.build.mcu=cortex-m0plus" % (name))
    print("%s.build.variant=%s" % (name, variant))
    print("%s.upload.tool=%s" % (name, uploadtool))
    print("%s.upload.maximum_size=%d" % (name, flashsize))
    print("%s.upload.maximum_data_size=%d" % (name, ramsize))
    print("%s.upload.wait_for_upload_port=true" % (name))
    print("%s.upload.erase_cmd=" % (name))
    print("%s.serial.disableDTR=false" % (name))
    print("%s.serial.disableRTS=false" % (name))
    print("%s.build.f_cpu=125000000" % (name))
    print("%s.build.led=" % (name))
    print("%s.build.core=rp2040" % (name))
    print("%s.build.ldscript=memmap_default.ld" % (name))
    print("%s.build.ram_length=%dk" % (name, ramsize / 1024))
    print("%s.build.boot2=%s" % (name, boot2))
    print("%s.build.vid=%s" % (name, vid))
    print("%s.build.pid=%s" % (name, pid))
    print('%s.build.usb_manufacturer="%s"' % (name, vendor_name))
    print('%s.build.usb_product="%s"' % (name, product_name))

def WriteWarning():
    print("# WARNING - DO NOT EDIT THIS FILE, IT IS MACHINE GENERATED")
    print("#           To change something here, edit tools/makeboards.py and")
    print("#           run 'python3 makeboards.py > ../boards.txt' to regenerate")
    print()

def BuildGlobalMenuList():
    print("menu.BoardModel=Model")
    print("menu.flash=Flash Size")
    print("menu.freq=CPU Speed")
    print("menu.opt=Optimize")
    print("menu.rtti=RTTI")
    print("menu.dbgport=Debug Port")
    print("menu.dbglvl=Debug Level")
    print("menu.boot2=Boot Stage 2")
    print("menu.usbstack=USB Stack")
    print("menu.freertos=Use FreeRTOS")


def MakeBoard(name, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2):
    for a, b, c in [ ["", "", "uf2conv"], ["picoprobe", " (Picoprobe)", "picoprobe"], ["picodebug", " (pico-debug)", "picodebug"]]:
        n = name + a
        p = product_name + b
        fssizelist = [ 0, 64 * 1024, 128 * 1024, 256 * 1024, 512 * 1024 ]
        for i in range(1, flashsizemb):
            fssizelist.append(i * 1024 * 1024)
        vidtouse = vid;
        ramsizekb = 256;
        if a == "picoprobe":
            pidtouse = '0x0004'
        elif a == "picodebug":
            vidtouse = '0x1209'
            pidtouse = '0x2488'
            ramsizekb = 240;
        else:
            pidtouse = pid
        BuildHeader(n, vendor_name, p, vidtouse, pidtouse, vid, pid, pwr, boarddefine, name, c, flashsizemb * 1024 * 1024, ramsizekb * 1024, boot2)
        if name == "generic":
            BuildFlashMenu(n, 2*1024*1024, [0, 1*1024*1024])
            BuildFlashMenu(n, 4*1024*1024, [0, 2*1024*1024])
            BuildFlashMenu(n, 8*1024*1024, [0, 4*1024*1024])
            BuildFlashMenu(n, 16*1024*1024, [0, 8*1024*1024])
        else:
            BuildFlashMenu(n, flashsizemb * 1024 * 1024, fssizelist)
        BuildFreq(n)
        BuildOptimize(n)
        BuildRTTI(n)
        BuildDebugPort(n)
        BuildDebugLevel(n)
        if a == "picodebug":
            BuildWithoutUSBStack(n)
        else:
            BuildUSBStack(n)
        if name == "generic":
            BuildBoot(n)
        BuildFreeRTOS(n)
    MakeBoardJSON(name, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2)

def MakeBoardJSON(name, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2):
    json = """{
  "build": {
    "core": "earlephilhower",
    "cpu": "cortex-m0plus",
    "extra_flags": "-D ARDUINO_BOARDDEFINE -DARDUINO_ARCH_RP2040 -DUSBD_MAX_POWER_MA=USBPWR",
    "f_cpu": "133000000L",
    "hwids": [
      [
        "0x2E8A",
        "0x00C0"
      ]
    ],
    "mcu": "rp2040",
    "arduino": {
      "earlephilhower": {
        "variant": "VARIANTNAME",
        "boot2_source": "BOOT2.S",
        "usb_vid": "VID",
        "usb_pid": "PID",
        "usb_manufacturer": "VENDORNAME",
        "usb_product": "PRODUCTNAME"
      }
    }
  },
  "debug": {
    "jlink_device": "RP2040_M0_0",
    "openocd_target": "rp2040.cfg",
    "svd_path": "rp2040.svd"
  },
  "frameworks": [
    "arduino"
  ],
  "name": "PRODUCTNAME",
  "upload": {
    "maximum_ram_size": 270336,
    "maximum_size": FLASHSIZE,
    "require_upload_port": true,
    "native_usb": true,
    "use_1200bps_touch": true,
    "wait_for_upload_port": false,
    "protocol": "picotool",
    "protocols": [
      "cmsis-dap",
      "jlink",
      "raspberrypi-swd",
      "picotool",
      "picoprobe"
    ]
  },
  "url": "https://www.raspberrypi.org/products/raspberry-pi-pico/",
  "vendor": "VENDORNAME"
}""".replace('VARIANTNAME', name).replace('BOARDDEFINE', boarddefine).replace('BOOT2', boot2).replace('VID', vid).replace('PID', pid).replace('VENDORNAME', vendor_name).replace('PRODUCTNAME', product_name).replace('FLASHSIZE', str(1024*1024*flashsizemb)).replace('USBPWR', str(pwr))
    jsondir = os.path.abspath(os.path.dirname(__file__)) + "/json"
    f = open(jsondir + "/" + name + ".json", "w")
    f.write(json)
    f.close()

sys.stdout = open(os.path.abspath(os.path.dirname(__file__)) + "/../boards.txt", "w")
WriteWarning()
BuildGlobalMenuList()

# Note to new board manufacturers:  Please add your board so that it sorts
# alphabetically starting with the company name and then the board name.
# Otherwise it is difficult to find a specific board in the menu.

# Raspberry Pi
MakeBoard("rpipico", "Raspberry Pi", "Pico", "0x2e8a", "0x000a", 250, "RASPBERRY_PI_PICO", 2, "boot2_w25q080_2_padded_checksum")


# Adafruit
MakeBoard("adafruit_feather", "Adafruit", "Feather RP2040", "0x239a", "0x80f1", 250, "ADAFRUIT_FEATHER_RP2040", 8, "boot2_w25x10cl_4_padded_checksum")
MakeBoard("adafruit_itsybitsy", "Adafruit", "ItsyBitsy RP2040", "0x239a", "0x80fd", 250, "ADAFRUIT_ITSYBITSY_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_qtpy", "Adafruit", "QT Py RP2040", "0x239a", "0x80f7", 250, "ADAFRUIT_QTPY_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_stemmafriend", "Adafruit", "STEMMA Friend RP2040", "0x239a", "0x80e3", 250, "ADAFRUIT_STEMMAFRIEND_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_trinkeyrp2040qt", "Adafruit", "Trinkey RP2040 QT", "0x239a", "0x8109", 250, "ADAFRUIT_TRINKEYQT_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_macropad2040", "Adafruit", "MacroPad RP2040", "0x239a", "0x8107", 250, "ADAFRUIT_MACROPAD_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_kb2040", "Adafruit", "KB2040", "0x239a", "0x8105", 250, "ADAFRUIT_KB2040_RP2040", 8, "boot2_w25q080_2_padded_checksum")

# Arduino
MakeBoard("arduino_nano_connect", "Arduino", "Nano RP2040 Connect", "0x2341", "0x0058", 250, "NANO_RP2040_CONNECT", 16, "boot2_w25q080_2_padded_checksum")

# Cytron
MakeBoard("cytron_maker_nano_rp2040", "Cytron", "Maker Nano RP2040", "0x2e8a", "0x100f", 250, "CYTRON_MAKER_NANO_RP2040", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("cytron_maker_pi_rp2040", "Cytron", "Maker Pi RP2040", "0x2e8a", "0x1000", 250, "CYTRON_MAKER_PI_RP2040", 2, "boot2_w25q080_2_padded_checksum")

# DeRuiLab
MakeBoard("flyboard2040_core", "DeRuiLab", "FlyBoard2040Core", "0x2e8a", "0x008a", 500, "FLYBOARD2040_CORE", 4, "boot2_generic_03h_4_padded_checksum")

# DFRobot
MakeBoard("dfrobot_beetle_rp2040", "DFRobot", "Beetle RP2040", "0x3343", "0x4253", 250, "DFROBOT_BEETLE_RP2040", 2, "boot2_w25q080_2_padded_checksum")

# iLabs
MakeBoard("challenger_2040_lora", "iLabs", "Challenger 2040 LoRa", "0x2e8a", "0x1023", 250, "CHALLENGER_2040_LORA_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_wifi", "iLabs", "Challenger 2040 WiFi", "0x2e8a", "0x1006", 250, "CHALLENGER_2040_WIFI_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_lte", "iLabs", "Challenger 2040 LTE", "0x2e8a", "0x100b", 500, "CHALLENGER_2040_LTE_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_nb_2040_wifi", "iLabs", "Challenger NB 2040 WiFi", "0x2e8a", "0x100b", 500, "CHALLENGER_NB_2040_WIFI_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("ilabs_rpico32", "iLabs", "RPICO32", "0x2e8a", "0x1010", 250, "ILABS_2040_RPICO32_RP2040", 8, "boot2_w25q080_2_padded_checksum")

# Melopera
MakeBoard("melopero_shake_rp2040", "Melopero", "Shake RP2040", "0x2e8a", "0x1005", 250, "MELOPERO_SHAKE_RP2040", 16, "boot2_w25q080_2_padded_checksum")

# Solder Party
MakeBoard("solderparty_rp2040_stamp", "Solder Party", "RP2040 Stamp", "0x1209", "0xa182", 500, "SOLDERPARTY_RP2040_STAMP", 8, "boot2_generic_03h_4_padded_checksum")

# SparkFun
MakeBoard("sparkfun_promicrorp2040", "SparkFun", "ProMicro RP2040", "0x1b4f", "0x0026", 250, "SPARKFUN_PROMICRO_RP2040", 16, "boot2_generic_03h_4_padded_checksum")

# Upesy
MakeBoard("upesy_rp2040_devkit", "uPesy", "RP2040 DevKit", "0x2e8a", "0x1007", 250, "UPESY_RP2040_DEVKIT", 2, "boot2_w25q080_2_padded_checksum")

# WIZnet
MakeBoard("wiznet_5100s_evb_pico", "WIZnet", "W5100S-EVB-Pico", "0x2e8a", "0x1008", 250, "WIZNET_5100S_EVB_PICO", 2, "boot2_w25q080_2_padded_checksum")

# Generic
MakeBoard("generic", "Generic", "RP2040", "0x2e8a", "0xf00a", 250, "GENERIC_RP2040", 16, "boot2_generic_03h_4_padded_checksum")



sys.stdout.close()
