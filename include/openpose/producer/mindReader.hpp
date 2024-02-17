#ifndef OPENPOSE_PRODUCER_MIND_READER_HPP
#define OPENPOSE_PRODUCER_MIND_READER_HPP

#include <openpose/core/common.hpp>
#include <openpose/producer/producer.hpp>


namespace op
{
    class MindReaderImpl;

    /**
     * MindReader is an abstract class to extract frames from a MindVision stereo-camera system.
     */
    class OP_API MindReader : public Producer
    {
    public:
        explicit MindReader(const std::string& cameraParametersPath, const Point<int>& cameraResolution,
            bool undistortImage=true, int cameraIndex=-1, int cameraTriggerMode=0, double captureFps=-1);
        virtual ~MindReader();

    public:
        std::vector<Matrix> getCameraMatrices();
        std::vector<Matrix> getCameraExtrinsics();
        std::vector<Matrix> getCameraIntrinsics();
        virtual std::string getNextFrameName();
        virtual bool isOpened() const;
        virtual void release();
        virtual double get(const int capProperty);
        virtual void set(const int capProperty, const double value);

    protected:
        Matrix getRawFrame();
        std::vector<Matrix> getRawFrames();

    protected:
        std::shared_ptr<MindReaderImpl> upImpl;
        Point<int> mResolution;
        unsigned long long mFrameNameCounter;

    protected:
        DELETE_COPY(MindReader);
    };
}

#endif // OPENPOSE_PRODUCER_MIND_READER_HPP
