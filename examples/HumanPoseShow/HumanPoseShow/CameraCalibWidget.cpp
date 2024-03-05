#include "CameraCalibWidget.h"
#include "ui_CameraCalibWidget.h"
#include <QMessageBox>

CameraCalibWidget::CameraCalibWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraCalibWidget)
{
    ui->setupUi(this);
}

CameraCalibWidget::~CameraCalibWidget()
{
    delete ui;
}

void CameraCalibWidget::setHumanPoseProcessor(HumanPoseProcessor *processor, HumanPoseParams *params)
{
    humanPoseProcessor = processor;
    humanPoseParams = params;
}

void CameraCalibWidget::on_combCameraList_currentIndexChanged(int index)
{
}

void CameraCalibWidget::on_buttonCameraInfo_clicked()
{
    if (humanPoseProcessor)
    {
        HumanPoseParams params = *humanPoseParams;
        std::string cameraType;
        std::vector<std::string> cameraNames;
        if (humanPoseProcessor->queryCameraList(params, cameraType, cameraNames))
        {
            ui->combCameraList->clear();
            for (const std::string& name : cameraNames)
                ui->combCameraList->addItem(QString::fromStdString(name));
            ui->editCameraType->setText(QString::fromStdString(cameraType));
        }
        else
        {
            QMessageBox::information(this, tr("提示"), tr("查询相机失败！"));
        }
    }
}

void CameraCalibWidget::on_buttonIntStartCapture_clicked()
{
    if (humanPoseProcessor && !humanPoseProcessor->isRunning())
    {
        setCursor(Qt::WaitCursor);
        HumanPoseParams params = *humanPoseParams;
        params.algorithmParams.algoType = HumanPoseParams::AlgorithmParams::AlgoTypeNo;
        params.inputParams.cameraIndex = ui->combCameraList->currentIndex();
        params.inputParams.captureFps = 2;
        params.inputParams.frameUndistort = false;
        params.outputParams.saveVideo = false;
        params.outputParams.saveVideo3d = false;
        params.outputParams.saveImage = true;
        std::string cameraName = ui->combCameraList->currentText().toStdString();
        params.outputParams.imageSavePath = params.inputParams.cameraParamPath + "/intrinsics/" + cameraName;
        humanPoseProcessor->start(params);
        emit processStatusChanged();
        unsetCursor();
    }
}

void CameraCalibWidget::on_buttonIntStopCapture_clicked()
{
    if (humanPoseProcessor && humanPoseProcessor->isRunning())
    {
        setCursor(Qt::WaitCursor);
        humanPoseProcessor->stop();
        emit processStatusChanged();
        unsetCursor();
    }
}

void CameraCalibWidget::on_buttonIntStartCalibrate_clicked()
{
    if (humanPoseProcessor)
    {
        int res = QMessageBox::information(this, tr("提示"), tr("相机标定时间较长，请耐心等待！"),
                                           QMessageBox::Ok, QMessageBox::Cancel);
        if (res == QMessageBox::Ok)
        {
            setCursor(Qt::WaitCursor);
            HumanPoseParams params = *humanPoseParams;
            bool res = humanPoseProcessor->calibrateCameraIntrinsics(params);
            unsetCursor();
            if (!res)
                QMessageBox::information(this, tr("提示"), tr("标定失败！"));
        }
    }
}


void CameraCalibWidget::on_buttonExtStartCapture_clicked()
{
    if (humanPoseProcessor && !humanPoseProcessor->isRunning())
    {
        setCursor(Qt::WaitCursor);
        HumanPoseParams params = *humanPoseParams;
        params.algorithmParams.algoType = HumanPoseParams::AlgorithmParams::AlgoTypeNo;
        params.inputParams.cameraIndex = -1;
        params.inputParams.captureFps = 2;
        params.inputParams.frameUndistort = true;
        params.outputParams.saveVideo = false;
        params.outputParams.saveVideo3d = false;
        params.outputParams.saveImage = true;
        params.outputParams.imageSavePath = params.inputParams.cameraParamPath + "/extrinsics";
        humanPoseProcessor->start(params);
        emit processStatusChanged();
        unsetCursor();
    }
}

void CameraCalibWidget::on_buttonExtStopCapture_clicked()
{
    if (humanPoseProcessor && humanPoseProcessor->isRunning())
    {
        setCursor(Qt::WaitCursor);
        humanPoseProcessor->stop();
        emit processStatusChanged();
        unsetCursor();
    }
}

void CameraCalibWidget::on_buttonExtStartCalibrate_clicked()
{
    if (humanPoseProcessor)
    {
        int res = QMessageBox::information(this, tr("提示"), tr("相机标定时间较长，请耐心等待！"),
                                           QMessageBox::Ok, QMessageBox::Cancel);
        if (res == QMessageBox::Ok)
        {
            setCursor(Qt::WaitCursor);
            HumanPoseParams params = *humanPoseParams;
            bool res = humanPoseProcessor->calibrateCameraExtrinsics(params);
            unsetCursor();
            if (!res)
                QMessageBox::information(this, tr("提示"), tr("标定失败！"));
        }
    }
}

