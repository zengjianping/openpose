#!/bin/bash

data_dir="datas/calib_datas/hikv_camera/test01"
camera_index=-1
task_name="intrinsics"
capture_fps=2
camera_resolution="1224x1024"
save_images=0

if [ $save_images != 1 ]; then
    out_image_dir=""
elif [ $camera_index >= 0 ]; then
    let j=$camera_index+1
    serial_no=`printf "camera%02d" $j`
    out_image_dir="$data_dir/$task_name/$serial_no"
    mkdir -p $out_image_dir
else
    out_image_dir="$data_dir/$task_name"
    mkdir -p $out_image_dir
fi

./build/examples/openpose/openpose.bin \
    --hikv_camera --num_gpu 0 \
    --hikv_camera_index $camera_index \
    --camera_trigger_mode 1 \
    --capture_fps $capture_fps \
    --camera_resolution $camera_resolution \
    --write_images $out_image_dir \
    --write_image_mode 1


