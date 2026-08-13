#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QScrollArea_device_selection.h"
#include "ui_custom_QScrollArea_device_selection.h"

CustomQScrollAreaDeviceSelection::CustomQScrollAreaDeviceSelection(QWidget *parent)
    : QScrollArea(parent)
    , ui(new Ui::CustomQScrollAreaDeviceSelection)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQScrollAreaDeviceSelection::~CustomQScrollAreaDeviceSelection()
{
    delete ui;
}

void CustomQScrollAreaDeviceSelection::updateView()
{
    if (!cl_gridLayout_)
        return;

    // 清空布局
    QLayoutItem *item;
    while ((item = cl_gridLayout_->takeAt(0)) != nullptr) {
        delete item;
    }

    // qDebug() << "=== 宽度调试 ===";
    // qDebug() << "QScrollArea 宽度:" << this->geometry ().width ();
    // qDebug() << "viewport 宽度:" << this->viewport()->width();
    // qDebug() << "差值:" << (this->width() - this->viewport()->width());

    // int widgetWidth = this->viewport()->width();
    // int leftMargin = 30;    //左边距
    // int rightMargin = 20;   //右边距
    // int inMargin = 20;      //内部部件边距
    // int rightComWidth = 10; //右侧滑动部件宽度

    int widgetWidth = this->geometry().width();

    ///单列数 取 (视口的实际可用宽度- 左边距 - 右边距 + 右侧滚动条宽度) 除以 内部控件最小宽度 + 部件边距（网格布局： 238 + inMargin ）
    this->cl_columnCount_ = (widgetWidth - left_margin_ - right_margin_ + spacing_) / (344 + spacing_);

    if (cl_columnCount_ < 1)
        cl_columnCount_ = 1;

    // 重新添加 所有设备信息 居中/左上角开始
    // if (cl_all_device_list_.size() < cl_columnCount_) {
    // cl_gridLayout_->setAlignment(Qt::AlignCenter|Qt::AlignTop);
    // } else {
    //     cl_gridLayout_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    // }
    cl_gridLayout_->setAlignment(Qt::AlignCenter|Qt::AlignTop);

    // 设置网格布局
    int maxRow = 0;
    for (int i = 0; i < cl_all_device_list_.size(); ++i) {
        int row = i / cl_columnCount_;
        int col = i % cl_columnCount_ + 1;
        cl_gridLayout_->addWidget(cl_all_device_list_.at(i), row, col);
        if (row > maxRow)
            maxRow = row;
    }
    cl_rowCount_ = maxRow + 1; ///< 行数 = 最大行索引 + 1

    update();
}

void CustomQScrollAreaDeviceSelection::InitUIInformation()
{
    {
        this->setMinimumHeight(426); // 最小高度
        this->setStyleSheet(R"(
            QScrollArea {
                background: transparent;
                border: none;
            }
        )");
        // 测试用
        //     this->setStyleSheet(R"(
        //     QScrollArea {
        //         background-color: #2196F3;
        //         border: 2px solid red;
        //     }
        // )");

        // 滚动条策略
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

        // 测试用
        //     // 内容控件背景为绿色
        //     cl_content_widget_->setStyleSheet(R"(
        //     QWidget {
        //         background-color: #4CAF50;
        //         border: 1px solid yellow;
        //     }
        // )");
        // 内容控件的尺寸策略
        cl_content_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // 将内容控件设置为滚动区域的内容
        this->setWidget(cl_content_widget_);
        this->setWidgetResizable(true); ///自适应大小
    }
    {
        // 垂直布局
        cl_gridLayout_ = new QGridLayout(cl_content_widget_);
        cl_gridLayout_->setSpacing(spacing_);
        cl_gridLayout_->setContentsMargins(left_margin_, top_margin_, right_margin_, bottom_margin_);
    }
}

void CustomQScrollAreaDeviceSelection::InitMember() {}

void CustomQScrollAreaDeviceSelection::InitConnect() {}
