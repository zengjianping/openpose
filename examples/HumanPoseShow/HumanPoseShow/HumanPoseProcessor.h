#ifndef HUMAN_POSE_PROCESSOR_H_
#define HUMAN_POSE_PROCESSOR_H_

#include <openpose/utilities/keypoint.hpp>
#include <opencv2/opencv.hpp>
#include <memory>


struct HumanPoseParams
{
    HumanPoseParams();
    HumanPoseParams(const std::string& taskPrefix);

    bool loadFromFile(const std::string& paramFile, std::string& md5);
    bool saveToFile(const std::string& paramFile, std::string& md5);

    struct InputParams
    {
        enum InputType
        {
            VideoFile = 0,
            VideoDirectory,
            WebCamera,
            MindCamera,
            HikvCamera,
            InputTypeNum
        };
        InputType inputType = MindCamera;
        std::string videoPath = "";
        int viewNumber = -1;
        int cameraIndex = -1;
        int cameraTriggerMode = 1;
        double captureFps = -1.;
        std::string cameraResolution = "1224x1024";
        std::string cameraParamPath = "calibration";
        bool frameUndistort = false;
    };

    struct OutputParams
    {
        bool notUseTime = false;
        bool saveImage = false;
        std::string imageSavePath = "";
        int writeImageMode = 0;
        bool saveVideo = false;
        std::string videoSavePath = "";
        bool saveVideo3d = false;
        std::string video3dSavePath = "";
        bool savePose = false;
        std::string poseSavePath = "";
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
    virtual void setKeypoints(const op::Array<float>& poseKeypoints3D, const op::Array<float>& faceKeypoints3D,
        const op::Array<float>& leftHandKeypoints3D, const op::Array<float>& rightHandKeypoints3D) = 0;
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

private:
};

bool queryCameraList(const HumanPoseParams& params, std::string& cameraType,
    std::vector<std::string>& cameraNames);
bool calibrateCameraIntrinsics(const std::vector<std::string>& cameraNames,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize);
bool calibrateCameraExtrinsics(const std::vector<std::string>& cameraNames,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize);
bool calibrateCameraPose(const std::vector<std::string>& cameraNames,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize);
bool refineCameraExtrinsics(const std::vector<std::string>& cameraNames,
    const std::string& calibrateDir, const std::string& gridLayout, float gridSize);


#endif // HUMAN_POSE_PROCESSOR_H_

