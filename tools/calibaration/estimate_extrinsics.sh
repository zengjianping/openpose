#!/bin/bash

data_dir="datas/calib_datas/test_datas/calib-3_stereo_from_JY"
camera_param_dir="$data_dir/camera_params"
grid_number="8x6"
square_size=60
num_cameras=2
copy_data=0

calib_image_dir="${data_dir}/extrinsics"
mkdir -p "$calib_image_dir"

if [ $copy_data == 1 ]; then
    for ((i=0; i<$num_cameras; i++))
    do
        let j=i+1
        serial_no=`printf "camera%02d" $j`
        echo "Processing $serial_no..."
        intrinsics_image_dir="${data_dir}/intrinsics/${serial_no}"
        cp ${intrinsics_image_dir}/images_undistorted/*.png ${calib_image_dir}/
    done
fi

for ((i=0; i<$num_cameras-1; i++))
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
        --grid_square_size_mm ${square_size} \
        --grid_number_inner_corners ${grid_number} \
        --camera_parameter_folder ${camera_param_dir}/ \
        --calibration_image_dir ${calib_image_dir} \
        --cam0 $i --cam1 $j $extra_param
done




