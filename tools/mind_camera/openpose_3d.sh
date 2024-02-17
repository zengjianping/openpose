#!/bin/bash

data_dir="datas/pose_tests/mind_camera/test03"

./build/examples/openpose/openpose.bin \
    --mind_camera --mind_camera_index -1 --camera_trigger_mode 1 \
    --body 1 --num_gpu 1 --camera_resolution "1224x1024" --capture_fps 5 \
    --camera_parameter_path ${data_dir}/cameras/ \
    --3d --3d_views 2 --3d_min_views 2 --number_people_max 1 \
    --frame_undistort --output_resolution "612x512"

#    --video ${data_dir}/video.mp4 \
#    --write_video tools/datas/scenes/panoptic_dance2/results/pose_2d.mp4 \
#    --write_video_3d tools/datas/scenes/panoptic_dance2/results/pose_3d.mp4 \
#    --write_video_fps 30
