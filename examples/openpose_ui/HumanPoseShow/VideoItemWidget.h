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

protected:
    //布局管理
    void LayoutRest(/*int nGroup*/);
    void Layout1(int nGroup);
    void Layout4(int nGroup);
    void Layout8(int nGroup);
    void Layout9(int nGroup);
    void Layout13(int nGroup);
    void Layout16(int nGroup);
    void Layout25(int nGroup);
    void Layout32(int nGroup);
    void Layout64(int nGroup);

private:
    QPixmap currImage_;
    int index_; // 索引号
    int status_; // 状态
};

#endif // VIDEOITEMWIDGET_H
