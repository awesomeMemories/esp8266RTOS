#!/bin/bash
#list port
ls /dev/ttyUSB*

#If you're not in the dialout group, add yourself using
sudo usermod -a -G dialout $USER

#permissions.... full read, write and execute permissions to everyone
sudo chmod -R 777 /dev/ttyUSB0


