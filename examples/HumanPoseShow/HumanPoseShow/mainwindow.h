#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include "VideoGroupWidget.h"
#include "VideoItemWidget.h"
#include "LayoutVideoWidget.h"
#include "LayoutPageWidget.h"
#include "CameraCalibWidget.h"

#include "HumanPoseProcessor.h"


QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextEdit;
class QListWidget;
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public HumanPoseProcessorCallback
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void set2dPoseImage(int index, const cv::Mat& image) override;
    void set3dPoseImage(const cv::Mat& image) override;

private slots:
    void newTask();
    void openTask();
    void saveTask();
    void about();
    void configMindCamera();
    void configHikCamera();
    void configInput();
    void configOutput();
    void configAlgorithm();
    void calibrateCamera();
    void startExecute();
    void stopExecute();
    void exeOptionSaveOutput(bool checked);
    void exeOptionAlgorithmNo();
    void exeOptionAlgorithm2d();
    void exeOptionAlgorithm3d();
    void showLayoutVideoWidget();
    void showLayoutPageWidget();
    void layoutVideoGroup(int count);
    void updateExecuteStatus();

private:
    void createActions();
    void createStatusBar();
    void createDockWindows();
    void layoutPageChange();

private:
    LayoutVideoWidget *widgetVideoLayout;    //视频布局
    LayoutPageWidget *widgetVideoPage;       //视频分页
    VideoGroupWidget *widgetVideoGroup;
    VideoItemWidget *widget3dPoseView;
    QListWidget *paragraphsList;
    CameraCalibWidget *widgetCameraCalib;

    QActionGroup* exeOptionAlgoGroup;
    QAction *startExecuteAct;
    QAction *stopExecuteAct;

    QMenu *viewMenu;
    QToolBar* viewToolBar;
    QAction* layoutVideoAct;
    QAction* layoutPageAct;

    QString paramFile;
    HumanPoseParams humanPoseParams;
    std::shared_ptr<HumanPoseProcessor> humanPoseProcessor;
};

#endif // MAINWINDOW_H

