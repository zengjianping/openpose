#include "mainwindow.h"
#include "CommonUtils.h"
#include "MindCameraConfig.h"
#include "DialogInputConfig.h"
#include "DialogOutputConfig.h"
#include "DialogAlgoConfig.h"
#include "DialogTaskList.h"
#include <QtWidgets>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    humanPoseProcessor = HumanPoseProcessor::createInstance();
    humanPoseProcessor->setCallback(this);

    widgetVideoGroup = new VideoGroupWidget(this);
    setCentralWidget(widgetVideoGroup);

    createActions();
    createStatusBar();
    createDockWindows();

    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
    DELETE_OBJECT(widgetVideoLayout);
    DELETE_OBJECT(widgetVideoPage);
}

bool MainWindow::initialize()
{
    int res = openTaskHelper();
    if (res == 1)
        QMessageBox::information(this, tr("提示"), tr("打开任务失败！"), QMessageBox::Ok);
    return res == 0;
}

void MainWindow::newTask()
{
    int res = newTaskHelper();
    if (res == 0)
        QMessageBox::information(this, tr("提示"), tr("新建任务成功！"), QMessageBox::Ok);
    else if (res == 1)
        QMessageBox::information(this, tr("提示"), tr("新建任务失败！"), QMessageBox::Ok);
}

int MainWindow::newTaskHelper()
{
    DialogTaskList dialog(this, true);
    if (QDialog::Accepted != dialog.exec())
        return 2;

    QString newTaskName, newTaskFile, calibrationDir;
    dialog.getCurrentTaskInfo(newTaskName, newTaskFile, calibrationDir);
    HumanPoseParams params = HumanPoseParams();
    params.inputParams.cameraParamPath = calibrationDir.toStdString();
    if (!params.saveToFile(newTaskFile.toStdString()))
        return 1;
    humanPoseParams = params;

    taskName = newTaskName;
    taskFile = newTaskFile;
    setWindowTitle(tr("3D人体姿态识别 - %1").arg(taskName));

    return 0;
}

void MainWindow::openTask()
{
    int res = openTaskHelper();
    if (res == 0)
        QMessageBox::information(this, tr("提示"), tr("打开任务成功！"), QMessageBox::Ok);
    else if (res == 1)
        QMessageBox::information(this, tr("提示"), tr("打开任务失败！"), QMessageBox::Ok);
}

int MainWindow::openTaskHelper()
{
    //QString fileName = QFileDialog::getOpenFileName(this, tr("选择文件"), ".", "Json Files (*.json)");
    DialogTaskList dialog(this, false);
    if (QDialog::Accepted != dialog.exec())
        return 2;

    QString newTaskName, newTaskFile, calibrationDir;
    dialog.getCurrentTaskInfo(newTaskName, newTaskFile, calibrationDir);
    if (!humanPoseParams.loadFromFile(newTaskFile.toStdString()))
        return 1;

    taskName = newTaskName;
    taskFile = newTaskFile;
    setWindowTitle(tr("3D人体姿态识别 - %1").arg(taskName));

    return 0;
}

void MainWindow::saveTask()
{
    int res = saveTaskHelper();
    if (res == 0)
        QMessageBox::information(this, tr("提示"), tr("保存任务（%1）成功！").arg(taskName), QMessageBox::Ok);
    else if (res == 1)
        QMessageBox::information(this, tr("提示"), tr("保存任务（%1）失败！").arg(taskName), QMessageBox::Ok);
}

int MainWindow::saveTaskHelper()
{
    //QString fileName = QFileDialog::getSaveFileName(this, tr("选择文件"), ".", "Json Files (*.json)");
    if (!humanPoseParams.saveToFile(taskFile.toStdString()))
        return 1;
    return 0;
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("关于3D人体姿态识别"),
                       tr("使用多个相机建立人体姿态的三维模型。"));
}

void MainWindow::configMindCamera()
{
    MindCameraConfig config(this);
    config.exec();
}

void MainWindow::configHikCamera()
{
    // TODO...
}

void MainWindow::configInput()
{
    DialogInputConfig dlg(this);
    dlg.setParams(humanPoseParams.inputParams);
    if (QDialog::Accepted == dlg.exec())
    {
        dlg.getParams(humanPoseParams.inputParams);
    }
}

