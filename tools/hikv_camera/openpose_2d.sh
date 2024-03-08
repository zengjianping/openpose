#!/bin/bash

./build/examples/openpose/openpose.bin \
    --hikv_camera --hikv_camera_index -1 --camera_trigger_mode 1 \
    --capture_fps -1 --batch_process --net_resolution "-1x256" \
    --camera_resolution "1224x1024" \
    --body 1 --num_gpu 1  --profile_speed 100

