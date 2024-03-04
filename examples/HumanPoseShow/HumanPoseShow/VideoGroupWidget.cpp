#include "VideoGroupWidget.h"
#include "VideoItemWidget.h"
#include "CommonUtils.h"


#define MAX_VIDEO_COUNT 64 // 当前支持的最大视频数
#define INVALID_VIDEO_COUNT 18 //无效的视频框个数

enum
{
    VIDEO_LAYOUT_RESET = 0,     // 视频重新设置索引号
    VIDEO_LAYOUT_1     = 1,     // 视频布局1
    VIDEO_LAYOUT_4     = 4,     // 视频布局4
    VIDEO_LAYOUT_8     = 8,     // 视频布局8
    VIDEO_LAYOUT_9     = 9,     // 视频布局9
    VIDEO_LAYOUT_13    = 13,    // 视频布局13
    VIDEO_LAYOUT_16    = 16,    // 视频布局16
    VIDEO_LAYOUT_25    = 25,    // 视频布局25
    VIDEO_LAYOUT_32    = 32,    // 视频布局32
    VIDEO_LAYOUT_64    = 64,    // 视频布局64
};

VideoGroupWidget::VideoGroupWidget(QWidget *parent)
    : QWidget{parent}
{
    initializeUI();
}

VideoGroupWidget::~VideoGroupWidget()
{
    terminateUI();
}

void VideoGroupWidget::initializeUI()
{
    videoIds.clear();

    for (int i = 0; i< MAX_VIDEO_COUNT; i++)
    {
        VideoItemWidget *viewItem = new VideoItemWidget(this);
        viewItem->setIndex(i);
        videoItemVec.push_back(viewItem);

        connect(viewItem, SIGNAL(signalClicked(int)), this, SLOT(videoItemClicked(int)));
        connect(viewItem, SIGNAL(signalDoubleClicked()), this, SLOT(videoItemDoubleClicked()));

        if (i == 0)
        {
            viewItem->show();
            viewItem->setSelected(true);
            viewItem->setStyleSheet("QWidget{border: 3px solid #ff0000;}");
        }
        else
        {
            viewItem->hide();
            viewItem->setSelected(false);
            viewItem->setStyleSheet("QWidget{border: 2px solid #adadad;}");
        }
    }

    for (int i = 0; i< INVALID_VIDEO_COUNT; i++)
    {
        VideoItemWidget *viewItem = new VideoItemWidget(this);
        viewItem->setStyleSheet("QWidget{border: 1px solid #adadad;background:#d5d5d5;image:url(:/icon/UnConfig.png);}");
        viewItem->setAcceptDrops(false);
        viewItem->hide();
        reservedVideoItemVec.push_back(viewItem);
    }

    videoLayout = new QGridLayout(this);
    videoLayout->setMargin(8);
    videoLayout->setSpacing(5);
    setLayout(videoLayout);

    currVideoCount = 1;
    currVideoIndex = 0;

    layoutVideos(4);
}

void VideoGroupWidget::terminateUI()
{
    videoIds.clear();

    for (size_t i = 0;i < videoItemVec.size(); i++)
        DELETE_OBJECT(videoItemVec[i]);
    videoItemVec.clear();

    for (size_t i = 0;i < reservedVideoItemVec.size(); i++)
        DELETE_OBJECT(reservedVideoItemVec[i]);
    reservedVideoItemVec.clear();

    DELETE_OBJECT(videoLayout)
}

void VideoGroupWidget::setImage(int index, QPixmap& pixmap)
{
    VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[index]);
    videoItem->setImage(pixmap);
}

void VideoGroupWidget::resetAllImage()
{
    for (size_t i = 0;i < videoItemVec.size(); i++)
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
        videoItem->resetImage();
    }

    for (size_t i = 0;i < reservedVideoItemVec.size(); i++)
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(reservedVideoItemVec[i]);
        videoItem->resetImage();
    }
}

int VideoGroupWidget::getVideoIndex(int index)
{
    for (int i=0; i < videoItemVec.size(); i++)
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
        if (index == videoItem->getIndex())
            return i;
    }
    return INVALID_VALUE;
}

QWidget* VideoGroupWidget::getVideoWidget(int index)
{
    for (int i = 0; i < videoItemVec.size(); i++)
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
        if (index == videoItem->getIndex())
            return videoItem;
    }
    return nullptr;
}

void VideoGroupWidget::getLayoutInfo(int& layoutCount, int& layoutIndex, int& totalCount)
{
    layoutCount = currVideoCount;
    layoutIndex = currVideoIndex;
    totalCount = MAX_VIDEO_COUNT;
}

