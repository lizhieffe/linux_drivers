#!/bin/sh

# Quit on error
set -e  

if sudo lsmod | grep platform_driver; then
  echo "Removing the existing driver with the same name..."
  sudo rmmod platform_driver
fi

make clean
make

echo
echo "Installing driver..."
sudo insmod platform_driver.ko
