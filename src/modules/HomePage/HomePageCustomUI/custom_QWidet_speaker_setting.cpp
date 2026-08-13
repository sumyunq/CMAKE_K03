#include "modules/HomePage/HomePageCustomUI/custom_QWidet_speaker_setting.h"
#include "ui_custom_QWidet_speaker_setting.h"

CustomQWidetSpeakerSetting::CustomQWidetSpeakerSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomQWidetSpeakerSetting)
{
    ui->setupUi(this);
    InitUIInformation();
    InitMember();
    InitConnect();
}

CustomQWidetSpeakerSetting::~CustomQWidetSpeakerSetting()
{
    delete ui;
}

void CustomQWidetSpeakerSetting::changeTextColor(bool targetStatus, bool isAnimation)
{
    cl_sequential_animation_group_->stop();

    // 目标状态为 启用
    if (targetStatus) {
        cl_spk_->setDisplayIcon(QIcon(":/Skin/Images/cBox/normal_close.png"));
        cl_icon_->setStyleSheet(
            "QLabel {"
            "    border: 0px;"
            "    background-image: url(:/Skin/Images/home/speaker_icon_close.png);"
            "    background-position: center;"
            "    background-repeat: no-repeat;"
            "}");

        if (isAnimation) {
            /// 三色渐变
            cl_animation_M_->setDuration(300);
            cl_animation_M_->setStartValue(cl_color_change_.at(0)); // 起始颜色
            cl_animation_M_->setEndValue(cl_color_change_.at(1));   // 结束颜色

            disconnect(cl_sequential_animation_group_,
                       &QSequentialAnimationGroup::finished,
                       this,
                       nullptr);

            // 保存连接句柄
            QMetaObject::Connection *conn = new QMetaObject::Connection;

            *conn = connect(cl_sequential_animation_group_,
                            &QSequentialAnimationGroup::finished,
                            this,
                            [this, conn]() {
                                cl_animation_M_->setDuration(200);
                                cl_animation_M_->setStartValue(cl_color_change_.at(1));
                                cl_animation_M_->setEndValue(cl_color_change_.at(2));
                                cl_sequential_animation_group_->start();

                                // 手动断开连接
                                disconnect(*conn);
                                delete conn;
                            });

            cl_sequential_animation_group_->start();
        } else {
            cl_animation_M_->setDuration(0);
            cl_animation_M_->setStartValue(cl_color_change_.at(0)); // 起始颜色
            cl_animation_M_->setEndValue(cl_color_change_.at(2));   // 结束颜色
            cl_sequential_animation_group_->start();
        }
    } else {
        // 目标状态为 禁用
        if (isAnimation) {
            cl_spk_->setDisplayIcon(QIcon(":/Skin/Images/cBox/sel.png"));
            cl_icon_->setStyleSheet(
                "QLabel {"
                "    border: 0px;"
                "    background-image: url(:/Skin/Images/homePage/speaker_icon_open.png);"
                "    background-position: center;"
                "    background-repeat: no-repeat;"
                "}"
                );
            /// 三色渐变
            cl_animation_M_->setDuration(300);
            cl_animation_M_->setStartValue(cl_color_change_.at(2)); // 起始颜色
            cl_animation_M_->setEndValue(cl_color_change_.at(1));   // 结束颜色

            disconnect(cl_sequential_animation_group_,
                       &QSequentialAnimationGroup::finished,
                       this,
                       nullptr);

            // 保存连接句柄
            QMetaObject::Connection *conn = new QMetaObject::Connection;

            *conn = connect(cl_sequential_animation_group_,
                            &QSequentialAnimationGroup::finished,
                            this,
                            [this, conn]() {
                                cl_animation_M_->setDuration(200);
                                cl_animation_M_->setStartValue(cl_color_change_.at(1));
                                cl_animation_M_->setEndValue(cl_color_change_.at(0));
                                cl_sequential_animation_group_->start();

                                // 手动断开连接
                                disconnect(*conn);
                                delete conn;
                            });

            cl_sequential_animation_group_->start();
        } else {
            cl_animation_M_->setDuration(0);
            cl_animation_M_->setStartValue(cl_color_change_.at(2)); // 起始颜色
            cl_animation_M_->setEndValue(cl_color_change_.at(0));   // 结束颜色
            cl_sequential_animation_group_->start();
        }
    }
}

