#ifndef DIALOGINPUTCONFIG_H
#define DIALOGINPUTCONFIG_H

#include <QDialog>
#include "HumanPoseProcessor.h"

namespace Ui {
class DialogInputConfig;
}

class DialogInputConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInputConfig(QWidget *parent = nullptr);
    ~DialogInputConfig();

public:
    void setParams(const HumanPoseParams::InputParams& params);
    void getParams(HumanPoseParams::InputParams& params);

private:
    void initializeUI();
    void loadParams();
    void updateParams();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_comboInputType_currentIndexChanged(int index);
    void on_comboCameraIndex_currentIndexChanged(int index);
    void on_checkAllCamera_stateChanged(int arg1);
    void on_buttonVideoPath_clicked();
    void on_checkMaxVideoChannels_stateChanged(int arg1);
    void on_checkMaxFps_stateChanged(int arg1);
    void on_buttonCameraDirectory_clicked();

private:
    Ui::DialogInputConfig *ui;
    HumanPoseParams::InputParams configParams;
};

#endif // DIALOGINPUTCONFIG_H
