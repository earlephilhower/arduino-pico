#!/usr/bin/env python3

def BuildFlashMenu(name, flashsize, fssizelist):
    for fssize in fssizelist:
        if fssize == 0:
            fssizename = "no FS"
        elif fssize < 1024 * 1024:
            fssizename = "FS: %dKB" % (fssize / 1024)
        else:
            fssizename = "FS: %dMB" % (fssize / (1024 * 1024))
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
    for l in [ ("None", ""), ("Core", "-DDEBUG_RP2040_CORE"), ("SPI", "-DDEBUG_RP2040_SPI"), ("Wire", "DDEBUG_RP2040_WIRE"),
               ("All", "-DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE"), ("NDEBUG", "-DNDEBUG") ]:
        print("%s.menu.dbglvl.%s=%s" % (name, l[0], l[0]))
        print("%s.menu.dbglvl.%s.build.debug_level=%s" % (name, l[0], l[1]))

def BuildFreq(name):
    for f in [ 125, 50, 100, 133, 150, 175, 200, 225, 250, 275, 300]:
        warn = ""
        if f > 133: warn = " (Overclock)"
        print("%s.menu.freq.%s=%s MHz%s" % (name, f, f, warn))
        print("%s.menu.freq.%s.build.f_cpu=%dL" % (name, f, f * 1000000))

def BuildHeader(name, prettyname, pid, boarddefine, variant, uploadtool, flashsize):
    print("%s.name=%s" % (name, prettyname))
    print("%s.vid.0=0x2e8a" % (name))
    print("%s.pid.0=%s" % (name, pid))
    print("%s.build.usbpid=-DSERIALUSB_PID=%s" % (name, pid))
    print("%s.build.board=%s" % (name, boarddefine))
    print("%s.build.mcu=cortex-m0plus" % (name))
    print("%s.build.variant=%s" % (name, variant))
    print("%s.upload.tool=%s" % (name, uploadtool))
    print("%s.upload.maximum_size=%d" % (name, flashsize))
    print("%s.upload.maximum_data_size=262144" % (name))
    print("%s.upload.wait_for_upload_port=true" % (name))
    print("%s.upload.erase_cmd=" % (name))
    print("%s.serial.disableDTR=false" % (name))
    print("%s.serial.disableRTS=false" % (name))
    print("%s.build.f_cpu=125000000" % (name))
    print("%s.build.led=" % (name))
    print("%s.build.core=rp2040" % (name))
    print("%s.build.mcu=rp2040" % (name))
    print("%s.build.ldscript=memmap_default.ld" % (name))

def BuildGlobalMenuList():
    print("menu.BoardModel=Model")
    print("menu.flash=Flash Size")
    print("menu.freq=CPU Speed")
    print("menu.dbgport=Debug Port")
    print("menu.dbglvl=Debug Level")

BuildGlobalMenuList()

BuildHeader("rpipico", "Raspberry Pi Pico", "0x000a", "RASPBERRY_PI_PICO", "rpipico", "uf2conv", 2 * 1024*1024)
BuildFlashMenu("rpipico", 2 * 1024 * 1024, [ 0, 64 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024 ])
BuildFreq("rpipico")
BuildDebugPort("rpipico")
BuildDebugLevel("rpipico")

BuildHeader("adafruitfeather", "Adafruit Feather RP2040", "0x000b", "ADAFRUIT_FEATHER_RP2040", "adafruitfeather", "uf2conv", 8 *1024*1024)
BuildFlashMenu("adafruitfeather", 8 * 1024 * 1024, [ 0, 64 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024, 2 * 1024 * 1024, 3 * 1024 * 1024, 4 * 1024 *1024, 5 * 1024 *1024, 6 * 1024*1024, 7*1024*1024 ])
BuildFreq("adafruitfeather")
BuildDebugPort("adafruitfeather")
BuildDebugLevel("adafruitfeather")

BuildHeader("generic", "Generic RP2040", "0xf00a", "GENERIC_RP2040", "generif", "uf2conv", 2 * 1024*1024)
BuildFlashMenu("generic", 2 * 1024 * 1024, [ 0, 64 * 1024, 256 * 1024, 512 * 1024, 1024 * 1024 ])
BuildFreq("generic")
BuildDebugPort("generic")
BuildDebugLevel("generic")
