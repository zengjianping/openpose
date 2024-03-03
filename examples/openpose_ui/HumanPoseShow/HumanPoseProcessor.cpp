#include "HumanPoseProcessor.h"
#include "openpose/headers.hpp"


class HumanPoseProcessorOP : public HumanPoseProcessor
{
public:
    HumanPoseProcessorOP(const HumanPoseParams& params);
    virtual ~HumanPoseProcessorOP();

public:
    bool start() override;
    void stop() override;
    bool isRunning() override;

private:
    HumanPoseParams params_;
    std::shared_ptr<op::Wrapper> opWrapper_;
};

std::shared_ptr<HumanPoseProcessor> HumanPoseProcessor::createInstance(const HumanPoseParams& params)
{
    std::shared_ptr<HumanPoseProcessor> processor;
    processor.reset(new HumanPoseProcessorOP(params));
    return processor;
}


HumanPoseProcessorOP::HumanPoseProcessorOP(const HumanPoseParams& params)
{
    params_ = params;
}

HumanPoseProcessorOP::~HumanPoseProcessorOP()
{
}

void configureWrapper(op::Wrapper& opWrapper, const HumanPoseParams& params);

bool HumanPoseProcessorOP::start()
{
    try
    {
        op::opLog("Starting OpenPose demo...", op::Priority::High);
        const auto opTimer = op::getTimerInit();

        // Configure OpenPose
        op::opLog("Configuring OpenPose...", op::Priority::High);
        opWrapper_.reset(new op::Wrapper(op::ThreadManagerMode::AsynchronousOut));
        configureWrapper(*opWrapper_, params_);

        // Start, run, and stop processing - exec() blocks this thread until OpenPose wrapper has finished
        op::opLog("Starting thread(s)...", op::Priority::High);
        opWrapper_->start();

        // Measuring total time
        op::printTime(opTimer, "OpenPose demo successfully finished. Total time: ", " seconds.", op::Priority::High);

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
        if (opWrapper_.get() && opWrapper_->isRunning())
            opWrapper_->stop();
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

void configureWrapper(op::Wrapper& opWrapper, const HumanPoseParams& params)
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
        double cli_verbose = 1.;
        std::string write_keypoint = "";
        std::string write_keypoint_format = "yml";
        std::string write_json = "";
        std::string write_coco_json = "";
        int write_coco_json_variants = 1;
        int write_coco_json_variant = 0;
        std::string write_images = "";
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
        int display_mode = -1;
        bool no_gui_verbose = false;
        bool fullscreen = false;

        switch (params.inputParams.inputType)
        {
        case HumanPoseParams::InputParams::MindCamera:
            mind_camera = true;
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


        // logging_level
        op::ConfigureLog::setPriorityThreshold(logging_level);
        op::Profiler::setDefaultX(profile_speed);

        // Applying user defined configuration - GFlags to program variables
        // producerType
        op::ProducerType producerType;
        op::String producerString;
        std::tie(producerType, producerString) = op::flagsToProducer(
            op::String(image_dir), op::String(video_path), op::String(ip_camera), camera_index,
            flir_camera, camera_index, mind_camera, camera_index);
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
                                   || producerType == op::ProducerType::StereoVideo);
        // Face and hand detectors
        const auto faceDetector = op::flagsToDetector(face_detector);
        const auto handDetector = op::flagsToDetector(hand_detector);
        // Enabling Google Logging
        const bool enableGoogleLogging = true;

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
            op::String(udp_port)};
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

