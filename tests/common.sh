#!/usr/bin/env bash

# return 1 if this test should not be built in CI (for other archs, not needed, etc.)
function skip_ino()
{
    local ino=$1
    local skiplist=""
    # Add items to the following list with "\n" netween them to skip running.  No spaces, tabs, etc. allowed
    read -d '' skiplist << EOL || true
/#attic/
/AvrAdcLogger/
/examplesV1/
/RtcTimestampTest/
/SoftwareSpi/
/TeensyDmaAdcLogger/
/TeensyRtcTimestamp/
/TeensySdioDemo/
/TeensySdioLogger/
/UserChipSelectFunction/
/UserSPIDriver/
/Adafruit_TinyUSB_Arduino/
/ArduinoISP/
/p08_DigitalHourglass/
/p13_TouchSensorLamp/
/StringComparisonOperators/
/PDMSerialPlotter/
EOL
    echo $ino | grep -q -F "$skiplist"
    echo $(( 1 - $? ))
}

function print_size_info()
{
    elf_file=$1

    if [ -z "$elf_file" ]; then
        printf "sketch                       data     rodata   bss      text      dram     flash\n"
        return 0
    fi

    elf_name=$(basename $elf_file)
    sketch_name="${elf_name%.*}"
    # echo $sketch_name
    arm-none-eabi-size --format=sysv $elf_file | sed s/irom0.text/irom0text/g > size.txt
    declare -A segments
    for seg in data rodata bss text irom0text; do
        segments[$seg]=$(grep ^.$seg size.txt | awk '{sum += $2} END {print sum}')
    done

    total_ram=$((${segments[data]} + ${segments[rodata]} + ${segments[bss]}))
    total_flash=$((${segments[data]} + ${segments[rodata]} + ${segments[text]}))

    printf "%-28s %-8d %-8d %-8d %-8d %-8d     %-8d %-8d\n" $sketch_name ${segments[data]} ${segments[rodata]} ${segments[bss]} ${segments[text]} $total_ram $total_flash
    return 0
}

