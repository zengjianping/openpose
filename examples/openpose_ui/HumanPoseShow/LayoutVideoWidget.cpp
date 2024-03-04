#include "LayoutVideoWidget.h"
#include "CommonUtils.h"
#include <QLayout>
#include <QStyleOption>
#include <QPainter>
#include <QLabel>


#define BUTTON_SIZE_W 47    //按钮宽度
#define BUTTON_SIZE_H 35    //按钮高度

#define LAYOUTVIDEOWIDGET_SIZE_W 261  //宽度
#define LAYOUTVIDEOWIDGET_SIZE_H 110  //高度

enum ButtonCount
{
    BUTTON_RESET = 0,   //重新设置索引号
    BUTTON_1     = 1,   //1个视频布局
    BUTTON_4     = 4,   //4个视频布局
    BUTTON_8     = 8,   //8个视频布局
    BUTTON_9     = 9,   //9个视频布局
    BUTTON_13    = 13,  //13个视频布局
    BUTTON_16    = 16,  //16个视频布局
    BUTTON_25    = 25,  //25个视频布局
    BUTTON_32    = 32,  //32个视频布局
    BUTTON_64    = 64,  //64个视频布局
    BUTTON_COUNT = 10   //按钮个数
};

LayoutVideoWidget::LayoutVideoWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUI();
}

LayoutVideoWidget::~LayoutVideoWidget()
{
    for (int i = 0; i < mapButtons.count(); ++i)
    {
        QPushButton *button = mapButtons.key(i);
        DELETE_OBJECT(button)
    }
    mapButtons.clear();
}

void LayoutVideoWidget::setButtonEnabled(bool enabled)
{
    for (int i = 0; i < mapButtons.count(); ++i)
    {
        QPushButton *button = mapButtons.key(i);
        button->setEnabled(enabled);
    }
}

void LayoutVideoWidget::buttonClick()
{
    QObject *obj = sender();
    int count = mapButtons.value(qobject_cast<QPushButton*>(obj));

    switch(count)
    {
    case 0:
        count = BUTTON_RESET;
        break;
    case 1:
        count = BUTTON_1;
        break;
    case 2:
        count = BUTTON_4;
        break;
    case 3:
        count = BUTTON_8;
        break;
    case 4:
        count = BUTTON_9;
        break;
    case 5:
        count = BUTTON_13;
        break;
    case 6:
        count = BUTTON_16;
        break;
    case 7:
        count = BUTTON_25;
        break;
    case 8:
        count = BUTTON_32;
        break;
    case 9:
        count = BUTTON_64;
        break;
    default:
        count = INVALID_VALUE;
        break;
    }

    if ( count != INVALID_VALUE )
        emit signalVideoCount(count);
}

void LayoutVideoWidget::initializeUI()
{
    setStyleSheet("QWidget{background-color:#474747;border-radius: 1px}");

    QGridLayout *layout = new QGridLayout;

    for (int i = 0; i < BUTTON_COUNT; ++i)
    {
        QPushButton *button = new QPushButton(this);
        QString strStyleSheet = QString(
            "QPushButton:pressed{background-image: url(:/images/VideoAreaCut%1hover.png);} \
            QPushButton:hover{background-image: url(:/images/VideoAreaCut%2hover.png);} \
            QPushButton{background-image: url(:/images/VideoAreaCut%3.png);max-height: 35;min-height: 35;max-width: 47px;min-width: 47px;}").arg(i+1).arg(i+1).arg(i+1);
        button->setStyleSheet(strStyleSheet);
        layout->addWidget(button, i/5, i%5 );
        button->setFixedSize(BUTTON_SIZE_W, BUTTON_SIZE_H);
        connect(button,SIGNAL(clicked()),this,SLOT(buttonClick()));
        mapButtons.insert(button, i);
    }
    layout->setSpacing(0);

    QVBoxLayout *hLayout = new QVBoxLayout;
    QLabel *label = new QLabel(this);
    label->setText(QStringLiteral("视频窗口分割"));
    label->setStyleSheet("color:white");
    hLayout->addWidget(label);
    hLayout->addLayout(layout);
    setLayout(hLayout);

    setWindowFlags(Qt::Popup);
    resize(LAYOUTVIDEOWIDGET_SIZE_W, LAYOUTVIDEOWIDGET_SIZE_H);
}

void LayoutVideoWidget::paintEvent( QPaintEvent * event )
{
    QStyleOption option;
    option.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}

