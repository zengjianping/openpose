#ifndef DIALOGTASKLIST_H
#define DIALOGTASKLIST_H

#include <QDialog>

namespace Ui {
class DialogTaskList;
}

class DialogTaskList : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTaskList(QWidget *parent = nullptr, bool newTaskMode = false);
    ~DialogTaskList();

public:
    void getCurrentTaskInfo(QString& taskName, QString& taskFile, QString& calibrationDir);

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_listTaskNames_itemSelectionChanged();
    void on_editTaskName_textChanged(const QString &arg1);
    void on_buttonTaskDelete_clicked();

private:
    void initialzieUI();
    void queryTaskInfos();
    QString getTaskDirectory();

private:
    Ui::DialogTaskList *ui;
    QStringList taskNames;
    QStringList taskFiles;
    int currSelIndex;
    bool newTaskMode;
};

#endif // DIALOGTASKLIST_H
