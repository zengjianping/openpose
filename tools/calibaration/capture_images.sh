#!/bin/bash

data_dir="datas/calib_datas/hikv_camera/test01"
camera_resolution="1224x1024"
camera_index=-1
capture_fps=2
task_name="extrinsics"
save_images=1
write_mode=2

if [ $save_images -eq 0 ]; then
    save_param=""
else
    if [ $camera_index -ge 0 ]; then
        let j=$camera_index+1
        serial_no=`printf "camera%02d" $j`
        out_image_dir="$data_dir/$task_name/$serial_no"
    else
        out_image_dir="$data_dir/$task_name"
    fi
    mkdir -p $out_image_dir
    save_param="--write_images $out_image_dir"
fi

./build/examples/openpose/openpose.bin \
    --hikv_camera --num_gpu 0 \
    --hikv_camera_index $camera_index \
    --camera_trigger_mode 1 \
    --capture_fps $capture_fps \
    --camera_resolution $camera_resolution \
    --write_image_mode $write_mode $save_param


