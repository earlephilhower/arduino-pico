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

export PICO_BOARD=rp2040

install_arduino nodebug
build_sketches_with_arduino "$mod" "$rem" ""

rm -rf "$cache_dir"
