#!/bin/bash

#data_dir="datas/pose_tests/panoptic/dance2a"
data_dir="../GolfDataExchange"

./build/examples/openpose/openpose.bin \
    --video ${data_dir}/golf_test.mp4 \
    --camera_parameter_path ${data_dir}/calibrations/golf_test/ \
    --3d --3d_views 2 --3d_min_views 2 --number_people_max 1 \
    --frame_first 0 --frame_step 1 --output_resolution "640x360" \
    --body 1 --num_gpu 1 --batch_process --display 1 \
    --net_resolution -1x256 --profile_speed 100

#    --output_resolution "640x360" \
#    --video ${data_dir}/video.mp4 \
#    --write_video tools/datas/scenes/panoptic_dance2/results/pose_2d.mp4 \
#    --write_video_3d tools/datas/scenes/panoptic_dance2/results/pose_3d.mp4 \
#    --write_video_fps 30 --process_real_time
