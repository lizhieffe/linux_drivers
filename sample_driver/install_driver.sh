#!/bin/sh

# Quit on error
set -e  

if sudo lsmod | grep sample_driver; then
  echo "Removing the existing driver with the same name..."
  sudo rmmod sample_driver
fi

make clean
make

echo
echo "Installing driver..."
sudo insmod sample_driver.ko
