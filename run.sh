#!/bin/bash

echo
echo "Compiling with 'make'..."
make
echo

./find_image.exe
echo

echo "Executing test #1..."
./find_image.exe 1920 1080 1920 1080 100
echo

echo "Executing test #2..."
./find_image.exe 1920 1080 1000 1000 100
echo

echo "Executing test #3..."
./find_image.exe 1920 1080 800 800 100
echo

echo "Executing test #4..."
./find_image.exe 1920 1080 700 700 100
echo

echo "Executing test #5..."
./find_image.exe 1920 1080 600 600 100
echo

echo "Executing test #6..."
./find_image.exe 1920 1080 500 500 100
echo

echo "Executing test #7..."
./find_image.exe 1920 1080 400 400 100
echo

echo "Executing test #8..."
./find_image.exe 1920 1080 300 300 100
echo

echo "Executing test #9..."
./find_image.exe 1920 1080 200 200 100
echo

echo "Executing test #10..."
./find_image.exe 1920 1080 120 120 100
echo

echo "Executing test #11..."
./find_image.exe 1920 1080 60 60 100
echo

echo "Executing test #12..."
./find_image.exe 1920 1080 40 40 100
echo

echo "Executing test #13..."
./find_image.exe 1920 1080 30 30 100
echo

echo "Executing test #14..."
./find_image.exe 1920 1080 20 20 100
echo

echo "Executing test #15..."
./find_image.exe 1920 1080 15 15 100
echo

echo "Executing test #16..."
./find_image.exe 1920 1080 12 12 100
echo

echo "Executing test #17..."
./find_image.exe 1920 1080 10 10 100
echo

echo "Executing test #18..."
./find_image.exe 1920 1080 8 8 100
echo

echo "Executing test #19..."
./find_image.exe 1920 1080 6 6 100
echo

echo "Executing test #20..."
./find_image.exe 1920 1080 5 5 100
echo

echo "Executing test #21..."
./find_image.exe 1920 1080 4 4 100
echo

echo "Executing test #22..."
./find_image.exe 1920 1080 3 3 100
echo

echo "Executing test #23..."
./find_image.exe 1920 1080 2 2 100
echo

echo "Executing test #24..."
./find_image.exe 1920 1080 1 1 100
echo
