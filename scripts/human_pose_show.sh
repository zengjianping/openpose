#!/bin/bash

ROOT_PATH=$(cd "$(dirname $(dirname "$0"))"; pwd)
cd $ROOT_PATH
echo "Work directory: $(pwd)"

export LD_LIBRARY_PATH=/opt/MVS/lib/64:./lib:$LD_LIBRARY_PATH

#./build/examples/HumanPoseShow/HumanPoseShow.bin
./bin/HumanPoseShow.bin

