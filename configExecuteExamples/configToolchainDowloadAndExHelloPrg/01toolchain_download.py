#!/usr/bin/python3

import optparse
import sys
import os

#Toolchain for esp8266 in linux

command = f"wget https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz"
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")

#tar extact it. The toolchain will be extracted into directory/xtensa-lx106-elf/
command = f"tar -xzvf xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz"
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")


