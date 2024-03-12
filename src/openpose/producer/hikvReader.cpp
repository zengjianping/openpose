#include <openpose/producer/hikvReader.hpp>
#include <openpose/utilities/fastMath.hpp>
#include <openpose/utilities/string.hpp>
#include <openpose/core/point.hpp>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <mutex>
#include "MvCameraControl.h"


namespace op
{
    class OP_API HikvReaderImpl
    {
    public:
        /**
         * Constructor of HikvReaderImpl. It opens all the available HikVision cameras
         * cameraIndex = -1 means that all cameras are taken
         */
        explicit HikvReaderImpl(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
                    bool undistortImage, int cameraIndex=-1, int cameraTriggerMode=0, double captureFps=-1);
        virtual ~HikvReaderImpl();

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
        std::vector<void*> mCameraHandles;
        std::vector<MV_CC_DEVICE_INFO> mCameraDevInfos;
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
        DELETE_COPY(HikvReaderImpl);
    };

    namespace {

    std::vector<std::string> getSerialNumbers(MV_CC_DEVICE_INFO* cameraList[], int cameraCount)
    {
        try
        {
            std::vector<std::string> serialNumbers(cameraCount);
            for (int i = 0; i < cameraCount; i++)
            {
                const MV_USB3_DEVICE_INFO& devInfo = cameraList[i]->SpecialInfo.stUsb3VInfo;
                serialNumbers[i] = (char*)&devInfo.chSerialNumber[0];
            }

            return serialNumbers;
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return {};
        }
    }

