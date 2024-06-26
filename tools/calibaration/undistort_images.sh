#!/bin/bash

data_dir="datas/calib_datas/hikv_camera/test01"
camera_param_dir="$data_dir/camera_params"
num_cameras=4

in_image_dir="${data_dir}/campose"
out_image_dir="${data_dir}/campose"
mkdir -p "$out_image_dir"

./build/examples/openpose/openpose.bin \
    --video ${in_image_dir} \
    --camera_parameter_path ${camera_param_dir} \
    --write_images ${out_image_dir} \
    --3d_views ${num_cameras} --num_gpu 0 \
    --frame_undistort --write_image_mode 1


