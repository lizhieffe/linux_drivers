#!/bin/sh

g++ -std=c++11 test_device_read_write.cc -o out
sudo ./out
rm out