function build_sketches()
{
    set +e
    local arduino=$1
    local srcpath=$2
    local build_arg=$3
    local build_dir=build.tmp
    local build_mod=$4
    local build_rem=$5

    mkdir -p $build_dir
    local build_cmd="python3 tools/build.py -b generic -v -w all -v -k --build_cache $cache_dir -p ./$build_dir $build_arg "
    if [ "$WINDOWS" = "1" ]; then
        # Paths to the arduino builder need to be / referenced, not our native ones
        build_cmd=$(echo $build_cmd --ide_path $arduino | sed 's/ \/c\// \//g' ) # replace '/c/' with '/'
    fi
    local sketches=$(find $srcpath -name *.ino | sort)
    print_size_info >size.log
    export ARDUINO_IDE_PATH=$arduino
    local testcnt=0
    for sketch in $sketches; do
        testcnt=$(( ($testcnt + 1) % $build_mod ))
        if [ $testcnt -ne $build_rem ]; then
            continue  # Not ours to do
        fi

        if [ -e $cache_dir/core/*.a ]; then
            # We need to preserve the build.options.json file and replace the last .ino
            # with this sketch's ino file, or builder will throw everything away.
            jq '."sketchLocation" = "'$sketch'"' $build_dir/build.options.json > $build_dir/build.options.json.tmp
            mv $build_dir/build.options.json.tmp $build_dir/build.options.json
            # Set the time of the cached core.a file to the future so the GIT header
            # we regen won't cause the builder to throw it out and rebuild from scratch.
            touch -d 'now + 1 day' $cache_dir/core/*.a
        fi

        # Clear out the last built sketch, map, elf, bin files, but leave the compiled
        # objects in the core and libraries available for use so we don't need to rebuild
        # them each sketch.
        rm -rf $build_dir/sketch $build_dir/*.bin $build_dir/*.map $build_dir/*.elf

        local sketchdir=$(dirname $sketch)
        local sketchdirname=$(basename $sketchdir)
        local sketchname=$(basename $sketch)
        if [[ "${sketchdirname}.ino" != "$sketchname" ]]; then
            echo "Skipping $sketch, because it is not the main sketch file";
            continue
        fi;
        if [[ -f "$sketchdir/.test.skip" ]]; then
            echo -e "\n ------------ Skipping $sketch ------------ \n";
            continue
        fi
        if [[ $(skip_ino $sketch) = 1 ]]; then
            echo -e "\n ------------ Skipping $sketch ------------ \n";
            continue
        fi
        echo -e "\n ------------ Building $sketch ------------ \n";
        # $arduino --verify $sketch;
        if [ "$WINDOWS" == "1" ]; then
            sketch=$(echo $sketch | sed 's/^\/c//')
            # MINGW will try to be helpful and silently convert args that look like paths to point to a spot inside the MinGW dir.  This breaks everything.
            # http://www.mingw.org/wiki/Posix_path_conversion
            # https://stackoverflow.com/questions/7250130/how-to-stop-mingw-and-msys-from-mangling-path-names-given-at-the-command-line#34386471
            export MSYS2_ARG_CONV_EXC="*"
            export MSYS_NO_PATHCONV=1
        fi
        echo "$build_cmd $sketch"
        time ($build_cmd $sketch >build.log)
        local result=$?
        if [ $result -ne 0 ]; then
            echo "Build failed ($1)"
            echo "Build log:"
            cat build.log
            set -e
            return $result
        else
            local warns=$( grep -c warning: build.log )
            if [ $warns -ne 0 ]; then
                echo "Warnings detected, log follows:"
                cat build.log
            fi
        fi
        rm build.log
        print_size_info $build_dir/*.elf >>size.log
    done
    set -e
}

function install_libraries()
{
    mkdir -p $HOME/Arduino/libraries
    pushd $HOME/Arduino/libraries

    # install ArduinoJson library
    { test -r ArduinoJson-v6.11.0.zip || curl -sS --output ArduinoJson-v6.11.0.zip -L https://github.com/bblanchon/ArduinoJson/releases/download/v6.11.0/ArduinoJson-v6.11.0.zip; } && unzip -qo ArduinoJson-v6.11.0.zip
    { test -r Adafruit_SPIFlash-3.4.1.zip || curl -sS --output Adafruit_SPIFlash-3.4.1.zip -L https://github.com/adafruit/Adafruit_SPIFlash/archive/refs/tags/3.4.1.zip; } && unzip -qo Adafruit_SPIFlash-3.4.1.zip
    { test -r Adafruit_Seesaw-1.4.4.zip || curl -sS --output Adafruit_Seesaw-1.4.4.zip -L https://github.com/adafruit/Adafruit_Seesaw/archive/refs/tags/1.4.4.zip; } && unzip -qo Adafruit_Seesaw-1.4.4.zip
    { test -r Adafruit_BusIO-1.7.3.zip || curl -sS --output Adafruit_BusIO-1.7.3.zip -L https://github.com/adafruit/Adafruit_BusIO/archive/refs/tags/1.7.3.zip; } && unzip -qo Adafruit_BusIO-1.7.3.zip
    { test -r Adafruit_CircuitPlayground-1.11.3.zip || curl -sS --output Adafruit_CircuitPlayground-1.11.3.zip -L https://github.com/adafruit/Adafruit_CircuitPlayground/archive/refs/tags/1.11.3.zip; } && unzip -qo Adafruit_CircuitPlayground-1.11.3.zip
    { test -r Adafruit_NeoPixel-1.8.1.zip || curl -sS --output Adafruit_NeoPixel-1.8.1.zip -L https://github.com/adafruit/Adafruit_NeoPixel/archive/refs/tags/1.8.1.zip; } && unzip -qo Adafruit_NeoPixel-1.8.1.zip
    { test -r Arduino_MIDI_Library-5.0.2.zip || curl -sS --output Arduino_MIDI_Library-5.0.2.zip -L https://github.com/FortySevenEffects/arduino_midi_library/archive/refs/tags/5.0.2.zip; } && unzip -qo Arduino_MIDI_Library-5.0.2.zip
    popd
}

function install_ide()
{
    local idever='nightly'
    local ideurl='https://downloads.arduino.cc/arduino-nightly' #'https://www.arduino.cc/download.php?f=/arduino-nightly'

    #local idever='1.8.10'
    #local ideurl="https://downloads.arduino.cc/arduino-$idever"

    echo "using Arduino IDE distribution ${idever}"

    local ide_path=$1
    local core_path=$2
    local debug=$3
    mkdir -p ${core_path}/tools/dist
    if [ "$WINDOWS" = "1" ]; then
        test -r ${core_path}/tools/dist/arduino-windows.zip || curl --output ${core_path}/tools/dist/arduino-windows.zip -L "${ideurl}-windows.zip"
        unzip -q ${core_path}/tools/dist/arduino-windows.zip
        mv arduino-${idever} arduino-distrib
    elif [ "$MACOSX" = "1" ]; then
        # MACOS only has next-to-obsolete Python2 installed.  Install Python 3 from python.org
        wget -q https://www.python.org/ftp/python/3.7.4/python-3.7.4-macosx10.9.pkg
        sudo installer -pkg python-3.7.4-macosx10.9.pkg -target /
        # Install the Python3 certificates, because SSL connections fail w/o them and of course they aren't installed by default.
        ( cd "/Applications/Python 3.7/" && sudo "./Install Certificates.command" )
        # Hack to place arduino-builder in the same spot as sane OSes
        test -r ${core_path}/tools/dist/arduino-macos.zip || wget -q -O ${core_path}/tools/dist/arduino-macos.zip "${ideurl}-macosx.zip"
        unzip -q ${core_path}/tools/dist/arduino-macos.zip
        mv Arduino.app arduino-distrib
        mv arduino-distrib/Contents/Java/* arduino-distrib/.
    else
        test -r ${core_path}/tools/dist/arduino-linux.tar.xz || wget -q -O ${core_path}/tools/dist/arduino-linux.tar.xz "${ideurl}-linux64.tar.xz"
        tar xf ${core_path}/tools/dist/arduino-linux.tar.xz
        mv arduino-${idever} arduino-distrib
    fi
    mv arduino-distrib $ide_path
    cd $ide_path/hardware
    mkdir pico
    cd pico
    if [ "$WINDOWS" = "1" ]; then
        cp -a $core_path rp2040
    else
        ln -s $core_path rp2040
    fi
    local debug_flags=""
    if [ "$debug" = "debug" ]; then
        debug_flags="-DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE -DDEBUG_RP2040_PORT=Serial"
    fi
    # Set custom warnings for all builds (i.e. could add -Wextra at some point)
    echo "compiler.c.extra_flags=-Wall -Wextra -Werror -Wno-ignored-qualifiers $debug_flags" > rp2040/platform.local.txt
    echo "compiler.cpp.extra_flags=-Wall -Wextra -Werror -Wno-ignored-qualifiers $debug_flags" >> rp2040/platform.local.txt
    echo -e "\n----platform.local.txt----"
    cat rp2040/platform.local.txt
    echo -e "\n----\n"
    cd rp2040/tools
    python3 get.py
    if [ "$WINDOWS" = "1" ]; then
        # Because the symlinks don't work well under Win32, we need to add the path to this copy, not the original...
        relbin=$(realpath $PWD/../system/arm-none-eabi/bin)
        export PATH="$ide_path:$relbin:$PATH"
    else
        export PATH="$ide_path:$core_path/system/arm-none-eabi/bin:$PATH"
    fi
}

function install_arduino()
{
    local debug=$1
    # Install Arduino IDE and required libraries
    cd $TRAVIS_BUILD_DIR
    install_ide $HOME/arduino_ide $TRAVIS_BUILD_DIR $debug
    cd $TRAVIS_BUILD_DIR
    install_libraries
}

function build_sketches_with_arduino()
{
    local build_mod=$1
    local build_rem=$2
    local build_extra=$3

    # Compile sketches
    build_sketches $HOME/arduino_ide $HOME/arduino_ide/examples "-l $HOME/Arduino/libraries ${build_extra}" $build_mod $build_rem
    build_sketches $HOME/arduino_ide $TRAVIS_BUILD_DIR/libraries "-l $HOME/Arduino/libraries ${build_extra}" $build_mod $build_rem

    # Generate size report
    cat size.log
}


set -e

if [ -z "$TRAVIS_BUILD_DIR" ]; then
    echo "TRAVIS_BUILD_DIR is not set, trying to guess:"
    pushd $(dirname $0)/../ > /dev/null
    TRAVIS_BUILD_DIR=$PWD
    popd > /dev/null
    echo "TRAVIS_BUILD_DIR=$TRAVIS_BUILD_DIR"
fi

