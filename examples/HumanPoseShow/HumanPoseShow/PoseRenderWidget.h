#ifndef POSERENDERWIDGET_H
#define POSERENDERWIDGET_H

#include <QOpenGLWidget>
#include <openpose/core/common.hpp>
#include <openpose/pose/enumClasses.hpp>
#include <openpose/utilities/keypoint.hpp>
#include <opencv2/opencv.hpp>
#include <atomic>
#include <mutex>
#include <stdio.h>


struct Keypoints3D
{
    op::Array<float> poseKeypoints;
    op::Array<float> faceKeypoints;
    op::Array<float> leftHandKeypoints;
    op::Array<float> rightHandKeypoints;
    bool validKeypoints;
    std::mutex mutex;
};

enum class CameraMode {
    CAM_DEFAULT,
    CAM_ROTATE,
    CAM_PAN,
    CAM_PAN_Z
};

class PoseRenderWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    PoseRenderWidget(QWidget* parent = nullptr);
    virtual ~PoseRenderWidget();

public:
    void setKeypoints(const op::Array<float>& poseKeypoints3D, const op::Array<float>& faceKeypoints3D,
        const op::Array<float>& leftHandKeypoints3D, const op::Array<float>& rightHandKeypoints3D);
    void clearKeypoints();

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    void drawConeByTwoPts(const cv::Point3f& pt1, const cv::Point3f& pt2, const float ptSize);
    void renderHumanBody(const op::Array<float>& keypoints, const std::vector<unsigned int>& pairs,
            const std::vector<float> colors, const float ratio);
    void initGraphics();
    void renderFloor();
    void renderMain();
    void mouseButton(const int button, const int state, const int x, const int y);
    void mouseMotion(const int x, const int y);
    void keyPressed(const unsigned char key, const int x, const int y);
    //void initializeVisualization();
    //void terminateVisualization();

private:
    std::atomic<bool> sConstructorSet{false};
    Keypoints3D gKeypoints3D;
    op::PoseModel sPoseModel = op::PoseModel::BODY_25;
    int sLastKeyPressed = -1;
    CameraMode gCameraMode = CameraMode::CAM_DEFAULT;

    //View Change by Mouse
    int gWindowHandle = -1;
    bool gBButton1Down = false;
    double gXClick = 0.f;
    double gYClick = 0.f;
    double gGViewDistance = -100.f; // -82.3994f; //-45;
    double gMouseXRotateDeg = 180.f; // -63.2f; //0;
    double gMouseYRotateDeg = -5.f; // 7.f; //60;
    double gMouseXPan = -100.f; // 0;
    double gMouseYPan = -40.f; // 0;
    double gMouseZPan = 0.f;
    double gScaleForMouseMotion = 0.1f;
    double gAspectRatio = 1.0;
};

#endif // POSERENDERWIDGET_H
