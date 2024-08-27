#!/usr/bin/env python3
import os
import sys
import json

def BuildFlashMenu(name, chip, flashsize, fssizelist):
    if chip == "rp2350":
        delta = 8192
    elif chip == "rp2040":
        delta = 4096
    for fssize in fssizelist:
        if fssize == 0:
            fssizename = "no FS"
        elif fssize < 1024 * 1024:
            fssizename = "Sketch: %dKB, FS: %dKB" % ((flashsize - fssize) / 1024, fssize / 1024)
        else:
            fssizename = "Sketch: %dMB, FS: %dMB" % ((flashsize - fssize) / (1024 * 1024), fssize / (1024 * 1024))
        mn="%d_%d" % (flashsize, fssize)
        print("%s.menu.flash.%s=%dMB (%s)" % (name, mn, flashsize / (1024 * 1024), fssizename))
        print("%s.menu.flash.%s.upload.maximum_size=%d" % (name, mn, flashsize - delta - fssize))
        print("%s.menu.flash.%s.build.flash_total=%d" % (name, mn, flashsize))
        print("%s.menu.flash.%s.build.flash_length=%d" % (name, mn, flashsize - delta - fssize))
        print("%s.menu.flash.%s.build.eeprom_start=%d" % (name, mn, int("0x10000000",0) + flashsize - delta))
        print("%s.menu.flash.%s.build.fs_start=%d" % (name, mn, int("0x10000000",0) + flashsize - delta - fssize))
        print("%s.menu.flash.%s.build.fs_end=%d" % (name, mn, int("0x10000000",0) + flashsize - delta))

def BuildDebugPort(name):
    print("%s.menu.dbgport.Disabled=Disabled" % (name))
    print("%s.menu.dbgport.Disabled.build.debug_port=" % (name))
    for p in ["Serial", "Serial1", "Serial2"]:
        print("%s.menu.dbgport.%s=%s" % (name, p, p))
        print("%s.menu.dbgport.%s.build.debug_port=-DDEBUG_RP2040_PORT=%s" % (name, p, p))

def BuildDebugLevel(name):
    for l in [ ("None", ""), ("Core", "-DDEBUG_RP2040_CORE"), ("SPI", "-DDEBUG_RP2040_SPI"), ("Wire", "-DDEBUG_RP2040_WIRE"), ("Bluetooth", "-DDEBUG_RP2040_BLUETOOTH"),
               ("All", "-DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE -DDEBUG_RP2040_BLUETOOTH"), ("NDEBUG", "-DNDEBUG") ]:
        print("%s.menu.dbglvl.%s=%s" % (name, l[0], l[0]))
        print("%s.menu.dbglvl.%s.build.debug_level=%s" % (name, l[0], l[1]))

def BuildFreq(name, defmhz):
    out = 0
    for f in [ defmhz, 50, 100, 120, 125, 128, 133, 150, 175, 200, 225, 240, 250, 275, 300]:
        warn = ""
        if f > defmhz: warn = " (Overclock)"
        if (out == 1) and (f == defmhz):
            continue
        out = 1
        print("%s.menu.freq.%s=%s MHz%s" % (name, f, f, warn))
        print("%s.menu.freq.%s.build.f_cpu=%dL" % (name, f, f * 1000000))

def BuildPSRAM(name):
    for s in [ 0, 2, 4, 8]:
        print("%s.menu.psram.%dmb=%dMByte PSRAM" % (name, s, s))
        print("%s.menu.psram.%dmb.build.psram_length=0x%d00000" % (name, s, s))

def BuildPSRAMCS(name):
    for s in range(0, 32):
        print("%s.menu.psramcs.GPIO%d=GPIO %d" % (name, s, s))
        print("%s.menu.psramcs.GPIO%d.build.psram_cs=-DRP2350_PSRAM_CS=%d" % (name, s, s))

