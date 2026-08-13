#include "modules/AdvertisementSelectionPage/AdvertisementSelectionPageCustomUI/custom_QScrollarea_for_advertisement_pushbutton.h"
#include "ui_custom_QScrollarea_for_advertisement_pushbutton.h"

CustomQScrollAreaForAdvertisementPushButton::CustomQScrollAreaForAdvertisementPushButton(
    QWidget *parent)
    : QScrollArea(parent)
    , ui(new Ui::CustomQScrollAreaForAdvertisementPushButton)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQScrollAreaForAdvertisementPushButton::~CustomQScrollAreaForAdvertisementPushButton()
{
    delete ui;
}

void CustomQScrollAreaForAdvertisementPushButton::updateView()
{
    if (!cl_hBoxLayout_)
        return;

    if (!cl_all_advertisement_buttonGroup_)
        return;

    if (cl_all_advertisement_list_.size() == 0)
        return;

    for (CustomQPushButtonForSingleAdvertisement *btn : cl_all_advertisement_list_) {
        btn->show();
    }
    update();
}

void CustomQScrollAreaForAdvertisementPushButton::onAdvertisementClicked(int index)
{
    emit changeGameTypeVideos(index);
}

void CustomQScrollAreaForAdvertisementPushButton::InitUIInformation()
{
    {
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
        cl_all_advertisement_buttonGroup_ = new QButtonGroup(this);
        cl_all_advertisement_buttonGroup_->setExclusive(true); /// 单选模式
    }
    {
        // // 默认创建十个按键,更新时，只更新显示的内容，以及是否隐藏
        // for (int var = 0; var < 10; ++var) {
        //     CustomQPushButtonForSingleAdvertisement *btn
        //         = new CustomQPushButtonForSingleAdvertisement(cl_content_widget_);

        //     if (var>5)
        //         btn->cl_isShow_.store(true);
        //     btn->setCheckable(true);
        //     cl_all_advertisement_list_.append(btn);
        //     cl_all_advertisement_buttonGroup_->addButton(btn, var);
        // }

        // if (cl_all_advertisement_buttonGroup_->buttons().size() != 0) {
        //     cl_all_advertisement_buttonGroup_->buttons().at(0)->setChecked(true);
        // };

        // // 不修改布局内容，只更新对应的 按键的 hide /show
        // for (int i = 0; i < cl_all_advertisement_list_.size(); ++i) {
        //     cl_hBoxLayout_->addWidget(cl_all_advertisement_list_.at(i));
        // }
        // cl_hBoxLayout_->addStretch();
    }
}

void CustomQScrollAreaForAdvertisementPushButton::InitMember() {}

void CustomQScrollAreaForAdvertisementPushButton::InitConnect()
{
    connect(cl_all_advertisement_buttonGroup_,
            &QButtonGroup::idClicked,
            this,
            &CustomQScrollAreaForAdvertisementPushButton::onAdvertisementClicked,
            Qt::UniqueConnection);
}

void CustomQScrollAreaForAdvertisementPushButton::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    // 延迟更新
    QTimer::singleShot(0, this, [this]() { this->updateView(); });
}