void VideoGroupWidget::layoutVideos(int count, int group)
{
    if (count == -1 && group == -1)
        return;

    for (int i = 0; i < INVALID_VIDEO_COUNT; i++)
    {
        reservedVideoItemVec[i]->hide();
        videoLayout->removeWidget(reservedVideoItemVec[i]);
    }

    if (count != VIDEO_LAYOUT_RESET)
    {
        // 如果只是重新分组,布局个数为当前个数,否则重新设置当前布局个数
        if ( -1 == count )
            count = currVideoCount;
        else
            currVideoCount = count;
    }

    // 如果当前组无效,则通过当前选择的视频框索引号与当前布局个数计算出当前所在组
    if (-1 == group)
    {
        group = getVideoIndex(currVideoIndex) / currVideoCount;
    }

    //重新布局
    switch(count)
    {
    case VIDEO_LAYOUT_RESET:
        layoutReset();
        break;
    case VIDEO_LAYOUT_1:
        layoutCount1(group);
        break;
    case VIDEO_LAYOUT_4:
        layoutCount4(group);
        break;
    case VIDEO_LAYOUT_8:
        layoutCount8(group);
        break;
    case VIDEO_LAYOUT_9:
        layoutCount9(group);
        break;
    case VIDEO_LAYOUT_13:
        layoutCount13(group);
        break;
    case VIDEO_LAYOUT_16:
        layoutCount16(group);
        break;
    case VIDEO_LAYOUT_25:
        layoutCount25(group);
        break;
    case VIDEO_LAYOUT_32:
        layoutCount32(group);
        break;
    case VIDEO_LAYOUT_64:
        layoutCount64(group);
        break;
    default:
        break;
    }
}

void VideoGroupWidget::layoutReset()
{
    // 获取所有视频框状态
    int states[MAX_VIDEO_COUNT] = {0};
    for (int i = 0; i< MAX_VIDEO_COUNT; i++ )
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(getVideoWidget(i));
        states[i] = videoItem->getStatus();
    }
    // 重置视频框索引和状态，并清空当前视频框图像
    for ( int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
        videoItem->setIndex(i);
        videoItem->setStatus(states[i]);
        videoItem->resetImage();
    }

    // 重新分屏,设置当前选中的视频框
    layoutVideos(currVideoCount, -1);
}

void VideoGroupWidget::layoutCount1(int group)
{
    //将被选择的设置到布局中
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
        if (i == group)
        {
            videoItemVec[i]->show();
            videoLayout->addWidget(videoItemVec[i], 0, 0);
        }
        else
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
    }
}

void VideoGroupWidget::layoutCount4(int group)
{
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if (i < group*VIDEO_LAYOUT_4 || i >= (group+1)*VIDEO_LAYOUT_4)
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            videoLayout->addWidget(videoItemVec[i], (i-group*VIDEO_LAYOUT_4)/2, (i-group*VIDEO_LAYOUT_4)%2);
        }
    }
}

void VideoGroupWidget::layoutCount8(int group)
{
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if (i < group*VIDEO_LAYOUT_8 || i >= (group+1)*VIDEO_LAYOUT_8)
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            if (i-group*VIDEO_LAYOUT_8 == 0)
            {
                videoLayout->addWidget(videoItemVec[i],0,0,3,3);
            }
            else if ( i-group*VIDEO_LAYOUT_8 > 0 && i-group*VIDEO_LAYOUT_8 < 4)
            {
                videoLayout->addWidget(videoItemVec[i],i-group*VIDEO_LAYOUT_8-1,3);
            }
            else
            {
                videoLayout->addWidget(videoItemVec[i],3,i-group*VIDEO_LAYOUT_8-4);
            }
        }
    }
}

void VideoGroupWidget::layoutCount9(int group)
{
    int j = 0;
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if (i < group*VIDEO_LAYOUT_9 || i>=(group+1)*VIDEO_LAYOUT_9)
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_9)/3,(i-group*VIDEO_LAYOUT_9)%3);
        }
    }

    if ( group == MAX_VIDEO_COUNT/VIDEO_LAYOUT_9 )
    {
        int nRemainder = (group+1)*VIDEO_LAYOUT_9 - MAX_VIDEO_COUNT;
        for ( int i=0;i<nRemainder;++i)
        {
            reservedVideoItemVec[i]->show();
            videoLayout->addWidget(reservedVideoItemVec[i],(8-i)/3,(8-i)%3);
        }
    }

}

void VideoGroupWidget::layoutCount13(int group)
{
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if (i < group*VIDEO_LAYOUT_13 || i>=(group+1)*VIDEO_LAYOUT_13 )
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            if ( i-group*VIDEO_LAYOUT_13 == 0 )
            {
                videoLayout->addWidget(videoItemVec[i],1,1,2,2);
            }
            else if ( i-group*VIDEO_LAYOUT_13 > 0 && i-group*VIDEO_LAYOUT_13 < 5 )
            {
                videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_13)/5,i-group*VIDEO_LAYOUT_13-1);
            }
            else if ( i-group*VIDEO_LAYOUT_13 >= 5 && i-group*VIDEO_LAYOUT_13 < 7 )
            {
                videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_13)/5,(i-group*VIDEO_LAYOUT_13)*3-15);
            }
            else if ( i-group*VIDEO_LAYOUT_13 >= 7 && i-group*VIDEO_LAYOUT_13 < 9 )
            {
                videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_13)/3,(i-group*VIDEO_LAYOUT_13)*3-21);
            }
            else
            {
                videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_13+3)/4,i-group*VIDEO_LAYOUT_13-9);
            }
        }
    }
    if ( group == MAX_VIDEO_COUNT/VIDEO_LAYOUT_13 )
    {
        int nRemainder = (group+1)*VIDEO_LAYOUT_13 - MAX_VIDEO_COUNT;
        for ( int i=0;i<nRemainder;++i)
        {
            reservedVideoItemVec[i]->show();
            if ( i < 4 )
            {
                videoLayout->addWidget(reservedVideoItemVec[i],3,3-i);
            }
            else if ( i == 4 || i == 5 )
            {
                videoLayout->addWidget(reservedVideoItemVec[i],2,15-3*i);
            }
            else if ( i == 6 )
            {
                videoLayout->addWidget(reservedVideoItemVec[i],1,3);
            }
        }
    }
}