    void printDeviceInfo(const MV_CC_DEVICE_INFO* cameraDevInfo, int cameraIndex)
    {
        try
        {
            opLog("Printing device information for camera " + std::to_string(cameraIndex) + "...\n", Priority::High);
            const MV_USB3_DEVICE_INFO& devInfo = cameraDevInfo->SpecialInfo.stUsb3VInfo;
            opLog(std::string("ProductFamily: ") + (char*)&devInfo.chFamilyName[0], Priority::High);
            opLog(std::string("ProductModel: ") + (char*)&devInfo.chModelName[0], Priority::High);
            opLog(std::string("DeviceGUID: ") + (char*)&devInfo.chDeviceGUID[0], Priority::High);
            opLog(std::string("DeviceSerialNo: ") + (char*)&devInfo.chSerialNumber[0], Priority::High);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    // This function configures the camera to use a trigger.
    void configCameraTrigger(void* cameraHandle, int triggerMode)
    {
        try
        {
            opLog("Setting camera trigger mode...", Priority::High);

            int nRet = MV_CC_SetBoolValue(cameraHandle, "AcquisitionFrameRateEnable", triggerMode==0);
            if (MV_OK != nRet)
            {
                std::string message = "Failed to set parameter(AcquisitionFrameRateEnable)!, Error code is "
                    + std::to_string(nRet) + ".";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }

            nRet = MV_CC_SetEnumValue(cameraHandle, "TriggerMode", triggerMode == 0 ? 0 : 1);
            if (MV_OK != nRet)
            {
                std::string message = "Failed to set trigger mode!, Error code is "
                    + std::to_string(nRet) + ".";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }

            if (triggerMode > 0)
            {
                nRet = MV_CC_SetEnumValue(cameraHandle, "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to set trigger source!, Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }
            }

            opLog("Trigger mode is set to " + std::to_string(triggerMode) + ".", Priority::High);
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void grabNextImageByTrigger(void* cameraHandle)
    {
        try
        {
            // Since the camera is currently in soft trigger mode, software is required to send a command to inform
            // the camera to take pictures (to avoid accidentally fetching old pictures in the camera cache, the cache
            // is cleared before the trigger command)
            //CameraClearBuffer(cameraHandle);
            int nRet = MV_CC_SetCommandValue(cameraHandle, "TriggerSoftware");
            if(MV_OK != nRet)
            {
                std::string message = "Failed to trigger the camera! Error code is "
                    + std::to_string(nRet) + ".";
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
    /*int SetCameraResolution(int cameraHandle, int offsetx, int offsety, int width, int height)
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
    }*/

    }

    HikvReaderImpl::HikvReaderImpl(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
                    bool undistortImage, int cameraIndex, int cameraTriggerMode, double captureFps)
    {
        mCameraIndex = cameraIndex;
        mCameraTriggerMode = cameraTriggerMode;
        mUndistortImage = undistortImage;
        mResolution = cameraResolution;
        mCameraParameterPath = cameraParameterPath;
        if (mCameraParameterPath.back() != '/')
            mCameraParameterPath.append(1, '/');
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

    HikvReaderImpl::~HikvReaderImpl()
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

    void HikvReaderImpl::clear()
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
                void* cameraHandle = mCameraHandles[i];
                configCameraTrigger(cameraHandle, 0);
                int nRet = MV_CC_StopGrabbing(cameraHandle);
                nRet = MV_CC_CloseDevice(cameraHandle);
                nRet = MV_CC_DestroyHandle(cameraHandle);
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

    void HikvReaderImpl::initialize()
    {
        try
        {
            clear();

            // Print application build information
            opLog(std::string{ "Application build date: " } + __DATE__ + " " + __TIME__, Priority::High);

            // enum device
            MV_CC_DEVICE_INFO_LIST stDeviceList;
            memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
            int nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);
            int cameraCount = stDeviceList.nDeviceNum;

            if (MV_OK != nRet || cameraCount == 0)
            {
                error("No cameras detected.", __LINE__, __FUNCTION__, __FILE__);
            }
            else if (mCameraIndex >= cameraCount)
            {
                std::string message = "Number of cameras detected is " + std::to_string(cameraCount)
                    + ", and camera index " + std::to_string(mCameraIndex) + " is too big!";
                error(message, __LINE__, __FUNCTION__, __FILE__);
            }

            MV_CC_DEVICE_INFO* pDeviceInfo[MV_MAX_DEVICE_NUM] = {0};
            for (int i = 0; i < cameraCount; i++)
                pDeviceInfo[i] = stDeviceList.pDeviceInfo[i];

            std::sort(&pDeviceInfo[0], &pDeviceInfo[cameraCount], [](MV_CC_DEVICE_INFO* a, MV_CC_DEVICE_INFO* b)
                    {return strcmp((char*)&a->SpecialInfo.stUsb3VInfo.chSerialNumber[0],
                                   (char*)&b->SpecialInfo.stUsb3VInfo.chSerialNumber[0]) <= 0;});

            opLog("Number of cameras detected: " + std::to_string(cameraCount), Priority::High);
            if (mCameraIndex >= 0)
            {
                opLog("Only use camera " + std::to_string(mCameraIndex) + ".");
                if (mCameraIndex > 0)
                {
                    pDeviceInfo[0] = pDeviceInfo[mCameraIndex];
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
                printDeviceInfo(pDeviceInfo[i], i);
            }
            
            // Retrieve device serial number for filename
            mSerialNumbers = getSerialNumbers(pDeviceInfo, mCameraCount);
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
                void* cameraHandle = nullptr;
                nRet = MV_CC_CreateHandle(&cameraHandle, pDeviceInfo[i]);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to initialize the camera! Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                nRet = MV_CC_OpenDevice(cameraHandle);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to open the camera! Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                // Configure trigger
                configCameraTrigger(cameraHandle, mCameraTriggerMode);

                // Camera exposure
                nRet = MV_CC_SetEnumValue(cameraHandle, "ExposureAuto", 2);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to set auto-exposure! Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                nRet = MV_CC_SetEnumValue(cameraHandle, "GainAuto", 2);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to set auto-gain! Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }
                
                // Get the camera's feature description
                /*tSdkCameraCapbility cameraCapbility;
                CameraGetCapability(cameraHandle, &cameraCapbility);
                
                // Mono cameras allow the ISP to directly output MONO data instead of the 24-bit grayscale expanded to R=G=B
                if(cameraCapbility.sIspCapacity.bMonoSensor)
                    CameraSetIspOutFormat(cameraHandle,CAMERA_MEDIA_TYPE_MONO8);
                else
                    CameraSetIspOutFormat(cameraHandle,CAMERA_MEDIA_TYPE_BGR8);

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
                SetCameraResolution(cameraHandle, offsetx, offsety, width, height);*/

                // Begin acquiring images
                nRet = MV_CC_StartGrabbing(cameraHandle);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to play the camera! Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }

                mCameraDevInfos.push_back(*pDeviceInfo[i]);
                mCameraHandles.push_back(cameraHandle);

                opLog("Camera " + std::to_string(i) + " started acquiring images...", Priority::High);
            }

            // Start buffering thread
            mThreadOpened = true;
            mCloseThread = false;
            mThread = std::thread{&HikvReaderImpl::bufferingThread, this};

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

    void HikvReaderImpl::release()
    {
        try
        {
            if (mInitialized)
            {
                opLog("HikvVision capture completed. Releasing cameras...", Priority::High);
                clear();
                opLog("Cameras released! Exiting program.", Priority::High);
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    bool HikvReaderImpl::isOpened() const
    {
        return mInitialized;
    }

    Point<int> HikvReaderImpl::getResolution() const
    {
        return mResolution;
    }

    std::vector<Matrix> HikvReaderImpl::getCameraMatrices() const
    {
        return mCameraParameterReader.getCameraMatrices();
    }

    std::vector<Matrix> HikvReaderImpl::getCameraExtrinsics() const
    {
        return mCameraParameterReader.getCameraExtrinsics();
    }

    std::vector<Matrix> HikvReaderImpl::getCameraIntrinsics() const
    {
        return mCameraParameterReader.getCameraIntrinsics();
    }

    std::vector<Matrix> HikvReaderImpl::getRawFrames()
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

    void HikvReaderImpl::bufferingThread()
    {
        try
        {
            mCloseThread = false;

            std::chrono::time_point<std::chrono::steady_clock> start_time, capture_time, current_time;
            start_time = std::chrono::steady_clock::now();
            int64_t captureCount = 0;
            unsigned char *payloadDatas[256] = {0};
            unsigned int payloadSizes[256] = {0};
            unsigned char *rgbImageDatas[256] = {0};
            unsigned int rgbImageSizes[256] = {0};

            for (int i = 0; i < mCameraCount; i++)
            {
                void* cameraHandle = mCameraHandles[i];
            
                MVCC_INTVALUE stParam;
                memset(&stParam, 0, sizeof(MVCC_INTVALUE));
                int nRet = MV_CC_GetIntValue(cameraHandle, "PayloadSize", &stParam);
                if (MV_OK != nRet)
                {
                    std::string message = "Failed to get payload size! Error code is "
                        + std::to_string(nRet) + ".";
                    error(message, __LINE__, __FUNCTION__, __FILE__);
                }
                payloadSizes[i] = stParam.nCurValue;
                payloadDatas[i] = (unsigned char *)malloc(sizeof(unsigned char) * stParam.nCurValue);
                rgbImageSizes[i] = 0;
                rgbImageDatas[i] = nullptr;
            }

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
                    void* cameraHandle = mCameraHandles[i];
                    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
                    int nRet = MV_CC_GetOneFrameTimeout(cameraHandle, payloadDatas[i], payloadSizes[i], &stImageInfo, 1000);
                    if (nRet == MV_OK)
                    {
                        //printf("GetOneFrame, Width[%d], Height[%d], nFrameLen[%d], enPixelType[%ld]\n",
                        //    stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameLen, stImageInfo.enPixelType);
                        //printf("mResolutionX[%d], mResolutionY[%d]\n", mResolution.x, mResolution.y);
                        if (rgbImageDatas[i] == nullptr)
                        {
                            rgbImageSizes[i] = stImageInfo.nWidth * stImageInfo.nHeight *  4 + 2048;
                            rgbImageDatas[i] = (unsigned char *)malloc(sizeof(unsigned char) * rgbImageSizes[i]);
                        }
        
                        // convert pixel format 
                        MV_CC_PIXEL_CONVERT_PARAM stConvertParam = {0};
                        // Top to bottom are：image width, image height, input data buffer, input data size, source pixel format, 
                        // destination pixel format, output data buffer, provided output buffer size
                        stConvertParam.nWidth = stImageInfo.nWidth;
                        stConvertParam.nHeight = stImageInfo.nHeight;
                        stConvertParam.pSrcData = payloadDatas[i];
                        stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
                        stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
                        stConvertParam.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
                        stConvertParam.pDstBuffer = rgbImageDatas[i];
                        stConvertParam.nDstBufferSize = rgbImageSizes[i];
                        nRet = MV_CC_ConvertPixelType(cameraHandle, &stConvertParam);
                        if (MV_OK != nRet)
                        {
                            std::string message = "Failed to convert image! Error code is "
                                + std::to_string(nRet) + ".";
                            error(message, __LINE__, __FUNCTION__, __FILE__);
                        }

                        int imgWidth = stImageInfo.nWidth;
                        int imgHeight = stImageInfo.nHeight;
                        cv::Mat matImage(cv::Size(imgWidth, imgHeight), CV_8UC3, rgbImageDatas[i]);
                        if (mResolution.x > 0 && mResolution.y > 0 && (imgWidth != mResolution.x || imgHeight != mResolution.y))
                            cv::resize(matImage, cvMats.at(i), cv::Size(mResolution.x, mResolution.y));
                        else
                            matImage.copyTo(cvMats.at(i));
                    }
                    else
                    {
                        std::string message = "Failed to capture image!, Error code is " + std::to_string(nRet) + ".";
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
                    int64_t microseconds = 1000000 * ++captureCount / mCaptureFps;
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

            for (int i = 0; i < mCameraCount; i++)
            {
                if (payloadDatas[i])
                    free(payloadDatas[i]);
                if (rgbImageDatas[i])
                    free(rgbImageDatas[i]);
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    void HikvReaderImpl::readAndUndistortImage(int i, const cv::Mat& cvMatDistorted,
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
    std::vector<Matrix> HikvReaderImpl::acquireImages(const std::vector<Matrix>& opCameraIntrinsics,
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
                        threads.at(i) = std::thread{&HikvReaderImpl::readAndUndistortImage, this, i,
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


    HikvReader::HikvReader(const std::string& cameraParameterPath, const Point<int>& cameraResolution,
            bool undistortImage, int cameraIndex, int cameraTriggerMode, double captureFps) :
        Producer{ProducerType::HikvCamera, cameraParameterPath, undistortImage, -1},
        mFrameNameCounter{0ull}
    {
        try
        {
            upImpl = std::make_shared<HikvReaderImpl>(cameraParameterPath, cameraResolution,
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

    HikvReader::~HikvReader()
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

    std::vector<Matrix> HikvReader::getCameraMatrices()
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

    std::vector<Matrix> HikvReader::getCameraExtrinsics()
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

    std::vector<Matrix> HikvReader::getCameraIntrinsics()
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

    std::string HikvReader::getNextFrameName()
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

    bool HikvReader::isOpened() const
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

    void HikvReader::release()
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

    Matrix HikvReader::getRawFrame()
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

    std::vector<Matrix> HikvReader::getRawFrames()
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

    double HikvReader::get(const int capProperty)
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

    void HikvReader::set(const int capProperty, const double value)
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