void MainWindow::configOutput()
{
    DialogOutputConfig dlg(this);
    dlg.setParams(humanPoseParams.outputParams);
    if (QDialog::Accepted == dlg.exec())
    {
        dlg.getParams(humanPoseParams.outputParams);
    }
}

void MainWindow::configAlgorithm()
{
    DialogAlgoConfig dlg(this);
    dlg.setParams(humanPoseParams.algorithmParams);
    if (QDialog::Accepted == dlg.exec())
    {
        dlg.getParams(humanPoseParams.algorithmParams);
    }
}

void MainWindow::calibrateCamera()
{
}

void MainWindow::startExecute()
{
    setCursor(Qt::WaitCursor);

    if (!humanPoseProcessor->isRunning())
        humanPoseProcessor->start(humanPoseParams);
    updateExecuteStatus();

    unsetCursor();

    if (!humanPoseProcessor->isRunning())
        QMessageBox::warning(this, tr("提示"), tr("任务启动失败！"), QMessageBox::Ok);
}

void MainWindow::stopExecute()
{
    setCursor(Qt::WaitCursor);

    if (humanPoseProcessor->isRunning())
        humanPoseProcessor->stop();    
    updateExecuteStatus();

    unsetCursor();
}

void MainWindow::updateExecuteStatus()
{
    if (humanPoseProcessor->isRunning())
    {
        startExecuteAct->setEnabled(false);
        stopExecuteAct->setEnabled(true);
    }
    else
    {
        startExecuteAct->setEnabled(true);
        stopExecuteAct->setEnabled(false);
        widgetVideoGroup->resetAllImage();
        //widget3dPoseView->resetImage();
        widgetPoseRender->clearKeypoints();
    }
}

void MainWindow::exeOptionSaveOutput(bool checked)
{
    // TODO...
}

void MainWindow::exeOptionAlgorithmNo()
{
    humanPoseParams.algorithmParams.algoType = HumanPoseParams::AlgorithmParams::AlgoTypeNo;
}

void MainWindow::exeOptionAlgorithm2d()
{
    humanPoseParams.algorithmParams.algoType = HumanPoseParams::AlgorithmParams::AlgoType2D;
}

void MainWindow::exeOptionAlgorithm3d()
{
    humanPoseParams.algorithmParams.algoType = HumanPoseParams::AlgorithmParams::AlgoType3D;
}

void MainWindow::set2dPoseImage(int index, const cv::Mat& image)
{
    //printf("2D pose image: %d\n", index);
    QImage qimage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimage.rgbSwapped());// 将 OpenCV 的 BGR 格式转换为 QImage 的 RGB 格式
    widgetVideoGroup->setImage(index, pixmap);
}

