#!/usr/bin/env bash

# This script will download the libusb source code
# and compile the setupapi.dll linking against the static version of libusb.
# This is useful when libusb-1.0.so shipped with your Linux distro
# was compiled with SSE instruction set enabled.
# The SSE opcodes require the ram address to be aligned to 16 bytes boundary,
# while our library use a 4 bytes alignment compatible with windows software.
# This will prevent a segfault when any of libusb functions is invoked.


set -e
cd "$(dirname "$0")"

# Remove and recreate libusb directory
rm -rf libusb && mkdir libusb && cd libusb

# Clone and build libusb
git clone https://github.com/libusb/libusb.git .
./bootstrap.sh
./configure CFLAGS="-m32 -mstackrealign -fPIC" --prefix="$PWD" --disable-shared
make -j$(nproc) install

# Compile the setupapi.dll
cd ..
make clean
make -j$(nproc) CFLAGS="-Ilibusb/include/ -m32 -mstackrealign" \
     LIBS="-Wl,--whole-archive libusb/lib/libusb-1.0.a -Wl,--no-whole-archive -ludev"
