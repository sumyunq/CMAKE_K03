#include "modules/HomePage/HomePageCustomUI/custom_QWidget_single_algorithm_setting.h"
#include "ui_custom_QWidget_single_algorithm_setting.h"

CustomQWidgetSingleAlgorithmSetting::CustomQWidgetSingleAlgorithmSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetSingleAlgorithmSetting)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQWidgetSingleAlgorithmSetting::~CustomQWidgetSingleAlgorithmSetting()
{
    delete ui;
}

void CustomQWidgetSingleAlgorithmSetting::updateAlgorithmEffect(QString algorithm_effect)
{
    cl_text_algorithm_effect_->setText(algorithm_effect);
    switch (stringToAlgorithmType(algorithm_effect)) {
    case AlgorithmType::FootstepEnhance: {
        // 脚步增强处理
        cl_icon_algorithm_type_->setStyleSheet(R"(

QLabel
{
    border-image: url(:/Skin/Images/Headphones/Algo/Footsteps-no.png);
}
QLabel:disabled
{
    border-image: url(:/Skin/Images/Headphones/Algo/Footsteps-di.png);
}

)");

        break;
    }
    case AlgorithmType::GunshotWeakening: {
        // 枪声弱化处理
        cl_icon_algorithm_type_->setStyleSheet(R"(

QLabel
{
    border-image: url(:/Skin/Images/Headphones/Algo/Gunshot-no.png);
}
QLabel:disabled
{
    border-image: url(:/Skin/Images/Headphones/Algo/Gunshot-di.png);
}

)");
        break;
    }

    case AlgorithmType::SoundFieldControl: {
        // 声场控制处理
        cl_icon_algorithm_type_->setStyleSheet(R"(

QLabel
{
    border-image: url(:/Skin/Images/Headphones/Algo/sfc-no.png);
}
QLabel:disabled
{
    border-image: url(:/Skin/Images/Headphones/Algo/sfc-di.png);
}

)");
        break;
    }

    case AlgorithmType::Clarity: {
        // 清晰度处理
        cl_icon_algorithm_type_->setStyleSheet(R"(
QLabel
{
    border-image: url(:/Skin/Images/Headphones/Algo/Clarity-no.png);
}
QLabel:disabled
{
    border-image: url(:/Skin/Images/Headphones/Algo/Clarity-di.png);
}
)");
        break;
    }
    case AlgorithmType::Unknown: {
        // qWarning() << "未知算法类型:" << algorithm_effect;
        break;
    }
    }
}

void CustomQWidgetSingleAlgorithmSetting::updateValueRange(int min_value, int max_value)
{
    cl_minValue_ = min_value;
    cl_maxValue_ = max_value;
    cl_algorithm_value_hSlider_->setRange(cl_minValue_, cl_maxValue_);
}

void CustomQWidgetSingleAlgorithmSetting::updateValue(int value)
{
    cl_text_algorithm_value_->setText(QString::number(value));
    cl_algorithm_value_hSlider_->setValue(value);
}

void CustomQWidgetSingleAlgorithmSetting::setEditStatus(bool status)
{
    // 特殊处理
    if (status) {

    } else {
    }

    {
        // 通用
        cl_add_algorithm_value_->setEnabled(status);
        cl_del_algorithm_value_->setEnabled(status);
        cl_algorithm_value_hSlider_->setEnabled(status);
        cl_icon_algorithm_type_->setEnabled(status);
    }

}

void CustomQWidgetSingleAlgorithmSetting::InitUIInformation()
{
    {
        // 算法类型图标
        cl_icon_algorithm_type_ = new QLabel(this);
        cl_icon_algorithm_type_->setFixedSize(cl_icon_algorithm_type_size_);
        cl_icon_algorithm_type_->move(cl_icon_algorithm_type_point_);
        cl_icon_algorithm_type_->setStyleSheet(R"(

QLabel
{
    border-image: url(:/Skin/Images/Headphones/Algo/Footsteps-no.png);
}
QLabel:disabled
{
    border-image: url(:/Skin/Images/Headphones/Algo/Footsteps-di.png);
}

)");
    }
    {
        // 算法效果文字
        cl_text_algorithm_effect_ = new QLabel(tr("算法效果"), this);
        cl_text_algorithm_effect_->setFixedSize(cl_text_algorithm_effect_size_);
        cl_text_algorithm_effect_->move(cl_text_algorithm_effect_point_);
        cl_text_algorithm_effect_->setStyleSheet(R"(

        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 12px;
        color: #A1A8B3;
)");
    }
    {
        // 算法数值文字
        cl_text_algorithm_value_ = new QLabel(this);
        cl_text_algorithm_value_->setFixedSize(cl_text_algorithm_value_size_);
        cl_text_algorithm_value_->move(cl_text_algorithm_value_point_);
        cl_text_algorithm_value_->setAlignment(Qt::AlignCenter);
        cl_text_algorithm_value_->setStyleSheet(R"(
            font-family: "YouSheBiaoTiHei";
            font-weight: 500;
            font-size: 20px;
            color: #009FEF;
        )");
    }
    {
        // 削弱按钮
        cl_del_algorithm_value_ = new QPushButton(this);
        cl_del_algorithm_value_->setFixedSize(cl_del_algorithm_value_size_);
        cl_del_algorithm_value_->move(cl_del_algorithm_value_point_);
        cl_del_algorithm_value_->setCursor(Qt::PointingHandCursor); /// 手型光标
        cl_del_algorithm_value_->setStyleSheet(R"(
QPushButton
{
    background: rgba(0, 0, 0, 0.3);
    border-radius: 2px;
    border-image: url(:/Skin/Images/Slider/sub-no.png);
}
QPushButton:hover
{
    border-image: url(:/Skin/Images/Slider/sub-ho.png);
}
QPushButton:pressed
{
    border-image: url(:/Skin/Images/Slider/sub-ch.png);
}
QPushButton:disabled
{
    border-image: url(:/Skin/Images/Slider/sub-dis.png);
}
    )");
    }
    {
        // 水平滑动条
        cl_algorithm_value_hSlider_ = new GearSlider(this);
        cl_algorithm_value_hSlider_->setMinimumSize(cl_algorithm_value_hSlider__size_);
        cl_algorithm_value_hSlider_->setCursor(Qt::PointingHandCursor); /// 手型光标
        cl_algorithm_value_hSlider_->move(cl_algorithm_value_hSlider_point_);
        cl_algorithm_value_hSlider_->setRange(cl_minValue_, cl_maxValue_);
        cl_algorithm_value_hSlider_->setValue(0); //默认值 0
        cl_algorithm_value_hSlider_->show();
    }
    {
        // 增加按钮
        cl_add_algorithm_value_ = new QPushButton(this);
        cl_add_algorithm_value_->setFixedSize(cl_add_algorithm_value_size_);
        cl_add_algorithm_value_->move(cl_add_algorithm_value_point_);
        cl_add_algorithm_value_->setCursor(Qt::PointingHandCursor); /// 手型光标
        cl_add_algorithm_value_->setStyleSheet(R"(
QPushButton
{
    background: rgba(0, 0, 0, 0.3);
    border-radius: 2px;
    border-image: url(:/Skin/Images/Slider/add-no.png);
}
QPushButton:hover
{
    border-image: url(:/Skin/Images/Slider/add-ho.png);
}
QPushButton:pressed
{
    border-image: url(:/Skin/Images/Slider/add-ch.png);
}
QPushButton:disabled
{
    border-image: url(:/Skin/Images/Slider/add-dis.png);
}
    )");
    }
}

