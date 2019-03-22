#!/bin/bash

mkdir build
cd build

# cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_UWS=ON -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++
cmake .. -DCMAKE_BUILD_TYPE=Release
make
