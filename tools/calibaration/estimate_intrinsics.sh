#!/bin/bash

data_dir="datas/calib_datas/hikv_camera/test01"
camera_param_dir="$data_dir/camera_params"
grid_number="11x8"
square_size=30
num_cameras=4
copy_data=0

mkdir -p "$camera_param_dir"

for ((i=0; i<$num_cameras; i++))
do
    let j=i+1
    serial_no=`printf "camera%02d" $j`
    echo "Processing $serial_no..."

    calib_image_dir="${data_dir}/intrinsics/${serial_no}"

    if [ $copy_data == 1 ]; then
        mkdir -p $calib_image_dir
        cp ${data_dir}/intrinsics/*camera_${i}.png $calib_image_dir/
    fi

    ./build/examples/calibration/calibration.bin \
        --mode 1 \
        --grid_square_size_mm $square_size \
        --grid_number_inner_corners ${grid_number} \
        --camera_serial_number ${serial_no} \
        --camera_parameter_folder ${camera_param_dir} \
        --calibration_image_dir ${calib_image_dir}
done



