#ifndef OPENPOSE_PRODUCER_HIKV_READER_HPP
#define OPENPOSE_PRODUCER_HIKV_READER_HPP

#include <openpose/core/common.hpp>
#include <openpose/producer/producer.hpp>


namespace op
{
    class HikvReaderImpl;

    /**
     * HikvReader is an abstract class to extract frames from a HikVision stereo-camera system.
     */
    class OP_API HikvReader : public Producer
    {
    public:
        explicit HikvReader(const std::string& cameraParametersPath, const Point<int>& cameraResolution,
            bool undistortImage=true, int cameraIndex=-1, int cameraTriggerMode=0, double captureFps=-1);
        virtual ~HikvReader();

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
        std::shared_ptr<HikvReaderImpl> upImpl;
        Point<int> mResolution;
        double mCaptureFps = -1.;
        unsigned long long mFrameNameCounter;

    protected:
        DELETE_COPY(HikvReader);
    };
}

#endif // OPENPOSE_PRODUCER_HIKV_READER_HPP
