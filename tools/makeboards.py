#!/usr/bin/env python3
import os
import sys
import json

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
        print("%s.menu.flash.%s.build.flash_total=%d" % (name, mn, flashsize))
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
    for f in [ 133,  50, 100, 120, 125, 150, 175, 200, 225, 240, 250, 275, 300]:
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

def BuildStackProtect(name):
    print("%s.menu.stackprotect.Disabled=Disabled" % (name))
    print("%s.menu.stackprotect.Disabled.build.flags.stackprotect=" % (name))
    print("%s.menu.stackprotect.Enabled=Enabled" % (name))
    print("%s.menu.stackprotect.Enabled.build.flags.stackprotect=-fstack-protector" % (name))

def BuildExceptions(name):
    print("%s.menu.exceptions.Disabled=Disabled" % (name))
    print("%s.menu.exceptions.Disabled.build.flags.exceptions=-fno-exceptions" % (name))
    print("%s.menu.exceptions.Disabled.build.flags.libstdcpp=-lstdc++" % (name))
    print("%s.menu.exceptions.Enabled=Enabled" % (name))
    print("%s.menu.exceptions.Enabled.build.flags.exceptions=-fexceptions" % (name))
    print("%s.menu.exceptions.Enabled.build.flags.libstdcpp=-lstdc++-exc" % (name))

def BuildBoot(name):
    for l in [ ("Generic SPI /2", "boot2_generic_03h_2_padded_checksum"),  ("Generic SPI /4", "boot2_generic_03h_4_padded_checksum"),
            ("IS25LP080 QSPI /2", "boot2_is25lp080_2_padded_checksum"), ("IS25LP080 QSPI /4", "boot2_is25lp080_4_padded_checksum"),
            ("W25Q080 QSPI /2", "boot2_w25q080_2_padded_checksum"), ("W25Q080 QSPI /4", "boot2_w25q080_4_padded_checksum"),
            ("W25X10CL QSPI /2", "boot2_w25x10cl_2_padded_checksum"), ("W25X10CL QSPI /4", "boot2_w25x10cl_4_padded_checksum"),
            ("W25Q64JV QSPI /4", "boot2_w25q64jv_4_padded_checksum"), ("W25Q16JVxQ QSPI /4", "boot2_w25q16jvxq_4_padded_checksum"),
            ("W25Q128JV QSPI /4", "boot2_w25q128jvxq_4_padded_checksum")]:
        print("%s.menu.boot2.%s=%s" % (name, l[1], l[0]))
        print("%s.menu.boot2.%s.build.boot2=%s" % (name, l[1], l[1]))

# Abbreviated Boot Stage 2 menu for some W25Q-equipped Adafruit boards.
# In extreme overclock situations, these may require QSPI /4 to work.
def BuildBootW25Q(name):
    for l in [ ("W25Q080 QSPI /2", "boot2_w25q080_2_padded_checksum"), ("W25Q080 QSPI /4", "boot2_w25q080_4_padded_checksum")]:
        print("%s.menu.boot2.%s=%s" % (name, l[1], l[0]))
        print("%s.menu.boot2.%s.build.boot2=%s" % (name, l[1], l[1]))

def BuildUSBStack(name):
    print("%s.menu.usbstack.picosdk=Pico SDK" % (name))
    print('%s.menu.usbstack.picosdk.build.usbstack_flags=' % (name))
    print("%s.menu.usbstack.tinyusb=Adafruit TinyUSB" % (name))
    print('%s.menu.usbstack.tinyusb.build.usbstack_flags=-DUSE_TINYUSB "-I{runtime.platform.path}/libraries/Adafruit_TinyUSB_Arduino/src/arduino"' % (name))
    print("%s.menu.usbstack.nousb=No USB" % (name))
    print('%s.menu.usbstack.nousb.build.usbstack_flags="-DNO_USB -DDISABLE_USB_SERIAL -I{runtime.platform.path}/tools/libpico"' % (name))

