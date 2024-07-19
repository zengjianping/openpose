#ifndef CAMERACALIBWIDGET_H
#define CAMERACALIBWIDGET_H

#include <QWidget>
#include "HumanPoseProcessor.h"

namespace Ui {
class CameraCalibWidget;
}

class CameraCalibWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraCalibWidget(QWidget *parent = nullptr);
    ~CameraCalibWidget();

signals:
    void processStatusChanged();

public:
    void setHumanPoseProcessor(HumanPoseProcessor *processor, HumanPoseParams *params);

private slots:
    void on_combCameraList_currentIndexChanged(int index);
    void on_buttonCameraInfo_clicked();
    void on_buttonIntStartCapture_clicked();
    void on_buttonIntStopCapture_clicked();
    void on_buttonIntStartCalibrate_clicked();
    void on_buttonExtStartCapture_clicked();
    void on_buttonExtStopCapture_clicked();
    void on_buttonExtStartCalibrate_clicked();

private:
    void initializeUI();
    void updateUI();
    std::string getImageDir(const std::string& calibrateDir, const std::string subDirName);

private:
    Ui::CameraCalibWidget *ui;
    HumanPoseParams *humanPoseParams = nullptr;
    HumanPoseProcessor *humanPoseProcessor = nullptr;
    std::vector<std::string> cameraNames;
};

#endif // CAMERACALIBWIDGET_H
