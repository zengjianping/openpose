#!/bin/bash

data_dir="datas/calibration/test04"
grid_number="11x8"

mkdir -p ${data_dir}/extrinsics

for ((i=0; i<3; i++))
do
    let j=i+1
    serial_no=`printf "camera%02d" $j`
    echo "Processing $serial_no..."

    cp ${data_dir}/${serial_no}/images_undistorted/*.png ${data_dir}/extrinsics/
done

for ((i=0; i<4; i++))
do
    let j=i+1
    serial_no=`printf "camera%02d" $j`
    echo "Processing $i -> $j"

    extra_param=
    if [[ $i > 0 ]]; then
        extra_param="--combine_cam0_extrinsics"
    fi

    ./build/examples/calibration/calibration.bin \
        --mode 2 --omit_distortion \
        --grid_square_size_mm 30.0 \
        --grid_number_inner_corners ${grid_number} \
        --camera_parameter_folder ${data_dir}/ \
        --calibration_image_dir ${data_dir}/extrinsics \
        --cam0 $i --cam1 $j $extra_param
done