def BuildCountry(name):
    countries = [ "Worldwide", "Australia", "Austria", "Belgium", "Brazil", "Canada", "Chile", "China", "Colombia", "Czech Republic",
                  "Denmark", "Estonia", "Finland", "France", "Germany", "Greece", "Hong Kong", "Hungary", "Iceland", "India", "Israel",
                  "Italy", "Japan", "Kenya", "Latvia", "Liechtenstein", "Lithuania", "Luxembourg", "Malaysia", "Malta", "Mexico",
                  "Netherlands", "New Zealand", "Nigeria", "Norway", "Peru", "Philippines", "Poland", "Portugal", "Singapore", "Slovakia",
                  "Slovenia", "South Africa", "South Korea", "Spain", "Sweden", "Switzerland", "Taiwan", "Thailand", "Turkey", "UK", "USA"]
    for c in countries:
        sane = c.replace(" ", "_").upper()
        print("%s.menu.wificountry.%s=%s" % (name, sane.lower(), c))
        print("%s.menu.wificountry.%s.build.wificc=-DWIFICC=CYW43_COUNTRY_%s" % (name, sane.lower(), sane))

def BuildIPBTStack(name):
    print("%s.menu.ipbtstack.ipv4only=IPv4 Only" % (name))
    print('%s.menu.ipbtstack.ipv4only.build.libpicow=libpicow-noipv6-nobtc-noble.a' % (name))
    print('%s.menu.ipbtstack.ipv4only.build.libpicowdefs=-DLWIP_IPV6=0 -DLWIP_IPV4=1' % (name))
    print("%s.menu.ipbtstack.ipv4ipv6=IPv4 + IPv6" % (name))
    print('%s.menu.ipbtstack.ipv4ipv6.build.libpicow=libpicow-ipv6-nobtc-noble.a' % (name))
    print('%s.menu.ipbtstack.ipv4ipv6.build.libpicowdefs=-DLWIP_IPV6=1 -DLWIP_IPV4=1' % (name))
    print("%s.menu.ipbtstack.ipv4btcble=IPv4 + Bluetooth" % (name))
    print('%s.menu.ipbtstack.ipv4btcble.build.libpicow=libpicow-noipv6-btc-ble.a' % (name))
    print('%s.menu.ipbtstack.ipv4btcble.build.libpicowdefs=-DLWIP_IPV6=0 -DLWIP_IPV4=1 -DENABLE_CLASSIC=1 -DENABLE_BLE=1' % (name))
    print("%s.menu.ipbtstack.ipv4ipv6btcble=IPv4 + IPv6 + Bluetooth" % (name))
    print('%s.menu.ipbtstack.ipv4ipv6btcble.build.libpicow=libpicow-ipv6-btc-ble.a' % (name))
    print('%s.menu.ipbtstack.ipv4ipv6btcble.build.libpicowdefs=-DLWIP_IPV6=1 -DLWIP_IPV4=1 -DENABLE_CLASSIC=1 -DENABLE_BLE=1' % (name))

def BuildUploadMethodMenu(name):
    for a, b, c, d, e, f in [ ["default", "Default (UF2)", 256, "picoprobe_cmsis_dap.tcl", "uf2conv", "uf2conv-network"],
                              ["picotool", "Picotool", 256, "picoprobe.tcl", "picotool", None],
                              ["picoprobe", "Picoprobe", 256, "picoprobe.tcl", "picoprobe", None],
                              ["picoprobe_cmsis_dap", "Picoprobe (CMSIS-DAP)", 256, "picoprobe_cmsis_dap.tcl", "picoprobe_cmsis_dap", None],
                              ["picodebug", "Pico-Debug", 240, "picodebug.tcl", "picodebug", None] ]:
        print("%s.menu.uploadmethod.%s=%s" % (name, a, b))
        print("%s.menu.uploadmethod.%s.build.ram_length=%dk" % (name, a, c))
        print("%s.menu.uploadmethod.%s.build.debugscript=%s" % (name, a, d))
        # For pico-debug, need to disable USB unconditionally
        if a == "picodebug":
            print("%s.menu.uploadmethod.%s.build.picodebugflags=-UUSE_TINYUSB -DNO_USB -DDISABLE_USB_SERIAL -I{runtime.platform.path}/tools/libpico" % (name, a))
        elif a == "picotool":
            print("%s.menu.uploadmethod.%s.build.picodebugflags=-DENABLE_PICOTOOL_USB" % (name, a))
        print("%s.menu.uploadmethod.%s.upload.maximum_data_size=%d" % (name, a, c * 1024))
        print("%s.menu.uploadmethod.%s.upload.tool=%s" % (name, a, e))
        print("%s.menu.uploadmethod.%s.upload.tool.default=%s" % (name, a, e))
        if f != None:
            print("%s.menu.uploadmethod.%s.upload.tool.network=%s" % (name, a, f))

