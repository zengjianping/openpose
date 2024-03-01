#!/bin/bash

data_dir="datas/calibration/test04"
grid_number="11x8"

for ((i=3; i<4; i++))
do
    let j=i+1
    serial_no=`printf "camera%02d" $j`
    echo "Processing $serial_no..."

    mkdir -p ${data_dir}/${serial_no}/
    cp ${data_dir}/intrinsics/*camera${i}.png ${data_dir}/${serial_no}/

    ./build/examples/calibration/calibration.bin \
        --mode 1 \
        --grid_square_size_mm 30.0 \
        --grid_number_inner_corners ${grid_number} \
        --camera_serial_number ${serial_no} \
        --camera_parameter_folder ${data_dir} \
        --calibration_image_dir ${data_dir}/${serial_no}
done



