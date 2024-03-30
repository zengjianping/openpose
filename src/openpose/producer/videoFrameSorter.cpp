#include <openpose/producer/videoFrameSorter.hpp>


namespace op
{

VideoFrameSorter::VideoFrameSorter(int cameraCount)
{
    mCameraCount = cameraCount;
    mCacheLength = cameraCount * 2 - 1;
}

VideoFrameSorter::~VideoFrameSorter()
{
}

bool VideoFrameSorter::pushFrame(const cv::Mat& frame, int cameraIndex, double timestamp)
{
    std::unique_lock<std::mutex> lock{mCacheMutex};

    if (cameraIndex < 0 || cameraIndex >= mCameraCount)
        return false;
    
    mFrameDataCache.push_back(std::make_shared<FrameData>(frame, cameraIndex, timestamp));
    while (mFrameDataCache.size() > (size_t)mCacheLength)
        mFrameDataCache.pop_front();
    
    if (mFrameDataCache.size() == (size_t)mCacheLength && mCacheLength > 1)
    {
        std::sort(mFrameDataCache.begin(), mFrameDataCache.end(), [](const std::shared_ptr<FrameData>& a,
            const std::shared_ptr<FrameData>& b) { return a->mTimestamp < b->mTimestamp || 
                (a->mTimestamp == b->mTimestamp && a->mCameraIndex <= b->mCameraIndex);
        });
    }

    return true;
}

bool VideoFrameSorter::popFrames(std::vector<cv::Mat>& frames)
{
    std::unique_lock<std::mutex> lock{mCacheMutex};

    if (mFrameDataCache.size() < (size_t)mCacheLength)
        return false;
    
    double minTimeDiff = std::numeric_limits<double>::max();
    int optPos = -1;

    for (int i = 0; i < mCameraCount; i++)
    {
        std::set<int> cameraIndexSet;
        for(int j = 0; j < mCameraCount; j++)
        {
            const std::shared_ptr<FrameData>& frameData = mFrameDataCache[i+j];
            cameraIndexSet.insert(frameData->mCameraIndex);
        }
        if (cameraIndexSet.size() == mCameraCount)
        {
            double timeDiff = mFrameDataCache[i+mCameraCount-1]->mTimestamp - mFrameDataCache[i]->mTimestamp;
            if (minTimeDiff > timeDiff)
            {
                minTimeDiff = timeDiff;
                optPos = i;
            }
        }
    }

    if (optPos >= 0)
    {
        frames.resize(mCameraCount);
        for (int i = 0; i < optPos; i++)
            mFrameDataCache.pop_front();
        //std::cout << "Pop frames:";
        for (int i = 0; i < mCameraCount; i++)
        {
            const std::shared_ptr<FrameData>& frameData = mFrameDataCache.front();
            //std::cout << " (" << frameData->mCameraIndex << ", " << (int64_t)round(frameData->mTimestamp) << ")";
            frames[frameData->mCameraIndex] = frameData->mFrame;
            mFrameDataCache.pop_front();
        }
        //std::cout << std::endl;
        return true;
    }

    return false;
}


}