def BuildHeader(name, vendor_name, product_name, vid, pid, pwr, boarddefine, variant, flashsize, boot2, extra):
    prettyname = vendor_name + " " + product_name
    print()
    print("# -----------------------------------")
    print("# %s" % (prettyname))
    print("# -----------------------------------")
    print("%s.name=%s" % (name, prettyname))
    usb = 0
    if type(pid) == list:
        for tp in pid:
            print("%s.vid.%d=%s" % (name, usb, vid))
            print("%s.pid.%d=0x%04x" % (name, usb, int(tp, 16)))
            usb = usb + 1
    else:
        for kb in [ "0", "0x8000" ]:
            for ms in [ "0", "0x4000" ]:
                for jy in [ "0", "0x0100" ]:
                    thispid = int(pid, 16) | int(kb, 16) | int(ms, 16) | int(jy, 16)
                    print("%s.vid.%d=%s" % (name, usb, vid))
                    print("%s.pid.%d=0x%04x" % (name, usb, thispid))
                    usb = usb + 1
    print("%s.build.usbvid=-DUSBD_VID=%s" % (name, vid))
    if type(pid) == list:
        print("%s.build.usbpid=-DUSBD_PID=%s" % (name, pid[0]))
    else:
        print("%s.build.usbpid=-DUSBD_PID=%s" % (name, pid))
    print("%s.build.usbpwr=-DUSBD_MAX_POWER_MA=%s" % (name, pwr))
    print("%s.build.board=%s" % (name, boarddefine))
    print("%s.build.mcu=cortex-m0plus" % (name))
    print("%s.build.variant=%s" % (name, variant))
    print("%s.upload.maximum_size=%d" % (name, flashsize))
    print("%s.upload.wait_for_upload_port=true" % (name))
    print("%s.upload.erase_cmd=" % (name))
    print("%s.serial.disableDTR=false" % (name))
    print("%s.serial.disableRTS=false" % (name))
    print("%s.build.f_cpu=125000000" % (name))
    print("%s.build.led=" % (name))
    print("%s.build.core=rp2040" % (name))
    print("%s.build.ldscript=memmap_default.ld" % (name))
    print("%s.build.boot2=%s" % (name, boot2))
    print('%s.build.usb_manufacturer="%s"' % (name, vendor_name))
    print('%s.build.usb_product="%s"' % (name, product_name))
    if extra != None:
        m_extra = ''
        for m_item in extra:
            m_extra += '-D' + m_item + ' '
        print('%s.build.extra_flags=%s' % (name, m_extra.rstrip()))

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
    print("menu.stackprotect=Stack Protector")
    print("menu.exceptions=C++ Exceptions")
    print("menu.dbgport=Debug Port")
    print("menu.dbglvl=Debug Level")
    print("menu.boot2=Boot Stage 2")
    print("menu.wificountry=WiFi Region")
    print("menu.usbstack=USB Stack")
    print("menu.ipbtstack=IP/Bluetooth Stack")
    print("menu.uploadmethod=Upload Method")

def MakeBoard(name, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2, extra = None):
    fssizelist = [ 0, 64 * 1024, 128 * 1024, 256 * 1024, 512 * 1024 ]
    for i in range(1, flashsizemb):
        fssizelist.append(i * 1024 * 1024)
    BuildHeader(name, vendor_name, product_name, vid, pid, pwr, boarddefine, name, flashsizemb * 1024 * 1024, boot2, extra)
    if (name == "generic") or (name == "vccgnd_yd_rp2040"):
        BuildFlashMenu(name, 2*1024*1024, [0, 1*1024*1024])
        BuildFlashMenu(name, 4*1024*1024, [0, 3*1024*1024, 2*1024*1024])
        BuildFlashMenu(name, 8*1024*1024, [0, 7*1024*1024, 4*1024*1024, 2*1024*1024])
        BuildFlashMenu(name, 16*1024*1024, [0, 15*1024*1024, 14*1024*1024, 12*1024*1024, 8*1024*1024, 4*1024*1024, 2*1024*1024])
    else:
        BuildFlashMenu(name, flashsizemb * 1024 * 1024, fssizelist)
    BuildFreq(name)
    BuildOptimize(name)
    BuildRTTI(name)
    BuildStackProtect(name)
    BuildExceptions(name)
    BuildDebugPort(name)
    BuildDebugLevel(name)
    BuildUSBStack(name)
    if name == "rpipicow":
        BuildCountry(name)
    BuildIPBTStack(name)
    if name == "generic":
        BuildBoot(name)
    elif name.startswith("adafruit") and "w25q080" in boot2:
        BuildBootW25Q(name)
    BuildUploadMethodMenu(name)
    MakeBoardJSON(name, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2, extra)
    global pkgjson
    thisbrd = {}
    thisbrd['name'] = "%s %s" % (vendor_name, product_name)
    pkgjson['packages'][0]['platforms'][0]['boards'].append(thisbrd)

