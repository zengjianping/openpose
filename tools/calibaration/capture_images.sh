#!/bin/bash

data_dir="datas/calibration/test04"

mkdir -p $data_dir

./build/examples/openpose/openpose.bin \
    --mind_camera --mind_camera_index -1 \
    --camera_trigger_mode 1 --capture_fps 2 \
    --camera_resolution "1224x1024" --num_gpu 0 \
    --write_images ${data_dir}/intrinsics \


