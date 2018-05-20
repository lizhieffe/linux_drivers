#!/bin/sh

# Quit on error
set -e  

if sudo lsmod | grep platform_device; then
  echo "Removing the existing driver with the same name..."
  sudo rmmod platform_device
fi

make clean
make

echo
echo "Installing driver..."
sudo insmod platform_device.ko
