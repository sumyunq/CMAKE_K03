#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QScrollarea_color_pushbuttons.h"
#include "ui_custom_QScrollarea_color_pushbuttons.h"

CustomQScrollAreaColorPushButtons::CustomQScrollAreaColorPushButtons(QWidget *parent)
    : QScrollArea(parent)
    , ui(new Ui::CustomQScrollAreaColorPushButtons)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQScrollAreaColorPushButtons::~CustomQScrollAreaColorPushButtons()
{
    delete ui;
}

void CustomQScrollAreaColorPushButtons::updateView()
{
    if (!cl_hBoxLayout_ || !cl_all_color_pushButton_buttonGroup_)
        return;

    // 清空内部按键
    QList<QAbstractButton *> buttons = cl_all_color_pushButton_buttonGroup_->buttons();
    for (QAbstractButton *btn : buttons) {
        cl_all_color_pushButton_buttonGroup_->removeButton(btn);
    }

    // 清空布局
    QLayoutItem *item;
    while ((item = cl_hBoxLayout_->takeAt(0)) != nullptr) {
        delete item;
    }

    // 重新添加所有按钮(居中)
    cl_hBoxLayout_->addStretch();
    for (int i = 0; i < cl_all_color_pushButton_list_.size(); ++i) {
        // 添加到布局
        cl_hBoxLayout_->addWidget(cl_all_color_pushButton_list_.at(i));
        // 添加到按键组
        cl_all_color_pushButton_buttonGroup_->addButton(cl_all_color_pushButton_list_.at(i), i);
    }
    cl_hBoxLayout_->addStretch();
    //cl_all_color_pushButton_buttonGroup_->setExclusive(true); // 单选模式

    update();
}

void CustomQScrollAreaColorPushButtons::InitUIInformation()
{
    {
        this->setMinimumHeight(8); // 最小高度
        this->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
        }
    )");

        // 滚动条策略
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    {
        // 内容控件
        cl_content_widget_ = new QWidget(this);
        cl_content_widget_->setStyleSheet(R"(
        QWidget {
            background: transparent;
            border: none;
        }
    )");
        // 内容控件的尺寸策略
        cl_content_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 将内容控件设置为滚动区域的内容
        this->setWidget(cl_content_widget_);
        this->setWidgetResizable(true); ///自适应大小
    }
    {
        // 水平布局
        cl_hBoxLayout_ = new QHBoxLayout(cl_content_widget_);
        cl_hBoxLayout_->setSpacing(spacing_);
        cl_hBoxLayout_->setContentsMargins(left_margin_, top_margin_, right_margin_, bottom_margin_);
        cl_hBoxLayout_->setAlignment(Qt::AlignRight); ///居中
    }
    {
        // 互斥按键组
        cl_all_color_pushButton_buttonGroup_ = new QButtonGroup(this);
        cl_all_color_pushButton_buttonGroup_->setExclusive(true); /// 单选模式
    }
}

void CustomQScrollAreaColorPushButtons::InitMember() {}

void CustomQScrollAreaColorPushButtons::InitConnect() {}