void CustomQWidetSpeakerSetting::InitUIInformation()
{
    {
        cl_icon_ = new QLabel(this); ///< 图标
        cl_icon_->setFixedSize (cl_icon_min_size_);
        cl_icon_->move(cl_icon_point_);
        cl_icon_->setStyleSheet(
            "QLabel {"
            "    border: 0px;"
            "    background-image: url(:/Skin/Images/homePage/speaker_icon_open.png);"
            "    background-position: center;"
            "    background-repeat: no-repeat;"
            "}"
            );
    }
    {
        cl_spk_ = new NewComboBox(this); ///< 扬声器
        cl_spk_->setMinimumSize(cl_spk_min_size_);
        cl_spk_->move(cl_spk_point_);

        cl_spk_->setStyleSheet(R"(

QComboBox
{
border:none;
border-top-left-radius: 2px;
border-top-right-radius: 2px;
border-bottom-left-radius: 2px;
border-bottom-right-radius: 2px;
    combobox-popup: 0;/*确保在非编辑模式下，下拉列表框的位置改变*/
    border-image: url(:/Skin/Images/cBox/dropdownCollapse_bk.png);
    padding-left: 10px; /* 为图标留出空间 */
    color: rgb(255, 255, 255);

font-family: "Noto Sans S Chinese";
                font-weight: 500;
font-size: 10px;
line-height: normal;
letter-spacing: 0em;

}
/*下拉箭头按钮样式-未选中*/
QComboBox::drop-down
{
    border-image: url(:/Skin/Images/cBox/droptriangle_no.png);
    margin-right:10px;
    height:10px;
    width:7px;
    subcontrol-origin: padding;  /* 基于内边距定位 */
    subcontrol-position: center right;  /* 居中且靠右 */
}
/*下拉箭头按钮样式-选中*/
QComboBox::drop-down:checked{
    border-image: url(:/Skin/Images/cBox/droptriangle_se.png);
    margin-top:0px;
    margin-right:10px;
    height:8px;
    width:8px;
subcontrol-origin: padding;  /* 基于内边距定位 */
    subcontrol-position: center right;  /* 居中且靠右 */
}
/*下拉箭头按钮样式-悬浮
QComboBox::drop-down:hover{
    border-image: url(:/image/LED/droptriangle_se.png);
    margin-top:10px;
    margin-right:10px;
    height:10px;
    width:8px;
}*/

QListView
{
padding-left: 30px; /* 为图标留出空间 */
    position: absolute;
    bottom: 0;
    image: url(:/Skin/Images/cBox/dropdownBtn_bk.png);
    color: rgb(206, 207, 211);

}
QListView::item {
    height: 31px;
border-bottom: 1px solid gray;
}
/*下方下拉列表项选中项的样式*/
QListView::item:selected
{
    /*background:rgb(71, 78, 96);*/
    border-image: url(:/Skin/Images/cBox/dropDownBtn_sel.png);
    color: rgb(206, 207, 211);
}
/*下方下拉列表项鼠标悬停的样式*/
QListView::item:hover
{
    /*background: rgb(71, 78, 96);*/
    color: rgb(206, 207, 211);
}

)");
    }

    {
        // 开关按键
        pBt_spk_switch_ = new CustomPushButton(this);
        pBt_spk_switch_->setMinimumSize(pBt_spk_switch_min_size_);
        // pBt_spk_switch_->setText("开");
        pBt_spk_switch_->setCheckable(true);
        pBt_spk_switch_->show();
        pBt_spk_switch_->move(pBt_spk_switch_point_);
    }
    {
        // 水平条
        cl_HSlider_ = new NewHSlider(this);

        // cl_HSlider_->setParent(nullptr);
        // cl_HSlider_->show();

        cl_HSlider_->setRange(0, 100);
        cl_HSlider_->move(cl_HSlider_point_);
        cl_HSlider_->setType(1, 12, 4,true,true);
        cl_HSlider_->setStyleSheet(R"(
QSlider {
    background: none;
    border: none;
}
QSlider::groove:horizontal {
    height: 10px;
    border: none;
    background: transparent;
}
QSlider::handle:horizontal {
    width: 12px;
    height: 0px;
    background: transparent;
    border: none;
    margin: 0px;
    padding: 0px;
}
QSlider::handle:horizontal:hover {
    width: 12px;
    height: 0px;
    background: transparent;
    border: none;
    margin: 0px;
    padding: 0px;
}
)");
    }


}

void CustomQWidetSpeakerSetting::InitMember()
{
    {
        cl_animTarget_M_ = new TextColorAnimator(cl_spk_,
                                                 cl_spk_); /// 自定义 文本文字颜色变化

        cl_animation_M_ = new QPropertyAnimation(cl_animTarget_M_, "textColor");
        cl_animation_M_->setDuration(300); // 0.3 秒
    }
    {
        cl_icon_Effect_ = new QGraphicsOpacityEffect(cl_spk_); ///< 图标 淡入淡出动画
        cl_spk_->setGraphicsEffect(cl_icon_Effect_);
    }
    {
        cl_icon_fadeOut_animation_ = new QPropertyAnimation(cl_icon_Effect_, "opacity");
        cl_icon_fadeOut_animation_->setDuration(300);
        cl_icon_fadeOut_animation_->setStartValue(1.0);
        cl_icon_fadeOut_animation_->setEndValue(0.0);
    }
    {
        // 动画组
        cl_sequential_animation_group_ = new QSequentialAnimationGroup(this);
        cl_sequential_animation_group_->addAnimation(cl_animation_M_);
        // cl_sequential_animation_group_->addAnimation(cl_icon_fadeOut_animation_);    ///淡入淡出动画 先不加

        // cl_sequential_animation_group_->setLoopCount(-1);// 无限循环
    }
}

void CustomQWidetSpeakerSetting::InitConnect() {
}

void CustomQWidetSpeakerSetting::resizeEvent(QResizeEvent *event)
{
    {
        // 图标
        cl_icon_->setGeometry(cl_icon_point_.x(),
                              cl_icon_point_.y(),
                              cl_icon_min_size_.width(),
                              cl_icon_min_size_.height());
    }
    {
        // 扬声器
        cl_spk_->setGeometry(cl_spk_point_.x(),
                             cl_spk_point_.y(),
                             cl_spk_min_size_.width(),
                             cl_spk_min_size_.height());
    }

    {
        // 开关按键
        pBt_spk_switch_->setGeometry(rect ().width () - pBt_spk_switch_min_size_.width () - 30,
                                    pBt_spk_switch_point_.y(),
                                    pBt_spk_switch_min_size_.width(),
                                    pBt_spk_switch_min_size_.height());
    }
    {
        // 水平进度条
        cl_HSlider_->setGeometry(cl_HSlider_point_.x(),
                                 cl_HSlider_point_.y(),
                                 rect().width() - 30 * 2,
                                 cl_HSlider_min_size_.height());
    }

    QWidget::resizeEvent(event);
}