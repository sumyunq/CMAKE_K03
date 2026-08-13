#include "modules/GeneralCustomUI/custom_QScrollArea_general_layout.h"
#include "qscrollbar.h"

#include <QResizeEvent>
#include <QTimer>

CustomQScrollAreaGeneralLayout::CustomQScrollAreaGeneralLayout(
    QWidget *parent, ScrollAreaDisplayMode mode)
    : QScrollArea(parent)
    , cl_showMode_(mode)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQScrollAreaGeneralLayout::~CustomQScrollAreaGeneralLayout()
{
}

void CustomQScrollAreaGeneralLayout::InitUIInformation()
{
    {
        // 滚动区域样式
        setStyleSheet(R"(
            QScrollArea {
                background: transparent;
                border: none;
            }
            QScrollBar:vertical {
                background: rgba(0, 0, 0, 0);
                border-radius: 5px;
                width: 10px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background: rgba(0, 0, 0, 0.2);
                border-radius: 5px;
                min-height: 30px;
            }
            QScrollBar::add-line:vertical,
            QScrollBar::sub-line:vertical {
                border: none;
                background: none;
                height: 0;
            }
            QScrollBar::add-page:vertical,
            QScrollBar::sub-page:vertical {
                background: none;
            }
        )");
        viewport()->setAutoFillBackground(false);
    }
    {
        // 内容控件
        clp_content_widget_ = new QWidget(this);
        clp_content_widget_->setObjectName("clp_content_widget_");
        clp_content_widget_->setStyleSheet(R"(
            QWidget#clp_content_widget_ {
                background: transparent;
                border: none;
            }
        )");
    }
    {
        // 根据模式创建对应布局
        switch (cl_showMode_) {
        case ScrollAreaDisplayMode::GridDisplay:
            clp_gridLayout_ = new QGridLayout(clp_content_widget_);
            clp_gridLayout_->setSpacing(cl_grid_itemSpacing_);
            clp_gridLayout_->setContentsMargins(cl_grid_leftMargin_, 0,
                                                cl_grid_rightMargin_, 0);
            clp_gridLayout_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            break;
        case ScrollAreaDisplayMode::SingleColumnDisplay:
            clp_vBoxLayout_ = new QVBoxLayout(clp_content_widget_);
            clp_vBoxLayout_->setSpacing(0);
            clp_vBoxLayout_->setContentsMargins(0, 0, 0, 0);
            break;
        case ScrollAreaDisplayMode::SingleLineDisplay:
            clp_hBoxLayout_ = new QHBoxLayout(clp_content_widget_);
            clp_hBoxLayout_->setSpacing(10);
            clp_hBoxLayout_->setContentsMargins(10, 0, 10, 0);
            break;
        }
    }
    {
        // 滚动区域设置
        setWidget(clp_content_widget_);
        setWidgetResizable(true);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        clp_content_widget_->setSizePolicy(QSizePolicy::Expanding,
                                           QSizePolicy::Expanding);
    }
}

void CustomQScrollAreaGeneralLayout::InitMember()
{
}

void CustomQScrollAreaGeneralLayout::InitConnect()
{
}

void CustomQScrollAreaGeneralLayout::setCl_showMode(ScrollAreaDisplayMode mode)
{
    cl_showMode_ = mode;
    updateView();
}

ScrollAreaDisplayMode CustomQScrollAreaGeneralLayout::cl_showMode() const
{
    return cl_showMode_;
}

void CustomQScrollAreaGeneralLayout::addWidget(QWidget *widget)
{
    if (!widget)
        return;
    cl_widget_list_.append(widget);
}

void CustomQScrollAreaGeneralLayout::removeWidget(QWidget *widget)
{
    if (!widget)
        return;
    cl_widget_list_.removeAll(widget);
    updateView();
}
//移除所有子控件
void CustomQScrollAreaGeneralLayout::removeAllWidgets()
{
    cl_widget_list_.clear();
}

void CustomQScrollAreaGeneralLayout::updateView()
{
    // 网格布局
    if (cl_showMode_ == ScrollAreaDisplayMode::GridDisplay && clp_gridLayout_) {
        // 清除布局项
        QLayoutItem *t_item;
        while ((t_item = clp_gridLayout_->takeAt(0)) != nullptr) {
            delete t_item;
        }

        // 动态计算列数
        const int t_viewportWidth = viewport()->width();

        cl_columnCount_ = (t_viewportWidth - cl_grid_leftMargin_
                           - cl_grid_rightMargin_ + cl_grid_itemSpacing_
                           + cl_grid_scrollbarWidth_)
                          / (cl_grid_minItemWidth_ + cl_grid_itemSpacing_);
        if (cl_columnCount_ < 1)
            cl_columnCount_ = 1;

        // 仅对可见控件按顺序排列，隐藏控件不占位
        int visibleIndex = 0;
        for (int i = 0; i < cl_widget_list_.size(); ++i)
        {
            if (cl_widget_list_.at(i)->isHidden())
            {
                continue;                     // 跳过隐藏控件
            }
            const int t_row = visibleIndex  / cl_columnCount_;//i
            const int t_col = visibleIndex  % cl_columnCount_;//i
            clp_gridLayout_->addWidget(cl_widget_list_.at(i), t_row, t_col);
            ++visibleIndex;
        }
        return;
    }

    // 单列布局
    if (cl_showMode_ == ScrollAreaDisplayMode::SingleColumnDisplay
        && clp_vBoxLayout_) {
        QLayoutItem *t_item;
        while ((t_item = clp_vBoxLayout_->takeAt(0)) != nullptr) {
            delete t_item;
        }
        for (int i = 0; i < cl_widget_list_.size(); ++i) {
            clp_vBoxLayout_->addWidget(cl_widget_list_.at(i));
        }
        clp_vBoxLayout_->addStretch();
        return;
    }

    // 单行布局
    if (cl_showMode_ == ScrollAreaDisplayMode::SingleLineDisplay
        && clp_hBoxLayout_) {
        QLayoutItem *t_item;
        while ((t_item = clp_hBoxLayout_->takeAt(0)) != nullptr) {
            delete t_item;
        }
        for (int i = 0; i < cl_widget_list_.size(); ++i) {
            clp_hBoxLayout_->addWidget(cl_widget_list_.at(i));
        }
        clp_hBoxLayout_->addStretch();
        return;
    }
}

void CustomQScrollAreaGeneralLayout::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    QTimer::singleShot(0, this, [this]() { updateView(); });
}
//设置边距和子控件宽度
void CustomQScrollAreaGeneralLayout::setMarginAndWidth(int leftMargin, int rightMargin, int itemMargin, int minItemWidth)
{

    cl_grid_leftMargin_ = leftMargin;
    cl_grid_rightMargin_ = rightMargin;
    cl_grid_itemSpacing_ = itemMargin;
    cl_grid_minItemWidth_ = minItemWidth;
}
//设置滚动条
void CustomQScrollAreaGeneralLayout::setScrollbar(int width)
{
    cl_grid_scrollbarWidth_ = width;
    // 滚动条样式
    verticalScrollBar()->setStyleSheet(R"(
    QScrollBar:vertical {
        background-color: transparent;
        width: 10px;
        margin: 0px;
        padding: 0px;
        border-radius: 5px;
    }
    QScrollBar::handle:vertical {
        background: rgba(0, 0, 0,51);
        border-radius: 5px;
    }
    QScrollBar::sub-line:vertical,
    QScrollBar::add-line:vertical {
        height: 0px;
        background: none;
    }
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical {
        background: none;
    }
)");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 隐藏水平滚动条
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}