void VideoGroupWidget::layoutCount16(int group)
{
    int j = 0;
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if ( i < group*VIDEO_LAYOUT_16 || i>=(group+1)*VIDEO_LAYOUT_16)
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_16)/4,(i-group*VIDEO_LAYOUT_16)%4);
        }
    }
}

void VideoGroupWidget::layoutCount25(int group)
{
    int j = 0;
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if (i < group*VIDEO_LAYOUT_25 || i>=(group+1)*VIDEO_LAYOUT_25)
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_25)/5,(i-group*VIDEO_LAYOUT_25)%5);
        }
    }
    if ( group == MAX_VIDEO_COUNT/VIDEO_LAYOUT_25 )
    {
        int nRemainder = (group+1)*VIDEO_LAYOUT_25 - MAX_VIDEO_COUNT;
        for ( int i=0;i<nRemainder;++i)
        {
            reservedVideoItemVec[i]->show();
            videoLayout->addWidget(reservedVideoItemVec[i],(24-i)/5,(24-i)%5);
        }
    }
}

void VideoGroupWidget::layoutCount32(int group)
{
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if (i < group*VIDEO_LAYOUT_32 || i>=(group+1)*VIDEO_LAYOUT_32 )
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            if ( i-group*VIDEO_LAYOUT_32 == 0 )
            {
                videoLayout->addWidget(videoItemVec[i],0,0,2,2);
            }
            else if ( i-group*VIDEO_LAYOUT_32>0 && i-group*VIDEO_LAYOUT_32<9 )
            {
                videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_32-1)/4,(i-group*VIDEO_LAYOUT_32-1)%4+2);
            }
            else
            {
                videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_32+3)/6,(i-group*VIDEO_LAYOUT_32+3)%6);
            }
        }
    }
}

void VideoGroupWidget::layoutCount64(int group)
{
    int j = 0;
    for (int i = 0; i < MAX_VIDEO_COUNT; i++)
    {
        if ( i < group*VIDEO_LAYOUT_64 || i>=(group+1)*VIDEO_LAYOUT_64)
        {
            videoItemVec[i]->hide();
            videoLayout->removeWidget(videoItemVec[i]);
        }
        else
        {
            videoItemVec[i]->show();
            videoLayout->addWidget(videoItemVec[i],(i-group*VIDEO_LAYOUT_64)/8,(i-group*VIDEO_LAYOUT_64)%8);
        }
    }
}

void VideoGroupWidget::videoItemClicked(int index)
{
    if (index == INVALID_VALUE)
    {
        // 视频被点击时，设置为选中状态，并设置边框为红色，没有选择的设置为蓝色，从发送的信号源来判断
        VideoItemWidget *videoSender = qobject_cast<VideoItemWidget*>(sender());
        for (int i = 0; i < videoItemVec.size(); i++)
        {
            VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
            if (videoSender == videoItem)
            {
                videoItem->setStyleSheet("QWidget{border: 3px solid #ff0000;}"); //选中
                videoItem->setSelected(true);
                index = videoItem->getIndex();
            }
            else
            {
                videoItem->setStyleSheet("QWidget{border: 2px solid #adadad;}");
                videoItem->setSelected(false);
            }
        }
    }
    else
    {
        // 将索引号对应的视频设置为选中状态，并设置边框为红色，没有选择的设置为蓝色
        for (int i = 0; i < videoItemVec.size(); i++)
        {
            VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(videoItemVec[i]);
            if ( index == videoItem->getIndex() )
            {
                videoItem->setStyleSheet("QWidget{border: 3px solid #ff0000;}");
                videoItem->setSelected(true);
                index = videoItem->getIndex();
            }
            else
            {
                videoItem->setStyleSheet("QWidget{border: 2px solid #adadad;}");
                videoItem->setSelected(0);
            }
        }

    }
    currVideoIndex = index;
}

void VideoGroupWidget::videoItemDoubleClicked()
{
    VideoItemWidget *videoItem = qobject_cast<VideoItemWidget*>(sender());
    if (videoItem)
    {
        int index = videoItem->getIndex();
        layoutVideos(1, index);
    }
}



