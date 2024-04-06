#!/bin/bash

./build/examples/openpose/openpose.bin \
    --mind_camera --mind_camera_index -1 --camera_trigger_mode 1 \
    --capture_fps 50 --net_resolution "-1x256" \
    --camera_resolution "1224x1024" --crop_image \
    --trigger_save_video \
    --write_video ../GolfDataExchange/output.mp4 \
    --body 1 --num_gpu 0 --batch_process --profile_speed 300

