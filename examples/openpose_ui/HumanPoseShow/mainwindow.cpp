#include <QtWidgets>
#include "mainwindow.h"
#include "MindCameraConfig.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    widgetVideoGroup = new VideoGroupWidget(this);
    setCentralWidget(widgetVideoGroup);

    createActions();
    createStatusBar();
    createDockWindows();

    setWindowTitle(tr("3D人体姿态识别"));

    newTask();
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::newTask()
{
}

void MainWindow::saveTask()
{
    QMimeDatabase mimeDatabase;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Choose a file name"), ".",
                                                    mimeDatabase.mimeTypeForName("text/html").filterString());
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Dock Widgets"),
                             tr("Cannot write file %1:\n%2.")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream out(&file);
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    //out << textEdit->toHtml();
    QGuiApplication::restoreOverrideCursor();

    statusBar()->showMessage(tr("Saved '%1'").arg(fileName), 2000);
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
    // TODO...
}

void MainWindow::configOutput()
{
    // TODO...
}

void MainWindow::configAlgorithm()
{
    // TODO...
}

void MainWindow::calibrateCamera()
{
    // TODO...
}

void MainWindow::startExecute()
{
    setCursor(Qt::WaitCursor);

    if (!humanPoseProcessor.get())
        humanPoseProcessor = HumanPoseProcessor::createInstance(humanPoseParams);
    if (!humanPoseProcessor->isRunning())
    {
        humanPoseProcessor->setCallback(this);
        humanPoseProcessor->start();
    }
    if (!humanPoseProcessor->isRunning())
        humanPoseProcessor.reset();
    bool sucDone = humanPoseProcessor.get() != nullptr;
    startExecuteAct->setEnabled(!sucDone);
    stopExecuteAct->setEnabled(sucDone);

    unsetCursor();
}

void MainWindow::stopExecute()
{
    setCursor(Qt::WaitCursor);

    if (humanPoseProcessor->isRunning())
        humanPoseProcessor->stop();
    humanPoseProcessor.reset();
    startExecuteAct->setEnabled(true);
    stopExecuteAct->setEnabled(false);

    widgetVideoGroup->resetAllImage();
    widget3dPoseView->resetImage();

    unsetCursor();
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
    widget3dPoseView->setImage(pixmap);
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

    QAction *calibrateCameraAct = new QAction(tr("标定相机..."), this);
    calibrateCameraAct->setStatusTip(tr("标定相机内参和外参"));
    connect(calibrateCameraAct, &QAction::triggered, this, &MainWindow::calibrateCamera);
    operMenu->addAction(calibrateCameraAct);

    operMenu->addSeparator();

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

    QAction* optionSaveOutputAct = new QAction(tr("保存输出数据"), this);
    optionSaveOutputAct->setCheckable(true);
    connect(optionSaveOutputAct, &QAction::toggled, this, &MainWindow::exeOptionSaveOutput);
    executeOptionMenu->addAction(optionSaveOutputAct);

    executeOptionMenu->addSeparator();

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

    menuBar()->addSeparator();
    QMenu *helpMenu = menuBar()->addMenu(tr("帮助"));

    QAction *aboutAct = helpMenu->addAction(tr("关于..."), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("显示应用程序信息"));

    QAction *aboutQtAct = helpMenu->addAction(tr("关于Qt..."), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("显示Qt库信息"));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("立体姿态视图"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    widget3dPoseView = new VideoItemWidget(dock);
    dock->setWidget(widget3dPoseView);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());

    dock = new QDockWidget(tr("Paragraphs"), this);
    paragraphsList = new QListWidget(dock);
    paragraphsList->addItems(QStringList()
                             << "Thank you for your payment which we have received today."
                             << "Your order has been dispatched and should be with you "
                                "within 28 days."
                             << "We have dispatched those items that were in stock. The "
                                "rest of your order will be dispatched once all the "
                                "remaining items have arrived at our warehouse. No "
                                "additional shipping charges will be made."
                             << "You made a small overpayment (less than $5) which we "
                                "will keep on account for you, or return at your request."
                             << "You made a small underpayment (less than $1), but we have "
                                "sent your order anyway. We'll add this underpayment to "
                                "your next bill."
                             << "Unfortunately you did not send enough money. Please remit "
                                "an additional $. Your order will be dispatched as soon as "
                                "the complete amount has been received."
                             << "You made an overpayment (more than $5). Do you wish to "
                                "buy more items, or should we return the excess to you?");
    dock->setWidget(paragraphsList);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    viewMenu->addAction(dock->toggleViewAction());
}
