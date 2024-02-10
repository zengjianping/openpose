#!/bin/bash

data_dir="datas/calibration/test01"
serial_no="camera02"
grid_number="7x5"

./build/examples/calibration/calibration.bin \
    --mode 1 \
    --grid_square_size_mm 40.0 \
    --grid_number_inner_corners ${grid_number} \
    --camera_serial_number ${serial_no} \
    --camera_parameter_folder ${data_dir} \
    --calibration_image_dir ${data_dir}/${serial_no} \




