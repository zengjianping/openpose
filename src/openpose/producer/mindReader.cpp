#include <openpose/producer/mindReader.hpp>
#include <openpose/utilities/fastMath.hpp>
#include <openpose/utilities/string.hpp>
#include <openpose/core/point.hpp>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <mutex>
#include "CameraApi.h"


namespace op
{
    class OP_API MindReaderImpl
    {
    public:
        /**
         * Constructor of MindReaderImpl. It opens all the available MindVision cameras
         * cameraIndex = -1 means that all cameras are taken
         */
        explicit MindReaderImpl(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
                    bool undistortImage, int cameraIndex=-1, int cameraTriggerMode=0);
        virtual ~MindReaderImpl();

    public:
        void release();
        bool isOpened() const;
        Point<int> getResolution() const;
        std::vector<Matrix> getCameraMatrices() const;
        std::vector<Matrix> getCameraExtrinsics() const;
        std::vector<Matrix> getCameraIntrinsics() const;
        std::vector<Matrix> getRawFrames();

    protected:
/*
        void bufferingThread();
        void readAndUndistortImage(const int i, const Spinnaker::ImagePtr& imagePtr,
                                    const cv::Mat& cameraIntrinsics = cv::Mat(),
                                    const cv::Mat& cameraDistorsions = cv::Mat());
        std::vector<Matrix> acquireImages(
            const std::vector<Matrix>& opCameraIntrinsics,
            const std::vector<Matrix>& opCameraDistorsions,
            const int cameraIndex = -1);
*/
    protected:
        bool mInitialized = false;
        int mCameraCount = 0;
        int mCameraIndex = -1;
        int mCameraTriggerMode = 0;
        Point<int> mResolution;
        std::vector<int> mCameraHandles;
        std::vector<tSdkCameraDevInfo> mCameraDevInfos;
        std::vector<std::string> mSerialNumbers;
        std::vector<cv::Mat> mCvMats;
        // Undistortion
        bool mUndistortImage = false;
        std::vector<cv::Mat> mRemoveDistortionMaps1;
        std::vector<cv::Mat> mRemoveDistortionMaps2;
        CameraParameterReader mCameraParameterReader;
        // Thread
        bool mThreadOpened;
        //std::vector<Spinnaker::ImagePtr> mBuffer;
        std::mutex mBufferMutex;
        std::atomic<bool> mCloseThread;
        std::thread mThread;

    protected:
        DELETE_COPY(MindReaderImpl);
    };

