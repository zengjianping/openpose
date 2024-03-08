#!/bin/bash

data_dir="datas/pose_tests/mind_camera/test03/cameras"
grid_number="11x8"

./build/examples/calibration/calibration.bin \
    --mode 3 --omit_distortion \
    --grid_square_size_mm 30.0 \
    --grid_number_inner_corners ${grid_number} \
    --camera_parameter_folder ${data_dir}/ \
    --calibration_image_dir ${data_dir}/extrinsics \
    --number_cameras 4

