#ifndef HUMAN_POSE_PROCESSOR_H_
#define HUMAN_POSE_PROCESSOR_H_

#include <memory>


struct HumanPoseParams
{
    struct InputParams
    {
        enum InputType
        {
            VideoFile = 0,
            VideoDirectory,
            WebCamera,
            MindCamera,
            HikCamera,
            InputTypeNum
        };
        InputType inputType = MindCamera;
        std::string videoPath = "datas/pose_tests/panoptic/dance2a/video.mp4";
        int viewNumber = -1;
        int cameraIndex = -1;
        int cameraTriggerMode = 0;
        double captureFps = -1.;
        std::string cameraResolution = "1224x1024";
        std::string cameraParamPath = "datas/pose_tests/mind_camera/test04/cameras/";
        //std::string cameraParamPath = "datas/pose_tests/panoptic/dance2a/cameras/";
        bool frameUndistort = false;
    };

    struct OutputParams
    {
    };

    struct AlgorithmParams
    {
        enum AlgoType
        {
            AlgoTypeNo = 0,
            AlgoType2D,
            AlgoType3D
        };
        AlgoType algoType = AlgoType3D;
        int minViews3d = -1;
        std::string modelResolution = "-1x192";
        std::string outputResolution = "-1x-1";
        bool batchProcess = false;
        bool realTimeProcess = false;
    };

    InputParams inputParams;
    OutputParams outputParams;
    AlgorithmParams algorithmParams;
};

class HumanPoseProcessor
{
public:
    static std::shared_ptr<HumanPoseProcessor> createInstance(const HumanPoseParams& params);

public:
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;

private:
    HumanPoseParams params_;
};


#endif // HUMAN_POSE_PROCESSOR_H_

