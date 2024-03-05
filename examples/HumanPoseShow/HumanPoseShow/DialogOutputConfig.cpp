#include "DialogOutputConfig.h"
#include "ui_DialogOutputConfig.h"
#include <QFileDialog>

DialogOutputConfig::DialogOutputConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogOutputConfig)
{
    ui->setupUi(this);
    initializeUI();
}

DialogOutputConfig::~DialogOutputConfig()
{
    delete ui;
}

void DialogOutputConfig::setParams(const HumanPoseParams::OutputParams& params)
{
    configParams = params;
    loadParams();
}

void DialogOutputConfig::getParams(HumanPoseParams::OutputParams& params)
{
    params = configParams;
}

void DialogOutputConfig::initializeUI()
{
    ui->editImageDir->setEnabled(false);
    ui->editVideoPath->setEnabled(false);
    ui->edit3dVideoPath->setEnabled(false);
}

void DialogOutputConfig::loadParams()
{
    ui->editImageDir->setText(QString::fromStdString(configParams.imageSavePath));
    ui->editVideoPath->setText(QString::fromStdString(configParams.videoSavePath));
    ui->edit3dVideoPath->setText(QString::fromStdString(configParams.video3dSavePath));
    ui->checkSaveImage->setChecked(configParams.saveImage);
    ui->checkSaveVideo->setChecked(configParams.saveVideo);
    ui->checkSaveVideo3d->setChecked(configParams.saveVideo3d);
}

void DialogOutputConfig::updateParams()
{
    configParams.imageSavePath = ui->editImageDir->text().toStdString();
    configParams.videoSavePath = ui->editVideoPath->text().toStdString();
    configParams.video3dSavePath = ui->edit3dVideoPath->text().toStdString();
    configParams.saveImage = ui->checkSaveImage->isChecked();
    configParams.saveVideo = ui->checkSaveVideo->isChecked();
    configParams.saveVideo3d = ui->checkSaveVideo3d->isChecked();
}

void DialogOutputConfig::on_buttonImageDir_clicked()
{
    QString imageDir = QFileDialog::getExistingDirectory(this, tr("选择目录"), ".");
    if (!imageDir.isEmpty())
        ui->editImageDir->setText(imageDir);
}

void DialogOutputConfig::on_buttonVideoPath_clicked()
{
    QString videoPath = QFileDialog::getSaveFileName(this, tr("选择文件"), ".", "Video Files (*.mp4)");
    if (!videoPath.isEmpty())
        ui->editVideoPath->setText(videoPath);
}

void DialogOutputConfig::on_button3dVideoPath_clicked()
{
    QString videoPath = QFileDialog::getSaveFileName(this, tr("选择文件"), ".", "Video Files (*.mp4)");
    if (!videoPath.isEmpty())
        ui->edit3dVideoPath->setText(videoPath);
}

void DialogOutputConfig::on_buttonBox_accepted()
{
    updateParams();
}

