#ifndef VIDEOITEMWIDGET_H
#define VIDEOITEMWIDGET_H

#include <QWidget>

class VideoItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoItemWidget(QWidget *parent = nullptr);

signals:
    void signalUpdate();
    // 通知视频管理类，视频被选择
    void signalClicked(int index);
    // 视频双击
    void signalDoubleClicked();

public:
    void slotUpdate();

public:
    int getIndex();
    void setIndex(int index);
    int getStatus();
    void setStatus(int status);
    bool getSelected();
    void setSelected(bool selected);

    QPixmap getImage();
    void setImage(QPixmap& pixmap);
    void resetImage();

protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);

private:
    QPixmap currImage_;
    int index_; // 索引号
    int status_; // 状态
    bool useBgImage;
};

#endif // VIDEOITEMWIDGET_H
