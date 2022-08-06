#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'

# Compile code for the microcontroller
echo -e "${RED}\nInstall Arm Toolchain...\n==================================\n${NC}"
sudo apt-get -y install gcc-arm-none-eabi

# Debug code on the microcontroller
echo -e "${RED}\nInstall openocd...\n==================================\n${NC}"
sudo apt-get -y install openocd

# GDB
echo -e "${RED}\nInstall gdb-multiarch...\n==================================\n${NC}"
sudo apt-get -y install gdb-multiarch

# Run make command and compile projects
echo -e "${RED}\nInstall make...\n==================================\n${NC}"
sudo apt-get -y install make