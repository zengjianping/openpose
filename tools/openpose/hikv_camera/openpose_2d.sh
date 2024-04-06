#!/bin/bash

./build/examples/openpose/openpose.bin \
    --hikv_camera --hikv_camera_index -1 --camera_trigger_mode 1 \
    --capture_fps 50 --net_resolution "-1x256" \
    --camera_resolution "960x768" --crop_image \
    --trigger_save_video --image_cache_time 1.0 --trigger_save_time 2.0 \
    --write_video datas/results/output.mp4 \
    --body 1 --num_gpu 0 --batch_process --profile_speed 100

