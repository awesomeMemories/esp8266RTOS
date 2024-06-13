#!/usr/bin/python3

import optparse
import sys
import os
import subprocess

#Able path to Tool, It only exist into this script, so when execute other command use again this setting
#command = f"get_lx106"
try:
    command = f"05_02_ableTool.sh"
    print(command)
    result = subprocess.run(["bash",command],capture_output=True, text=True, check=True)
    print("Script output")
    print(result.stdout)
except subprocess.CalledProcessError as e:
    print(f"Error executing the script: {e}")
    print(f"SOLUTION: Execute manual in shell   get_lx106")
#Note: if you don/t use get_lx106 is show error, but is ok if you don't use get_lx106


#Check path for tool. Remember it only exist into this script

command = f"echo $PATH"
print(command)
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")

command = f"echo $IDF_PATH"
print(command)
status = os.system(command)
if status == 0:
    print("Command executed successful")
else:
    print("Command execution failed")


