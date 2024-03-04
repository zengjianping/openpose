#include "VideoItemWidget.h"
#include <QPainter>


enum
{
    ST_INIT     = 0x00, // 状态初始值
    ST_SELECTED = 0x01, // 选择状态
    ST_OPENED   = 0x02, // 启动状态
    ST_STARTED  = 0x04, // 播放状态
    ST_ALL      = ST_SELECTED | ST_OPENED | ST_STARTED
};

VideoItemWidget::VideoItemWidget(QWidget *parent)
    : QWidget{parent}
{
    currImage_ = QPixmap(":/images/video_bg.jpg");
    index_ = -1;
    status_ = ST_INIT;
}

void VideoItemWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), currImage_);
}

int VideoItemWidget::getIndex()
{
    return index_;
}

void VideoItemWidget::setIndex(int index)
{
    index_ = index;
}

int VideoItemWidget::getStatus()
{
    return status_;
}

void VideoItemWidget::setStatus(int status)
{
    status_ = status;
}

bool VideoItemWidget::getSelected()
{
    return (status_ & ST_SELECTED) != 0;
}

void VideoItemWidget::setSelected(bool selected)
{
    if(selected)
        status_ |= ST_SELECTED;
    else
        status_ &= ~ST_SELECTED;

}

QPixmap VideoItemWidget::getImage()
{
    return currImage_;
}

void VideoItemWidget::setImage(QPixmap& pixmap)
{
    currImage_ = pixmap;
}

void VideoItemWidget::resetImage()
{
    currImage_ = QPixmap(":/images/video_bg.jpg");
}
