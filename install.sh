#!/bin/bash

RED='\033[0;31m'
NC='\033[0m'

# Compile code for the microcontroller
echo -e "${RED}\nInstall Arm Toolchain...\n==================================\n${NC}"
wget "https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi.tar.xz?rev=95edb5e17b9d43f28c74ce824f9c6f10&hash=176C4D884DBABB657ADC2AC886C8C095409547C4" --show-progress &&
mv "arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi.tar.xz?rev=95edb5e17b9d43f28c74ce824f9c6f10&hash=176C4D884DBABB657ADC2AC886C8C095409547C4" arm-none-eabi.tar.xz &&
echo "Extracting tar, this may take a few minutes" &&
tar -xf arm-none-eabi.tar.xz &&
rm arm-none-eabi.tar.xz &&
sudo mv arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi /usr/share/
sudo ln -s /usr/share/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc /usr/bin/arm-none-eabi-gcc
sudo ln -s /usr/share/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++ /usr/bin/arm-none-eabi-g++
sudo ln -s /usr/share/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-objcopy /usr/bin/arm-none-eabi-objcopy
sudo ln -s /usr/share/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-size /usr/bin/arm-none-eabi-size



# Debug code on the microcontroller
echo -e "${RED}\nInstall openocd...\n==================================\n${NC}"
sudo apt-get -y install openocd

# GDB
echo -e "${RED}\nInstall gdb-multiarch...\n==================================\n${NC}"
sudo apt-get -y install gdb-multiarch

# Run make command and compile projects
echo -e "${RED}\nInstall build-essential...\n==================================\n${NC}"
sudo apt-get -y install build-essential

# Make sure that necessary OS submodules are initialized
if [ -d ".git" ]; then
    echo -e "${RED}\nUpdate and initialize submodules...\n==================================\n${NC}"
    git submodule update --init --recursive
fi

echo -e "${RED}\nFinished! Jolly good!\n${NC}"