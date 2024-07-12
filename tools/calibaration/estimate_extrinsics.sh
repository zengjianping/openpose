#!/bin/bash

data_dir="datas/calib_datas/hikv_camera/test03"
camera_param_dir="$data_dir/camera_params"
grid_number="7x6"
square_size=120
num_cameras=4
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
    echo "Processing camera $i -> $j..."

    extra_param="--combine_cam0_extrinsics"
    #if [[ $i > 0 ]]; then
    #    extra_param="--combine_cam0_extrinsics"
    #fi

    ./build/examples/calibration/calibration.bin \
        --mode 2 --omit_distortion \
        --grid_square_size_mm ${square_size} \
        --grid_number_inner_corners ${grid_number} \
        --camera_parameter_folder ${camera_param_dir}/ \
        --calibration_image_dir ${calib_image_dir} \
        --cam0 $i --cam1 $j $extra_param
done




