#ifndef OPENPOSE_PRODUCER_STEREO_VIDEO_READER_HPP
#define OPENPOSE_PRODUCER_STEREO_VIDEO_READER_HPP

#include <openpose/core/common.hpp>
#include <openpose/producer/producer.hpp>

namespace op
{
    class OP_API StereoVideoReader : public Producer
    {
    public:
        explicit StereoVideoReader(const std::string& videoDir, const std::string& cameraParameterDir,
                                   const bool undistortImage, const int numberViews);
        virtual ~StereoVideoReader();
    
    public:
        virtual std::string getNextFrameName();
        virtual bool isOpened() const;
        virtual void release();
        virtual double get(const int capProperty);
        virtual void set(const int capProperty, const double value);

    protected:
        virtual Matrix getRawFrame();
        virtual std::vector<Matrix> getRawFrames();

    protected:
        std::vector<std::shared_ptr<Producer>> mVideoReaders;

    protected:
        DELETE_COPY(StereoVideoReader);
    };
}

#endif // OPENPOSE_PRODUCER_STEREO_VIDEO_READER_HPP
