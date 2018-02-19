#!/bin/bash

VID="0483"
PID="df11"

dfu-util -l | grep $VID:$PID &> /dev/null
DFU_EXISTS=$?
if [ $DFU_EXISTS != 0 ]; then
  echo "Hold BOOT0 key (next to JTAG header) and press reset"
  while [ $DFU_EXISTS != 0 ]; do
    dfu-util -l | grep $VID:$PID &> /dev/null
    DFU_EXISTS=$?
  done
fi
echo "STM32F427 DFU device detected.  Uploading..."
dfu-util -d $VID:$PID -c 1 -a 0 -s 0x08000000:leave -D /tmp/arduino_build_392563/test_suite.ino.bin -R
echo "Done."