#!/bin/bash

rm cork

mkdir build
cd build
cmake ..
make
cp cork ../cork
cd ..
