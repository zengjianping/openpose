#ifndef HUMAN_POSE_PROCESSOR_H_
#define HUMAN_POSE_PROCESSOR_H_

#include <opencv2/opencv.hpp>
#include <memory>


struct HumanPoseParams
{
    bool loadFromFile(const std::string& paramFile);
    bool saveToFile(const std::string& paramFile);

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
        int viewNumber = 4;
        int cameraIndex = -1;
        int cameraTriggerMode = 0;
        double captureFps = -1.;
        std::string cameraResolution = "1224x1024";
        std::string cameraParamPath = "datas/pose_tests/mind_camera/test04/cameras/";
        //std::string cameraResolution = "-1x-1";
        //std::string cameraParamPath = "datas/pose_tests/panoptic/dance2a/cameras/";
        bool frameUndistort = false;
    };

    struct OutputParams
    {
        bool saveImage = false;
        std::string imageSavePath = "";
        bool saveVideo = false;
        std::string videoSavePath = "";
        bool saveVideo3d = false;
        std::string video3dSavePath = "";
    };

    struct AlgorithmParams
    {
        enum AlgoType
        {
            AlgoTypeNo = 0,
            AlgoType2D,
            AlgoType3D
        };
        AlgoType algoType = AlgoTypeNo;
        int minViews3d = 2;
        std::string modelResolution = "-1x256";
        std::string outputResolution = "-1x-1";
        bool batchProcess = false;
        bool realTimeProcess = false;
    };

    InputParams inputParams;
    OutputParams outputParams;
    AlgorithmParams algorithmParams;
};

class HumanPoseProcessorCallback
{
public:
    virtual void set2dPoseImage(int index, const cv::Mat& image) = 0;
    virtual void set3dPoseImage(const cv::Mat& image) = 0;
};

class HumanPoseProcessor
{
public:
    static std::shared_ptr<HumanPoseProcessor> createInstance();

public:
    virtual void setCallback(HumanPoseProcessorCallback* callback) = 0;
    virtual bool start(const HumanPoseParams& params) = 0;
    virtual void stop() = 0;
    virtual bool isRunning() = 0;

    virtual bool queryCameraList(const HumanPoseParams& params, std::string& cameraType,
                                 std::vector<std::string>& cameraNames) = 0;

private:
    //HumanPoseParams params_;
};

bool calibrateCameraIntrinsics(const std::vector<std::string>& cameraNames,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize);
bool calibrateCameraExtrinsics(const std::vector<std::string>& cameraNames,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize);


#endif // HUMAN_POSE_PROCESSOR_H_

