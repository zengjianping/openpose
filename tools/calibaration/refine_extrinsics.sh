#!/bin/bash

data_dir="datas/calib_datas/hikv_camera/test03"
camera_param_dir="$data_dir/camera_params"
grid_number="7x6"
square_size=120
num_cameras=4

calib_image_dir="${data_dir}/extrinsics"

./build/examples/calibration/calibration.bin \
    --mode 3 --omit_distortion \
    --grid_square_size_mm ${square_size} \
    --grid_number_inner_corners ${grid_number} \
    --camera_parameter_folder ${camera_param_dir}/ \
    --calibration_image_dir ${calib_image_dir} \
    --number_cameras ${num_cameras}

