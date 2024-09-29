#!/usr/bin/env bash

cache_dir=$(mktemp -d)

source "$GITHUB_WORKSPACE"/tests/common.sh

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

export FQBN=pico:rp2040:rpipico2:flash=4194304_0,arch=arm,freq=150,opt=Small,rtti=Disabled,stackprotect=Disabled,exceptions=Disabled,dbgport=Disabled,dbglvl=None,usbstack=picosdk,ipbtstack=ipv4only,uploadmethod=default
export PICO_BOARD=rp2350

install_arduino nodebug
build_sketches_with_arduino "$mod" "$rem" ""

rm -rf "$cache_dir"
