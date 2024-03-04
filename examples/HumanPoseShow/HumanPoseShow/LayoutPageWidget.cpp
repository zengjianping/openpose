#include "LayoutPageWidget.h"
#include "CommonUtils.h"
#include <QPushButton>
#include <QComboBox>
#include <QLayout>
#include <QLabel>


#define LAYOUT_PAGE_WIDGET_W 273    //宽度
#define LAYOUT_PAGE_WIDGET_H 142    //高度
#define BUTTON_FIXED_WIDTH 80       //按钮宽度

LayoutPageWidget::LayoutPageWidget(QWidget *parent)
    : QWidget(parent)
{
    initializeUI();
}

LayoutPageWidget::~LayoutPageWidget()
{
    DELETE_OBJECT(buttonFirstPage)
    DELETE_OBJECT(buttonFrontPage)
    DELETE_OBJECT(buttonNextPage)
    DELETE_OBJECT(buttonLastPage)
    DELETE_OBJECT(combCurrPage)
    DELETE_OBJECT(labelTotalCount)
    DELETE_OBJECT(labelCurrIndex)
}

void LayoutPageWidget::setSlotObject(QObject* object)
{
    slotObject = object;
}

void LayoutPageWidget::setPageCount(int currIndex, int totalCount)
{
    combCurrPage->disconnect();
    combCurrPage->clear();

    QStringList strListItem;
    for (int i = 0; i < totalCount; ++i)
    {
        QString str;
        str.append(QStringLiteral("第%1页").arg(i+1));
        strListItem << str;
    }

    combCurrPage->addItems(strListItem);
    combCurrPage->setCurrentIndex(currIndex);
    labelTotalCount->setText(QStringLiteral("共%1页").arg(totalCount));

    if (slotObject)
        connect(combCurrPage, SIGNAL(currentIndexChanged(int)), slotObject, SLOT(layoutVideoGroup(int)));
}

void LayoutPageWidget::slotFirstPage()
{
    int index = combCurrPage->currentIndex();
    if (INVALID_VALUE == index)
        return;
    if (index != 0)
        combCurrPage->setCurrentIndex(0);
}

void LayoutPageWidget::slotFrontPage()
{
    int index = combCurrPage->currentIndex();
    if (INVALID_VALUE == index)
        return;
    if (index > 0)
        combCurrPage->setCurrentIndex(index-1);
}

void LayoutPageWidget::slotNextPage()
{
    int index = combCurrPage->currentIndex();
    int nCount = combCurrPage->count();
    if (INVALID_VALUE == index)
        return;
    if ( index+1 < nCount )
        combCurrPage->setCurrentIndex(index+1);
}

void LayoutPageWidget::slotLastPage()
{
    int index = combCurrPage->currentIndex();
    int nCount = combCurrPage->count();
    if (INVALID_VALUE == index)
        return;
    if (index != nCount-1)
        combCurrPage->setCurrentIndex(nCount-1);
}

void LayoutPageWidget::initializeUI()
{
    setStyleSheet("QWidget{background-color: #dbdbdb} \
                   QComboBox{background-color:none;} \
                   QScrollBar{background-color:none} \
                   QListView{background-color:none;}");

    labelCurrIndex = new QLabel(QStringLiteral("当前"),this);
    labelCurrIndex->setGeometry(57,24,25,14);

    combCurrPage = new QComboBox(this);
    combCurrPage->setGeometry(86,20,98,22);

    labelTotalCount = new QLabel(QStringLiteral("共  页"),this);
    labelTotalCount->setGeometry(188,24,40,14);

    buttonFrontPage = new QPushButton(QStringLiteral("上一页"),this);
    buttonFrontPage->setFixedWidth(BUTTON_FIXED_WIDTH);
    buttonFrontPage->setGeometry(57,61,80,24);
    buttonNextPage = new QPushButton(QStringLiteral("下一页"),this);
    buttonNextPage->setFixedWidth(BUTTON_FIXED_WIDTH);
    buttonNextPage->setGeometry(143,61,80,24);
    buttonFirstPage = new QPushButton(QStringLiteral("首页"),this);
    buttonFirstPage->setFixedWidth(BUTTON_FIXED_WIDTH);
    buttonFirstPage->setGeometry(57,93,80,24);
    buttonLastPage = new QPushButton(QStringLiteral("尾页"),this);
    buttonLastPage->setFixedWidth(BUTTON_FIXED_WIDTH);
    buttonLastPage->setGeometry(143,93,80,24);

    setWindowTitle(QStringLiteral("分屏"));
    setFixedSize(LAYOUT_PAGE_WIDGET_W,LAYOUT_PAGE_WIDGET_H);

    connect(buttonFirstPage, SIGNAL(clicked()), this, SLOT(slotFirstPage()));
    connect(buttonFrontPage, SIGNAL(clicked()), this, SLOT(slotFrontPage()));
    connect(buttonNextPage, SIGNAL(clicked()), this, SLOT(slotNextPage()));
    connect(buttonLastPage, SIGNAL(clicked()), this, SLOT(slotLastPage()));

    slotObject = nullptr;

    setWindowFlags(Qt::Dialog);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}
