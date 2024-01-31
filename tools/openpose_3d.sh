#!/bin/bash

data_dir="datas/pose_tests/panoptic/dance2a"

./build/examples/openpose/openpose.bin \
    --video ${data_dir}/video.mp4 \
    --camera_parameter_path ${data_dir}/cameras/ \
    --3d --3d_views 4 --3d_min_views 2 --number_people_max 1 \
    --frame_first 0 --frame_step 1 \
    --output_resolution "640x360"

#    --video ${data_dir}/video.mp4 \
#    --write_video tools/datas/scenes/panoptic_dance2/results/pose_2d.mp4 \
#    --write_video_3d tools/datas/scenes/panoptic_dance2/results/pose_3d.mp4 \
#    --write_video_fps 30
