#!/usr/bin/python3

import optparse
import sys
import os

#Copy hello world to mean directory
#original
#command = f"cp -r /home/pepito/myhome/esp8266/ESP8266_RTOS_SDK/examples/get-started/hello_world /home/pepito/myhome/esp8266/"
#personal version
command = f"cp -r /home/pepito/myhome/esp8266/esp8266RTOS/examples/get-started/hello_world /home/pepito/myhome/esp8266/"
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")



