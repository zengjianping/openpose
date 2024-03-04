#ifndef LAYOUT_VIDEO_WIDGET_H_
#define LAYOUT_VIDEO_WIDGET_H_

#include <QWidget>
#include <QPushButton>
#include <QHash>


//视频布局类
class LayoutVideoWidget : public QWidget
{
    Q_OBJECT

public:
    LayoutVideoWidget(QWidget *parent = NULL);
    ~LayoutVideoWidget();

public:
    void setButtonEnabled(bool enabled);

public slots:
    void buttonClick();

signals:
    void signalVideoCount(int count);

protected:
    virtual void paintEvent(QPaintEvent * event);

private:
    void initializeUI();

private:
    QHash<QPushButton*,int> mapButtons;
};

#endif // LAYOUT_VIDEO_WIDGET_H_