void MainWindow::set3dPoseImage(const cv::Mat& image)
{
    //printf("3D pose image\n");
    QImage qimage((uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimage.rgbSwapped());// 将 OpenCV 的 BGR 格式转换为 QImage 的 RGB 格式
    //widget3dPoseView->setImage(pixmap);
}

void MainWindow::setKeypoints(const op::Array<float>& poseKeypoints3D, const op::Array<float>& faceKeypoints3D,
    const op::Array<float>& leftHandKeypoints3D, const op::Array<float>& rightHandKeypoints3D)
{
    widgetPoseRender->setKeypoints(poseKeypoints3D, faceKeypoints3D, leftHandKeypoints3D, rightHandKeypoints3D);
}

void MainWindow::showLayoutVideoWidget()
{
    QPoint point = viewToolBar->pos();
    //point.setX(point.x());
    point.setY(point.y() + viewToolBar->height());
    widgetVideoLayout->move(mapToGlobal(point));
    widgetVideoLayout->show();
}

void MainWindow::showLayoutPageWidget()
{
    QPoint point = viewToolBar->pos();
    //point.setX(point.x());
    point.setY(point.y() + viewToolBar->height());
    widgetVideoPage->move(mapToGlobal(point));
    layoutPageChange();
    widgetVideoPage->show();
}

void MainWindow::layoutVideoGroup(int count)
{
    // 获取信号来源,重新分割或者是分页
    QObject *obj = sender();
    if (widgetVideoLayout == obj)
        widgetVideoGroup->layoutVideos(count);
    else
        widgetVideoGroup->layoutVideos(-1, count);
}

void MainWindow::layoutPageChange()
{
    int layouVideoCount = INVALID_VALUE;
    int layouVideoIndex = INVALID_VALUE;
    int totalVideoCount = INVALID_VALUE;

    // 获取当前选中项与当前视频框个数
    widgetVideoGroup->getLayoutInfo(layouVideoCount, layouVideoIndex, totalVideoCount);
    if (INVALID_VALUE == layouVideoCount || INVALID_VALUE == layouVideoIndex)
        return;

    int totalPages = totalVideoCount / layouVideoCount;
    if ( totalVideoCount % layouVideoCount != 0 )
        totalPages++;

    // 设置当前总共分页和当前是第几分页
    widgetVideoPage->setPageCount(layouVideoIndex/layouVideoCount, totalPages);
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("文件"));
    QToolBar *fileToolBar = addToolBar(tr("文件"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newTaskAct = new QAction(newIcon, tr("新建任务"), this);
    newTaskAct->setShortcuts(QKeySequence::New);
    newTaskAct->setStatusTip(tr("新建识别任务"));
    connect(newTaskAct, &QAction::triggered, this, &MainWindow::newTask);
    fileMenu->addAction(newTaskAct);
    fileToolBar->addAction(newTaskAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openTaskAct = new QAction(openIcon, tr("打开任务"), this);
    openTaskAct->setShortcuts(QKeySequence::New);
    openTaskAct->setStatusTip(tr("打开识别任务"));
    connect(openTaskAct, &QAction::triggered, this, &MainWindow::openTask);
    fileMenu->addAction(openTaskAct);
    fileToolBar->addAction(openTaskAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveTaskAct = new QAction(saveIcon, tr("保存任务..."), this);
    saveTaskAct->setShortcuts(QKeySequence::Save);
    saveTaskAct->setStatusTip(tr("保存识别任务"));
    connect(saveTaskAct, &QAction::triggered, this, &MainWindow::saveTask);
    fileMenu->addAction(saveTaskAct);
    fileToolBar->addAction(saveTaskAct);

    fileMenu->addSeparator();

    QAction *quitAct = fileMenu->addAction(tr("退出"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("退出应用程序"));

    QMenu *editMenu = menuBar()->addMenu(tr("编辑"));
    //QToolBar *editToolBar = addToolBar(tr("编辑"));

    QAction *configMindCameraAct = new QAction(tr("迈德威视相机..."), this);
    connect(configMindCameraAct, &QAction::triggered, this, &MainWindow::configMindCamera);
    QAction *configHikCameraAct = new QAction(tr("海康威视相机..."), this);
    connect(configHikCameraAct, &QAction::triggered, this, &MainWindow::configHikCamera);
    QMenu* configCameraMenu = editMenu->addMenu(tr("配置相机"));
    configCameraMenu->addAction(configMindCameraAct);
    configCameraMenu->addAction(configHikCameraAct);

    editMenu->addSeparator();

    QAction *configInputAct = new QAction(tr("配置输入..."), this);
    configInputAct->setStatusTip(tr("配置数据输入参数"));
    connect(configInputAct, &QAction::triggered, this, &MainWindow::configInput);
    editMenu->addAction(configInputAct);

    QAction *configOutputAct = new QAction(tr("配置输出..."), this);
    configOutputAct->setStatusTip(tr("配置数据输出参数"));
    connect(configOutputAct, &QAction::triggered, this, &MainWindow::configOutput);
    editMenu->addAction(configOutputAct);

    QAction *configAlgorithmAct = new QAction(tr("配置算法..."), this);
    configAlgorithmAct->setStatusTip(tr("配置算法处理参数"));
    connect(configAlgorithmAct, &QAction::triggered, this, &MainWindow::configAlgorithm);
    editMenu->addAction(configAlgorithmAct);

    QMenu *operMenu = menuBar()->addMenu(tr("操作"));

    //QAction *calibrateCameraAct = new QAction(tr("标定相机..."), this);
    //calibrateCameraAct->setStatusTip(tr("标定相机内参和外参"));
    //connect(calibrateCameraAct, &QAction::triggered, this, &MainWindow::calibrateCamera);
    //operMenu->addAction(calibrateCameraAct);
    //operMenu->addSeparator();

    startExecuteAct = new QAction(tr("启动运行"), this);
    startExecuteAct->setStatusTip(tr("启动任务运行"));
    connect(startExecuteAct, &QAction::triggered, this, &MainWindow::startExecute);
    operMenu->addAction(startExecuteAct);

    stopExecuteAct = new QAction(tr("停止运行"), this);
    stopExecuteAct->setStatusTip(tr("停止任务运行"));
    stopExecuteAct->setEnabled(false);
    connect(stopExecuteAct, &QAction::triggered, this, &MainWindow::stopExecute);
    operMenu->addAction(stopExecuteAct);

    QMenu* executeOptionMenu = operMenu->addMenu(tr("运行选项"));

    //QAction* optionSaveOutputAct = new QAction(tr("保存输出数据"), this);
    //optionSaveOutputAct->setCheckable(true);
    //connect(optionSaveOutputAct, &QAction::toggled, this, &MainWindow::exeOptionSaveOutput);
    //executeOptionMenu->addAction(optionSaveOutputAct);

    //executeOptionMenu->addSeparator();

    QAction* optionAlgorithmNo = new QAction(tr("不启用算法"), this);
    optionAlgorithmNo->setCheckable(true);
    connect(optionAlgorithmNo, &QAction::triggered, this, &MainWindow::exeOptionAlgorithmNo);
    executeOptionMenu->addAction(optionAlgorithmNo);

    QAction* optionAlgorithm2d = new QAction(tr("2D姿态识别"), this);
    optionAlgorithm2d->setCheckable(true);
    connect(optionAlgorithm2d, &QAction::triggered, this, &MainWindow::exeOptionAlgorithm2d);
    executeOptionMenu->addAction(optionAlgorithm2d);

    QAction* optionAlgorithm3d = new QAction(tr("3D姿态识别"), this);
    optionAlgorithm3d->setCheckable(true);
    connect(optionAlgorithm3d, &QAction::triggered, this, &MainWindow::exeOptionAlgorithm3d);
    executeOptionMenu->addAction(optionAlgorithm3d);

    exeOptionAlgoGroup = new QActionGroup(this);
    exeOptionAlgoGroup->addAction(optionAlgorithmNo);
    exeOptionAlgoGroup->addAction(optionAlgorithm2d);
    exeOptionAlgoGroup->addAction(optionAlgorithm3d);
    optionAlgorithmNo->setChecked(true);

    viewMenu = menuBar()->addMenu(tr("视图"));
    viewToolBar = addToolBar(tr("视图"));

    layoutVideoAct = new QAction(QIcon(":/images/VideoAreaCutEntrance.png"), tr("分隔"), this);
    connect(layoutVideoAct, &QAction::triggered, this, &MainWindow::showLayoutVideoWidget);
    viewToolBar->addAction(layoutVideoAct);

    layoutPageAct = new QAction(QIcon(":/images/VideoAreaCutEntrance.png"), tr("分屏"), this);
    connect(layoutPageAct, &QAction::triggered, this, &MainWindow::showLayoutPageWidget);
    viewToolBar->addAction(layoutPageAct);

    menuBar()->addSeparator();
    QMenu *helpMenu = menuBar()->addMenu(tr("帮助"));

    QAction *aboutAct = helpMenu->addAction(tr("关于..."), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("显示应用程序信息"));

    QAction *aboutQtAct = helpMenu->addAction(tr("关于Qt..."), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("显示Qt库信息"));

    widgetVideoLayout = new LayoutVideoWidget(this);
    connect(widgetVideoLayout, SIGNAL(signalVideoCount(int)), this, SLOT(layoutVideoGroup(int)));
    widgetVideoLayout->hide();

    widgetVideoPage = new LayoutPageWidget(this);
    widgetVideoPage->setSlotObject(this);
    widgetVideoPage->hide();
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock;

    dock = new QDockWidget(tr("相机标定"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    widgetCameraCalib = new CameraCalibWidget(dock);
    widgetCameraCalib->setHumanPoseProcessor(humanPoseProcessor.get(), &humanPoseParams);
    connect(widgetCameraCalib, &CameraCalibWidget::processStatusChanged, this, &MainWindow::updateExecuteStatus);
    dock->setWidget(widgetCameraCalib);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("3D姿态视图"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //widget3dPoseView = new VideoItemWidget(dock);
    //dock->setWidget(widget3dPoseView);
    widgetPoseRender = new PoseRenderWidget(dock);
    dock->setWidget(widgetPoseRender);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("3D姿态数据"), this);
    paragraphsList = new QListWidget(dock);
    paragraphsList->addItems(QStringList()
                             << "TODO");
    dock->setWidget(paragraphsList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
}
