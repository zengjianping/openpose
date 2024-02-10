#!/bin/bash

data_dir="datas/calibration/test02"
serial_no="camera02"
grid_number="8x6"

./build/examples/openpose/openpose.bin \
    --image_dir ${data_dir}/${serial_no} \
    --camera_parameter_path ${data_dir}/${serial_no}.xml \
    --write_images ${data_dir}/extrinsics \
    --frame_undistort 




