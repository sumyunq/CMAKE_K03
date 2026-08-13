#ifndef CUSTOM_QSCROLLAREA_GENERAL_LAYOUT_H
#define CUSTOM_QSCROLLAREA_GENERAL_LAYOUT_H

#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>

#include "data/api_global.h" ///< 取 ScrollAreaDisplayMode 枚举值

///
/// \brief The CustomQScrollAreaGeneralLayout class
/// 通用滚动区域布局，支持网格/单列/单行三种模式，可展示任意 QWidget 子类实例
class CustomQScrollAreaGeneralLayout : public QScrollArea
{
    Q_OBJECT

public:
    explicit CustomQScrollAreaGeneralLayout(
        QWidget *parent = nullptr, ScrollAreaDisplayMode mode = ScrollAreaDisplayMode::GridDisplay);
    ~CustomQScrollAreaGeneralLayout();

    void setCl_showMode(ScrollAreaDisplayMode mode);
    ScrollAreaDisplayMode cl_showMode() const;

    void addWidget(QWidget *widget);    ///< 添加子控件并刷新布局
    void removeWidget(QWidget *widget); ///< 移除子控件并刷新布局
    void removeAllWidgets();            ///移除所有子控件
    void updateView();                  ///< 刷新布局

    void setMarginAndWidth(int leftMargin, int rightMargin, int itemMargin, int minItemWidth);//设置边距和子控件宽度
    void setScrollbar(int width);       //设置滚动条

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QList<QWidget *> cl_widget_list_; ///< 子控件列表

    int cl_grid_leftMargin_ = 0;     ///< 网格左边距
    int cl_grid_rightMargin_ = 0;    ///< 网格右边距
    int cl_grid_itemSpacing_ = 10;    ///< 网格内部部件间距
    int cl_grid_scrollbarWidth_ = 0; ///< 右侧滚动条宽度
    int cl_grid_minItemWidth_ = 323;  ///< 子控件最小宽度

private:
    QWidget *clp_content_widget_ = nullptr; ///< 内容显示区域
    QGridLayout *clp_gridLayout_ = nullptr; ///< 网格布局
    QHBoxLayout *clp_hBoxLayout_ = nullptr; ///< 水平布局
    QVBoxLayout *clp_vBoxLayout_ = nullptr; ///< 垂直布局

    ScrollAreaDisplayMode cl_showMode_ = ScrollAreaDisplayMode::GridDisplay; ///< 当前显示模式
    int cl_columnCount_ = 2; ///< 网格列数（运行时根据视口宽度动态计算）

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QSCROLLAREA_GENERAL_LAYOUT_H