def MakeBoardJSON(name, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2, extra):
    if type(pid) == list:
        pid = pid[0]
    if extra != None:
        m_extra = ' '
        for m_item in extra:
            m_extra += '-D' + m_item + ' '
    else:
        m_extra = ''
    json = """{
  "build": {
    "arduino": {
      "earlephilhower": {
        "boot2_source": "BOOT2.S",
        "usb_vid": "VID",
        "usb_pid": "PID"
      }
    },
    "core": "earlephilhower",
    "cpu": "cortex-m0plus",
    "extra_flags": "-D ARDUINO_BOARDDEFINE -DARDUINO_ARCH_RP2040 -DUSBD_MAX_POWER_MA=USBPWR EXTRA_INFO",
    "f_cpu": "133000000L",
    "hwids": [
      [
        "0x2E8A",
        "0x00C0"
      ],
      [
        "VID",
        "PID"
      ]
    ],
    "mcu": "rp2040",
    "variant": "VARIANTNAME"
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
      "blackmagic",
      "cmsis-dap",
      "jlink",
      "raspberrypi-swd",
      "picotool",
      "picoprobe",
      "pico-debug"
    ]
  },
  "url": "https://www.raspberrypi.org/products/raspberry-pi-pico/",
  "vendor": "VENDORNAME"
}\n"""\
.replace('VARIANTNAME', name)\
.replace('BOARDDEFINE', boarddefine)\
.replace('BOOT2', boot2)\
.replace('VID', vid.upper().replace("X", "x"))\
.replace('PID', pid.upper().replace("X", "x"))\
.replace('VENDORNAME', vendor_name)\
.replace('PRODUCTNAME', product_name)\
.replace('FLASHSIZE', str(1024*1024*flashsizemb))\
.replace('USBPWR', str(pwr))\
.replace(' EXTRA_INFO', m_extra.rstrip())
    jsondir = os.path.abspath(os.path.dirname(__file__)) + "/json"
    f = open(jsondir + "/" + name + ".json", "w")
    f.write(json)
    f.close()

pkgjson = json.load(open(os.path.abspath(os.path.dirname(__file__)) + '/../package/package_pico_index.template.json'))
pkgjson['packages'][0]['platforms'][0]['boards'] = []

sys.stdout = open(os.path.abspath(os.path.dirname(__file__)) + "/../boards.txt", "w")
WriteWarning()
BuildGlobalMenuList()

# Note to new board manufacturers:  Please add your board so that it sorts
# alphabetically starting with the company name and then the board name.
# Otherwise it is difficult to find a specific board in the menu.

