#ifndef LAYOUT_PAGE_WIDGET_H__
#define LAYOUT_PAGE_WIDGET_H__

#include <QWidget>

class QPushButton;
class QComboBox;
class QLabel;


//分屏工具界面类
class LayoutPageWidget : public QWidget
{
    Q_OBJECT

public:
    LayoutPageWidget(QWidget *parent = NULL);
    ~LayoutPageWidget();

    // 设置总页数和当前页
    void setPageCount(int currIndex, int totalCount);
    // 设置响应信号槽的类
    void setSlotObject(QObject* object);

public slots:
    // 首页
    void slotFirstPage();
    // 上一页
    void slotFrontPage();
    // 下一页
    void slotNextPage();
    // 尾页
    void slotLastPage();

private:
    // 界面初始化
    void initializeUI();

private:
    QPushButton *buttonFirstPage;   // 首页
    QPushButton *buttonFrontPage;   // 上一页
    QPushButton *buttonNextPage;    // 下一页
    QPushButton *buttonLastPage;    // 尾页
    QComboBox *combCurrPage;        // 当前页
    QLabel *labelTotalCount;        // 共多少页描述
    QLabel *labelCurrIndex;         // 当前描述
    QObject *slotObject;            // 连接信号槽的类
};

#endif //LAYOUT_PAGE_WIDGET_H__


