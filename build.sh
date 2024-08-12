#!/bin/bash
export PICO_SDK_PATH=/opt/pico-sdk/
export PICO_EXTRAS_PATH=/opt/pico-extras/

mkdir -p "build/" && cd "build/"
if [[ "$?" != 0 ]]; then
    echo 'directory error!'
    exit
fi

cmake -DCMAKE_BUILD_TYPE=Release .. && make

if [[ "$?" != 0 ]]; then
    echo 'build failed at make or cmake!'
    exit
fi

if [[ "$1" != "" ]]; then
    echo "Press enter to view pico TTY"
    read
    screen /dev/ttyACM* 9600
fi
