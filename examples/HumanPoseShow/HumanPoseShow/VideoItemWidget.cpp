#include "VideoItemWidget.h"
#include <QPainter>
#include <QStyleOption>


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
    useBgImage = true;
    index_ = -1;
    status_ = ST_INIT;
    connect(this, &VideoItemWidget::signalUpdate, this, &VideoItemWidget::slotUpdate);
}

void VideoItemWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    QPen pen(QColor(160,160,160));
    //pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(rect());

    QRect rc = rect();

    if (!useBgImage)
    {
        if (rc.width() * currImage_.height() > rc.height() * currImage_.width())
        {
            int width0 = rc.width();
            int width = int(rc.height() * 1. / currImage_.height() * currImage_.width()+ 0.5);
            rc.setLeft(rc.left() + (width0 - width) / 2);
            rc.setRight(rc.right() - (width0 - width) / 2);
        }
        else
        {
            int height0 = rc.height();
            int height = int(rc.width() * 1. / currImage_.width() * currImage_.height()+ 0.5);
            rc.setTop(rc.top() + (height0 - height) / 2);
            rc.setBottom(rc.bottom() - (height0 - height) / 2);
        }
    }
    painter.drawPixmap(rc, currImage_);

    //设置样式必需
    QStyleOption option;
    option.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

void VideoItemWidget::mousePressEvent(QMouseEvent* event)
{
    emit signalClicked(index_);
}

void VideoItemWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit signalDoubleClicked();
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
    useBgImage = false;
    emit signalUpdate();
}

void VideoItemWidget::resetImage()
{
    currImage_ = QPixmap(":/images/video_bg.jpg");
    useBgImage = true;
    emit signalUpdate();
}

void VideoItemWidget::slotUpdate()
{
    update();
}

