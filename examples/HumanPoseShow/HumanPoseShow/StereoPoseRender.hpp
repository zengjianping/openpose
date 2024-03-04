#ifndef STEREO_POSE_RENDER_HPP
#define STEREO_POSE_RENDER_HPP

#include <openpose/core/common.hpp>
#include <openpose/pose/enumClasses.hpp>


namespace op
{

class OP_API StereoPoseRender
{
public:
    StereoPoseRender(const bool copyGlToCvMat = false, const PoseModel poseModel = PoseModel::BODY_25);

    virtual ~StereoPoseRender();

    void initialize();

    void setKeypoints(const Array<float>& poseKeypoints3D, const Array<float>& faceKeypoints3D,
        const Array<float>& leftHandKeypoints3D, const Array<float>& rightHandKeypoints3D);

    void update();

    Matrix readCvMat();

private:
    const bool mCopyGlToCvMat;
};

}

#endif // STEREO_POSE_RENDER_HPP