# Raspberry Pi
MakeBoard("rpipico", "Raspberry Pi", "Pico", "0x2e8a", "0x000a", 250, "RASPBERRY_PI_PICO", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("rpipicow", "Raspberry Pi", "Pico W", "0x2e8a", "0xf00a", 250, "RASPBERRY_PI_PICO_W", 2, "boot2_w25q080_2_padded_checksum")

# 0xCB
MakeBoard("0xcb_helios", "0xCB", "Helios", "0x1209", "0xCB74", 500, "0XCB_HELIOS", 16, "boot2_w25q128jvxq_4_padded_checksum")

# Adafruit
MakeBoard("adafruit_feather", "Adafruit", "Feather RP2040", "0x239a", "0x80f1", 250, "ADAFRUIT_FEATHER_RP2040", 8, "boot2_w25x10cl_4_padded_checksum")
MakeBoard("adafruit_feather_scorpio", "Adafruit", "Feather RP2040 SCORPIO", "0x239a", "0x8121", 250, "ADAFRUIT_FEATHER_RP2040_SCORPIO", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_dvi", "Adafruit", "Feather RP2040 DVI", "0x239a", "0x8127", 250, "ADAFRUIT_FEATHER_RP2040_DVI", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_rfm", "Adafruit", "Feather RP2040 RFM", "0x239a", "0x812D", 250, "ADAFRUIT_FEATHER_RP2040_RFM", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_thinkink", "Adafruit", "Feather RP2040 ThinkINK", "0x239a", "0x812B", 250, "ADAFRUIT_FEATHER_RP2040_THINKINK", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_usb_host", "Adafruit", "Feather RP2040 USB Host", "0x239a", "0x8129", 250, "ADAFRUIT_FEATHER_RP2040_USB_HOST", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_can", "Adafruit", "Feather RP2040 CAN", "0x239a", "0x812f", 250, "ADAFRUIT_FEATHER_RP2040_CAN", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_prop_maker", "Adafruit", "Feather RP2040 Prop-Maker", "0x239a", "0x8131", 250, "ADAFRUIT_FEATHER_RP2040_PROP_MAKER", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_itsybitsy", "Adafruit", "ItsyBitsy RP2040", "0x239a", "0x80fd", 250, "ADAFRUIT_ITSYBITSY_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_qtpy", "Adafruit", "QT Py RP2040", "0x239a", "0x80f7", 250, "ADAFRUIT_QTPY_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_stemmafriend", "Adafruit", "STEMMA Friend RP2040", "0x239a", "0x80e3", 250, "ADAFRUIT_STEMMAFRIEND_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_trinkeyrp2040qt", "Adafruit", "Trinkey RP2040 QT", "0x239a", "0x8109", 250, "ADAFRUIT_TRINKEYQT_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_macropad2040", "Adafruit", "MacroPad RP2040", "0x239a", "0x8107", 250, "ADAFRUIT_MACROPAD_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_kb2040", "Adafruit", "KB2040", "0x239a", "0x8105", 250, "ADAFRUIT_KB2040_RP2040", 8, "boot2_w25q080_2_padded_checksum")

# Arduino
MakeBoard("arduino_nano_connect", "Arduino", "Nano RP2040 Connect", "0x2341", ["0x005e", "0x805e", "0x015e", "0x025e"] , 250, "NANO_RP2040_CONNECT", 16, "boot2_w25q080_2_padded_checksum")

# ArtronShop
MakeBoard("artronshop_rp2_nano", "ArtronShop", "RP2 Nano", "0x2e8a", "0x000a", 250, "ARTRONSHOP_RP2_NANO", 2, "boot2_w25q080_2_padded_checksum")

# BridgeTek
MakeBoard("bridgetek_idm2040-7a", "BridgeTek", "IDM2040-7A", "0x2e8a", "0x1041", 250, "BRIDGETEK_IDM2040-7A", 8, "boot2_w25q080_2_padded_checksum", ["FT8XX_TYPE=BT817", "DISPLAY_RES=WVGA", "PLATFORM_RP2040"])

# Cytron
MakeBoard("cytron_maker_nano_rp2040", "Cytron", "Maker Nano RP2040", "0x2e8a", "0x100f", 250, "CYTRON_MAKER_NANO_RP2040", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("cytron_maker_pi_rp2040", "Cytron", "Maker Pi RP2040", "0x2e8a", "0x1000", 250, "CYTRON_MAKER_PI_RP2040", 2, "boot2_w25q080_2_padded_checksum")

# DatanoiseTV
MakeBoard("datanoisetv_picoadk", "DatanoiseTV", "PicoADK", "0x2e8a", "0x000a", 250, "DATANOISETV_PICOADK", 2, "boot2_w25q080_2_padded_checksum")

# DeRuiLab
MakeBoard("flyboard2040_core", "DeRuiLab", "FlyBoard2040Core", "0x2e8a", "0x008a", 500, "FLYBOARD2040_CORE", 4, "boot2_w25q080_2_padded_checksum")

# DFRobot
MakeBoard("dfrobot_beetle_rp2040", "DFRobot", "Beetle RP2040", "0x3343", "0x4253", 250, "DFROBOT_BEETLE_RP2040", 2, "boot2_w25q080_2_padded_checksum")

# ElectronicCat
MakeBoard("electroniccats_huntercat_nfc", "ElectronicCats", "HunterCat NFC RP2040", "0x2E8A", "0x1037", 500, "ELECTRONICCATS_HUNTERCAT_NFC", 2, "boot2_w25q080_2_padded_checksum")

# ExtremeElectronics
MakeBoard("extelec_rc2040", "ExtremeElectronics", "RC2040", "0x2e8a", "0xee20", 250, "EXTREMEELEXTRONICS_RC2040", 2, "boot2_w25q080_2_padded_checksum")

# iLabs
MakeBoard("challenger_2040_lte", "iLabs", "Challenger 2040 LTE", "0x2e8a", "0x100b", 500, "CHALLENGER_2040_LTE_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_lora", "iLabs", "Challenger 2040 LoRa", "0x2e8a", "0x1023", 250, "CHALLENGER_2040_LORA_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_subghz", "iLabs", "Challenger 2040 SubGHz", "0x2e8a", "0x1032", 250, "CHALLENGER_2040_SUBGHZ_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_wifi", "iLabs", "Challenger 2040 WiFi", "0x2e8a", "0x1006", 250, "CHALLENGER_2040_WIFI_RP2040", 8, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2040_wifi_ble", "iLabs", "Challenger 2040 WiFi/BLE", "0x2e8a", "0x102C", 500, "CHALLENGER_2040_WIFI_BLE_RP2040", 8, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2040_wifi6_ble", "iLabs", "Challenger 2040 WiFi6/BLE", "0x2e8a", "0x105F", 500, "CHALLENGER_2040_WIFI6_BLE_RP2040", 8, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_nb_2040_wifi", "iLabs", "Challenger NB 2040 WiFi", "0x2e8a", "0x100d", 500, "CHALLENGER_NB_2040_WIFI_RP2040", 8, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2040_sdrtc", "iLabs", "Challenger 2040 SD/RTC", "0x2e8a", "0x102d", 250, "CHALLENGER_2040_SDRTC_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_nfc", "iLabs", "Challenger 2040 NFC", "0x2e8a", "0x1036", 250, "CHALLENGER_2040_NFC_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_uwb", "iLabs", "Challenger 2040 UWB", "0x2e8a", "0x1052", 500, "CHALLENGER_2040_UWB_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("ilabs_rpico32", "iLabs", "RPICO32", "0x2e8a", "0x1010", 250, "ILABS_2040_RPICO32_RP2040", 8, "boot2_w25q080_2_padded_checksum")

# Melopero
MakeBoard("melopero_cookie_rp2040", "Melopero", "Cookie RP2040", "0x2e8a", "0x1011", 250, "MELOPERO_COOKIE_RP2040", 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("melopero_shake_rp2040", "Melopero", "Shake RP2040", "0x2e8a", "0x1005", 250, "MELOPERO_SHAKE_RP2040", 16, "boot2_w25q080_2_padded_checksum")

# Neko Systems
MakeBoard("nekosystems_bl2040_mini", "Neko Systems", "BL2040 Mini", "0x2e8a", "0x000a", 500, "NEKOSYSTEMS_BL2040_MINI", 4, "boot2_generic_03h_2_padded_checksum")

# nullbits
MakeBoard("nullbits_bit_c_pro", "nullbits", "Bit-C PRO", "0x2e8a", "0x6e61", 500, "NULLBITS_BIT_C_PRO", 4, "boot2_w25x10cl_4_padded_checksum")

# Pimoroni
MakeBoard("pimoroni_pga2040", "Pimoroni", "PGA2040", "0x2e8a", "0x1008", 250, "PIMORONI_PGA2040", 8, "boot2_w25q64jv_4_padded_checksum")
MakeBoard("pimoroni_plasma2040", "Pimoroni", "Plasma2040", "0x2e8a", "0x100a", 500, "PIMORONI_PLASMA2040", 2, "boot2_w25q080_2_padded_checksum")

# Solder Party
MakeBoard("solderparty_rp2040_stamp", "Solder Party", "RP2040 Stamp", "0x1209", "0xa182", 500, "SOLDERPARTY_RP2040_STAMP", 8, "boot2_generic_03h_4_padded_checksum")

# SparkFun
MakeBoard("sparkfun_promicrorp2040", "SparkFun", "ProMicro RP2040", "0x1b4f", "0x0026", 250, "SPARKFUN_PROMICRO_RP2040", 16, "boot2_generic_03h_4_padded_checksum")
MakeBoard("sparkfun_thingplusrp2040", "SparkFun", "Thing Plus RP2040", "0x1b4f", "0x0026", 250, "SPARKFUN_THINGPLUS_RP2040", 16, "boot2_w25q080_2_padded_checksum")

# Upesy
MakeBoard("upesy_rp2040_devkit", "uPesy", "RP2040 DevKit", "0x2e8a", "0x1007", 250, "UPESY_RP2040_DEVKIT", 2, "boot2_w25q080_2_padded_checksum")

# Seeed
MakeBoard("seeed_indicator_rp2040", "Seeed", "INDICATOR RP2040", "0x2886", "0x0050", 250, "SEEED_INDICATOR_RP2040", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("seeed_xiao_rp2040", "Seeed", "XIAO RP2040", "0x2e8a", "0x000a", 250, "SEEED_XIAO_RP2040", 2, "boot2_w25q080_2_padded_checksum")

# VCC-GND YD-2040 - Use generic SPI/4 because boards seem to come with varied flash modules but same name
MakeBoard('vccgnd_yd_rp2040', "VCC-GND", "YD RP2040", "0x2e8a", "0x800a", 500, "YD_RP2040", 16, "boot2_generic_03h_4_padded_checksum")

# Viyalab
MakeBoard("viyalab_mizu", "Viyalab", "Mizu RP2040", "0x2e8a", "0x000a", 250, "VIYALAB_MIZU_RP2040", 8, "boot2_generic_03h_4_padded_checksum")

# Waveshare
MakeBoard("waveshare_rp2040_zero", "Waveshare", "RP2040 Zero", "0x2e8a", "0x0003", 500, "WAVESHARE_RP2040_ZERO", 2, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_one", "Waveshare", "RP2040 One", "0x2e8a", "0x103a", 500, "WAVESHARE_RP2040_ONE", 4, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_plus_4mb", "Waveshare", "RP2040 Plus 4MB", "0x2e8a", "0x1020", 500, "WAVESHARE_RP2040_PLUS", 4, "boot2_w25q080_2_padded_checksum")
MakeBoard("waveshare_rp2040_plus_16mb", "Waveshare", "RP2040 Plus 16MB", "0x2e8a", "0x1020", 500, "WAVESHARE_RP2040_PLUS", 16, "boot2_w25q080_2_padded_checksum")
MakeBoard("waveshare_rp2040_lcd_0_96", "Waveshare", "RP2040 LCD 0.96", "0x2e8a", "0x1021", 500, "WAVESHARE_RP2040_LCD_0_96", 2, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_lcd_1_28", "Waveshare", "RP2040 LCD 1.28", "0x2e8a", "0x1039", 500, "WAVESHARE_RP2040_LCD_1_28", 2, "boot2_w25q16jvxq_4_padded_checksum")

# WIZnet
MakeBoard("wiznet_5100s_evb_pico", "WIZnet", "W5100S-EVB-Pico", "0x2e8a", "0x1027", 250, "WIZNET_5100S_EVB_PICO", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("wiznet_wizfi360_evb_pico", "WIZnet", "WizFi360-EVB-Pico", "0x2e8a", "0x1028", 250, "WIZNET_WIZFI360_EVB_PICO", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("wiznet_5500_evb_pico", "WIZnet", "W5500-EVB-Pico", "0x2e8a", "0x1029", 250, "WIZNET_5500_EVB_PICO", 2, "boot2_w25q080_2_padded_checksum")

# Generic
MakeBoard("generic", "Generic", "RP2040", "0x2e8a", "0xf00a", 250, "GENERIC_RP2040", 16, "boot2_generic_03h_4_padded_checksum")

sys.stdout.close()
with open(os.path.abspath(os.path.dirname(__file__)) + '/../package/package_pico_index.template.json', 'w') as f:
    f.write(json.dumps(pkgjson, indent=3))
