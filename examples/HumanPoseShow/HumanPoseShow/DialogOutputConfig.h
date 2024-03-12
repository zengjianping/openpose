#ifndef DIALOGOUTPUTCONFIG_H
#define DIALOGOUTPUTCONFIG_H

#include <QDialog>
#include "HumanPoseProcessor.h"

namespace Ui {
class DialogOutputConfig;
}

class DialogOutputConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogOutputConfig(QWidget *parent = nullptr);
    ~DialogOutputConfig();

public:
    void setParams(const HumanPoseParams::OutputParams& params);
    void getParams(HumanPoseParams::OutputParams& params);

private:
    void initializeUI();
    void loadParams();
    void updateParams();

private slots:
    void on_buttonImageDir_clicked();
    void on_buttonVideoPath_clicked();
    void on_button3dVideoPath_clicked();
    void on_buttonPosePath_clicked();
    void on_buttonBox_accepted();


private:
    Ui::DialogOutputConfig *ui;
    HumanPoseParams::OutputParams configParams;
};

#endif // DIALOGOUTPUTCONFIG_H
