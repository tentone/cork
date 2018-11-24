#!/bin/bash

mkdir build
cd build
cmake ..
make
cp cork ../cork
cp save_image ../save_image
cd ..
