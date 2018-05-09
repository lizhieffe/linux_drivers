#!/bin/sh

if [ "$#" -ne 1 ]; then
  echo "Illegal number of parameters!"
  exit
fi

SAMPLE_DRIVER_NAME="sample_driver"

cd $HOME/development/linux_drivers
cp -rf "$SAMPLE_DRIVER_NAME" "$1"
cd "$1"
make clean
mv "$SAMPLE_DRIVER_NAME".c "$1".c
sed -i "s/"$SAMPLE_DRIVER_NAME"/"$1"/g" Makefile
