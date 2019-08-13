#!/bin/bash

cpu_cycles=$(cat nintendulator_nestest.log | grep -o "CPUC:.*" | awk -F  ":| " '{print $2}')
A=$(cat nintendulator_nestest.log | grep -o "A:.*" | awk -F  ":| " '{print $2}')
X=$(cat nintendulator_nestest.log | grep -o "X:.*" | awk -F  ":| " '{print $2}')
Y=$(cat nintendulator_nestest.log | grep -o "Y:.*" | awk -F  ":| " '{print $2}')
P=$(cat nintendulator_nestest.log | grep -o "P:.*" | awk -F  ":| " '{print $2}')
SP=$(cat nintendulator_nestest.log | grep -o "SP:.*" | awk -F  ":| " '{print $2}')
PC=$(cat nintendulator_nestest.log | grep "CPUC" | awk -F  ":| " '{print $1}')
OPCODE=$(cat nintendulator_nestest.log | grep "CPUC" | awk '{print $2}')

paste -d " " <(printf %s "$PC") \
             <(printf %s "$OPCODE") \
             <(printf %s "$A") \
             <(printf %s "$X") \
             <(printf %s "$Y") \
             <(printf %s "$P") \
             <(printf %s "$SP") \
             <(printf %s "$cpu_cycles")
