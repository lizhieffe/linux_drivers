#!/bin/sh

# Quit on error
set -e  

if sudo lsmod | grep char_device_sram; then
  echo "Removing the existing driver with the same name..."
  sudo rmmod char_device_sram
fi

make clean
make

echo
echo "Installing driver..."
sudo insmod char_device_sram.ko
