#!/bin/bash
#Copy gpio example 
cp -r /home/pepito/myhome/esp8266/esp8266RTOS/examples/wifi/getting_started/station /home/pepito/myhome/esp8266/

#In the menu, navigate to Serial flashe config > Default serial port  to configue the seial
#port, where proyect will be loaded to. Confirrm selection by pressing enter, save configuation by
#selecting  <save> and then exit application by selecting <Exit>

#Mode soft AP
#In the menu, navigate to Example Configuration > (myssid) WiFi SSID > 
# >   WiFi Password
# >   (4) Maximal STA connections
#it will save in softAP/sdkconfig. 
#You can change in this file the  ssid and password
#CONFIG_ESP_WIFI_SSID="llllimyssid"
#CONFIG_ESP_WIFI_PASSWORD="mypassword"
#CONFIG_ESP_MAX_STA_CONN=4


#Set menu config
cd $HOME/myhome/esp8266/station
make menuconfig


