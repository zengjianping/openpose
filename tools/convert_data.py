
import os, sys, math
import argparse
import json, cv2
import numpy as np


def convert_camera_params():
    #data_dir = 'datas/camera_params/panoptic/dance2'
    data_dir = 'datas/camera_params/panoptic/ultimatum1'
    in_file = os.path.join(data_dir, 'calibration.json')
    out_dir = os.path.join(data_dir, 'results')
    os.makedirs(out_dir, exist_ok=True)

    cameras = json.loads(open(in_file, 'r', encoding='utf-8').read())['cameras']
    for camera in cameras:
        if camera['type'] == 'hd':
            out_file = os.path.join(out_dir, camera['name']+'.xml')
            extrinsic_mat = np.zeros((3,4), dtype=np.float64)
            extrinsic_mat[0:3,0:3] = camera['R']
            extrinsic_mat[0:3,3:4] = camera['t']
            extrinsic_mat[0:3,3:4] /= 100
            intrinsic_mat = np.array(camera['K'], dtype=np.float64)
            distortion_mat = np.array(camera['distCoef'], dtype=np.float64)
            fs = cv2.FileStorage(out_file, cv2.FileStorage_WRITE)
            fs.write('CameraMatrix', extrinsic_mat)
            fs.write('Intrinsics', intrinsic_mat)
            fs.write('Distortion', distortion_mat)
            fs.release()

def parse_args():
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('--work_mode', dest='work_mode', type=str,
                        help='configure file name')
    args = parser.parse_args()
    return args

def main():
    args = parse_args()
    convert_camera_params()

if __name__ == '__main__':
    main()

