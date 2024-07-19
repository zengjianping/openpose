#include "CameraCalibWidget.h"
#include "openpose/headers.hpp"
#include "ui_CameraCalibWidget.h"
#include <QMessageBox>
#include <QDir>


CameraCalibWidget::CameraCalibWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraCalibWidget)
{
    ui->setupUi(this);
    initializeUI();
    updateUI();
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

void CameraCalibWidget::initializeUI()
{
    ui->editChessGridSize->setText(QString::number(30));
    ui->editChessGridLayout->setText("11x8");
}

void CameraCalibWidget::updateUI()
{
    bool enabled = ui->combCameraList->count();
    ui->buttonIntStartCapture->setEnabled(enabled);
    ui->buttonIntStopCapture->setEnabled(enabled);
    ui->buttonIntStartCalibrate->setEnabled(enabled);
    ui->buttonExtStartCapture->setEnabled(enabled);
    ui->buttonExtStopCapture->setEnabled(enabled);
    ui->buttonExtStartCalibrate->setEnabled(enabled);
}

void CameraCalibWidget::on_combCameraList_currentIndexChanged(int index)
{
}

void CameraCalibWidget::on_buttonCameraInfo_clicked()
{
    ui->combCameraList->clear();
    if (humanPoseProcessor)
    {
        HumanPoseParams params = *humanPoseParams;
        std::string cameraType;
        cameraNames.clear();
        if (queryCameraList(params, cameraType, cameraNames))
        {
            for (const std::string& name : cameraNames)
                ui->combCameraList->addItem(QString::fromStdString(name));
            ui->editCameraType->setText(QString::fromStdString(cameraType));
        }
        else
        {
            QMessageBox::information(this, tr("提示"), tr("查询相机失败！"));
        }
    }
    updateUI();
}

std::string CameraCalibWidget::getImageDir(const std::string& calibrateDir, const std::string subDirName)
{
    std::string parentDir = op::getFileParentFolderPath(calibrateDir);
    std::string imgRootDir = parentDir + "calibimages";
    op::makeDirectory(imgRootDir);
    std::string imageDir = op::formatAsDirectory(imgRootDir) + subDirName;
    op::makeDirectory(imageDir);
    return imageDir;
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
        params.inputParams.cameraTriggerMode = 1;
        params.inputParams.frameUndistort = false;
        params.outputParams.saveVideo = false;
        params.outputParams.saveVideo3d = false;
        params.outputParams.saveImage = true;
        params.outputParams.writeImageMode = 1;
        params.outputParams.notUseTime = true;
        std::string cameraName = ui->combCameraList->currentText().toStdString();
        std::string calibPath = params.inputParams.cameraParamPath;
        //QDir calibDir(QString::fromStdString(calibPath));
        //calibDir.mkdir("intrinsics");
        std::string subDirName = "intrinsics/" + cameraName;
        params.outputParams.imageSavePath = getImageDir(calibPath, subDirName);
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
            std::string calibrateDir = params.inputParams.cameraParamPath;
            std::string gridLayout = ui->editChessGridLayout->text().toStdString();
            float gridSize = ui->editChessGridSize->text().toDouble();
            int cameraIndex = ui->combCameraList->currentIndex();
            std::string cameraName = cameraNames[cameraIndex];
            std::string subDirName = "intrinsics/" + cameraName;
            std::string calibImageDir = getImageDir(calibrateDir, subDirName);
            bool res = calibrateCameraIntrinsics(cameraName, calibImageDir, calibrateDir, gridLayout, gridSize);

            unsetCursor();
            if (res)
                QMessageBox::information(this, tr("提示"), tr("标定成功！"));
            else
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
        params.inputParams.cameraTriggerMode = 1;
        params.inputParams.frameUndistort = true;
        params.outputParams.saveVideo = false;
        params.outputParams.saveVideo3d = false;
        params.outputParams.saveImage = true;
        params.outputParams.writeImageMode = 1;
        params.outputParams.notUseTime = true;
        std::string calibPath = params.inputParams.cameraParamPath;
        //QDir calibDir(QString::fromStdString(calibPath));
        bool calibratePose = ui->checkCameraPose->isChecked();
        if (calibratePose)
        {
            //calibDir.mkdir("camerapose");
            //params.outputParams.imageSavePath = calibPath + "/camerapose";
            params.outputParams.imageSavePath = getImageDir(calibPath, "camerapose");
        }
        else
        {
            //calibDir.mkdir("extrinsics");
            //params.outputParams.imageSavePath = calibPath + "/extrinsics";
            params.outputParams.imageSavePath = getImageDir(calibPath, "extrinsics");
        }
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
            std::string calibrateDir = params.inputParams.cameraParamPath;
            std::string gridLayout = ui->editChessGridLayout->text().toStdString();
            float gridSize = ui->editChessGridSize->text().toDouble();
            bool calibratePose = ui->checkCameraPose->isChecked();
            bool res = false;

            if (calibratePose)
            {
                std::string calibImageDir = getImageDir(calibrateDir, "camerapose");
                res = calibrateCameraPose(cameraNames, calibImageDir, calibrateDir, gridLayout, gridSize);
            }
            else
            {
                std::string calibImageDir = getImageDir(calibrateDir, "extrinsics");
                res = calibrateCameraExtrinsics(cameraNames, calibImageDir, calibrateDir, gridLayout, gridSize);
                if (res) {
                    res = refineCameraExtrinsics(cameraNames, calibImageDir, calibrateDir, gridLayout, gridSize);
                }
            }

            unsetCursor();
            if (res)
                QMessageBox::information(this, tr("提示"), tr("标定成功！"));
            else
                QMessageBox::information(this, tr("提示"), tr("标定失败！"));
        }
    }
}

