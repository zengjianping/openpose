#include <openpose/producer/stereoVideoReader.hpp>
#include <openpose/utilities/fastMath.hpp>
#include <openpose/utilities/fileSystem.hpp>
#include <openpose_private/utilities/openCvMultiversionHeaders.hpp>
#include <openpose/producer/videoReader.hpp>
#include <openpose/producer/imageDirectoryReader.hpp>


namespace op
{
    bool getVideoFiles(const std::string& videoDir, std::vector<std::string>& videoFilePaths, int numberViews)
    {
        try
        {
            videoFilePaths = getFilesOnDirectory(videoDir, Extensions::Videos);
            return videoFilePaths.size() > 0 && (numberViews < 0 || videoFilePaths.size() == (size_t)numberViews);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }

    bool getImageDirectories(const std::string& videoDir, std::vector<std::string>& imageDirPaths, int numberViews)
    {
        try
        {
            imageDirPaths = getSubdirsOnDirectory(videoDir);
            return imageDirPaths.size() > 0 && (numberViews < 0 || imageDirPaths.size() == (size_t)numberViews);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
    }

    StereoVideoReader::StereoVideoReader(const std::string& videoDir, const std::string& cameraParameterDir,
            const bool undistortImage, const int numberViews) :
        Producer(ProducerType::StereoVideo, cameraParameterDir, undistortImage, numberViews)
    {
        //int numberViews = positiveIntRound(Producer::get(ProducerProperty::NumberViews));
        std::vector<std::string> videoFilePaths, imageDirPaths;

        if (getVideoFiles(videoDir, videoFilePaths, numberViews))
        {
            mVideoReaders.reserve(videoFilePaths.size());
            for (const std::string& videoFile : videoFilePaths)
            {
                VideoReader* producer = new VideoReader(videoFile);
                mVideoReaders.emplace_back(producer);
            }
        }
        else if (getImageDirectories(videoDir, imageDirPaths, numberViews))
        {
            mVideoReaders.reserve(imageDirPaths.size());
            for (const std::string& imageDir : imageDirPaths)
            {
                ImageDirectoryReader* producer = new ImageDirectoryReader(imageDir);
                mVideoReaders.emplace_back(producer);
            }
        }
    }

    StereoVideoReader::~StereoVideoReader()
    {
        release();
    }

    std::string StereoVideoReader::getNextFrameName()
    {
        if (mVideoReaders.size() == 0)
            return std::string();
        return mVideoReaders[0]->getNextFrameName();
    }

    bool StereoVideoReader::isOpened() const
    {
        if (mVideoReaders.size() == 0)
            return false;

        bool opened = true;
        for (const auto& videoReader : mVideoReaders)
            opened = opened && videoReader->isOpened();
        
        return opened;
    }

    void StereoVideoReader::release()
    {
        for (const auto& videoReader : mVideoReaders)
            videoReader->release();
    }

    double StereoVideoReader::get(const int capProperty)
    {
        if (mVideoReaders.size() == 0)
            return -1;
        return mVideoReaders[0]->get(capProperty);
    }

    void StereoVideoReader::set(const int capProperty, const double value)
    {
        for (const auto& videoReader : mVideoReaders)
            videoReader->set(capProperty, value);
    }

    Matrix StereoVideoReader::getRawFrame()
    {
        error("StereoVideoReader do not support getRawFrame!", __LINE__, __FUNCTION__, __FILE__);
        return Matrix();
    }

    std::vector<Matrix> StereoVideoReader::getRawFrames()
    {
        std::vector<Matrix> frames;
        if (mVideoReaders.size() == 0)
            return frames;

        frames.reserve(mVideoReaders.size());
        for (const auto& videoReader : mVideoReaders)
        {
            const Matrix& frame = videoReader->getRawFrame();
            frames.push_back(frame);
        }
        return frames;
    }

}
