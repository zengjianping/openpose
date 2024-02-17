#!/bin/bash

./build/examples/openpose/openpose.bin \
    --mind_camera --mind_camera_index -1 --camera_trigger_mode 1 \
    --body 1 --num_gpu 1 --camera_resolution "1224x1024" --capture_fps 5

