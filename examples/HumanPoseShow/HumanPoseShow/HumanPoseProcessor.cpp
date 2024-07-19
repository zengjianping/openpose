#include "HumanPoseProcessor.h"
#include "StereoPoseRender.hpp"
#include "openpose/headers.hpp"
#include <openpose/utilities/profiler.hpp>
#include <json/json.h>
#include "CameraApi.h"
#include "MvCameraControl.h"
#include <iostream>
#include <cryptopp/md5.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
 


class HumanPoseProcessorOP : public HumanPoseProcessor
{
public:
    HumanPoseProcessorOP();
    virtual ~HumanPoseProcessorOP();

public:
    void setCallback(HumanPoseProcessorCallback* callback) override;
    bool start(const HumanPoseParams& params) override;
    void stop() override;
    bool isRunning() override;

private:
    HumanPoseProcessorCallback* callback_;
    std::shared_ptr<op::Wrapper> opWrapper_;
    std::chrono::time_point<std::chrono::high_resolution_clock> opTimer_;
};

std::shared_ptr<HumanPoseProcessor> HumanPoseProcessor::createInstance()
{
    std::shared_ptr<HumanPoseProcessor> processor;
    processor.reset(new HumanPoseProcessorOP());
    return processor;
}


HumanPoseProcessorOP::HumanPoseProcessorOP()
{
    callback_ = nullptr;
}

HumanPoseProcessorOP::~HumanPoseProcessorOP()
{
}

void HumanPoseProcessorOP::setCallback(HumanPoseProcessorCallback* callback)
{
    callback_ = callback;
}

void configureWrapper(op::Wrapper& opWrapper, const HumanPoseParams& params, HumanPoseProcessorCallback* callback);

