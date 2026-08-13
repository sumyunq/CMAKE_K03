#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QPushButton_roundbutton.h"

CustomQPushButtonRoundButton::CustomQPushButtonRoundButton(QWidget *parent)
    : QPushButton(parent)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQPushButtonRoundButton::CustomQPushButtonRoundButton(QWidget *parent,
                                                           const QColor &defaultColor,
                                                           const QColor &hoverColor,
                                                           const QColor &checkedColor)
    : QPushButton(parent)
    , cl_default_color_(defaultColor)
    , cl_hover_color_(checkedColor)
    , cl_checked_color_(checkedColor)
{
    {
        InitUIInformation(); ///< 初始化UI的默认信息
        InitMember();        ///< 初始化内部成员
        InitConnect();       ///< 连接默认的信号槽
    }
}

CustomQPushButtonRoundButton::~CustomQPushButtonRoundButton() {}

void CustomQPushButtonRoundButton::InitUIInformation()
{
    {
        // 自身属性
        setFixedSize(cl_pushbutton_min_size_);   // 固定尺寸
        setCursor(Qt::PointingHandCursor);       // 手型光标
        setCheckable(true);                      // 可选中
        setStyleSheet(R"(
        QPushButton {
            border-radius: 4px;
        }
        QPushButton::tooltip {
            background-color: #4CAF50;
            color: white;
            border-radius: 4px;
        }
    )");
    }
}

void CustomQPushButtonRoundButton::InitMember()
{
    {
        cl_size_anim_ = std::make_unique<QVariantAnimation>(this);
        cl_size_anim_->setDuration(200); // 渐变时间
    }
    {
        cl_color_anim_ = std::make_unique<QVariantAnimation>(this);
        cl_color_anim_->setDuration(200); // 渐变时间
    }
    {
        cl_parallel_group_ = new QParallelAnimationGroup(this);
        // 添加动画（同时执行）
        cl_parallel_group_->addAnimation(cl_size_anim_.get());
        cl_parallel_group_->addAnimation(cl_color_anim_.get());
    }

}

void CustomQPushButtonRoundButton::InitConnect()
{
    // 尺寸
    connect(cl_size_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_current_size_ = value.value<QSize>();
                update();
            });
    // 颜色
    connect(cl_color_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_current_color_ = value.value<QColor>();
                update();
            });
    // 按键状态变化时
    connect(this, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            cl_size_anim_->setKeyValues({{0.0, cl_current_size_}, {1.0, cl_checked_size_}});
            cl_color_anim_->setKeyValues({{0.0, cl_default_color_}, {1.0, cl_checked_color_}});

        } else {
            cl_size_anim_->setKeyValues({{0.0, cl_current_size_}, {1.0, cl_default_size_}});
            cl_color_anim_->setKeyValues({{0.0, cl_checked_color_}, {1.0, cl_default_color_}});
        }
        // 启动动画
        cl_parallel_group_->start();
    });
}

void CustomQPushButtonRoundButton::drawCheckedStyle(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(cl_current_color_);
    painter.drawRoundedRect(rect(), 4, 4);

    painter.restore();
}

void CustomQPushButtonRoundButton::drawUnCheckedStyle(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(cl_current_color_);
    painter.drawRoundedRect(rect(), 4, 4);
    painter.restore();
}

void CustomQPushButtonRoundButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取按钮的状态（按下、悬停、选中、禁用等）
    // bool is_Pressed = isDown();
    // bool is_Hovered = underMouse();
    bool is_Checked = isCheckable() && isChecked(); // 选中
    // bool is_Enabled = this->isEnabled();

    // 选中状态
    if (is_Checked) {
        drawCheckedStyle(painter);
    } else {
        drawUnCheckedStyle(painter);
    }
}

void CustomQPushButtonRoundButton::enterEvent(QEvent *event)
{
    // 如果处于选中
    if (isCheckable() && isChecked()) {
        return;
    }
    // 非选中状态，进入悬停态
    cl_size_anim_->setKeyValues({{0.0, cl_current_size_}, {1.0, cl_hover_size_}});
    cl_color_anim_->setKeyValues({{0.0, cl_default_color_}, {1.0, cl_hover_color_}});
    // 启动动画
    cl_parallel_group_->start();
    QPoint globalCenter = this->mapToGlobal(this->rect().center());
    // emit hoverCentrePositon(globalCenter);

    // QToolTip::showText(globalCenter, cl_device_info_->DeviceColorName, this);

    QPushButton::enterEvent(event);
}

void CustomQPushButtonRoundButton::leaveEvent(QEvent *event)
{
    // 如果处于选中
    if (isCheckable() && isChecked()) {
        return;
    }
    // 非选中状态，进入默认态

    cl_size_anim_->setKeyValues({{0.0, cl_current_size_}, {1.0, cl_default_size_}});
    cl_color_anim_->setKeyValues({{0.0, cl_hover_color_}, {1.0, cl_default_color_}});
    // 启动动画
    cl_parallel_group_->start();
    // QToolTip::hideText();
    QPushButton::leaveEvent(event);
}
