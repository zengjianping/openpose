#include "DialogAlgoConfig.h"
#include "ui_DialogAlgoConfig.h"

DialogAlgoConfig::DialogAlgoConfig(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogAlgoConfig)
{
    ui->setupUi(this);
    initializeUI();
}

DialogAlgoConfig::~DialogAlgoConfig()
{
    delete ui;
}

void DialogAlgoConfig::setParams(const HumanPoseParams::AlgorithmParams& params)
{
    configParams = params;
    loadParams();
}

void DialogAlgoConfig::getParams(HumanPoseParams::AlgorithmParams& params)
{
    params = configParams;
}

void DialogAlgoConfig::initializeUI()
{
    // 模型输入分辨率
    QStringList inputResolutions = {"-1x368", "-1x256", "-1x192"};
    ui->combModelInputResolution->addItems(inputResolutions);

    // 模型输出分辨率
    QStringList outputResolutions = {"-1x-1"};
    ui->combModelOutputResolution->addItems(outputResolutions);

    // 最小3D视角数
    for (int i = 0; i < 3; i++)
        ui->combMin3dViews->addItem(QString::number(i+2));

    // 图像批处理
    ui->checkImageBatchProcess->setChecked(false);

    // 实时处理
    ui->checkRealTimeProcess->setChecked(false);
}

void DialogAlgoConfig::loadParams()
{
    // 模型输入分辨率
    int index = ui->combModelInputResolution->findText(QString::fromStdString(configParams.modelResolution));
    if (index == -1) index = 0;
    ui->combModelInputResolution->setCurrentIndex(index);

    // 模型输出分辨率
    index = ui->combModelOutputResolution->findText(QString::fromStdString(configParams.outputResolution));
    if (index == -1) index = 0;
    ui->combModelOutputResolution->setCurrentIndex(index);

    // 最小3D视角数
    ui->combMin3dViews->setCurrentIndex(configParams.minViews3d-2);

    // 图像批处理
    ui->checkImageBatchProcess->setChecked(configParams.batchProcess);

    // 实时处理
    ui->checkRealTimeProcess->setChecked(configParams.realTimeProcess);
}

void DialogAlgoConfig::updateParams()
{
    // 模型输入分辨率
    configParams.modelResolution = ui->combModelInputResolution->currentText().toStdString();

    // 模型输出分辨率
    configParams.outputResolution = ui->combModelOutputResolution->currentText().toStdString();

    // 最小3D视角数
    configParams.minViews3d = ui->combMin3dViews->currentIndex() + 2;

    // 图像批处理
    configParams.batchProcess = ui->checkImageBatchProcess->isChecked();

    // 实时处理
    configParams.realTimeProcess = ui->checkRealTimeProcess->isChecked();
}

void DialogAlgoConfig::on_buttonBox_accepted()
{
    updateParams();
}


void DialogAlgoConfig::on_buttonBox_rejected()
{
}

