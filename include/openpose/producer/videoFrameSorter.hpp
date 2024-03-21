#ifndef OPENPOSE_VIDEO_FRAME_SORTER_HPP
#define OPENPOSE_VIDEO_FRAME_SORTER_HPP

#include <openpose/core/common.hpp>
#include <opencv2/opencv.hpp>


namespace op
{

class VideoFrameSorter
{
public:
    VideoFrameSorter(int cameraCount);
    ~VideoFrameSorter();

public:
    bool pushFrame(const cv::Mat& frame, int cameraIndex, double timestamp);
    bool popFrames(std::vector<cv::Mat>& frames);

protected:
    struct FrameData
    {
        FrameData(const cv::Mat& frame, int cameraIndex, double timestamp)
            : mFrame(frame), mCameraIndex(cameraIndex), mTimestamp(timestamp) {}
        cv::Mat mFrame;
        int mCameraIndex;
        double mTimestamp;
    };
    std::deque<std::shared_ptr<FrameData>> mFrameDataCache;
    std::mutex mCacheMutex;
    int mCameraCount;
    int mCacheLength;
};

}

#endif // OPENPOSE_VIDEO_FRAME_SORTER_HPP
