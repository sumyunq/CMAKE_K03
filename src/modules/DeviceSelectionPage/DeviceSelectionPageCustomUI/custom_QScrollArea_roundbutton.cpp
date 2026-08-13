#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QScrollArea_roundbutton.h"
#include "ui_custom_QScrollArea_roundbutton.h"

CustomQScrollAreaRoundbutton::CustomQScrollAreaRoundbutton(QWidget *parent)
    : CustomQScrollAreaRoundbutton(0, parent) ///< 默认使用网格布局
{
}

CustomQScrollAreaRoundbutton::CustomQScrollAreaRoundbutton(int showMode, QWidget *parent)
    : QScrollArea(parent)
    , ui(new Ui::CustomQScrollAreaRoundbutton)
    , cl_show_mode_(showMode)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQScrollAreaRoundbutton::~CustomQScrollAreaRoundbutton()
{
    delete ui;
}

void CustomQScrollAreaRoundbutton::updateView()
{
    // 先根据当前布局类型清除
    QLayout *activeLayout = nullptr;
    if (cl_show_mode_ == 0) {
        activeLayout = cl_gridLayout_;
    } else if (cl_show_mode_ == 1) {
        activeLayout = cl_hBoxLayout_;
    } else if (cl_show_mode_ == 2) {
        activeLayout = cl_vBoxLayout_;
    }

    if (!activeLayout)
        return;

    // 清空布局
    QLayoutItem *item;
    while ((item = activeLayout->takeAt(0)) != nullptr) {
        delete item;
    }

    // 清空按键组
    if (cl_buttonGroup_) {
        for (auto *btn : cl_buttonGroup_->buttons()) {
            cl_buttonGroup_->removeButton(btn);
        }
    }

    if (cl_all_rows_CustomQPushButtonRoundButton_.isEmpty())
        return;

    // 将按键加入互斥组，设为可选中
    for (int i = 0; i < cl_all_rows_CustomQPushButtonRoundButton_.size(); ++i) {
        cl_all_rows_CustomQPushButtonRoundButton_.at (i)->setCheckable(true);
        if (cl_buttonGroup_) {
            cl_buttonGroup_->addButton(cl_all_rows_CustomQPushButtonRoundButton_.at (i),i);///< 确保滚轮滚动
        }
    }

    // 网格布局模式：计算列数
    if (cl_show_mode_ == 0) {
        int viewportWidth = this->viewport()->width();
        cl_columnCount_ = viewportWidth / cl_itemMinWidth_;
        if (cl_columnCount_ < 1)
            cl_columnCount_ = 1;

        for (int i = 0; i < cl_all_rows_CustomQPushButtonRoundButton_.size(); ++i) {
            int row = i / cl_columnCount_;
            int col = i % cl_columnCount_;
            cl_gridLayout_->addWidget(cl_all_rows_CustomQPushButtonRoundButton_.at(i), row, col);
        }
    }
    // 水平布局模式：单行
    else if (cl_show_mode_ == 1) {
        for (auto *btn : cl_all_rows_CustomQPushButtonRoundButton_) {
            cl_hBoxLayout_->addWidget(btn);
        }
    }
    // 垂直布局模式：单列
    else if (cl_show_mode_ == 2) {
        cl_vBoxLayout_->addStretch();
        for (int i = 0; i < cl_all_rows_CustomQPushButtonRoundButton_.size(); ++i) {
            cl_vBoxLayout_->addWidget(cl_all_rows_CustomQPushButtonRoundButton_.at (i));
        }
        cl_vBoxLayout_->addStretch();
    }

    update();
}

void CustomQScrollAreaRoundbutton::InitUIInformation()
{
    setStyleSheet(R"(
        QScrollArea {
           background: transparent;
            background: none;
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

    this->viewport()->setAutoFillBackground(false); // 视口单独设置透明

    cl_content_widget_ = new QWidget(this);
    cl_content_widget_->setObjectName("cl_content_widget_");
    cl_content_widget_->setStyleSheet(R"(
        QWidget#cl_content_widget_ {
            background: transparent;
            border: none;
        }
)");

    // 创建三种布局（后续按 cl_show_mode_ 选用）
    cl_gridLayout_ = new QGridLayout();
    cl_gridLayout_->setSpacing(30);
    cl_gridLayout_->setContentsMargins(00, 0, 0, 0);

    cl_hBoxLayout_ = new QHBoxLayout();
    cl_hBoxLayout_->setSpacing(30);
    cl_hBoxLayout_->setContentsMargins(0, 0, 0, 0);

    cl_vBoxLayout_ = new QVBoxLayout();
    cl_vBoxLayout_->setSpacing(30);
    cl_vBoxLayout_->setContentsMargins(0, 0, 0, 0);

    // 根据显示模式设置内容控件的布局
    QLayout *activeLayout = nullptr;
    if (cl_show_mode_ == 0) {
        activeLayout = cl_gridLayout_;
    } else if (cl_show_mode_ == 1) {
        activeLayout = cl_hBoxLayout_;
    } else if (cl_show_mode_ == 2) {
        activeLayout = cl_vBoxLayout_;
    } else {
        activeLayout = cl_gridLayout_;
    }
    cl_content_widget_->setLayout(activeLayout);

    // 将内容控件设置为滚动区域的内容
    this->setWidget(cl_content_widget_);
    this->setWidgetResizable(true); ///自适应大小

    // 滚动条策略
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 内容控件的尺寸策略
    cl_content_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    updateView();
}

void CustomQScrollAreaRoundbutton::InitMember()
{
    cl_buttonGroup_ = new QButtonGroup(this);
    cl_buttonGroup_->setExclusive(true); // 互斥模式
}

void CustomQScrollAreaRoundbutton::InitConnect() {}

void CustomQScrollAreaRoundbutton::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    // 网格模式下窗口大小变化时重新计算列数
    if (cl_show_mode_ == 0) {
        updateView();
    }
}