bool HumanPoseProcessorOP::start(const HumanPoseParams& params)
{
    try
    {
        op::opLog("Starting OpenPose demo...", op::Priority::High);
        opTimer_ = op::getTimerInit();

        // Configure OpenPose
        op::opLog("Configuring OpenPose...", op::Priority::High);
        opWrapper_.reset(new op::Wrapper());
        //opWrapper_.reset(new op::Wrapper(op::ThreadManagerMode::AsynchronousOut));
        configureWrapper(*opWrapper_, params, callback_);

        // Start, run, and stop processing - exec() blocks this thread until OpenPose wrapper has finished
        op::opLog("Starting thread(s)...", op::Priority::High);
        opWrapper_->start();

        // Return successful message
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

void HumanPoseProcessorOP::stop()
{
    try
    {
        if (opWrapper_.get())
        {
            if (opWrapper_->isRunning())
                opWrapper_->stop();

            // Measuring total time
            op::printTime(opTimer_, "OpenPose demo successfully finished. Total time: ", " seconds.", op::Priority::High);
        }
        opWrapper_.reset();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

bool HumanPoseProcessorOP::isRunning()
{
    return opWrapper_.get() && opWrapper_->isRunning();
}

bool enumerateMindCamera(std::vector<std::string>& cameraNames)
{
    tSdkCameraDevInfo cameraList[16];
    int cameraCount = 15;
    CameraSdkStatus status = CameraEnumerateDevice(cameraList, &cameraCount);

    if (status == CAMERA_STATUS_SUCCESS && cameraCount > 0)
    {
        std::sort(&cameraList[0], &cameraList[cameraCount], [](tSdkCameraDevInfo& a, tSdkCameraDevInfo& b)
                                                              {return strcmp(a.acSn, b.acSn) <= 0;});
        for (int i = 0; i < cameraCount; i++)
            cameraNames.push_back(cameraList[i].acSn);
        return true;
    }
    return false;
}

bool enumerateHikvCamera(std::vector<std::string>& cameraNames)
{
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);
    int cameraCount = stDeviceList.nDeviceNum;

    if (MV_OK == nRet || cameraCount > 0)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo[MV_MAX_DEVICE_NUM];
        for (int i = 0; i < cameraCount; i++)
            pDeviceInfo[i] = stDeviceList.pDeviceInfo[i];
        std::sort(&pDeviceInfo[0], &pDeviceInfo[cameraCount], [](MV_CC_DEVICE_INFO* a, MV_CC_DEVICE_INFO* b)
                {return strcmp((char*)&a->SpecialInfo.stUsb3VInfo.chSerialNumber[0],
                                (char*)&b->SpecialInfo.stUsb3VInfo.chSerialNumber[0]) <= 0;});
        for (int i = 0; i < cameraCount; i++)
            cameraNames.push_back((char*)&pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chSerialNumber[0]);
        return true;
    }
    return false;
}

bool queryCameraList(const HumanPoseParams& params, std::string& cameraType,
    std::vector<std::string>& cameraNames)
{
    if (params.inputParams.inputType == HumanPoseParams::InputParams::MindCamera)
    {
        if (enumerateMindCamera(cameraNames))
        {
            if (cameraNames.size() > 0)
            {
                cameraType = "迈德威视相机";
                return true;
            }
        }
    }
    else if (params.inputParams.inputType == HumanPoseParams::InputParams::HikvCamera)
    {
        if (enumerateHikvCamera(cameraNames))
        {
            if (cameraNames.size() > 0)
            {
                cameraType = "海康威视相机";
                return true;
            }
        }
    }

    return false;
}

bool calibrateCameraIntrinsics(const std::string& cameraName, const std::string& imageDir,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize)
{
    try
    {
        int flags = cv::CALIB_RATIONAL_MODEL; // 8 parameters
        bool saveImagesWithCorners = true;
        float gridSqureSizeMm = (float)gridSize;
        op::Point<int> gridInnerCorners = op::flagsToPoint(op::String(gridLayout), "11x8");

        // Run calibration
        op::opLog("Running calibration (intrinsic parameters)...", op::Priority::High);
        op::estimateAndSaveIntrinsics(gridInnerCorners, gridSqureSizeMm, flags,
            op::formatAsDirectory(calibrateDir), imageDir, cameraName,
            saveImagesWithCorners, false);
        op::opLog("Intrinsic calibration completed!", op::Priority::High);
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

bool calibrateCameraExtrinsics(const std::vector<std::string>& cameraNames, const std::string& imageDir,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize)
{
    try
    {
        float gridSqureSizeMm = (float)gridSize;
        op::Point<int> gridInnerCorners = op::flagsToPoint(op::String(gridLayout), "11x8");

        for (size_t i = 0; i < cameraNames.size() - 1; i++)
        {
            // Run calibration
            op::opLog("Running calibration (extrinsic parameters)...", op::Priority::High);
            op::estimateAndSaveExtrinsics(op::formatAsDirectory(calibrateDir), imageDir,
                gridInnerCorners, gridSqureSizeMm, i, i+1, true, true);//i > 0);
            op::opLog("Extrinsic calibration completed!", op::Priority::High);
        }
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

bool calibrateCameraPose(const std::vector<std::string>& cameraNames, const std::string& imageDir,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize)
{
    try
    {
        float gridSqureSizeMm = (float)gridSize;
        op::Point<int> gridInnerCorners = op::flagsToPoint(op::String(gridLayout), "11x8");

        for (size_t i = 0; i < cameraNames.size(); i++)
        {
            // Run calibration
            op::opLog("Running calibration (camera pose parameters)...", op::Priority::High);
            op::estimateAndSaveCameraPose(op::formatAsDirectory(calibrateDir), imageDir,
                gridInnerCorners, gridSqureSizeMm, i, true);
            op::opLog("Extrinsic calibration completed!", op::Priority::High);
        }
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}

bool refineCameraExtrinsics(const std::vector<std::string>& cameraNames, const std::string& imageDir,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize)
{
    try
    {
        bool saveImagesWithCorners = true;
        float gridSqureSizeMm = (float)gridSize;
        op::Point<int> gridInnerCorners = op::flagsToPoint(op::String(gridLayout), "11x8");

        op::opLog("Running calibration (refine extrinsics parameters)...", op::Priority::High);
        op::refineAndSaveExtrinsics(op::formatAsDirectory(calibrateDir), imageDir,
            gridInnerCorners, gridSqureSizeMm, cameraNames.size(), true, true);
        op::opLog("Extrinsics refine completed!", op::Priority::High);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
}


// This worker will just read and return all the jpg files in a directory
class WUserOutput : public op::WorkerConsumer<std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>>>
{
public:
    WUserOutput(HumanPoseProcessorCallback* callback, bool stereoPose, bool displayImage, bool printInfo)
    {
        callback_ = callback;
        stereoPose_ = stereoPose;
        //if (stereoPose)
        //    stereoPoseRender_.reset(new op::StereoPoseRender(true));
        displayImage_ = displayImage;
        printInfo_ = printInfo;
    }

    ~WUserOutput()
    {
    }

    void initializationOnThread()
    {
        if (stereoPoseRender_.get())
            stereoPoseRender_->initialize();
    }

    void workConsumer(const std::shared_ptr<std::vector<std::shared_ptr<op::Datum>>>& datumsPtr)
    {
        try
        {
            // User's displaying/saving/other processing here
                // datumPtr->cvOutputData: rendered frame with pose or heatmaps
                // datumPtr->poseKeypoints: Array<float> with the estimated pose
            if (datumsPtr != nullptr && !datumsPtr->empty())
            {
                if (stereoPoseRender_.get())
                {
                    // Profiling speed
                    const auto profilerKey = op::Profiler::timerInit(__LINE__, __FUNCTION__, __FILE__);
                    // Update keypoints
                    auto& tDatumPtr = (*datumsPtr)[0];
                    stereoPoseRender_->setKeypoints(
                        tDatumPtr->poseKeypoints3D, tDatumPtr->faceKeypoints3D, tDatumPtr->handKeypoints3D[0],
                        tDatumPtr->handKeypoints3D[1]);
                    // Refresh/update GUI
                    stereoPoseRender_->update();
                    // Read OpenCV mat equivalent
                    tDatumPtr->cvOutputData3D = stereoPoseRender_->readCvMat();
                    cv::Mat cvMat = OP_OP2CVMAT(tDatumPtr->cvOutputData3D);
                    callback_->set3dPoseImage(cvMat);
                    // Profiling speed
                    op::Profiler::timerEnd(profilerKey);
                    op::Profiler::printAveragedTimeMsOnIterationX(profilerKey, __LINE__, __FUNCTION__, __FILE__);
                }
                else if (stereoPose_)
                {
                    auto& tDatumPtr = (*datumsPtr)[0];
                    callback_->setKeypoints(
                        tDatumPtr->poseKeypoints3D, tDatumPtr->faceKeypoints3D, tDatumPtr->handKeypoints3D[0],
                        tDatumPtr->handKeypoints3D[1]);
                }

                for (size_t i = 0; i < datumsPtr->size(); i++)
                {
                    // Display results (if enabled)
                    if (displayImage_ && callback_)
                    {
                        // Display rendered output image
                        const cv::Mat cvMat = OP_OP2CVCONSTMAT(datumsPtr->at(i)->cvOutputData);
                        if (!cvMat.empty())
                        {
                            callback_->set2dPoseImage(i, cvMat);
                        }
                        else
                        {
                            op::opLog("Empty cv::Mat as output.", op::Priority::High, __LINE__, __FUNCTION__, __FILE__);
                        }

                    }

                    if (printInfo_)
                    {
                        // Show in command line the resulting pose keypoints for body, face and hands
                        op::opLog("\nKeypoints:");
                        // Accessing each element of the keypoints
                        const auto& poseKeypoints = datumsPtr->at(i)->poseKeypoints;
                        op::opLog("Person pose keypoints:");
                        for (auto person = 0 ; person < poseKeypoints.getSize(0) ; person++)
                        {
                            op::opLog("Person " + std::to_string(person) + " (x, y, score):");
                            for (auto bodyPart = 0 ; bodyPart < poseKeypoints.getSize(1) ; bodyPart++)
                            {
                                std::string valueToPrint;
                                for (auto xyscore = 0 ; xyscore < poseKeypoints.getSize(2) ; xyscore++)
                                {
                                    valueToPrint += std::to_string(   poseKeypoints[{person, bodyPart, xyscore}]   ) + " ";
                                }
                                op::opLog(valueToPrint);
                            }
                        }

                        op::opLog("\n3D Keypoints:");
                        // Accessing each element of the keypoints
                        const auto& poseKeypoints3D = datumsPtr->at(i)->poseKeypoints3D;
                        op::opLog("Person pose 3d keypoints:");
                        for (auto person = 0 ; person < poseKeypoints3D.getSize(0) ; person++)
                        {
                            op::opLog("Person " + std::to_string(person) + " (x, y, z, score):");
                            for (auto bodyPart = 0 ; bodyPart < poseKeypoints3D.getSize(1) ; bodyPart++)
                            {
                                std::string valueToPrint;
                                for (auto xyscore = 0 ; xyscore < poseKeypoints3D.getSize(2) ; xyscore++)
                                {
                                    valueToPrint += std::to_string(   poseKeypoints3D[{person, bodyPart, xyscore}]   ) + " ";
                                }
                                op::opLog(valueToPrint);
                            }
                        }

                        op::opLog(" ");
                        // Alternative: just getting std::string equivalent
                        op::opLog("Face keypoints: " + datumsPtr->at(i)->faceKeypoints.toString());
                        op::opLog("Left hand keypoints: " + datumsPtr->at(i)->handKeypoints[0].toString());
                        op::opLog("Right hand keypoints: " + datumsPtr->at(i)->handKeypoints[1].toString());

                        // Heatmaps
                        const auto& poseHeatMaps = datumsPtr->at(i)->poseHeatMaps;
                        if (!poseHeatMaps.empty())
                        {
                            op::opLog("Pose heatmaps size: [" + std::to_string(poseHeatMaps.getSize(0)) + ", "
                                    + std::to_string(poseHeatMaps.getSize(1)) + ", "
                                    + std::to_string(poseHeatMaps.getSize(2)) + "]");
                            const auto& faceHeatMaps = datumsPtr->at(i)->faceHeatMaps;
                            op::opLog("Face heatmaps size: [" + std::to_string(faceHeatMaps.getSize(0)) + ", "
                                    + std::to_string(faceHeatMaps.getSize(1)) + ", "
                                    + std::to_string(faceHeatMaps.getSize(2)) + ", "
                                    + std::to_string(faceHeatMaps.getSize(3)) + "]");
                            const auto& handHeatMaps = datumsPtr->at(i)->handHeatMaps;
                            op::opLog("Left hand heatmaps size: [" + std::to_string(handHeatMaps[0].getSize(0)) + ", "
                                    + std::to_string(handHeatMaps[0].getSize(1)) + ", "
                                    + std::to_string(handHeatMaps[0].getSize(2)) + ", "
                                    + std::to_string(handHeatMaps[0].getSize(3)) + "]");
                            op::opLog("Right hand heatmaps size: [" + std::to_string(handHeatMaps[1].getSize(0)) + ", "
                                    + std::to_string(handHeatMaps[1].getSize(1)) + ", "
                                    + std::to_string(handHeatMaps[1].getSize(2)) + ", "
                                    + std::to_string(handHeatMaps[1].getSize(3)) + "]");
                        }
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            this->stop();
            op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

private:
    HumanPoseProcessorCallback* callback_;
    bool stereoPose_;
    bool displayImage_;
    bool printInfo_;
    std::shared_ptr<op::StereoPoseRender> stereoPoseRender_;
};

std::string formatTimeString() {
    time_t t = time(0);
    char buffer[256];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", localtime(&t));
    return std::string(buffer);
}

void configureWrapper(op::Wrapper& opWrapper, const HumanPoseParams& params, HumanPoseProcessorCallback* callback)
{
    try
    {
        // Configuring OpenPose
        bool disable_multi_thread = false;
        op::Priority logging_level = op::Priority::High;
        int profile_speed = 1000;
        std::string image_dir = "";
        std::string video_path = "";
        std::string ip_camera = "";
        int camera_index = -1;
        bool flir_camera = false;
        bool mind_camera = false;
        bool hikv_camera = false;
        std::string camera_resolution = "-1x-1";
        std::string output_resolution = "-1x-1";
        std::string net_resolution = "-1x368";
        std::string face_net_resolution = "368x368";
        std::string hand_net_resolution = "368x368";
        int body_mode = 1;
        std::string model_pose = "BODY_25";
        int keypoint_scale_mode = 0;
        bool heatmaps_add_parts = false;
        bool heatmaps_add_bkg = false;
        bool heatmaps_add_PAFs = false;
        int heatmaps_scale_mode = 2;
        bool enable_3d = false;
        int num_views_3d = -1;
        int min_views_3d = -1;
        int face_detector = 0;
        int hand_detector = 0;
        double net_resolution_dynamic = 1.;
        int num_gpu = -1;
        int num_gpu_start = 0;
        int scale_number = 1;
        double scale_gap = 0.25;
        int render_pose = -1;
        bool disable_blending = false;
        double alpha_pose = 0.6;
        double alpha_heatmap = 0.7;
        int part_to_show = 0;
        std::string model_folder = "models/";
        bool part_candidates = false;
        double render_threshold = 0.05;
        int number_people_max = -1;
        bool maximize_positives = false;
        double fps_max = -1;
        std::string prototxt_path = "";
        std::string caffemodel_path = "";
        double upsampling_ratio = 0.;
        bool enable_face = false;
        int face_render = -1;
        double face_render_threshold = 0.4;
        double face_alpha_pose = 0.6;
        double face_alpha_heatmap = 0.7;
        bool enable_hand = false;
        int hand_scale_number = 1;
        double hand_scale_range = 0.4;
        int hand_render = -1;
        double hand_render_threshold = 0.2;
        double hand_alpha_pose = 0.6;
        double hand_alpha_heatmap = 0.7;
        bool identification = false;
        int tracking = -1;
        int ik_threads = 0;
        unsigned long long frame_first = 0;
        unsigned long long frame_step = 1;
        unsigned long long frame_last = -1;
        bool process_real_time = false;
        bool frame_flip = false;
        int frame_rotate = 0;
        bool frames_repeat = false;
        std::string camera_parameter_path = "models/cameraParameters/flir/";
        bool frame_undistort = false;
        int camera_trigger_mode = 0;
        double capture_fps = -1.;
        bool batch_process = false;
        double cli_verbose = -1.;
        std::string write_keypoint = "";
        std::string write_keypoint_format = "yml";
        std::string write_json = "";
        std::string write_coco_json = "";
        int write_coco_json_variants = 1;
        int write_coco_json_variant = 0;
        std::string write_images = "";
        int write_image_mode = 0;
        std::string write_images_format = "png";
        std::string write_video = "";
        double write_video_fps = -1;
        bool write_video_with_audio = false;
        std::string write_heatmaps = "";
        std::string write_heatmaps_format = "png";
        std::string write_video_3d = "";
        std::string write_video_adam = "";
        std::string write_bvh = "";
        std::string udp_host = "";
        std::string udp_port = "8051";
        int display_mode = 0;
        bool no_gui_verbose = false;
        bool fullscreen = false;

        switch (params.inputParams.inputType)
        {
        case HumanPoseParams::InputParams::MindCamera:
            mind_camera = true;
            break;
        case HumanPoseParams::InputParams::HikvCamera:
            hikv_camera = true;
            break;
        case HumanPoseParams::InputParams::VideoFile:
        case HumanPoseParams::InputParams::VideoDirectory:
            video_path = params.inputParams.videoPath;
            break;
        default:
            break;
        }
        num_views_3d = params.inputParams.viewNumber;
        camera_index = params.inputParams.cameraIndex;
        camera_trigger_mode = params.inputParams.cameraTriggerMode;
        capture_fps = params.inputParams.captureFps;
        camera_resolution = params.inputParams.cameraResolution;
        camera_parameter_path = params.inputParams.cameraParamPath;
        frame_undistort = params.inputParams.frameUndistort;

        switch (params.algorithmParams.algoType)
        {
        case HumanPoseParams::AlgorithmParams::AlgoTypeNo:
            num_gpu = 0;
            break;
        case HumanPoseParams::AlgorithmParams::AlgoType2D:
            enable_3d = false;
            break;
        case HumanPoseParams::AlgorithmParams::AlgoType3D:
            enable_3d = true;
            number_people_max = 1;
            frame_undistort = true;
            break;
        default:
            break;
        }
        min_views_3d = params.algorithmParams.minViews3d;
        net_resolution = params.algorithmParams.modelResolution;
        output_resolution = params.algorithmParams.outputResolution;
        batch_process = params.algorithmParams.batchProcess;
        process_real_time = params.algorithmParams.realTimeProcess;

        std::string strTime  = formatTimeString();
        if (params.outputParams.saveImage && !params.outputParams.imageSavePath.empty()) {
            std::string strDir = op::formatAsDirectory(params.outputParams.imageSavePath);
            if (params.outputParams.notUseTime) {
                write_images = strDir;
            }
            else {
                std::string strSubDir = strTime;
                write_images = strDir + strSubDir;
            }
            op::makeDirectory(write_images);
            write_image_mode = params.outputParams.writeImageMode;
        }
        if (params.outputParams.saveVideo && !params.outputParams.videoSavePath.empty()) {
            std::string strFile = strTime + ".mp4";
            std::string strDir = op::formatAsDirectory(params.outputParams.videoSavePath);
            write_video = strDir + strFile;
        }
        if (params.outputParams.saveVideo3d && !params.outputParams.video3dSavePath.empty()) {
            std::string strFile = strTime + "_3d.mp4";
            std::string strDir = op::formatAsDirectory(params.outputParams.video3dSavePath);
            write_video_3d = strDir + strFile;
        }
        if (params.outputParams.savePose && !params.outputParams.poseSavePath.empty()) {
            std::string strSubDir = strTime;
            std::string strDir = op::formatAsDirectory(params.outputParams.poseSavePath);
            write_json = strDir + strSubDir;
            op::makeDirectory(write_json);
        }

        // logging_level
        op::ConfigureLog::setPriorityThreshold(logging_level);
        op::Profiler::setDefaultX(profile_speed);

        // Applying user defined configuration - GFlags to program variables
        // producerType
        op::ProducerType producerType;
        op::String producerString;
        std::tie(producerType, producerString) = op::flagsToProducer(
            op::String(image_dir), op::String(video_path), op::String(ip_camera), camera_index,
            flir_camera, camera_index, mind_camera, camera_index, hikv_camera, camera_index);
        // cameraSize
        const auto cameraSize = op::flagsToPoint(op::String(camera_resolution), "-1x-1");
        // outputSize
        const auto outputSize = op::flagsToPoint(op::String(output_resolution), "-1x-1");
        // netInputSize
        const auto netInputSize = op::flagsToPoint(op::String(net_resolution), "-1x368");
        // faceNetInputSize
        const auto faceNetInputSize = op::flagsToPoint(op::String(face_net_resolution), "368x368 (multiples of 16)");
        // handNetInputSize
        const auto handNetInputSize = op::flagsToPoint(op::String(hand_net_resolution), "368x368 (multiples of 16)");
        // poseMode
        const auto poseMode = op::flagsToPoseMode(body_mode);
        // poseModel
        const auto poseModel = op::flagsToPoseModel(op::String(model_pose));
        // keypointScaleMode
        const auto keypointScaleMode = op::flagsToScaleMode(keypoint_scale_mode);
        // heatmaps to add
        const auto heatMapTypes = op::flagsToHeatMaps(heatmaps_add_parts, heatmaps_add_bkg, heatmaps_add_PAFs);
        const auto heatMapScaleMode = op::flagsToHeatMapScaleMode(heatmaps_scale_mode);
        // >1 camera view?
        const auto multipleView = (enable_3d || num_views_3d > 1 || flir_camera || mind_camera
                                   || hikv_camera || producerType == op::ProducerType::StereoVideo);
        // Face and hand detectors
        const auto faceDetector = op::flagsToDetector(face_detector);
        const auto handDetector = op::flagsToDetector(hand_detector);
        // Enabling Google Logging
        const bool enableGoogleLogging = true;

        // Initializing the user custom classes
        // GUI (Display)
        auto wUserOutput = std::make_shared<WUserOutput>(callback, enable_3d, true, false);
        // Add custom processing
        const auto workerOutputOnNewThread = true;
        opWrapper.setWorker(op::WorkerType::Output, wUserOutput, workerOutputOnNewThread);

        // Pose configuration (use WrapperStructPose{} for default and recommended configuration)
        const op::WrapperStructPose wrapperStructPose{
            poseMode, netInputSize, net_resolution_dynamic, outputSize, keypointScaleMode, num_gpu,
            num_gpu_start, scale_number, (float)scale_gap,
            op::flagsToRenderMode(render_pose, multipleView), poseModel, !disable_blending,
            (float)alpha_pose, (float)alpha_heatmap, part_to_show, op::String(model_folder),
            heatMapTypes, heatMapScaleMode, part_candidates, (float)render_threshold,
            number_people_max, maximize_positives, fps_max, op::String(prototxt_path),
            op::String(caffemodel_path), (float)upsampling_ratio, enableGoogleLogging};
        opWrapper.configure(wrapperStructPose);
    
        // Face configuration (use op::WrapperStructFace{} to disable it)
        const op::WrapperStructFace wrapperStructFace{
            enable_face, faceDetector, faceNetInputSize,
            op::flagsToRenderMode(face_render, multipleView, render_pose),
            (float)face_alpha_pose, (float)face_alpha_heatmap, (float)face_render_threshold};
        opWrapper.configure(wrapperStructFace);

        // Hand configuration (use op::WrapperStructHand{} to disable it)
        const op::WrapperStructHand wrapperStructHand{
            enable_hand, handDetector, handNetInputSize, hand_scale_number, (float)hand_scale_range,
            op::flagsToRenderMode(hand_render, multipleView, render_pose), (float)hand_alpha_pose,
            (float)hand_alpha_heatmap, (float)hand_render_threshold};
        opWrapper.configure(wrapperStructHand);

        // Extra functionality configuration (use op::WrapperStructExtra{} to disable it)
        const op::WrapperStructExtra wrapperStructExtra{
            enable_3d, min_views_3d, identification, tracking, ik_threads};
        opWrapper.configure(wrapperStructExtra);

        // Producer (use default to disable any input)
        const op::WrapperStructInput wrapperStructInput{
            producerType, producerString, frame_first, frame_step, frame_last,
            process_real_time, frame_flip, frame_rotate, frames_repeat,
            cameraSize, op::String(camera_parameter_path), frame_undistort, num_views_3d,
            camera_trigger_mode, capture_fps, batch_process};
        opWrapper.configure(wrapperStructInput);

        // Output (comment or use default argument to disable any output)
        const op::WrapperStructOutput wrapperStructOutput{
            cli_verbose, op::String(write_keypoint), op::stringToDataFormat(write_keypoint_format),
            op::String(write_json), op::String(write_coco_json), write_coco_json_variants,
            write_coco_json_variant, op::String(write_images), op::String(write_images_format),
            op::String(write_video), write_video_fps, write_video_with_audio,
            op::String(write_heatmaps), op::String(write_heatmaps_format), op::String(write_video_3d),
            op::String(write_video_adam), op::String(write_bvh), op::String(udp_host),
            op::String(udp_port), write_image_mode};
        opWrapper.configure(wrapperStructOutput);

        // GUI (comment or use default argument to disable any visual output)
        const op::WrapperStructGui wrapperStructGui{
            op::flagsToDisplayMode(display_mode, enable_3d), !no_gui_verbose, fullscreen};
        opWrapper.configure(wrapperStructGui);

        // Set to single-thread (for sequential processing and/or debugging and/or reducing latency)
        if (disable_multi_thread)
            opWrapper.disableMultiThreading();
    }
    catch (const std::exception& e)
    {
        op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
    }
}

std::string getMD5(const std::string& input) {
    CryptoPP::MD5 md5;
    //byte digest[CryptoPP::MD5::DIGESTSIZE];
    std::string digest;
    CryptoPP::StringSource(
        input, true, 
        new CryptoPP::HashFilter(md5, 
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(digest)
            )
        ) // HexEncoder
    ); // StringSource
 
    return digest;
}

HumanPoseParams::HumanPoseParams()
{
}

HumanPoseParams::HumanPoseParams(const std::string& taskPrefix)
{
    inputParams.cameraParamPath = op::formatAsDirectory(taskPrefix) + "calibration";
    op::makeDirectory(inputParams.cameraParamPath);

    std::string outputDir = op::formatAsDirectory(op::formatAsDirectory(taskPrefix) + "outputs");
    op::makeDirectory(outputDir);

    outputParams.imageSavePath = outputDir + "images";
    op::makeDirectory(outputParams.imageSavePath);
    outputParams.videoSavePath = outputDir + "videos";
    op::makeDirectory(outputParams.videoSavePath);
    outputParams.video3dSavePath = outputParams.videoSavePath;
}

bool HumanPoseParams::loadFromFile(const std::string& paramFile, std::string& md5)
{
    Json::Value root;
    std::ifstream ifs;
    ifs.open(paramFile);
    if (!ifs.is_open())
        return false;

    std::string jsonString((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
    //std::cout << "Load json: " << jsonString << std::endl;
    md5 = getMD5(jsonString);
    std::istringstream iss(jsonString);

    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    JSONCPP_STRING errs;
    if (!Json::parseFromStream(builder, iss, &root, &errs)) {
        std::cout << errs << std::endl;
        return false;
    }

    // input parameters
    Json::Value& inputValue = root["inputParams"];
    inputParams.inputType = (InputParams::InputType)inputValue["inputType"].asInt();
    inputParams.videoPath = inputValue["videoPath"].asString();
    inputParams.viewNumber = inputValue["viewNumber"].asInt();
    inputParams.cameraIndex = inputValue["cameraIndex"].asInt();
    inputParams.cameraTriggerMode = inputValue["cameraTriggerMode"].asInt();
    inputParams.captureFps = inputValue["captureFps"].asFloat();
    inputParams.cameraResolution = inputValue["cameraResolution"].asString();
    inputParams.cameraParamPath = inputValue["cameraParamPath"].asString();
    inputParams.frameUndistort = inputValue["frameUndistort"].asBool();

    Json::Value& algorithmValue = root["algorithmParams"];
    algorithmParams.minViews3d = algorithmValue["minViews3d"].asInt();
    algorithmParams.modelResolution = algorithmValue["modelResolution"].asString();
    algorithmParams.outputResolution = algorithmValue["outputResolution"].asString();
    algorithmParams.batchProcess = algorithmValue["batchProcess"].asBool();
    algorithmParams.realTimeProcess = algorithmValue["realTimeProcess"].asBool();

    Json::Value& outputValue = root["outputParams"];
    outputParams.saveImage = outputValue["saveImage"].asBool();
    outputParams.imageSavePath = outputValue["imageSavePath"].asString();
    outputParams.saveVideo = outputValue["saveVideo"].asBool();
    outputParams.videoSavePath = outputValue["videoSavePath"].asString();
    outputParams.saveVideo3d = outputValue["saveVideo3d"].asBool();
    outputParams.video3dSavePath = outputValue["video3dSavePath"].asString();
    outputParams.savePose = outputValue["savePose"].asBool();
    outputParams.poseSavePath = outputValue["poseSavePath"].asString();

    return true;
}

bool HumanPoseParams::saveToFile(const std::string& paramFile, std::string& md5)
{
    Json::Value root, inputValue, outputValue, algorithmValue;

    inputValue["inputType"] = Json::Value(int(inputParams.inputType));
    inputValue["videoPath"] = Json::Value(inputParams.videoPath);
    inputValue["viewNumber"] = Json::Value(inputParams.viewNumber);
    inputValue["cameraIndex"] = Json::Value(inputParams.cameraIndex);
    inputValue["cameraTriggerMode"] = Json::Value(inputParams.cameraTriggerMode);
    inputValue["captureFps"] = Json::Value(inputParams.captureFps);
    inputValue["cameraResolution"] = Json::Value(inputParams.cameraResolution);
    inputValue["cameraParamPath"] = Json::Value(inputParams.cameraParamPath);
    inputValue["frameUndistort"] = Json::Value(inputParams.frameUndistort);

    algorithmValue["minViews3d"] = Json::Value(algorithmParams.minViews3d);
    algorithmValue["modelResolution"] = Json::Value(algorithmParams.modelResolution);
    algorithmValue["outputResolution"] = Json::Value(algorithmParams.outputResolution);
    algorithmValue["batchProcess"] = Json::Value(algorithmParams.batchProcess);
    algorithmValue["realTimeProcess"] = Json::Value(algorithmParams.realTimeProcess);

    outputValue["saveImage"] = Json::Value(outputParams.saveImage);
    outputValue["imageSavePath"] = Json::Value(outputParams.imageSavePath);
    outputValue["saveVideo"] = Json::Value(outputParams.saveVideo);
    outputValue["videoSavePath"] = Json::Value(outputParams.videoSavePath);
    outputValue["saveVideo3d"] = Json::Value(outputParams.saveVideo3d);
    outputValue["video3dSavePath"] = Json::Value(outputParams.video3dSavePath);
    outputValue["savePose"] = Json::Value(outputParams.savePose);
    outputValue["poseSavePath"] = Json::Value(outputParams.poseSavePath);

    root["inputParams"] = inputValue;
    root["outputParams"] = outputValue;
    root["algorithmParams"] = algorithmValue;

	Json::StreamWriterBuilder builder;
	static Json::Value def = []() {
		Json::Value def;
		Json::StreamWriterBuilder::setDefaults(&def);
		def["emitUTF8"] = true;
		return def;
	}();
 
	builder.settings_ = def; //Config emitUTF8
	const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    std::ostringstream oss;
 	writer->write(root, &oss);
    std::string jsonString = oss.str();
    //std::cout << "Save json: " << jsonString << std::endl;
    md5 = getMD5(jsonString);

    if (!paramFile.empty())
    {
        std::ofstream ofs;
        ofs.open(paramFile);
        if (!ofs.is_open())
            return false;
        ofs << jsonString;
        ofs.close();
    }

    return true;
}

