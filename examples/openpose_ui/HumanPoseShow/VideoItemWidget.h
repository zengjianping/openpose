#ifndef VIDEOITEMWIDGET_H
#define VIDEOITEMWIDGET_H

#include <QWidget>

class VideoItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoItemWidget(QWidget *parent = nullptr);

signals:

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
    virtual void paintEvent(QPaintEvent * event);

private:
    QPixmap currImage_;
    int index_; // 索引号
    int status_; // 状态
    bool useBgImage;
};

#endif // VIDEOITEMWIDGET_H
