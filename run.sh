#!/bin/bash

echo
echo "Compiling with 'make'..."
make
echo

./find_image.exe
echo

echo "Executing small test..."
./find_image.exe 4 4 2 2 debug
echo

echo "Executing large test #1..."
./find_image.exe 1920 1080 1920 1080
echo

echo "Executing large test #2..."
./find_image.exe 1920 1080 24 24
echo
