#!/bin/bash

### INSTALL PREREQUISITES

UBUNTU_VERSION="$(lsb_release -r)"

# Basic
sudo apt-get --assume-yes update
sudo apt-get --assume-yes install build-essential
# General dependencies
sudo apt-get --assume-yes install libatlas-base-dev libprotobuf-dev libleveldb-dev libsnappy-dev libhdf5-serial-dev protobuf-compiler
sudo apt-get --assume-yes install libboost-all-dev libopencv-dev libceres-dev
sudo apt-get --assume-yes install libopengl-dev libglu1-mesa-dev freeglut3 freeglut3-dev libxmu-dev libxi-dev
# Remaining dependencies
sudo apt-get --assume-yes install qtbase5-dev qt5-qmake qtbase5-dev-tools
sudo apt-get --assume-yes install libgflags-dev libgoogle-glog-dev liblmdb-dev
sudo apt-get --assume-yes install libyaml-cpp-dev libjsoncpp-dev libcrypto++-dev