void CustomQWidgetSingleAlgorithmSetting::InitMember() {}

void CustomQWidgetSingleAlgorithmSetting::InitConnect()
{


    // 减少数值
    QObject::connect(cl_del_algorithm_value_, &QPushButton::clicked, this, [=]() {

            int newVal = cl_algorithm_value_hSlider_->value() - 1;
            newVal = qBound(cl_algorithm_value_hSlider_->minimum(), newVal, cl_algorithm_value_hSlider_->maximum());
            cl_algorithm_value_hSlider_->setValue(newVal);
            cl_text_algorithm_value_->setText(QString::number(newVal));
    });



    // connect(pair.subBtn, &QPushButton::clicked, this, [this, slider = pair.slider]() {
    //     if (!slider) return;
    //     int newVal = slider->value() - 1;
    //     newVal = qBound(slider->minimum(), newVal, slider->maximum());
    //     slider->setValue(newVal);
    // });
    // connect(pair.addBtn, &QPushButton::clicked, this, [this, slider = pair.slider]() {
    //     if (!slider) return;
    //     int newVal = slider->value() + 1;
    //     newVal = qBound(slider->minimum(), newVal, slider->maximum());
    //     slider->setValue(newVal);
    // });

    // 增加数值
    QObject::connect(cl_add_algorithm_value_, &QPushButton::clicked, this, [=]() {
        int newVal = cl_algorithm_value_hSlider_->value() + 1;
        newVal = qBound(cl_algorithm_value_hSlider_->minimum(), newVal, cl_algorithm_value_hSlider_->maximum());
        cl_algorithm_value_hSlider_->setValue(newVal);
        cl_text_algorithm_value_->setText(QString::number(newVal));
    });
}

AlgorithmType CustomQWidgetSingleAlgorithmSetting::stringToAlgorithmType(const QString &str)
{
    if (str == tr("脚步增强"))
        return AlgorithmType::FootstepEnhance;
    if (str == tr("枪声弱化"))
        return AlgorithmType::GunshotWeakening;
    if (str == tr("声场控制"))
        return AlgorithmType::SoundFieldControl;
    if (str == tr("清晰度"))
        return AlgorithmType::Clarity;
    return AlgorithmType::Unknown;
}

void CustomQWidgetSingleAlgorithmSetting::resizeEvent(QResizeEvent *event)
{
    {
        // 算法类型图标
        cl_icon_algorithm_type_->setGeometry(0,
                                             0,
                                             cl_icon_algorithm_type_size_.width(),
                                             cl_icon_algorithm_type_size_.height());
    }
    {
        // 算法效果文字
        cl_text_algorithm_effect_->setGeometry(32,
                                               0,
                                               cl_text_algorithm_effect_size_.width(),
                                               cl_text_algorithm_effect_size_.height());
    }
    {
        // 算法数值文字
        cl_text_algorithm_value_->setGeometry(rect().width() - 20,
                                             0,
                                             cl_text_algorithm_value_size_.width(),
                                             cl_text_algorithm_value_size_.height());

        cl_text_algorithm_value_->setAlignment(Qt::AlignCenter);
    }
    {
        // 削弱按钮
        cl_del_algorithm_value_->setGeometry(0,
                                             28,
                                             cl_del_algorithm_value_size_.width(),
                                             cl_del_algorithm_value_size_.height());
    }
    {
        // 水平滑动条
        cl_algorithm_value_hSlider_->setGeometry(32,
                                                 28,
                                                 rect().width() - 64,
                                                 cl_algorithm_value_hSlider__size_.height());
    }
    {
        // 增加按钮
        cl_add_algorithm_value_->setGeometry(rect().width() - 20,
                                             28,
                                             cl_add_algorithm_value_size_.width(),
                                             cl_add_algorithm_value_size_.height());
    }

    QWidget::resizeEvent(event);
}