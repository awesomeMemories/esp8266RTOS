#!/usr/bin/python3

import optparse
import sys
import os

#Check USB
command = f"lsusb"
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")

#View tty for USB
command = f"ls -l /sys/class/tty/ttyUSB*"
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")