def BuildPSRAMFreq(name):
    for s in [ 109, 133 ]:
        print("%s.menu.psramfreq.freq%d=%d MHz" % (name, s, s))
        print("%s.menu.psramfreq.freq%d.build.psram_freq=-DRP2350_PSRAM_MAX_SCK_HZ=%d" % (name, s, s * 1000000))

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
    print("%s.menu.usbstack.tinyusb_host=Adafruit TinyUSB Host (native)" % (name))
    print('%s.menu.usbstack.tinyusb_host.build.usbstack_flags=-DUSE_TINYUSB -DUSE_TINYUSB_HOST "-I{runtime.platform.path}/libraries/Adafruit_TinyUSB_Arduino/src/arduino"' % (name))
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
    print('%s.menu.ipbtstack.ipv4only.build.libpicow=libipv4.a' % (name))
    print('%s.menu.ipbtstack.ipv4only.build.libpicowdefs=-DLWIP_IPV6=0 -DLWIP_IPV4=1' % (name))
    print("%s.menu.ipbtstack.ipv4ipv6=IPv4 + IPv6" % (name))
    print('%s.menu.ipbtstack.ipv4ipv6.build.libpicow=libipv4-ipv6.a' % (name))
    print('%s.menu.ipbtstack.ipv4ipv6.build.libpicowdefs=-DLWIP_IPV6=1 -DLWIP_IPV4=1' % (name))
    print("%s.menu.ipbtstack.ipv4btcble=IPv4 + Bluetooth" % (name))
    print('%s.menu.ipbtstack.ipv4btcble.build.libpicow=libipv4-bt.a' % (name))
    print('%s.menu.ipbtstack.ipv4btcble.build.libpicowdefs=-DLWIP_IPV6=0 -DLWIP_IPV4=1 -DENABLE_CLASSIC=1 -DENABLE_BLE=1' % (name))
    print("%s.menu.ipbtstack.ipv4ipv6btcble=IPv4 + IPv6 + Bluetooth" % (name))
    print('%s.menu.ipbtstack.ipv4ipv6btcble.build.libpicow=libipv4-ipv6-bt.a' % (name))
    print('%s.menu.ipbtstack.ipv4ipv6btcble.build.libpicowdefs=-DLWIP_IPV6=1 -DLWIP_IPV4=1 -DENABLE_CLASSIC=1 -DENABLE_BLE=1' % (name))
    print("%s.menu.ipbtstack.ipv4onlybig=IPv4 Only - 32K" % (name))
    print('%s.menu.ipbtstack.ipv4onlybig.build.libpicow=libipv4-big.a' % (name))
    print('%s.menu.ipbtstack.ipv4onlybig.build.libpicowdefs=-DLWIP_IPV6=0 -DLWIP_IPV4=1 -D__LWIP_MEMMULT=2' % (name))
    print("%s.menu.ipbtstack.ipv4ipv6big=IPv4 + IPv6 - 32K" % (name))
    print('%s.menu.ipbtstack.ipv4ipv6big.build.libpicow=libipv4-ipv6-big.a' % (name))
    print('%s.menu.ipbtstack.ipv4ipv6big.build.libpicowdefs=-DLWIP_IPV6=1 -DLWIP_IPV4=1 -D__LWIP_MEMMULT=2' % (name))
    print("%s.menu.ipbtstack.ipv4btcblebig=IPv4 + Bluetooth - 32K" % (name))
    print('%s.menu.ipbtstack.ipv4btcblebig.build.libpicow=libipv4-bt-big.a' % (name))
    print('%s.menu.ipbtstack.ipv4btcblebig.build.libpicowdefs=-DLWIP_IPV6=0 -DLWIP_IPV4=1 -DENABLE_CLASSIC=1 -DENABLE_BLE=1 -D__LWIP_MEMMULT=2' % (name))
    print("%s.menu.ipbtstack.ipv4ipv6btcblebig=IPv4 + IPv6 + Bluetooth - 32K" % (name))
    print('%s.menu.ipbtstack.ipv4ipv6btcblebig.build.libpicow=libipv4-ipv6-bt-big.a' % (name))
    print('%s.menu.ipbtstack.ipv4ipv6btcblebig.build.libpicowdefs=-DLWIP_IPV6=1 -DLWIP_IPV4=1 -DENABLE_CLASSIC=1 -DENABLE_BLE=1 -D__LWIP_MEMMULT=2' % (name))


def BuildUploadMethodMenu(name, ram):
    for a, b, c, d, e, f in [ ["default", "Default (UF2)", ram, "picoprobe_cmsis_dap.tcl", "uf2conv", "uf2conv-network"],
                              ["picotool", "Picotool", ram, "picoprobe.tcl", "picotool", None],
                              ["picoprobe_cmsis_dap", "Picoprobe/Debugprobe (CMSIS-DAP)", ram, "picoprobe_cmsis_dap.tcl", "picoprobe_cmsis_dap", None],
                              ["picodebug", "Pico-Debug", 240, "picodebug.tcl", "picodebug", None] ]:
        if (ram > 256) and (a == "picodebug"):
            continue
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

def BuildHeader(name, chip, chaintuple, chipoptions, vendor_name, product_name, vid, pid, pwr, boarddefine, variant, flashsize, psramsize, boot2, extra):
    prettyname = vendor_name + " " + product_name
    print()
    print("# -----------------------------------")
    print("# %s" % (prettyname))
    print("# -----------------------------------")
    print("%s.name=%s" % (name, prettyname))

    # USB Vendor ID / Product ID (VID/PID) pairs for board detection
    if isinstance(pid, list):
        # Explicitly specified list of PIDs (with the same VID)
        usb_pids = pid
    else:
        # When the RP2040 is used as a composite device, the PID is modified
        # (see cores/rp2040/RP2040USB.cpp) because Windows wants a different
        # VID:PID for different device configurations [citation needed?].
        # See https://github.com/earlephilhower/arduino-pico/issues/796
        #
        # TODO FIX: Some PIDs already have these bits set, and there's no
        # guarantee mangling PIDs this way won't collide with other devices.
        usb_pids = []
        for k_bit in [0, 0x8000]:
            for m_bit in [0, 0x4000]:
                for j_bit in [0, 0x0100]:
                    this_pid = "0x%04x" % (int(pid, 16) | k_bit | m_bit | j_bit)
                    if this_pid not in usb_pids:
                        usb_pids.append(this_pid)

    main_pid = usb_pids[0]

    # Old style VID/PID list for compatibility with older Arduino tools
    for i, pid in enumerate(usb_pids):
        print("%s.vid.%d=%s" % (name, i, vid))
        print("%s.pid.%d=%s" % (name, i, pid))

    # Since our platform.txt enables pluggable discovery, we are also required
    # to list VID/PID in this format
    for i, pid in enumerate(usb_pids):
        print("%s.upload_port.%d.vid=%s" % (name, i, vid))
        print("%s.upload_port.%d.pid=%s" % (name, i, pid))

    print("%s.build.usbvid=-DUSBD_VID=%s" % (name, vid))
    print("%s.build.usbpid=-DUSBD_PID=%s" % (name, main_pid))
    print("%s.build.usbpwr=-DUSBD_MAX_POWER_MA=%s" % (name, pwr))
    print("%s.build.board=%s" % (name, boarddefine))
