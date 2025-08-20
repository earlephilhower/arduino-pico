#!/bin/bash
#
# CI job which builds the boards manager package

set -ev

export PKG_URL=https://github.com/esp8266/Arduino/releases/download/$GITHUB_REF/esp8266-$GITHUB_REF.zip
export DOC_URL=https://arduino-esp8266.readthedocs.io/en/$GITHUB_REF/

if [ -z "$CI_GITHUB_API_KEY" ]; then
    echo "Github API key not set. Skip building the package."
    exit 0
fi

cd $GITHUB_WORKSPACE/package
./build_boards_manager_package.sh
