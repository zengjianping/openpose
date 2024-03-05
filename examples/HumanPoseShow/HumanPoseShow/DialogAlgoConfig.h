#ifndef DIALOGALGOCONFIG_H
#define DIALOGALGOCONFIG_H

#include <QDialog>
#include "HumanPoseProcessor.h"

namespace Ui {
class DialogAlgoConfig;
}

class DialogAlgoConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAlgoConfig(QWidget *parent = nullptr);
    ~DialogAlgoConfig();

public:
    void setParams(const HumanPoseParams::AlgorithmParams& params);
    void getParams(HumanPoseParams::AlgorithmParams& params);

private:
    void initializeUI();
    void loadParams();
    void updateParams();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::DialogAlgoConfig *ui;
    HumanPoseParams::AlgorithmParams configParams;
};

#endif // DIALOGALGOCONFIG_H