#    print("%s.build.mcu=cortex-m0plus" % (name))
    print("%s.build.chip=%s" % (name, chip))
    print("%s.build.toolchain=%s" % (name, chaintuple))
    print("%s.build.toolchainopts=%s" % (name, chipoptions))
    if chip == "rp2350":
        uf2family = "--family rp2350-arm-s --abs-block"
    elif chip == "rp2040":
        uf2family = "--family rp2040"
    print("%s.build.uf2family=%s" % (name, uf2family))
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
    if (chip == "rp2350") and (name != "generic_rp2350"):
        print("%s.build.psram_length=0x%d00000" % (name, psramsize))
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
    print("menu.psram=PSRAM Size")
    print("menu.psramcs=PSRAM CS")
    print("menu.psramfreq=PSRAM Speed")
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

def MakeBoard(name, chip, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, psramsize, boot2, extra = None, board_url = None):
    fssizelist = [ 0, 64 * 1024, 128 * 1024, 256 * 1024, 512 * 1024 ]
    for i in range(1, flashsizemb):
        fssizelist.append(i * 1024 * 1024)
    if chip == "rp2040":
        tup = "arm-none-eabi"
        opts = "-march=armv6-m -mcpu=cortex-m0plus -mthumb"
    elif chip == "rp2350":
        tup =  "arm-none-eabi"
        opts = "-mcpu=cortex-m33 -mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=softfp -mcmse"
    elif chip == "rp2530rv":
        tup = "riscv-none-eabi"
        opts = "TBD"
    else:
        raise Exception("Unknown board type " + str(chip));
    BuildHeader(name, chip, tup, opts, vendor_name, product_name, vid, pid, pwr, boarddefine, name, flashsizemb * 1024 * 1024, psramsize, boot2, extra)
    if (name == "generic") or (name == "vccgnd_yd_rp2040"):
        BuildFlashMenu(name, chip, 2*1024*1024, [0, 1*1024*1024])
        BuildFlashMenu(name, chip, 4*1024*1024, [0, 3*1024*1024, 2*1024*1024])
        BuildFlashMenu(name, chip, 8*1024*1024, [0, 7*1024*1024, 4*1024*1024, 2*1024*1024])
        BuildFlashMenu(name, chip, 16*1024*1024, [0, 15*1024*1024, 14*1024*1024, 12*1024*1024, 8*1024*1024, 4*1024*1024, 2*1024*1024])
    elif name == "pimoroni_tiny2040":
        BuildFlashMenu(name, chip, 2*1024*1024, fssizelist)
        BuildFlashMenu(name, chip, 8*1024*1024, [0, 7*1024*1024, 4*1024*1024, 2*1024*1024])
    elif name == "akana_r1":
        BuildFlashMenu(name, chip, 2*1024*1024, [0, 1*1024*1024])
        BuildFlashMenu(name, chip, 8*1024*1024, [0, 7*1024*1024, 4*1024*1024, 2*1024*1024])
        BuildFlashMenu(name, chip, 16*1024*1024, [0, 15*1024*1024, 14*1024*1024, 12*1024*1024, 8*1024*1024, 4*1024*1024, 2*1024*1024])
    else:
        BuildFlashMenu(name, chip, flashsizemb * 1024 * 1024, fssizelist)
    if chip == "rp2350":
        BuildFreq(name, 150)
        if name == "generic_rp2350":
            BuildPSRAM(name)
            BuildPSRAMCS(name)
            BuildPSRAMFreq(name)
    else:
        BuildFreq(name, 133)
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
    if chip == "rp2040":
        BuildUploadMethodMenu(name, 256)
    else:
        BuildUploadMethodMenu(name, 512)
    MakeBoardJSON(name, chip, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2, extra, board_url)
    global pkgjson
    thisbrd = {}
    thisbrd['name'] = "%s %s" % (vendor_name, product_name)
    pkgjson['packages'][0]['platforms'][0]['boards'].append(thisbrd)

