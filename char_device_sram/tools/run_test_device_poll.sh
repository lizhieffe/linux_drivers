#!/bin/sh

set -e

g++ -std=c++11 test_device_poll.cc -o out
echo "Build successfully!"

echo "Running test..."
sudo ./out

rm out
