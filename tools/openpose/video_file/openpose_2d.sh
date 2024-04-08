#!/bin/bash

./build/examples/openpose/openpose.bin \
    --video datas/pose_tests/panoptic/dance2a/videos/ \
    --model_pose BODY_25 --batch_process \
    --body 1 --num_gpu 1 --net_resolution -1x192

#    --mind_camera --mind_camera_index -1 --camera_trigger_mode 1 \
#    --body 1 --num_gpu 1 --camera_resolution "1224x1024" --capture_fps 5
#    --write_images datas/output_images \
#    --body 1 --disable_multi_thread --logging_level 2 --num_gpu 0 --camera_resolution "1280x720" --profile_speed 10
#    --write_video  datas/output.mp4 --display 1 --render_pose 1 --write_video_fps 10 \
#    --camera_resolution "1920x1080"
#    --mind_camera --mind_camera_index 0 --camera_trigger_mode 1
#    --video datas/golf_videos/test01.mp4 \
#    --camera 0 \
#    --face --hand \