def MakeBoardJSON(name, chip, vendor_name, product_name, vid, pid, pwr, boarddefine, flashsizemb, boot2, extra, board_url):
    # TODO FIX: Use the same expanded PID list as in BuildHeader above?
    if isinstance(pid, list):
        pid = pid[0]
    if extra != None:
        m_extra = ' '
        for m_item in extra:
            m_extra += '-D' + m_item + ' '
    else:
        m_extra = ''
    if chip == "rp2040":
        cpu = "cortex-m0plus"
        ramsize = 256
        jlink = "RP2040_M0_0"
        fcpu = "133000000L"
    elif chip == "rp2350":
        cpu = "cortex-m33"
        ramsize = 512
        jlink = "RP2350_0"
        fcpu = "150000000L"
    j = {
    "build": {
        "arduino": {
            "earlephilhower": {
                "boot2_source": boot2 + ".S",
                "usb_vid": vid.upper().replace("X", "x"),
                "usb_pid": pid.upper().replace("X", "x"),
            }
        },
        "core": "earlephilhower",
        "cpu": cpu,
        "extra_flags": "-DARDUINO_" + boarddefine + " -DARDUINO_ARCH_RP2040 -DUSBD_MAX_POWER_MA=" + str(pwr) + " " + m_extra.rstrip(),
        "f_cpu": fcpu,
        "hwids": [
            [
                "0x2E8A",
                "0x00C0"
            ],
            [
                vid.upper().replace("X", "x"),
                pid.upper().replace("X", "x"),
            ]
        ],
        "mcu": chip,
        "variant": name,
    },
    "debug": {
        "jlink_device": jlink,
        "openocd_target": chip + ".cfg",
        "svd_path": chip + ".svd"
    },
    "frameworks": [
        "arduino"
    ],
    "name": product_name,
    "upload": {
        "maximum_ram_size": ramsize * 1024,
        "maximum_size": 1024 * 1024 * flashsizemb,
        "require_upload_port": True,
        "native_usb": True,
        "use_1200bps_touch": True,
        "wait_for_upload_port": False,
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
    "url": board_url or 'https://www.raspberrypi.org/products/raspberry-pi-pico/',
    "vendor": vendor_name,
    }

    jsondir = os.path.abspath(os.path.dirname(__file__)) + "/json"
    with open(jsondir + "/" + name + ".json", "w", newline='\n') as jout:
        json.dump(j, jout, indent=4)

pkgjson = json.load(open(os.path.abspath(os.path.dirname(__file__)) + '/../package/package_pico_index.template.json'))
pkgjson['packages'][0]['platforms'][0]['boards'] = []

sys.stdout = open(os.path.abspath(os.path.dirname(__file__)) + "/../boards.txt", "w", newline='\n')
WriteWarning()
BuildGlobalMenuList()

# Note to new board manufacturers:  Please add your board so that it sorts
# alphabetically starting with the company name and then the board name.
# Otherwise it is difficult to find a specific board in the menu.

# Raspberry Pi
MakeBoard("rpipico", "rp2040", "Raspberry Pi", "Pico", "0x2e8a", "0x000a", 250, "RASPBERRY_PI_PICO", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("rpipicow", "rp2040", "Raspberry Pi", "Pico W", "0x2e8a", "0xf00a", 250, "RASPBERRY_PI_PICO_W", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("rpipico2", "rp2350", "Raspberry Pi", "Pico 2", "0x2e8a", "0x000f", 250, "RASPBERRY_PI_PICO_2", 4, 0, "boot2_generic_03h_2_padded_checksum")

# 0xCB
MakeBoard("0xcb_helios", "rp2040", "0xCB", "Helios", "0x1209", "0xCB74", 500, "0XCB_HELIOS", 16, 0, "boot2_w25q128jvxq_4_padded_checksum")

# Adafruit
MakeBoard("adafruit_feather", "rp2040", "Adafruit", "Feather RP2040", "0x239a", "0x80f1", 250, "ADAFRUIT_FEATHER_RP2040", 8, 0, "boot2_w25x10cl_4_padded_checksum")
MakeBoard("adafruit_feather_scorpio", "rp2040", "Adafruit", "Feather RP2040 SCORPIO", "0x239a", "0x8121", 250, "ADAFRUIT_FEATHER_RP2040_SCORPIO", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_dvi", "rp2040", "Adafruit", "Feather RP2040 DVI", "0x239a", "0x8127", 250, "ADAFRUIT_FEATHER_RP2040_DVI", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_adalogger", "rp2040", "Adafruit", "Feather RP2040 Adalogger", "0x239a", "0x815d", 250, "ADAFRUIT_FEATHER_RP2040_ADALOGGER", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_rfm", "rp2040", "Adafruit", "Feather RP2040 RFM", "0x239a", "0x812D", 250, "ADAFRUIT_FEATHER_RP2040_RFM", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_thinkink", "rp2040", "Adafruit", "Feather RP2040 ThinkINK", "0x239a", "0x812B", 250, "ADAFRUIT_FEATHER_RP2040_THINKINK", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_usb_host", "rp2040", "Adafruit", "Feather RP2040 USB Host", "0x239a", "0x8129", 250, "ADAFRUIT_FEATHER_RP2040_USB_HOST", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_can", "rp2040", "Adafruit", "Feather RP2040 CAN", "0x239a", "0x812f", 250, "ADAFRUIT_FEATHER_RP2040_CAN", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_feather_prop_maker", "rp2040", "Adafruit", "Feather RP2040 Prop-Maker", "0x239a", "0x8131", 250, "ADAFRUIT_FEATHER_RP2040_PROP_MAKER", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_itsybitsy", "rp2040", "Adafruit", "ItsyBitsy RP2040", "0x239a", "0x80fd", 250, "ADAFRUIT_ITSYBITSY_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_metro", "rp2040", "Adafruit", "Metro RP2040", "0x239a", "0x813d", 250, "ADAFRUIT_METRO_RP2040", 16, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_qtpy", "rp2040", "Adafruit", "QT Py RP2040", "0x239a", "0x80f7", 250, "ADAFRUIT_QTPY_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_stemmafriend", "rp2040", "Adafruit", "STEMMA Friend RP2040", "0x239a", "0x80e3", 250, "ADAFRUIT_STEMMAFRIEND_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_trinkeyrp2040qt", "rp2040", "Adafruit", "Trinkey RP2040 QT", "0x239a", "0x8109", 250, "ADAFRUIT_TRINKEYQT_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_macropad2040", "rp2040", "Adafruit", "MacroPad RP2040", "0x239a", "0x8107", 250, "ADAFRUIT_MACROPAD_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruit_kb2040", "rp2040", "Adafruit", "KB2040", "0x239a", "0x8105", 250, "ADAFRUIT_KB2040_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")


#Amken
MakeBoard("amken_bunny", "rp2040","Amken","BunnyBoard","0x2770",["0x7303"],250,"AMKEN_BB",128,0,"boot2_w25q128jvxq_4_padded_checksum","","https://www.amken3d.com")
MakeBoard("amken_revelop", "rp2040","Amken","Revelop","0x2770",["0x7304"],250,"AMKEN_REVELOP",32,0,"boot2_W25Q32JVxQ_4_padded_checksum","","https://www.amken3d.com")
MakeBoard("amken_revelop_plus", "rp2040","Amken","Revelop Plus","0x2770",["0x7305"],250,"AMKEN_REVELOP_PLUS",32,0,"boot2_W25Q32JVxQ_4_padded_checksum","","https://www.amken3d.com")
MakeBoard("amken_revelop_es", "rp2040","Amken","Revelop eS","0x2770",["0x7306"],250,"AMKEN_ES",16,0,"boot2_w25q16jvxq_4_padded_checksum","","https://www.amken3d.com")

# Arduino
MakeBoard("arduino_nano_connect", "rp2040", "Arduino", "Nano RP2040 Connect", "0x2341", ["0x005e", "0x805e", "0x015e", "0x025e"] , 250, "NANO_RP2040_CONNECT", 16, 0, "boot2_w25q080_2_padded_checksum")

# ArtronShop
MakeBoard("artronshop_rp2_nano", "rp2040", "ArtronShop", "RP2 Nano", "0x2e8a", "0x000a", 250, "ARTRONSHOP_RP2_NANO", 2, 0, "boot2_w25q080_2_padded_checksum")

# Breadstick
MakeBoard("breadstick_raspberry", "rp2040", "Breadstick", "Raspberry", "0x2e8a", "0x105e" , 500, "Breadstick_Raspberry", 16, 0, "boot2_w25q080_2_padded_checksum", board_url="https://shop.breadstick.ca/products/raspberry-breadstick-rp2040")

# BridgeTek
MakeBoard("bridgetek_idm2040_7a", "rp2040", "BridgeTek", "IDM2040-7A", "0x2e8a", "0x1041", 250, "BRIDGETEK_IDM2040_7A", 8, 0, "boot2_w25q080_2_padded_checksum", ["FT8XX_TYPE=BT817", "DISPLAY_RES=WVGA", "PLATFORM_RP2040"])
MakeBoard("bridgetek_idm2040_43a", "rp2040", "BridgeTek", "IDM2040-43A", "0x2e8b", "0xf00a", 250, "BRIDGETEK_IDM2040_43A", 8, 0, "boot2_w25q080_2_padded_checksum", ["FT8XX_TYPE=BT883", "DISPLAY_RES=WQVGA", "PLATFORM_RP2040"])
 
# Cytron
MakeBoard("cytron_iriv_io_controller", "rp2350", "Cytron", "IRIV IO Controller", "0x2e8a", "0x1093", 250, "CYTRON_IRIV_IO_CONTROLLER", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("cytron_maker_nano_rp2040", "rp2040", "Cytron", "Maker Nano RP2040", "0x2e8a", "0x100f", 250, "CYTRON_MAKER_NANO_RP2040", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("cytron_maker_pi_rp2040", "rp2040", "Cytron", "Maker Pi RP2040", "0x2e8a", "0x1000", 250, "CYTRON_MAKER_PI_RP2040", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("cytron_maker_uno_rp2040", "rp2040", "Cytron", "Maker Uno RP2040", "0x2e8a", "0x1071", 250, "CYTRON_MAKER_UNO_RP2040", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("cytron_motion_2350_pro", "rp2350", "Cytron", "Motion 2350 Pro", "0x2e8a", "0x1096", 250, "CYTRON_MOTION_2350_PRO", 2, 0, "boot2_generic_03h_2_padded_checksum")

# DatanoiseTV
MakeBoard("datanoisetv_picoadk", "rp2040", "DatanoiseTV", "PicoADK", "0x2e8a", "0x000a", 250, "DATANOISETV_PICOADK", 2, 0, "boot2_w25q080_2_padded_checksum")

# Degz Robotics
MakeBoard("degz_suibo", "rp2040", "Degz Robotics", "Suibo RP2040", "0x2e8a", "0xf00a", 250, "DEGZ_SUIBO_RP2040", 16, 0, "boot2_generic_03h_4_padded_checksum", board_url="https://www.degzrobotics.com/suibo")

# DeRuiLab
MakeBoard("flyboard2040_core", "rp2040", "DeRuiLab", "FlyBoard2040Core", "0x2e8a", "0x008a", 500, "FLYBOARD2040_CORE", 4, 0, "boot2_w25q080_2_padded_checksum")

# DFRobot
MakeBoard("dfrobot_beetle_rp2040", "rp2040", "DFRobot", "Beetle RP2040", "0x3343", "0x4253", 250, "DFROBOT_BEETLE_RP2040", 2, 0, "boot2_w25q080_2_padded_checksum")

# DudesCab
MakeBoard("DudesCab", "rp2040", "L'atelier d'Arnoz", "DudesCab", "0x2e8a", "0x106F", 250, "RASPBERRY_PI_PICO", 4, 0, "boot2_w25q080_2_padded_checksum")

# ElectronicCat
MakeBoard("electroniccats_huntercat_nfc", "rp2040", "ElectronicCats", "HunterCat NFC RP2040", "0x2E8A", "0x1037", 500, "ELECTRONICCATS_HUNTERCAT_NFC", 2, 0, "boot2_w25q080_2_padded_checksum")

# EVN
MakeBoard("evn_alpha", "rp2040", "EVN", "Alpha", "0x2e8a", "0xf00a", 500, "EVN_ALPHA", 16, 0, "boot2_generic_03h_2_padded_checksum", board_url="https://coresg.tech/evn")

# ExtremeElectronics
MakeBoard("extelec_rc2040", "rp2040", "ExtremeElectronics", "RC2040", "0x2e8a", "0xee20", 250, "EXTREMEELEXTRONICS_RC2040", 2, 0, "boot2_w25q080_2_padded_checksum")

# GroundStudio
MakeBoard('groundstudio_marble_pico', "rp2040", "GroundStudio", "Marble Pico", "0x2e8a", "0x0003", 500, "MARBLE_PICO", 8, 0, "boot2_w25q16jvxq_4_padded_checksum", None, "https://ardushop.ro/2652-marble-pico.html")

# iLabs
MakeBoard("challenger_2040_lte", "rp2040", "iLabs", "Challenger 2040 LTE", "0x2e8a", "0x100b", 500, "CHALLENGER_2040_LTE_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_lora", "rp2040", "iLabs", "Challenger 2040 LoRa", "0x2e8a", "0x1023", 250, "CHALLENGER_2040_LORA_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_subghz", "rp2040", "iLabs", "Challenger 2040 SubGHz", "0x2e8a", "0x1032", 250, "CHALLENGER_2040_SUBGHZ_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_wifi", "rp2040", "iLabs", "Challenger 2040 WiFi", "0x2e8a", "0x1006", 250, "CHALLENGER_2040_WIFI_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2040_wifi_ble", "rp2040", "iLabs", "Challenger 2040 WiFi/BLE", "0x2e8a", "0x102C", 500, "CHALLENGER_2040_WIFI_BLE_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2040_wifi6_ble", "rp2040", "iLabs", "Challenger 2040 WiFi6/BLE", "0x2e8a", "0x105F", 500, "CHALLENGER_2040_WIFI6_BLE_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_nb_2040_wifi", "rp2040", "iLabs", "Challenger NB 2040 WiFi", "0x2e8a", "0x100d", 500, "CHALLENGER_NB_2040_WIFI_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2040_sdrtc", "rp2040", "iLabs", "Challenger 2040 SD/RTC", "0x2e8a", "0x102d", 250, "CHALLENGER_2040_SDRTC_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_nfc", "rp2040", "iLabs", "Challenger 2040 NFC", "0x2e8a", "0x1036", 250, "CHALLENGER_2040_NFC_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("challenger_2040_uwb", "rp2040", "iLabs", "Challenger 2040 UWB", "0x2e8a", "0x1052", 500, "CHALLENGER_2040_UWB_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("connectivity_2040_lte_wifi_ble", "rp2040", "iLabs", "Connectivity 2040 LTE/WiFi/BLE", "0x2e8a", "0x107b", 500, "CONNECTIVITY_2040_LTE_WIFI_BLE_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("ilabs_rpico32", "rp2040", "iLabs", "RPICO32", "0x2e8a", "0x1010", 250, "ILABS_2040_RPICO32_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2350_wifi6_ble5", "rp2350", "iLabs", "Challenger 2350 WiFi/BLE", "0x2e8a", "0x109a", 500, "CHALLENGER_2350_WIFI_BLE_RP2350", 8, 8, "boot2_generic_03h_2_padded_checksum", ["WIFIESPAT2"])
MakeBoard("challenger_2350_bconnect", "rp2350", "iLabs", "Challenger 2350 BConnect", "0x2e8a", "0x109b", 500, "CHALLENGER_2350_BCONNECT_RP2350", 8, 8, "boot2_generic_03h_2_padded_checksum")

# Melopero
MakeBoard("melopero_cookie_rp2040", "rp2040", "Melopero", "Cookie RP2040", "0x2e8a", "0x1011", 250, "MELOPERO_COOKIE_RP2040", 8, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("melopero_shake_rp2040", "rp2040", "Melopero", "Shake RP2040", "0x2e8a", "0x1005", 250, "MELOPERO_SHAKE_RP2040", 16, 0, "boot2_w25q080_2_padded_checksum")

# Mete Hoca
MakeBoard("akana_r1", "rp2040", "METE HOCA", "Akana R1", "0x2e8a", "0x3001", 500, "METEHOCA_AKANA_R1", 16, 0, "boot2_generic_03h_4_padded_checksum", board_url="https://www.metehoca.com/")

# Neko Systems
MakeBoard("nekosystems_bl2040_mini", "rp2040", "Neko Systems", "BL2040 Mini", "0x2e8a", "0x000a", 500, "NEKOSYSTEMS_BL2040_MINI", 4, 0, "boot2_generic_03h_2_padded_checksum")

# Newsan
MakeBoard("newsan_archi", "rp2040", "Newsan", "Archi", "0x2E8A", "0x1043", 250, "NEWSAN_ARCHI", 4, 0, "boot2_generic_03h_4_padded_checksum", None, "https://archikids.com.ar/")

# nullbits
MakeBoard("nullbits_bit_c_pro", "rp2040", "nullbits", "Bit-C PRO", "0x2e8a", "0x6e61", 500, "NULLBITS_BIT_C_PRO", 4, 0, "boot2_w25x10cl_4_padded_checksum")

# Olimex
MakeBoard("olimex_rp2040pico30_2mb", "rp2040", "Olimex", "RP2040-Pico30 2MB", "0x15ba", "0x0026", 250, "OLIMEX_RP2040_PICO30_2MB", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("olimex_rp2040pico30_16mb", "rp2040", "Olimex", "RP2040-Pico30 16MB", "0x15ba", "0x0026", 250, "OLIMEX_RP2040_PICO30_16MB", 16, 0, "boot2_w25q080_2_padded_checksum")

# Pimoroni
MakeBoard("pimoroni_pga2040", "rp2040", "Pimoroni", "PGA2040", "0x2e8a", "0x1008", 250, "PIMORONI_PGA2040", 8, 0, "boot2_w25q64jv_4_padded_checksum")
MakeBoard("pimoroni_plasma2040", "rp2040", "Pimoroni", "Plasma2040", "0x2e8a", "0x100a", 500, "PIMORONI_PLASMA2040", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("pimoroni_tiny2040", "rp2040", "Pimoroni", "Tiny2040", "0x2e8a", "0x100a", 500, "PIMORONI_TINY2040", 2, 0, "boot2_w25q64jv_4_padded_checksum")

#Pintronix
MakeBoard("pintronix_pinmax", "rp2040", "Pintronix", "PinMax", "0x2e8a", "0x9101", 250, "PINTRONIX_PINMAX", 4, 0, "boot2_w25q080_2_padded_checksum")

# RAKwireless
MakeBoard("rakwireless_rak11300", "rp2040", "RAKwireless", "RAK11300", "0x2e8a", "0x00c0", 500, "RAKWIRELESS_RAK11300", 2, 0, "boot2_w25q16jvxq_4_padded_checksum", None, "https://store.rakwireless.com/products/wisduo-lpwan-module-rak11300")

# Redscorp
MakeBoard("redscorp_rp2040_eins", "rp2040", "redscorp", "RP2040-Eins", "0x2341", ["0x005f", "0x805f", "0x015f", "0x025f"] , 250, "REDSCORP_RP2040_EINS", 16, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("redscorp_rp2040_promini", "rp2040", "redscorp", "RP2040-ProMini", "0x2341", ["0x005f", "0x805f", "0x015f", "0x025f"] , 250, "REDSCORP_RP2040_PROMINI", 16, 0, "boot2_w25q080_2_padded_checksum")

# Sea-Picro
MakeBoard("sea_picro", "rp2040", "Generic", "Sea-Picro", "0x2e8a", "0xf00a", 500, "SEA_PICRO", 8, 0, "boot2_w25q64jv_4_padded_checksum", None, "https://github.com/joshajohnson/sea-picro")

# Silicognition
MakeBoard("silicognition_rp2040_shim", "rp2040", "Silicognition", "RP2040-Shim", "0x1209", "0xf502", 500, "SILICOGNITION_RP2040_SHIM", 4, 0, "boot2_generic_03h_4_padded_checksum")

# Solder Party
MakeBoard("solderparty_rp2040_stamp", "rp2040", "Solder Party", "RP2040 Stamp", "0x1209", "0xa182", 500, "SOLDERPARTY_RP2040_STAMP", 8, 0, "boot2_generic_03h_4_padded_checksum", None, "https://www.solder.party/docs/rp2040-stamp/")
MakeBoard("solderparty_rp2350_stamp", "rp2350", "Solder Party", "RP2350 Stamp", "0x1209", "0xa183", 500, "SOLDERPARTY_RP2350_STAMP", 16, 0, "boot2_generic_03h_2_padded_checksum", None, "https://www.solder.party/docs/rp2350-stamp/")
MakeBoard("solderparty_rp2350_stamp_xl", "rp2350", "Solder Party", "RP2350 Stamp XL", "0x1209", "0xa184", 500, "SOLDERPARTY_RP2350_STAMP_XL", 16, 0, "boot2_generic_03h_2_padded_checksum", None, "https://www.solder.party/docs/rp2350-stamp-xl/")

# SparkFun
MakeBoard("sparkfun_micromodrp2040", "rp2040", "SparkFun", "MicroMod RP2040", "0x1b4f", "0x0026", 250, "SPARKFUN_MICROMOD_RP2040", 16, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("sparkfun_promicrorp2040", "rp2040", "SparkFun", "ProMicro RP2040", "0x1b4f", "0x0026", 250, "SPARKFUN_PROMICRO_RP2040", 16, 0, "boot2_generic_03h_4_padded_checksum")
MakeBoard("sparkfun_promicrorp2350", "rp2350", "SparkFun", "ProMicro RP2350", "0x1b4f", "0x0026", 250, "SPARKFUN_PROMICRO_RP2350", 16, 8, "boot2_w25q080_2_padded_checksum")
MakeBoard("sparkfun_thingplusrp2040", "rp2040", "SparkFun", "Thing Plus RP2040", "0x1b4f", "0x0026", 250, "SPARKFUN_THINGPLUS_RP2040", 16, 0, "boot2_w25q080_2_padded_checksum")

# Upesy
MakeBoard("upesy_rp2040_devkit", "rp2040", "uPesy", "RP2040 DevKit", "0x2e8a", "0x1007", 250, "UPESY_RP2040_DEVKIT", 2, 0, "boot2_w25q080_2_padded_checksum")

# Seeed
MakeBoard("seeed_indicator_rp2040", "rp2040", "Seeed", "INDICATOR RP2040", "0x2886", "0x0050", 250, "SEEED_INDICATOR_RP2040", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("seeed_xiao_rp2040", "rp2040", "Seeed", "XIAO RP2040", "0x2e8a", "0x000a", 250, "SEEED_XIAO_RP2040", 2, 0, "boot2_w25q080_2_padded_checksum")

# VCC-GND YD-2040 - Use generic SPI/4 because boards seem to come with varied flash modules but same name
MakeBoard('vccgnd_yd_rp2040', "rp2040", "VCC-GND", "YD RP2040", "0x2e8a", "0x800a", 500, "YD_RP2040", 16, 0, "boot2_generic_03h_4_padded_checksum")

# Viyalab
MakeBoard("viyalab_mizu", "rp2040", "Viyalab", "Mizu RP2040", "0x2e8a", "0x000a", 250, "VIYALAB_MIZU_RP2040", 8, 0, "boot2_generic_03h_4_padded_checksum")

# Waveshare
MakeBoard("waveshare_rp2040_zero", "rp2040", "Waveshare", "RP2040 Zero", "0x2e8a", "0x0003", 500, "WAVESHARE_RP2040_ZERO", 2, 0, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_one", "rp2040", "Waveshare", "RP2040 One", "0x2e8a", "0x103a", 500, "WAVESHARE_RP2040_ONE", 4, 0, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_matrix", "rp2040", "Waveshare", "RP2040 Matrix", "0x2e8a", "0x103a", 500, "WAVESHARE_RP2040_MATRIX", 2, 0, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_pizero", "rp2040", "Waveshare", "RP2040 PiZero", "0x2e8a", "0x0003", 500, "WAVESHARE_RP2040_PIZERO", 16, 0, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_plus_4mb", "rp2040", "Waveshare", "RP2040 Plus 4MB", "0x2e8a", "0x1020", 500, "WAVESHARE_RP2040_PLUS", 4, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("waveshare_rp2040_plus_16mb", "rp2040", "Waveshare", "RP2040 Plus 16MB", "0x2e8a", "0x1020", 500, "WAVESHARE_RP2040_PLUS", 16, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("waveshare_rp2040_lcd_0_96", "rp2040", "Waveshare", "RP2040 LCD 0.96", "0x2e8a", "0x1021", 500, "WAVESHARE_RP2040_LCD_0_96", 2, 0, "boot2_w25q16jvxq_4_padded_checksum")
MakeBoard("waveshare_rp2040_lcd_1_28", "rp2040", "Waveshare", "RP2040 LCD 1.28", "0x2e8a", "0x1039", 500, "WAVESHARE_RP2040_LCD_1_28", 2, 0, "boot2_w25q16jvxq_4_padded_checksum")

# WIZnet
MakeBoard("wiznet_5100s_evb_pico", "rp2040", "WIZnet", "W5100S-EVB-Pico", "0x2e8a", "0x1027", 250, "WIZNET_5100S_EVB_PICO", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("wiznet_wizfi360_evb_pico", "rp2040", "WIZnet", "WizFi360-EVB-Pico", "0x2e8a", "0x1028", 250, "WIZNET_WIZFI360_EVB_PICO", 2, 0, "boot2_w25q080_2_padded_checksum")
MakeBoard("wiznet_5500_evb_pico", "rp2040", "WIZnet", "W5500-EVB-Pico", "0x2e8a", "0x1029", 250, "WIZNET_5500_EVB_PICO", 2, 0, "boot2_w25q080_2_padded_checksum")

# Generic
MakeBoard("generic", "rp2040", "Generic", "RP2040", "0x2e8a", "0xf00a", 250, "GENERIC_RP2040", 16, 0, "boot2_generic_03h_4_padded_checksum")
MakeBoard("generic_rp2350", "rp2350", "Generic", "RP2350", "0x2e8a", "0xf00f", 250, "GENERIC_RP2350", 16, 8, "boot2_generic_03h_4_padded_checksum")

sys.stdout.close()
with open(os.path.abspath(os.path.dirname(__file__)) + '/../package/package_pico_index.template.json', 'w', newline='\n') as f:
    f.write(json.dumps(pkgjson, indent=3))
