#!/bin/bash

# c++ pre build
sudo apt-get install libssl-dev
git submodule update --init
git submodule update --init --recursive -- cpp/thirdparty/uWebSockets/
