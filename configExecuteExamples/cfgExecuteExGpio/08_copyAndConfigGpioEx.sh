#!/bin/bash
#Copy gpio example 
cp -r /home/pepito/myhome/esp8266/esp8266RTOS/examples/peripherals/gpio /home/pepito/myhome/esp8266/

#In the menu, navigate to Serial flashe config > Default serial port  to configue the seial
#port, where proyect will be loaded to. Confirrm selection by pressing enter, save configuation by
#selecting  <save> and then exit application by selecting <Exit>

#Set menu config
cd $HOME/myhome/esp8266/gpio
make menuconfig


