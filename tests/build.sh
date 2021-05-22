#!/usr/bin/env bash

cache_dir=$(mktemp -d)

source "$TRAVIS_BUILD_DIR"/tests/common.sh

if [ -z "$BUILD_PARITY" ]; then
    mod=1
    rem=0
elif [ "$BUILD_PARITY" = "even" ]; then
    mod=2
    rem=0
elif [ "$BUILD_PARITY" = "odd" ]; then
    mod=2
    rem=1
fi

install_arduino nodebug
# Replace the skip function to only build TinyUSB tests
function skip_ino()
{
    local ino=$1
    echo $ino | grep -q Adafruit_TinyUSB_Arduino
    echo $(( $? ))
}
build_sketches_with_arduino "$mod" "$rem" "--usbstack tinyusb"
build_sketches_with_arduino "$mod" "$rem" ""



rm -rf "$cache_dir"

