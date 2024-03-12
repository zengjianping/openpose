#include "DialogTaskList.h"
#include "ui_DialogTaskList.h"
#include <QDir>
#include <QPushButton>
#include <QMessageBox>


DialogTaskList::DialogTaskList(QWidget *parent, bool _newTaskMode)
    : QDialog(parent)
    , ui(new Ui::DialogTaskList)
{
    newTaskMode = _newTaskMode;
    ui->setupUi(this);
    initialzieUI();
}

DialogTaskList::~DialogTaskList()
{
    delete ui;
}

QString DialogTaskList::getTaskDirectory()
{
    //return QDir::currentPath() + "/datas/pose_tasks";
    return "datas/pose_tasks";
}

void DialogTaskList::getCurrentTaskInfo(QString& taskName, QString& taskFile, QString& taskDir)
{
    QString workPath = getTaskDirectory();
    if (newTaskMode)
    {
        taskName = ui->editTaskName->text();
        QDir(workPath).mkdir(taskName);
        taskFile = workPath + "/" + taskName + "/config.json";
    }
    else
    {
        currSelIndex = ui->listTaskNames->currentRow();
        taskName = taskNames[currSelIndex];
        taskFile = taskFiles[currSelIndex];
    }
    taskDir = workPath + "/" + taskName;
}

void DialogTaskList::initialzieUI()
{
    setWindowTitle(tr("任务设置"));

    queryTaskInfos();
    ui->listTaskNames->clear();
    ui->listTaskNames->addItems(taskNames);
    ui->listTaskNames->setCurrentRow(0);

    if (newTaskMode)
    {
        ui->buttonTaskDelete->setEnabled(false);
        ui->listTaskNames->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else
    {
        ui->labelTaskName->hide();
        ui->editTaskName->hide();
    }
}

void DialogTaskList::queryTaskInfos()
{
    QString workPath = getTaskDirectory();
    QDir workDir(workPath);
    workDir.makeAbsolute();
    QStringList subDirNames = workDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name);

    taskNames.clear();
    taskFiles.clear();
    for (const QString subDirName : subDirNames)
    {
        QFile taskFile(workPath + "/" + subDirName + "/config.json");
        if (taskFile.exists())
        {
            taskNames.push_back(subDirName);
            taskFiles.push_back(taskFile.fileName());
        }
    }
}

void DialogTaskList::on_buttonBox_accepted()
{
}

void DialogTaskList::on_buttonBox_rejected()
{
}

void DialogTaskList::on_listTaskNames_itemSelectionChanged()
{
    currSelIndex = ui->listTaskNames->currentRow();
    if (currSelIndex >= 0 && currSelIndex < taskNames.size())
    {
        QString selTaskName = taskNames[currSelIndex];
        static QStringList reservedTaskNames = {"default", "VideoFile01",
                            "VideoDir01", "MindCamera01", "HikvCamera01"};
        ui->buttonTaskDelete->setEnabled(!reservedTaskNames.contains(selTaskName));
    }
}


void DialogTaskList::on_editTaskName_textChanged(const QString &text)
{
    QPushButton* buttonOk = ui->buttonBox->button(QDialogButtonBox::Ok);

    if (taskNames.contains(text))
    {
        QMessageBox::information(this, tr("提示"), tr("与已有任务同名！"), QMessageBox::Ok);
        buttonOk->setEnabled(false);
    }
    else if (text.isEmpty())
    {
        buttonOk->setEnabled(false);
    }
    else
    {
        buttonOk->setEnabled(true);
    }
}

void DialogTaskList::on_buttonTaskDelete_clicked()
{
    currSelIndex = ui->listTaskNames->currentRow();
    QString selTaskName = taskNames[currSelIndex];

    if (QMessageBox::question(this, tr("删除任务"), tr("确定要删除此任务吗？")) == QMessageBox::Yes)
    {
        QDir taskDir(getTaskDirectory() + "/" + selTaskName);
        if (taskDir.removeRecursively())
        {
            queryTaskInfos();
            ui->listTaskNames->clear();
            ui->listTaskNames->addItems(taskNames);
            ui->listTaskNames->setCurrentRow(0);
        }
        else
        {
            QMessageBox::information(this, tr("提示"), tr("删除任务失败！"), QMessageBox::Ok);
        }
    }

}

