#!/bin/bash

echo "compiling user ..."

cd user

rm -rf build && mkdir build

C_FLAGS="-O3 -ffreestanding -Wall -fPIC -static"

echo "compiling: aarch64 user directory"
C_FLAGS="$C_FLAGS -DCONFIG_ARCH_AARCH64"

cd build
cmake .. -DCMAKE_C_FLAGS="$C_FLAGS" -G Ninja

ninja

echo "succeed in compiling user."

echo "building ramdisk ..."

mkdir -p ramdisk
echo "copy user/*.bin to ramdisk."
cp process/*.bin ramdisk/
cp shell/*.bin ramdisk/
echo "copy user/*.srv to ramdisk."
cp tmpfs/*.srv tmpfs/*.bin ramdisk/

cd ramdisk
echo "add fs_test files"
cpio -idmv < ../../tmpfs/fs_test.cpio 2> /dev/null
find . | cpio -o -Hnewc > ../ramdisk.cpio
echo "succeed in building ramdisk."
