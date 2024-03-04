#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include "VideoGroupWidget.h"
#include "VideoItemWidget.h"

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

private:
    void createActions();
    void createStatusBar();
    void createDockWindows();

private:
    VideoGroupWidget *widgetVideoGroup;
    VideoItemWidget *widget3dPoseView;
    QListWidget *paragraphsList;

    QMenu *viewMenu;
    QActionGroup* exeOptionAlgoGroup;
    QAction *startExecuteAct;
    QAction *stopExecuteAct;

    HumanPoseParams humanPoseParams;
    std::shared_ptr<HumanPoseProcessor> humanPoseProcessor;
};

#endif // MAINWINDOW_H

