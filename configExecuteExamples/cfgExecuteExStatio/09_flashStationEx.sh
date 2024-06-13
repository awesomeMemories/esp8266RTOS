#!/bin/bash

#If you have the next error
#[Errno 13] could not open port /dev/ttyUSB0: [Errno 13] Permission denied: '/dev/ttyUSB0
# execute the 09_01_to_able_port.sh  in the first steps (use to execute hello world example)

#compile program and flash to card chip
cd $HOME/myhome/esp8266/station
make flash


