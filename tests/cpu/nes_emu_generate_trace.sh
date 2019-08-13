#!/bin/bash

../../build/nes_emu nestest.nes > nes_emu.log

cpu_cycles=$(cat nes_emu.log | grep -o "CYC:.*" | awk -F  ":| " '{print $2}')
A=$(cat nes_emu.log | grep -o "A:.*" | awk -F  ":| " '{print $2}')
X=$(cat nes_emu.log | grep -o "X:.*" | awk -F  ":| " '{print $2}')
Y=$(cat nes_emu.log | grep -o "Y:.*" | awk -F  ":| " '{print $2}')
P=$(cat nes_emu.log | grep -o "P:.*" | awk -F  ":| " '{print $2}')
SP=$(cat nes_emu.log | grep -o "SP:.*" | awk -F  ":| " '{print $2}')
PC=$(cat nes_emu.log | grep -o "PC:.*" | awk -F  ":| " '{print $2}')
OPCODE=$(cat nes_emu.log | grep -o "opcode:.*" | awk -F  ":| " '{print $2}')

paste -d " " <(printf %s "$PC") \
             <(printf %s "$OPCODE") \
             <(printf %s "$A") \
             <(printf %s "$X") \
             <(printf %s "$Y") \
             <(printf %s "$P") \
             <(printf %s "$SP") \
             <(printf %s "$cpu_cycles")
