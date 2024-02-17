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
                    bool undistortImage, int cameraIndex=-1, int cameraTriggerMode=0, double captureFps=-1);
        virtual ~MindReaderImpl();

    public:
        void initialize();
        void release();
        bool isOpened() const;
        Point<int> getResolution() const;
        std::vector<Matrix> getCameraMatrices() const;
        std::vector<Matrix> getCameraExtrinsics() const;
        std::vector<Matrix> getCameraIntrinsics() const;
        std::vector<Matrix> getRawFrames();

    protected:
        void clear();
        void bufferingThread();
        std::vector<Matrix> acquireImages(const std::vector<Matrix>& opCameraIntrinsics,
            const std::vector<Matrix>& opCameraDistorsions);
        void readAndUndistortImage(int i, const cv::Mat& cvMatDistorted,
            const cv::Mat& cameraIntrinsics=cv::Mat(), const cv::Mat& cameraDistorsions=cv::Mat());

    protected:
        bool mInitialized = false;
        int mCameraCount = 0;
        int mCameraIndex = -1;
        int mCameraTriggerMode = 0;
        double mCaptureFps = -1;
        bool mZoomAfterCapture = true;
        Point<int> mResolution;
        std::vector<int> mCameraHandles;
        std::vector<tSdkCameraDevInfo> mCameraDevInfos;
        std::vector<std::string> mSerialNumbers;
        std::vector<cv::Mat> mCvMats;
        // Undistortion
        bool mUndistortImage = false;
        std::string mCameraParameterPath;
        std::vector<cv::Mat> mRemoveDistortionMaps1;
        std::vector<cv::Mat> mRemoveDistortionMaps2;
        CameraParameterReader mCameraParameterReader;
        // Thread
        bool mThreadOpened = false;
        std::atomic<bool> mCloseThread;
        std::vector<cv::Mat> mBuffer;
        std::mutex mBufferMutex;
        std::thread mThread;

    protected:
        DELETE_COPY(MindReaderImpl);
    };

    std::vector<std::string> getSerialNumbers(const tSdkCameraDevInfo* cameraList, int cameraCount)
    {
        try
        {
            std::vector<std::string> serialNumbers(cameraCount);
            for (int i = 0; i < cameraCount; i++)
            {
                std::string serialNumber(cameraList[i].acProductName);
                //serialNumbers[i] = serialNumber + "-" + std::to_string(cameraList[i].uInstance);
                //serialNumbers[i] = serialNumber + "-" + cameraList[i].acSn;
                serialNumbers[i] = cameraList[i].acSn;
            }
            //std::sort(serialNumbers.begin(), serialNumbers.end());

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
    /*void resetCameraTrigger(int cameraHandle)
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
    }*/

    // This function configures the camera to use a trigger.
    void configCameraTrigger(int cameraHandle, int triggerMode)
    {
        try
        {
            opLog("Setting camera trigger mode...", Priority::High);
            CameraSdkStatus status = CameraSetTriggerMode(cameraHandle, triggerMode);
            if (status != CAMERA_STATUS_SUCCESS)
            {
                std::string message = "Failed to set trigger mode!, Error code is "
                    + std::to_string(status) + ".";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }
            CameraSetTriggerCount(cameraHandle, 1);
            opLog("Trigger mode is set to " + std::to_string(triggerMode) + ".", Priority::High);
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

    int SetCameraResolutionEx(int cameraHandle, int width, int height, int widthMax, int heightMax)
    {
        tSdkImageResolution sRoiResolution = { 0 };
        
        // Set to 0xff for custom resolution, set to 0 to N for select preset resolution
        sRoiResolution.iIndex = 0xff;
        
        // iWidthFOV represents the camera's field of view width, iWidth represents the camera's
        // actual output width. In most cases iWidthFOV=iWidth. Some special resolution modes such
        // as BIN2X2:iWidthFOV=2*iWidth indicate that the field of view is twice the actual output width
        sRoiResolution.iWidth = width;
        sRoiResolution.iWidthFOV = widthMax;
        
        // height, refer to the description of the width above
        sRoiResolution.iHeight = height;
        sRoiResolution.iHeightFOV = heightMax;
        
        // Field of view offset
        sRoiResolution.iHOffsetFOV = 0;
        sRoiResolution.iVOffsetFOV = 0;
        
        // ISP software zoom width and height, all 0 means not zoom
        sRoiResolution.iWidthZoomSw = 0;
        sRoiResolution.iHeightZoomSw = 0;
        
        // ISP hardware zoom width and height, all 0 means not zoom
        sRoiResolution.iWidthZoomHd = width;
        sRoiResolution.iHeightZoomHd = height;
        
        // BIN SKIP mode setting (requires camera hardware support)
        sRoiResolution.uBinAverageMode = 0;
        sRoiResolution.uBinSumMode = 0;
        sRoiResolution.uResampleMask = 0;
        sRoiResolution.uSkipMode = 0;
        
        return CameraSetImageResolution(cameraHandle, &sRoiResolution);
    }

    MindReaderImpl::MindReaderImpl(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
                    bool undistortImage, int cameraIndex, int cameraTriggerMode, double captureFps)
    {
        mCameraIndex = cameraIndex;
        mCameraTriggerMode = cameraTriggerMode;
        mUndistortImage = undistortImage;
        mResolution = cameraResolution;
        mCameraParameterPath = cameraParameterPath;
        mCaptureFps = captureFps;

        try
        {
            initialize();
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

    void MindReaderImpl::clear()
    {
        try
        {
            if (mThreadOpened)
            {
                mCloseThread = true;
                mThread.join();
                mBuffer.clear();
                mThreadOpened = false;
            }

            for (size_t i = 0; i < mCameraHandles.size(); i++)
            {
                int cameraHandle = mCameraHandles[i];
                configCameraTrigger(cameraHandle, 0);
                CameraUnInit(cameraHandle);
            }
            mCameraHandles.clear();
            mCameraDevInfos.clear();
            mSerialNumbers.clear();
            mCvMats.clear();

            mRemoveDistortionMaps1.clear();
            mRemoveDistortionMaps2.clear();

            mCameraCount = 0;
            mInitialized = false;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void MindReaderImpl::initialize()
    {
        try
        {
            clear();

            // Print application build information
            opLog(std::string{ "Application build date: " } + __DATE__ + " " + __TIME__, Priority::High);

            // Enumerate all cameras
            tSdkCameraDevInfo cameraList[16];
            int cameraCount = 15;
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
            std::sort(&cameraList[0], &cameraList[cameraCount], [](tSdkCameraDevInfo& a, tSdkCameraDevInfo& b)
                                                                {return strcmp(a.acSn, b.acSn) <= 0;});

            opLog("Number of cameras detected: " + std::to_string(cameraCount), Priority::High);
            if (mCameraIndex >= 0)
            {
                opLog("Only use camera " + std::to_string(mCameraIndex) + ".");
                if (mCameraIndex > 0)
                {
                    cameraList[0] = cameraList[mCameraIndex];
                }
                mCameraCount = 1;
            }
            else
            {
                opLog("Use all cameras.");
                mCameraCount = cameraCount;
            }

            // Print camera information
            opLog("*** Camera device information ***", Priority::High);
            for(int i = 0; i < mCameraCount; i++)
            {
                printDeviceInfo(cameraList[i], i);
            }
            
            // Retrieve device serial number for filename
            mSerialNumbers = getSerialNumbers(&cameraList[0], mCameraCount);
            for(int i = 0; i < mCameraCount; i++)
            {
                opLog("Camera " + std::to_string(i) + " serial number set to "
                    + mSerialNumbers[i] + ".", Priority::High);
            }

            // Read camera parameters from SN
            if (mUndistortImage)
            {
                mCameraParameterReader.readParameters(mCameraParameterPath, mSerialNumbers);
            }

            opLog("Camera system initialized.", Priority::High);

            // Start all cameras
            mCameraDevInfos.reserve(mCameraCount);
            mCameraHandles.reserve(mCameraCount);

            for(int i = 0; i < mCameraCount; i++)
            {
                opLog("Starting camera " + std::to_string(i) + "...");

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

                // Camera exposure
                bool autoExposure = true;
                CameraSetAeState(cameraHandle, autoExposure);
                CameraSetAeTarget(cameraCount, 80);
                CameraSetAnalogGain(cameraHandle, 80);
                CameraSetExposureTime(cameraHandle, 20 * 1000);
                
                // Configure trigger
                configCameraTrigger(cameraHandle, mCameraTriggerMode);

                // Set camera resolution
                int widthMax = cameraCapbility.sResolutionRange.iWidthMax;
                int heightMax = cameraCapbility.sResolutionRange.iHeightMax;
                int offsetx = 0, offsety = 0;
                int width = widthMax, height = heightMax;

                if (mResolution.x <= 0 || mResolution.y <= 0)
                {
                    mResolution.x = widthMax;
                    mResolution.y = heightMax;
                }
                if (!mZoomAfterCapture)
                {
                    offsetx = (widthMax - mResolution.x) / 2;
                    offsety = (heightMax - mResolution.y) / 2;
                    width = mResolution.x;
                    height = mResolution.y;
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

            // Start buffering thread
            mThreadOpened = true;
            mCloseThread = false;
            mThread = std::thread{&MindReaderImpl::bufferingThread, this};

            // Get resolution
            const auto cvMats = getRawFrames();
            // Sanity check
            if (cvMats.empty())
                error("Cameras could not be opened.", __LINE__, __FUNCTION__, __FILE__);
            // Get resolution
            mResolution = Point<int>{cvMats[0].cols(), cvMats[0].rows()};
            opLog("Video resolution: " + std::to_string(mResolution.x) + "x" + std::to_string(mResolution.y));

            opLog("*** IMAGE ACQUISITION ***", Priority::High);

            mInitialized = true;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void MindReaderImpl::release()
    {
        try
        {
            if (mInitialized)
            {
                opLog("MindVision capture completed. Releasing cameras...", Priority::High);
                clear();
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
            if (mUndistortImage && mCameraCount != (int)mCameraParameterReader.getNumberCameras())
                error("The number of cameras must be the same as the INTRINSICS vector size.",
                    __LINE__, __FUNCTION__, __FILE__);
            // Return frames
            return acquireImages(mCameraParameterReader.getCameraIntrinsics(),
                mCameraParameterReader.getCameraDistortions());
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }

    void MindReaderImpl::bufferingThread()
    {
        try
        {
            mCloseThread = false;

            std::chrono::time_point<std::chrono::steady_clock> start_time, capture_time, current_time;
            start_time = std::chrono::steady_clock::now();
            int64_t capture_count = 0;

            while (!mCloseThread)
            {
                std::vector<cv::Mat> cvMats(mCameraCount);
                
                // Trigger
                if (mCameraTriggerMode == 1)
                {
                    for (int i = 0; i < mCameraCount; i++)
                        grabNextImageByTrigger(mCameraHandles[i]);
                }

                // Get frame
                bool imagesExtracted = true;
                for (int i = 0; i < mCameraCount; i++)
                {
                    int cameraHandle = mCameraHandles[i];
                    tSdkFrameHead sFrameInfo;
                    BYTE* pbyBuffer = nullptr;

                    //unsigned char*pRgbBuffer = (unsigned char*)malloc(mResolution.x*mResolution.y*3);
                    CameraSdkStatus status = CameraGetImageBuffer(cameraHandle, &sFrameInfo, &pbyBuffer, 1000);

                    if(status == CAMERA_STATUS_SUCCESS)
                    {
                        int imgWidth = sFrameInfo.iWidth;
                        int imgHeight = sFrameInfo.iHeight;
                        cv::Mat matImage(cv::Size(imgWidth, imgHeight), 
                            sFrameInfo.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3);
                        CameraImageProcess(cameraHandle, pbyBuffer, matImage.data, &sFrameInfo);
                        if (imgWidth != mResolution.x || imgHeight != mResolution.y)
                            cv::resize(matImage, cvMats.at(i), cv::Size(mResolution.x, mResolution.y));
                        else
                            cvMats.at(i) = matImage;
            			CameraReleaseImageBuffer(cameraHandle, pbyBuffer);
                    }
                    else
                    {
                        std::string message = "Failed to capture image!, Error code is " + std::to_string(status) + ".";
                        opLog(message);
                        //delete pRgbBuffer;
                        imagesExtracted = false;
                    }
                }

                if (imagesExtracted)
                {
                    std::unique_lock<std::mutex> lock{mBufferMutex};
                    //static int cnt = 0;
                    //opLog("Product images: " + std::to_string(++cnt));
                    std::swap(mBuffer, cvMats);
                    lock.unlock();
                    //std::this_thread::sleep_for(std::chrono::microseconds{1});
                }

                if (mCaptureFps > 0.)
                {
                    int64_t microseconds = 1000000 * ++capture_count / mCaptureFps;
                    capture_time = start_time + std::chrono::microseconds(microseconds);
                    current_time = std::chrono::steady_clock::now();
                    if (capture_time > current_time)
                        std::this_thread::sleep_until(capture_time);
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::microseconds{1});
                }
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void MindReaderImpl::readAndUndistortImage(int i, const cv::Mat& cvMatDistorted,
        const cv::Mat& cameraIntrinsics, const cv::Mat& cameraDistorsions)
    {
        try
        {
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
            {
                mCvMats[i] = cvMatDistorted.clone();
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    // This function acquires and displays images from each device.
    std::vector<Matrix> MindReaderImpl::acquireImages(const std::vector<Matrix>& opCameraIntrinsics,
        const std::vector<Matrix>& opCameraDistorsions)
    {
        try
        {
            OP_OP2CVVECTORMAT(cameraIntrinsics, opCameraIntrinsics)
            OP_OP2CVVECTORMAT(cameraDistorsions, opCameraDistorsions)
            std::vector<cv::Mat> cvMats;

            // Retrieve, convert, and return an image for each camera
            // In order to work with simultaneous camera streams, nested loops are
            // needed. It is important that the inner loop be the one iterating
            // through the cameras; otherwise, all images will be grabbed from a
            // single camera before grabbing any images from another.

            // Retrieve frame
            bool cvMatRetrieved = false;
            while (!cvMatRetrieved)
            {
                // Retrieve frame
                std::unique_lock<std::mutex> lock{mBufferMutex};
                if (!mBuffer.empty())
                {
                    //static int cnt = 0;
                    //opLog("Consume images: " + std::to_string(++cnt));
                    std::swap(cvMats, mBuffer);
                    cvMatRetrieved = true;
                }
                // No frames available -> sleep & wait
                else
                {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::microseconds{5});
                }
            }

            // All images completed
            bool imagesExtracted = true;
            for (const cv::Mat& cvMat : cvMats)
            {
                if (cvMat.empty())
                {
                    opLog("Image incomplete.", Priority::High, __LINE__, __FUNCTION__, __FILE__);
                    imagesExtracted = false;
                    break;
                }
            }

            mCvMats.clear();
            if (imagesExtracted)
            {
                // Init anti-distortion matrices first time
                if (mRemoveDistortionMaps1.empty())
                    mRemoveDistortionMaps1.resize(cvMats.size());
                if (mRemoveDistortionMaps2.empty())
                    mRemoveDistortionMaps2.resize(cvMats.size());
                mCvMats.resize(cvMats.size());

                // Undistort image
                if (mUndistortImage)
                {
                    std::vector<std::thread> threads(cvMats.size()-1);
                    for (auto i = 0u; i < threads.size(); i++)
                    {
                        // Multi-thread option
                        threads.at(i) = std::thread{&MindReaderImpl::readAndUndistortImage, this, i,
                            cvMats.at(i), cameraIntrinsics.at(i), cameraDistorsions.at(i)};
                        // // Single-thread option
                        // readAndUndistortImage(i, imagePtrs.at(i), cameraIntrinsics.at(i), cameraDistorsions.at(i));
                    }
                    readAndUndistortImage((int)cvMats.size()-1, cvMats.back(),
                        cameraIntrinsics.back(), cameraDistorsions.back());
            
                    // Close threads
                    for (std::thread& thread : threads)
                    {
                        if (thread.joinable())
                            thread.join();
                    }
                }
                // Do not undistort image
                else
                {
                    for (auto i = 0u; i < cvMats.size(); i++)
                        readAndUndistortImage(i, cvMats[i]);
                }
            }
            OP_CV2OPVECTORMAT(opMats, mCvMats)
            return opMats;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }


    MindReader::MindReader(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
            bool undistortImage, int cameraIndex, int cameraTriggerMode, double captureFps) :
        Producer{ProducerType::MindCamera, cameraParameterPath, undistortImage, -1},
        mFrameNameCounter{0ull}
    {
        try
        {
            upImpl = std::make_shared<MindReaderImpl>(cameraParameterPath, cameraResolution,
                undistortImage, cameraIndex, cameraTriggerMode, captureFps);
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

    std::vector<Matrix> MindReader::getCameraMatrices()
    {
        try
        {
            return upImpl->getCameraMatrices();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }

    std::vector<Matrix> MindReader::getCameraExtrinsics()
    {
        try
        {
            return upImpl->getCameraExtrinsics();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }

    std::vector<Matrix> MindReader::getCameraIntrinsics()
    {
        try
        {
            return upImpl->getCameraIntrinsics();
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
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
}
