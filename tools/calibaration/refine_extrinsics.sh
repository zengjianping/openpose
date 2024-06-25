#!/bin/bash

data_dir="datas/calib_datas/test_datas/calib-3_stereo_from_JY"
camera_param_dir="$data_dir/camera_params"
grid_number="8x6"
square_size=60
num_cameras=2

calib_image_dir="${data_dir}/extrinsics"

./build/examples/calibration/calibration.bin \
    --mode 3 --omit_distortion \
    --grid_square_size_mm ${square_size} \
    --grid_number_inner_corners ${grid_number} \
    --camera_parameter_folder ${camera_param_dir}/ \
    --calibration_image_dir ${calib_image_dir} \
    --number_cameras ${num_cameras}

