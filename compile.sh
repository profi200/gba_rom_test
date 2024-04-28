#!/bin/bash

make clean && make

PADDING_SIZE="$((33554432 - $(wc -c < ./gba_rom_test.gba)))"
< /dev/zero tr '\000' '\125' | head -c "$PADDING_SIZE" >> ./gba_rom_test.gba
echo "Added $PADDING_SIZE bytes of 0x55 padding."

HASH="$(sha1sum -b ./gba_rom_test.gba | cut -d ' ' -f 1)"
mv ./gba_rom_test.gba "./gba_rom_test_$HASH.gba"
echo "ROM SHA1: $HASH"