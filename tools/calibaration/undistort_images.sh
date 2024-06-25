#!/bin/bash

data_dir="datas/calib_datas/test_datas/calib-3_stereo_from_JY"
camera_param_dir="$data_dir/camera_params"
num_cameras=2

in_image_dir="${data_dir}/intrinsics"
out_image_dir="${data_dir}/extrinsics"
mkdir -p "$out_image_dir"

./build/examples/openpose/openpose.bin \
    --video ${in_image_dir} \
    --camera_parameter_path ${camera_param_dir} \
    --write_images ${out_image_dir} \
    --3d_views ${num_cameras} --num_gpu 0 \
    --frame_undistort --write_image_mode 1


