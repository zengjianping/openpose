#!/bin/bash

./build/examples/openpose/openpose.bin \
    --hikv_camera --hikv_camera_index -1 --camera_trigger_mode 1 \
    --capture_fps 50 --net_resolution "-1x256" \
    --camera_resolution "1280x1024" \
    --write_video datas/results/output.mp4 \
    --body 1 --num_gpu 0 --batch_process --profile_speed 100

