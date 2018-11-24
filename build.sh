#!/bin/bash

mkdir build
cd build
cmake ..
make
cp cork ../cork
cd ..