    MindReader::MindReader(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
                           const bool undistortImage, const int cameraIndex) :
        Producer{ProducerType::FlirCamera, cameraParameterPath, undistortImage, -1},
        mFrameNameCounter{0ull}
    {
        try
        {
            upImpl = std::make_shared<MindReaderImpl>(cameraParameterPath, cameraResolution, undistortImage, cameraIndex);
            // Get resolution
            const auto resolution = upImpl->getResolution();
            // Set resolution
            set(cv::CAP_PROP_FRAME_WIDTH, resolution.x);
            set(cv::CAP_PROP_FRAME_HEIGHT, resolution.y);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    MindReader::~MindReader()
    {
        try
        {
            release();
        }
        catch (const std::exception& e)
        {
            errorDestructor(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    std::string MindReader::getNextFrameName()
    {
        try
        {
            const unsigned long long stringLength = 12u;
            return toFixedLengthString(mFrameNameCounter, stringLength);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return "";
        }
    }

    bool MindReader::isOpened() const
    {
        try
        {
            return upImpl->isOpened();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return false;
        }
    }

    void MindReader::release()
    {
        try
        {
            upImpl->release();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    Matrix MindReader::getRawFrame()
    {
        try
        {
            return upImpl->getRawFrames().at(0);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return Matrix();
        }
    }

    std::vector<Matrix> MindReader::getRawFrames()
    {
        try
        {
            mFrameNameCounter++; // Simple counter: 0,1,2,3,...
            return upImpl->getRawFrames();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }

    double MindReader::get(const int capProperty)
    {
        try
        {
            if (capProperty == cv::CAP_PROP_FRAME_WIDTH)
            {
                if (Producer::get(ProducerProperty::Rotation) == 0.
                    || Producer::get(ProducerProperty::Rotation) == 180.)
                    return mResolution.x;
                else
                    return mResolution.y;
            }
            else if (capProperty == cv::CAP_PROP_FRAME_HEIGHT)
            {
                if (Producer::get(ProducerProperty::Rotation) == 0.
                    || Producer::get(ProducerProperty::Rotation) == 180.)
                    return mResolution.y;
                else
                    return mResolution.x;
            }
            else if (capProperty == cv::CAP_PROP_POS_FRAMES)
                return (double)mFrameNameCounter;
            else if (capProperty == cv::CAP_PROP_FRAME_COUNT)
                return -1.;
            else if (capProperty == cv::CAP_PROP_FPS)
                return -1.;
            else
            {
                opLog("Unknown property.", Priority::Max, __LINE__, __FUNCTION__, __FILE__);
                return -1.;
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return 0.;
        }
    }

    void MindReader::set(const int capProperty, const double value)
    {
        try
        {
            if (capProperty == cv::CAP_PROP_FRAME_WIDTH)
                mResolution.x = {(int)value};
            else if (capProperty == cv::CAP_PROP_FRAME_HEIGHT)
                mResolution.y = {(int)value};
            else if (capProperty == cv::CAP_PROP_POS_FRAMES)
                opLog("This property is read-only.", Priority::Max, __LINE__, __FUNCTION__, __FILE__);
            else if (capProperty == cv::CAP_PROP_FRAME_COUNT || capProperty == cv::CAP_PROP_FPS)
                opLog("This property is read-only.", Priority::Max, __LINE__, __FUNCTION__, __FILE__);
            else
                opLog("Unknown property.", Priority::Max, __LINE__, __FUNCTION__, __FILE__);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    std::vector<std::string> getSerialNumbers(const tSdkCameraDevInfo* cameraList, int cameraCount)
    {
        try
        {
            std::vector<std::string> serialNumbers(cameraCount);
            for (int i = 0; i < cameraCount; i++)
            {
                std::string serialNumber(cameraList[i].acProductName);
                serialNumbers[i] = serialNumber + "-" + std::to_string(cameraList[i].uInstance);
            }

            std::sort(serialNumbers.begin(), serialNumbers.end());

            return serialNumbers;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }

    void printDeviceInfo(const tSdkCameraDevInfo& cameraDevInfo, int cameraIndex)
    {
        try
        {
            opLog("Printing device information for camera " + std::to_string(cameraIndex) + "...\n", Priority::High);
            opLog(std::string("ProductSeries: ") + cameraDevInfo.acProductSeries, Priority::High);
            opLog(std::string("ProductName: ") + cameraDevInfo.acProductName, Priority::High);
            opLog(std::string("FriendlyName: ") + cameraDevInfo.acFriendlyName, Priority::High);
            opLog(std::string("PortType: ") + cameraDevInfo.acPortType, Priority::High);
            opLog(std::string("SerialNo: ") + cameraDevInfo.acSn, Priority::High);
            opLog(std::string("Instance: ") + std::to_string(cameraDevInfo.uInstance), Priority::High);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    // This function returns the camera to a normal state by turning off trigger mode.
    void resetCameraTrigger(int cameraHandle)
    {
        try
        {
            opLog("Turning off trigger...", Priority::High);
            CameraSdkStatus status = CameraSetTriggerMode(cameraHandle, 0);
            if (status != CAMERA_STATUS_SUCCESS)
            {
                std::string message = "Failed to turning off trigger mode!, Error code is "
                    + std::to_string(status) + ".";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            opLog("Trigger mode turned off.", Priority::High);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    // This function configures the camera to use a trigger.
    void configCameraTrigger(int cameraHandle)
    {
        try
        {
            opLog("Turning on trigger...", Priority::High);
            CameraSdkStatus status = CameraSetTriggerMode(cameraHandle, 1);
            if (status != CAMERA_STATUS_SUCCESS)
            {
                std::string message = "Failed to turning on trigger mode!, Error code is "
                    + std::to_string(status) + ".";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            CameraSetTriggerCount(cameraHandle, 1);
            opLog("Trigger mode turned on.", Priority::High);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void grabNextImageByTrigger(int cameraHandle)
    {
        try
        {
            // Since the camera is currently in soft trigger mode, software is required to send a command to inform
            // the camera to take pictures (to avoid accidentally fetching old pictures in the camera cache, the cache
            // is cleared before the trigger command)
            CameraClearBuffer(cameraHandle);
            CameraSdkStatus status = CameraSoftTrigger(cameraHandle);
            if (status != CAMERA_STATUS_SUCCESS)
            {
                std::string message = "Failed to trigger the camera! Error code is "
                    + std::to_string(status) + ".";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    // offsetx, offsety, width, height: offset width and height are all chosen to be 16 times
    // the best compatibility (different cameras have different requirements for this, some need
    // only a multiple of 2, some may require a multiple of 16)
    int SetCameraResolution(int cameraHandle, int offsetx, int offsety, int width, int height)
    {
        tSdkImageResolution sRoiResolution = { 0 };
        
        // Set to 0xff for custom resolution, set to 0 to N for select preset resolution
        sRoiResolution.iIndex = 0xff;
        
        // iWidthFOV represents the camera's field of view width, iWidth represents the camera's
        // actual output width. In most cases iWidthFOV=iWidth. Some special resolution modes such
        // as BIN2X2:iWidthFOV=2*iWidth indicate that the field of view is twice the actual output width
        sRoiResolution.iWidth = width;
        sRoiResolution.iWidthFOV = width;
        
        // height, refer to the description of the width above
        sRoiResolution.iHeight = height;
        sRoiResolution.iHeightFOV = height;
        
        // 视场偏移
        // Field of view offset
        sRoiResolution.iHOffsetFOV = offsetx;
        sRoiResolution.iVOffsetFOV = offsety;
        
        // ISP software zoom width and height, all 0 means not zoom
        sRoiResolution.iWidthZoomSw = 0;
        sRoiResolution.iHeightZoomSw = 0;
        
        // BIN SKIP mode setting (requires camera hardware support)
        sRoiResolution.uBinAverageMode = 0;
        sRoiResolution.uBinSumMode = 0;
        sRoiResolution.uResampleMask = 0;
        sRoiResolution.uSkipMode = 0;
        
        return CameraSetImageResolution(cameraHandle, &sRoiResolution);
    }

    /*Spinnaker::ImagePtr spinnakerImagePtrToColor(const Spinnaker::ImagePtr &imagePtr)
    {
        // Original image --> BGR uchar image
        // Print image information
        // Convert image to RGB
        // Interpolation methods
        // http://softwareservices.ptgrey.com/Spinnaker/latest/group___spinnaker_defs.html
        // DEFAULT     Default method.
        // NO_COLOR_PROCESSING     No color processing.
        // NEAREST_NEIGHBOR    Fastest but lowest quality. Equivalent to
        //                     FLYCAPTURE_NEAREST_NEIGHBOR_FAST in FlyCapture.
        // EDGE_SENSING    Weights surrounding pixels based on localized edge orientation.
        // HQ_LINEAR   Well-balanced speed and quality.
        // RIGOROUS    Slowest but produces good results.
        // IPP     Multi-threaded with similar results to edge sensing.
        // DIRECTIONAL_FILTER  Best quality but much faster than rigorous.
        // Colors
        // http://softwareservices.ptgrey.com/Spinnaker/latest/group___camera_defs__h.html#ggabd5af55aaa20bcb0644c46241c2cbad1a33a1c8a1f6dbcb4a4eaaaf6d4d7ff1d1
        // PixelFormat_BGR8

        // // Time tests
        // // DEFAULT
        // const auto reps = 1e2;
        // const auto begin1 = std::chrono::high_resolution_clock::now();
        // for (auto asdf = 0 ; asdf < reps ; asdf++)
        //     const auto imagePtrTemp = imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::DEFAULT);
        // const auto durationMs1 = std::chrono::duration_cast<std::chrono::nanoseconds>(
        //     std::chrono::high_resolution_clock::now()-begin1
        // ).count() * 1e-6;
        // // EDGE_SENSING
        // const auto begin2 = std::chrono::high_resolution_clock::now();
        // for (auto asdf = 0 ; asdf < reps ; asdf++)
        //     const auto imagePtrTemp = imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::EDGE_SENSING);
        // const auto durationMs2 = std::chrono::duration_cast<std::chrono::nanoseconds>(
        //     std::chrono::high_resolution_clock::now()-begin2
        // ).count() * 1e-6;
        // // IPP
        // const auto begin3 = std::chrono::high_resolution_clock::now();
        // for (auto asdf = 0 ; asdf < reps ; asdf++)
        //     const auto imagePtrTemp = imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::IPP);
        // const auto durationMs3 = std::chrono::duration_cast<std::chrono::nanoseconds>(
        //     std::chrono::high_resolution_clock::now()-begin3
        // ).count() * 1e-6;
        // // Print times
        // opLog("Time (ms) 1: " + std::to_string(durationMs1 / reps), Priority::High);
        // opLog("Time (ms) 2: " + std::to_string(durationMs2 / reps), Priority::High);
        // opLog("Time (ms) 3: " + std::to_string(durationMs3 / reps), Priority::High);

        // Return right one
        // ~ 1.3 ms but pixeled
        // return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::DEFAULT);
        // ~0.5 ms but BW
        // return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::NO_COLOR_PROCESSING);
        // ~6 ms, looks as good as best
        // return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::HQ_LINEAR);
        // ~2.2 ms default << edge << best
        // return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::EDGE_SENSING);
        // ~115, too slow
        // return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::RIGOROUS);
        // ~1.7 ms, slightly worse than HQ_LINEAR
        return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::IPP);
        // ~30 ms, ideally best quality?
        // return imagePtr->Convert(Spinnaker::PixelFormat_BGR8, Spinnaker::DIRECTIONAL_FILTER);
    }

    // This function converts between Spinnaker::ImagePtr container to cv::Mat container used in OpenCV.
    cv::Mat spinnakerWrapperToCvMat(const Spinnaker::ImagePtr &imagePtr)
    {
        try
        {
            const auto XPadding = imagePtr->GetXPadding();
            const auto YPadding = imagePtr->GetYPadding();
            const auto rowsize = imagePtr->GetWidth();
            const auto colsize = imagePtr->GetHeight();

            // Image data contains padding. When allocating cv::Mat container size, you need to account for the X,Y
            // image data padding.
            return cv::Mat((int)(colsize + YPadding), (int)(rowsize + XPadding), CV_8UC3, imagePtr->GetData(),
                            imagePtr->GetStride());
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return cv::Mat();
        }
    }*/

    MindReaderImpl::MindReaderImpl(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
                    bool undistortImage, int cameraIndex, int cameraTriggerMode)
            : mInitialized(false),  mCameraIndex(cameraIndex), mCameraTriggerMode(cameraTriggerMode),
              mUndistortImage(undistortImage)
    {
        try
        {
            // Clean previous unclosed builds (e.g., if core dumped in the previous code using the cameras)
            release();

            // Print application build information
            opLog(std::string{ "Application build date: " } + __DATE__ + " " + __TIME__, Priority::High);

            // Enumerate all cameras
            tSdkCameraDevInfo cameraList[16];
            int cameraCount = 16;
            CameraSdkStatus status = CameraEnumerateDevice(cameraList, &cameraCount);
            if (status != CAMERA_STATUS_SUCCESS || cameraCount == 0)
            {
                error("No cameras detected.", __LINE__, __FUNCTION__, __FILE__);
            }
            else if (mCameraIndex >= cameraCount)
            {
                std::string message = "Number of cameras detected is " + std::to_string(cameraCount)
                    + ", and camera index " + std::to_string(mCameraIndex) + " is too big!";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            else
            {
                opLog("Number of cameras detected: " + std::to_string(cameraCount), Priority::High);
                if (mCameraIndex >= 0)
                {
                    if (mCameraIndex > 0)
                    {
                        opLog("Only use camera " + std::to_string(mCameraIndex) + ".", Priority::High);
                        cameraList[0] = cameraList[mCameraIndex];
                    }
                    cameraCount = 1;
                }
                // Print camera information
                opLog("*** Camera information ***", Priority::High);
                for(int i = 0; i < cameraCount; i++)
                    printDeviceInfo(cameraList[i], i);

            }
            opLog("Camera system initialized.", Priority::High);

            // Start all cameras
            mCameraDevInfos.reserve(cameraCount);
            mCameraHandles.reserve(cameraCount);
            mCameraCount = cameraCount;

            for(int i = 0; i < mCameraCount; i++)
            {
                opLog("Starting camera(" + std::to_string(i) + ")...");

                // Initialize camera
                int cameraHandle = 0;
                status = CameraInit(&cameraList[i], -1, -1, &cameraHandle);
                if (status != CAMERA_STATUS_SUCCESS)
                {
                    std::string message = "Failed to initialize the camera! Error code is "
                        + std::to_string(status) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                // Get the camera's feature description
                tSdkCameraCapbility cameraCapbility;
                CameraGetCapability(cameraHandle, &cameraCapbility);
                
                // Mono cameras allow the ISP to directly output MONO data instead of the 24-bit grayscale expanded to R=G=B
                if(cameraCapbility.sIspCapacity.bMonoSensor)
                    CameraSetIspOutFormat(cameraHandle,CAMERA_MEDIA_TYPE_MONO8);
                else
                    CameraSetIspOutFormat(cameraHandle,CAMERA_MEDIA_TYPE_BGR8);

                // Manual exposure
                CameraSetAeState(cameraHandle, FALSE);
                //CameraSetExposureTime(cameraHandle, 30 * 1000);
                
                // Configure trigger
                if (cameraTriggerMode > 0)
                    configCameraTrigger(cameraHandle);
                else
                    resetCameraTrigger(cameraHandle);

                // Set camera resolution
                int offsetx, offsety, width, height;
                if (cameraResolution.x > 0 && cameraResolution.y > 0)
                {
                    offsetx = (cameraCapbility.sResolutionRange.iWidthMax - cameraResolution.x) / 2;
                    offsety = (cameraCapbility.sResolutionRange.iHeightMax - cameraResolution.y) / 2;
                    width = cameraResolution.x;
                    height = cameraResolution.y;
                }
                else
                {
                    offsetx = 0;
                    offsety = 0;
                    width = cameraCapbility.sResolutionRange.iWidthMax;
                    height = cameraCapbility.sResolutionRange.iHeightMax;
                }
                SetCameraResolution(cameraHandle, offsetx, offsety, width, height);

                // Begin acquiring images
                status = CameraPlay(cameraHandle);
                if (status != CAMERA_STATUS_SUCCESS)
                {
                    std::string message = "Failed to play the camera! Error code is "
                        + std::to_string(status) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                mCameraDevInfos.push_back(cameraList[i]);
                mCameraHandles.push_back(cameraHandle);

                opLog("Camera " + std::to_string(i) + " started acquiring images...", Priority::High);
            }

            // Retrieve device serial number for filename
            opLog("\nReading (and sorting by) serial numbers...", Priority::High);
            mSerialNumbers = getSerialNumbers(&cameraList[0], mCameraCount);
            for (size_t i = 0; i < mSerialNumbers.size(); i++)
            {
                opLog("Camera " + std::to_string(i) + " serial number set to "
                    + mSerialNumbers[i] + ".", Priority::High);
            }
            if (mCameraIndex >= 0)
            {
                opLog("Only using camera index " + std::to_string(mCameraIndex) + ", i.e., serial number "
                    + mSerialNumbers[mCameraIndex] + ".", Priority::High);
            }

            // Read camera parameters from SN
            if (mUndistortImage)
            {
                // If all images required
                if (mCameraIndex < 0)
                {
                    mCameraParameterReader.readParameters(cameraParameterPath, mSerialNumbers);
                }
                else
                {
                    mCameraParameterReader.readParameters(cameraParameterPath,
                        std::vector<std::string>(mSerialNumbers.size(), mSerialNumbers.at(mCameraIndex)));
                }
            }

            // Start buffering thread
            mThreadOpened = true;
            mCloseThread = false;
            //mThread = std::thread{&MindReaderImpl::bufferingThread, this};

            // Get resolution
            const auto cvMats = getRawFrames();
            // Sanity check
            if (cvMats.empty())
                error("Cameras could not be opened.", __LINE__, __FUNCTION__, __FILE__);
            // Get resolution
            mResolution = Point<int>{cvMats[0].cols(), cvMats[0].rows()};

            const std::string numberCameras = std::to_string(mCameraIndex < 0 ? mCameraCount : 1);
            opLog("\nRunning for " + numberCameras + " out of " + std::to_string(mCameraCount)
                + " camera(s)...\n\n*** IMAGE ACQUISITION ***\n", Priority::High);

            mInitialized = true;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    MindReaderImpl::~MindReaderImpl()
    {
        try
        {
            release();
        }
        catch (const std::exception& e)
        {
            errorDestructor(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void MindReaderImpl::release()
    {
        try
        {
            if (mInitialized)
            {
                // Stop thread, close and join thread
                if (mThreadOpened)
                {
                    mCloseThread = true;
                    mThread.join();
                }

                // End acquisition for each camera
                for (int i = 0; i < mCameraCount; i++)
                {
                    int cameraHandle = mCameraHandles[i];
                    // Reset camera trigger
                    resetCameraTrigger(cameraHandle);
                    // close camera
                    CameraUnInit(cameraHandle);
                }

                opLog("MindVision capture completed. Releasing cameras...", Priority::High);
                // Setting the class as released
                mInitialized = false;
                opLog("Cameras released! Exiting program.", Priority::High);
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    bool MindReaderImpl::isOpened() const
    {
        return mInitialized;
    }

    Point<int> MindReaderImpl::getResolution() const
    {
        return mResolution;
    }

    std::vector<Matrix> MindReaderImpl::getCameraMatrices() const
    {
        return mCameraParameterReader.getCameraMatrices();
    }

    std::vector<Matrix> MindReaderImpl::getCameraExtrinsics() const
    {
        return mCameraParameterReader.getCameraExtrinsics();
    }

    std::vector<Matrix> MindReaderImpl::getCameraIntrinsics() const
    {
        return mCameraParameterReader.getCameraIntrinsics();
    }

    std::vector<Matrix> MindReaderImpl::getRawFrames()
    {
        try
        {
            // Sanity check
            if (mUndistortImage && mCameraCount != mCameraParameterReader.getNumberCameras())
                error("The number of cameras must be the same as the INTRINSICS vector size.",
                    __LINE__, __FUNCTION__, __FILE__);
            // Return frames
            //return acquireImages(mCameraParameterReader.getCameraIntrinsics(),
            //                                mCameraParameterReader.getCameraDistortions(),
            //                                mCameraIndex);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }
/*
    void MindReaderImpl::readAndUndistortImage(const int i, const Spinnaker::ImagePtr& imagePtr,
                                const cv::Mat& cameraIntrinsics = cv::Mat(),
                                const cv::Mat& cameraDistorsions = cv::Mat())
    {
        try
        {
            // Original image --> BGR uchar image
            const auto imagePtrColor = spinnakerImagePtrToColor(imagePtr);
            // Spinnaker to cv::Mat
            const auto cvMatDistorted = spinnakerWrapperToCvMat(imagePtrColor);
            // const auto cvMatDistorted = spinnakerWrapperToCvMat(imagePtr);
            // Undistort
            if (mUndistortImage)
            {
                // Sanity check
                if (cameraIntrinsics.empty() || cameraDistorsions.empty())
                    error("Camera intrinsics/distortions were empty.", __LINE__, __FUNCTION__, __FILE__);
                // // Option a - 80 ms / 3 images
                // // http://docs.opencv.org/2.4/modules/imgproc/doc/geometric_transformations.html#undistort
                // cv::undistort(cvMatDistorted, mCvMats[i], cameraIntrinsics, cameraDistorsions);
                // // In OpenCV 2.4, cv::undistort is exactly equal than cv::initUndistortRectifyMap
                // (with CV_16SC2) + cv::remap (with LINEAR). I.e., opLog(cv::norm(cvMatMethod1-cvMatMethod2)) = 0.
                // Option b - 15 ms / 3 images (LINEAR) or 25 ms (CUBIC)
                // Distortion removal - not required and more expensive (applied to the whole image instead of
                // only to our interest points)
                if (mRemoveDistortionMaps1[i].empty() || mRemoveDistortionMaps2[i].empty())
                {
                    const auto imageSize = cvMatDistorted.size();
                    cv::initUndistortRectifyMap(cameraIntrinsics,
                                                cameraDistorsions,
                                                cv::Mat(),
                                                // cameraIntrinsics instead of cv::getOptimalNewCameraMatrix to
                                                // avoid black borders
                                                cameraIntrinsics,
                                                // #include <opencv2/calib3d/calib3d.hpp> for next line
                                                // cv::getOptimalNewCameraMatrix(cameraIntrinsics,
                                                //                               cameraDistorsions,
                                                //                               imageSize, 1,
                                                //                               imageSize, 0),
                                                imageSize,
                                                CV_16SC2, // Faster, less memory
                                                // CV_32FC1, // More accurate
                                                mRemoveDistortionMaps1[i],
                                                mRemoveDistortionMaps2[i]);
                }
                cv::remap(cvMatDistorted, mCvMats[i],
                            mRemoveDistortionMaps1[i], mRemoveDistortionMaps2[i],
                            // cv::INTER_NEAREST);
                            cv::INTER_LINEAR);
                            // cv::INTER_CUBIC);
                            // cv::INTER_LANCZOS4); // Smoother, but we do not need this quality & its >>expensive
            }
            // Baseline (do not undistort)
            else
                mCvMats[i] = cvMatDistorted.clone();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void MindReaderImpl::bufferingThread()
    {
        #ifdef USE_FLIR_CAMERA
            try
            {
                mCloseThread = false;
                // Get cameras - ~0.005 ms (3 cameras)
                std::vector<Spinnaker::CameraPtr> cameraPtrs(mCameraList.GetSize());
                for (auto i = 0u; i < cameraPtrs.size(); i++)
                    cameraPtrs.at(i) = mCameraList.GetBySerial(mSerialNumbers.at(i)); // Sorted by Serial Number
                    // cameraPtrs.at(i) = mCameraList.GetByIndex(i); // Sorted by however Spinnaker decided
                while (!mCloseThread)
                {
                    // Trigger
                    for (auto i = 0u; i < cameraPtrs.size(); i++)
                    {
                        // Retrieve GenICam nodemap
                        auto& iNodeMap = cameraPtrs[i]->GetNodeMap();
                        Spinnaker::GenApi::CEnumerationPtr ptrAcquisitionMode = iNodeMap.GetNode("AcquisitionMode");
                        const auto result = GrabNextImageByTrigger(iNodeMap);
                        if (result != 0)
                            error("Error in GrabNextImageByTrigger.", __LINE__, __FUNCTION__, __FILE__);
                    }
                    // Get frame
                    std::vector<Spinnaker::ImagePtr> imagePtrs(cameraPtrs.size());
                    for (auto i = 0u; i < cameraPtrs.size(); i++)
                        imagePtrs.at(i) = cameraPtrs.at(i)->GetNextImage();
                    // Move to buffer
                    bool imagesExtracted = true;
                    for (auto& imagePtr : imagePtrs)
                    {
                        if (imagePtr->IsIncomplete())
                        {
                            opLog("Image incomplete with image status " + std::to_string(imagePtr->GetImageStatus())
                                + "...", Priority::High, __LINE__, __FUNCTION__, __FILE__);
                            imagesExtracted = false;
                            break;
                        }
                    }
                    if (imagesExtracted)
                    {
                        std::unique_lock<std::mutex> lock{mBufferMutex};
                        std::swap(mBuffer, imagePtrs);
                        lock.unlock();
                        std::this_thread::sleep_for(std::chrono::microseconds{1});
                    }
                }
            }
            catch (const std::exception& e)
            {
                error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            }
        #endif
    }

    // This function acquires and displays images from each device.
    std::vector<Matrix> MindReaderImpl::acquireImages(
        const std::vector<Matrix>& opCameraIntrinsics,
        const std::vector<Matrix>& opCameraDistorsions,
        const int cameraIndex = -1)
    {
        try
        {
            OP_OP2CVVECTORMAT(cameraIntrinsics, opCameraIntrinsics)
            OP_OP2CVVECTORMAT(cameraDistorsions, opCameraDistorsions)
            // std::vector<cv::Mat> cvMats;

            // Retrieve, convert, and return an image for each camera
            // In order to work with simultaneous camera streams, nested loops are
            // needed. It is important that the inner loop be the one iterating
            // through the cameras; otherwise, all images will be grabbed from a
            // single camera before grabbing any images from another.

            // // Get cameras - ~0.005 ms (3 cameras)
            // std::vector<Spinnaker::CameraPtr> cameraPtrs(cameraList.GetSize());
            // for (auto i = 0u; i < cameraPtrs.size(); i++)
            //     cameraPtrs.at(i) = cameraList.GetByIndex(i);

            // Read raw images - ~0.15 ms (3 cameras)
            // std::vector<Spinnaker::ImagePtr> imagePtrs(cameraPtrs.size());
            // for (auto i = 0u; i < cameraPtrs.size(); i++)
            //     imagePtrs.at(i) = cameraPtrs.at(i)->GetNextImage();
            std::vector<Spinnaker::ImagePtr> imagePtrs;
            // Retrieve frame
            auto cvMatRetrieved = false;
            while (!cvMatRetrieved)
            {
                // Retrieve frame
                std::unique_lock<std::mutex> lock{mBufferMutex};
                if (!mBuffer.empty())
                {
                    std::swap(imagePtrs, mBuffer);
                    cvMatRetrieved = true;
                }
                // No frames available -> sleep & wait
                else
                {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::microseconds{5});
                }
            }
            // Getting frames
            // Retrieve next received image and ensure image completion
            // Spinnaker::ImagePtr imagePtr = cameraPtrs.at(i)->GetNextImage();

            // All images completed
            bool imagesExtracted = true;
            for (auto& imagePtr : imagePtrs)
            {
                if (imagePtr->IsIncomplete())
                {
                    opLog("Image incomplete with image status " + std::to_string(imagePtr->GetImageStatus())
                        + "...", Priority::High, __LINE__, __FUNCTION__, __FILE__);
                    imagesExtracted = false;
                    break;
                }
            }
            mCvMats.clear();
            // Convert to cv::Mat
            if (imagesExtracted)
            {
                // // Original image --> BGR uchar image - ~4 ms (3 cameras)
                // for (auto& imagePtr : imagePtrs)
                //     imagePtr = spinnakerImagePtrToColor(imagePtr);

                // Init anti-distortion matrices first time
                if (mRemoveDistortionMaps1.empty())
                    mRemoveDistortionMaps1.resize(imagePtrs.size());
                if (mRemoveDistortionMaps2.empty())
                    mRemoveDistortionMaps2.resize(imagePtrs.size());

                // Multi-thread undistort (slowest function in the class)
                //     ~7.7msec (3 cameras + multi-thread + (initUndistortRectifyMap + remap) + LINEAR)
                //     ~23.2msec (3 cameras + multi-thread + (initUndistortRectifyMap + remap) + CUBIC)
                //     ~35msec (3 cameras + multi-thread + undistort)
                //     ~59msec (2 cameras + single-thread + undistort)
                //     ~75msec (3 cameras + single-thread + undistort)
                mCvMats.resize(imagePtrs.size());
                // All cameras
                if (cameraIndex < 0)
                {
                    // Undistort image
                    if (mUndistortImage)
                    {
                        std::vector<std::thread> threads(imagePtrs.size()-1);
                        for (auto i = 0u; i < threads.size(); i++)
                        {
                            // Multi-thread option
                            threads.at(i) = std::thread{&ImplMindReaderImpl::readAndUndistortImage, this, i,
                                                        imagePtrs.at(i), cameraIntrinsics.at(i),
                                                        cameraDistorsions.at(i)};
                            // // Single-thread option
                            // readAndUndistortImage(i, imagePtrs.at(i), cameraIntrinsics.at(i), cameraDistorsions.at(i));
                        }
                        readAndUndistortImage((int)imagePtrs.size()-1, imagePtrs.back(), cameraIntrinsics.back(),
                                                cameraDistorsions.back());
                        // Close threads
                        for (auto& thread : threads)
                            if (thread.joinable())
                                thread.join();
                    }
                    // Do not undistort image
                    else
                    {
                        for (auto i = 0u; i < imagePtrs.size(); i++)
                            readAndUndistortImage(i, imagePtrs.at(i));
                    }
                }
                // Only 1 camera
                else
                {
                    // Sanity check
                    if ((unsigned int)cameraIndex >= imagePtrs.size())
                        error("There are only " + std::to_string(imagePtrs.size())
                                + " cameras, but you asked for the "
                                + std::to_string(cameraIndex+1) +"-th camera (i.e., `--flir_camera_index "
                                + std::to_string(cameraIndex) +"`), which doesn't exist. Note that the index is"
                                + " 0-based.", __LINE__, __FUNCTION__, __FILE__);
                    // Undistort image
                    if (mUndistortImage)
                        readAndUndistortImage(cameraIndex, imagePtrs.at(cameraIndex), cameraIntrinsics.at(cameraIndex),
                                                cameraDistorsions.at(cameraIndex));
                    // Do not undistort image
                    else
                        readAndUndistortImage(cameraIndex, imagePtrs.at(cameraIndex));
                    mCvMats = std::vector<cv::Mat>{mCvMats[cameraIndex]};
                }
            }
            OP_CV2OPVECTORMAT(opMats, mCvMats)
            return opMats;
        }
        catch (Spinnaker::Exception &e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }
*/
}

