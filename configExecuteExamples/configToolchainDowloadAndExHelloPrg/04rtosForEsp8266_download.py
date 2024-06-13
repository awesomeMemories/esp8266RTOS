#!/usr/bin/python3

import optparse
import sys
import os

#Original RTOS for esp8266 in linux
#command = f"git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git"
#Personal version of  RTOS for esp8266 in linux
#command = f"git clone --recursive https://github.com/awesomeMemories/esp8266RTOS.git"


#Use --recursive: sobmodules.To obtain a complete and functional copy of the proyect.
#Without it, the submodules won't be downloaded, and your proyect might be missing essential
#components
#command = f"git clone --recursive https://github.com/espressif/ESP8266_RTOS_SDK.git"
command = f"git clone --recursive https://github.com/awesomeMemories/esp8266RTOS.git"
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")



