#ifndef VIDEOGROUPWIDGET_H
#define VIDEOGROUPWIDGET_H

#include <QWidget>
#include <QGridLayout>


class VideoGroupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoGroupWidget(QWidget *parent = nullptr);
    ~VideoGroupWidget();

signals:

public:
    // 视频显示界面布局
    void layoutVideos(int count, int group = -1);
    QWidget* getVideoWidget(int index);
    void setImage(int index, QPixmap& pixmap);
    void resetAllImage();

protected:
    void initializeUI();
    void terminateUI();

    int getVideoIndex(int index);
    void layoutReset();
    void layoutCount1(int group);
    void layoutCount4(int group);
    void layoutCount8(int group);
    void layoutCount9(int group);
    void layoutCount13(int group);
    void layoutCount16(int group);
    void layoutCount25(int group);
    void layoutCount32(int group);
    void layoutCount64(int group);

private:
    QGridLayout *videoLayout; // 布局管理器
    QVector<QWidget*> videoItemVec; // 视频集合
    QVector<QWidget*> reservedVideoItemVec; // 补位视频集合
    QList<int> videoIds; // 视频IDs
    int currVideoCount; // 当前视频数目
    int currVideoIndex; // 当前视频索引
};

#endif // VIDEOGROUPWIDGET_H
