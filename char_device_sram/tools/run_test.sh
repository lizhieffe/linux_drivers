#!/bin/sh

g++ -std=c++11 test_device.cc -o out
sudo ./out
rm out
