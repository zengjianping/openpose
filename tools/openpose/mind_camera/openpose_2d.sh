#!/bin/bash

gdb --args ./build/examples/openpose/openpose.bin \
    --mind_camera --mind_camera_index -1 --camera_trigger_mode 1 \
    --capture_fps 50 --net_resolution "-1x256" \
    --camera_resolution "1224x1024" \
    --write_video datas/results/output.mp4 --trigger_save_video \
    --body 1 --num_gpu 0 --batch_process --profile_speed 300

