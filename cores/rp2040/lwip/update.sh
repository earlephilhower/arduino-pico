#!/usr/bin/bash

rm -rf ./src
for i in $(cd ../../../pico-sdk/lib/lwip/; find src -name *.c -o -name *.h | grep -v http); do
    dots=$(dirname $i | sed 's/\/.[a-z_46]*/\/../g' | sed 's/^[a-z_46]*/../')
    mkdir -p $(dirname $i)
    echo "#include \"$dots/../../../pico-sdk/lib/lwip/$i\"" > $i
done
