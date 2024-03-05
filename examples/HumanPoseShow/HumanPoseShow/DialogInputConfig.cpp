#include "DialogInputConfig.h"
#include "ui_DialogInputConfig.h"
#include <QFileDialog>


DialogInputConfig::DialogInputConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogInputConfig)
{
    ui->setupUi(this);
    initializeUI();
}

DialogInputConfig::~DialogInputConfig()
{
    delete ui;
}

void DialogInputConfig::setParams(const HumanPoseParams::InputParams& params)
{
    configParams = params;
    loadParams();
}

void DialogInputConfig::getParams(HumanPoseParams::InputParams& params)
{
    params = configParams;
}

void DialogInputConfig::initializeUI()
{
    // 输入类型
    QStringList inputTypes = {"视频文件", "视频目录", "网络相机", "迈德威视相机", "海康威视相机"};
    ui->comboInputType->addItems(inputTypes);

    // 相机序号
    for (int i = 0; i < 16; i++)
        ui->comboCameraIndex->addItem(QString::number(i));
    ui->checkAllCamera->setChecked(true);

    // 相机分辨率
    QStringList cameraResolutions = {"-1x-1", "1224x1024"};
    ui->combCameraResolution->addItems(cameraResolutions);

    // 相机帧率
    ui->editCameraFps->setText(QString::number(10));
    ui->checkMaxFps->setChecked(true);

    // 相机触发模式
    QStringList triggerModes = {"连续采集", "软件触发", "硬件触发"};
    ui->combTriggerMode->addItems(triggerModes);

    // 视频路径
    ui->editVideoPath->setText("");
    ui->editVideoPath->setEnabled(false);

    // 视频通道数
    for (int i = 0; i < 4; i++)
        ui->combVideoChannels->addItem(QString::number(i+1));
    ui->checkMaxVideoChannels->setChecked(true);

    // 相机标定目录
    ui->editCameraDirectory->setText("");
    ui->editCameraDirectory->setEnabled(false);

    // 畸变校正
    ui->checkUndistortCamera->setChecked(false);
}

void DialogInputConfig::loadParams()
{
    // 输入类型
    ui->comboInputType->setCurrentIndex((int)configParams.inputType);

    // 相机序号
    if (configParams.cameraIndex >= 0)
        ui->comboCameraIndex->setCurrentIndex(configParams.cameraIndex);
    ui->checkAllCamera->setChecked(configParams.cameraIndex < 0);

    // 相机分辨率
    int index = ui->combCameraResolution->findText(QString::fromStdString(configParams.cameraResolution));
    if (index == -1) index = 0;
    ui->combCameraResolution->setCurrentIndex(index);

    // 相机帧率
    if (configParams.captureFps > 0)
        ui->editCameraFps->setText(QString::number(configParams.captureFps));
    ui->checkMaxFps->setChecked(configParams.captureFps <= 0);

    // 相机触发模式
    ui->combTriggerMode->setCurrentIndex(configParams.cameraTriggerMode);

    // 视频路径
    ui->editVideoPath->setText(QString::fromStdString(configParams.videoPath));

    // 视频通道数
    if (configParams.viewNumber > 0)
        ui->combVideoChannels->setCurrentIndex(configParams.viewNumber-1);
    ui->checkMaxVideoChannels->setChecked(configParams.viewNumber <= 0);

    // 相机标定目录
    ui->editCameraDirectory->setText(QString::fromStdString(configParams.cameraParamPath));

    // 畸变校正
    ui->checkUndistortCamera->setChecked(configParams.frameUndistort);
}

void DialogInputConfig::updateParams()
{
    // 输入类型
    configParams.inputType = (HumanPoseParams::InputParams::InputType)ui->comboInputType->currentIndex();

    // 相机序号
    if (ui->checkAllCamera->isChecked())
        configParams.cameraIndex = -1;
    else
        configParams.cameraIndex = ui->comboCameraIndex->currentIndex();

    // 相机分辨率
    configParams.cameraResolution = ui->combCameraResolution->currentText().toStdString();

    // 相机帧率
    if (ui->checkMaxFps->isChecked())
        configParams.captureFps = -1.;
    else
        configParams.captureFps = ui->editCameraFps->text().toDouble();

    // 相机触发模式
    configParams.cameraTriggerMode = ui->combTriggerMode->currentIndex();

    // 视频路径
    configParams.videoPath = ui->editVideoPath->text().toStdString();

    // 视频通道数
    if (ui->checkMaxVideoChannels->isChecked())
        configParams.viewNumber = -1;
    else
        configParams.viewNumber = ui->combVideoChannels->currentIndex() + 1;

    // 相机标定目录
    configParams.cameraParamPath = ui->editCameraDirectory->text().toStdString();

    // 畸变校正
    configParams.frameUndistort = ui->checkUndistortCamera->isChecked();
}

void DialogInputConfig::on_buttonBox_accepted()
{
    updateParams();
}

void DialogInputConfig::on_buttonBox_rejected()
{
}

void DialogInputConfig::on_comboInputType_currentIndexChanged(int index)
{
    auto inputType = (HumanPoseParams::InputParams::InputType)index;
    bool isFile = (inputType == HumanPoseParams::InputParams::VideoFile
        || inputType == HumanPoseParams::InputParams::VideoDirectory);
    ui->buttonVideoPath->setEnabled(isFile);
    ui->combVideoChannels->setEnabled(isFile);
}

void DialogInputConfig::on_comboCameraIndex_currentIndexChanged(int index)
{
}

void DialogInputConfig::on_checkAllCamera_stateChanged(int arg1)
{
    ui->comboCameraIndex->setEnabled(arg1==0);
}

void DialogInputConfig::on_checkMaxFps_stateChanged(int arg1)
{
    ui->editCameraFps->setEnabled(arg1==0);
}

void DialogInputConfig::on_checkMaxVideoChannels_stateChanged(int arg1)
{
    ui->combVideoChannels->setEnabled(arg1==0);
}

void DialogInputConfig::on_buttonVideoPath_clicked()
{
    QString videoPath;

    int index = ui->comboInputType->currentIndex();
    auto inputType = (HumanPoseParams::InputParams::InputType)index;

    if (inputType == HumanPoseParams::InputParams::VideoFile)
        videoPath = QFileDialog::getOpenFileName(this, tr("选择文件"), ".", "Video Files (*.mp4)");
    else
        videoPath = QFileDialog::getExistingDirectory(this, tr("选择目录"), ".");

    if (!videoPath.isEmpty())
        ui->editVideoPath->setText(videoPath);
}

void DialogInputConfig::on_buttonCameraDirectory_clicked()
{
    QString cameraDir = QFileDialog::getExistingDirectory(this, tr("选择目录"), ".");
    if (!cameraDir.isEmpty())
        ui->editCameraDirectory->setText(cameraDir);
}

