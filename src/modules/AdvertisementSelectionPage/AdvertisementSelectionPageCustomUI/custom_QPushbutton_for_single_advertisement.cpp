#include "modules/AdvertisementSelectionPage/AdvertisementSelectionPageCustomUI/custom_QPushbutton_for_single_advertisement.h"

CustomQPushButtonForSingleAdvertisement::CustomQPushButtonForSingleAdvertisement(QWidget *parent)
    : QPushButton(parent)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQPushButtonForSingleAdvertisement::~CustomQPushButtonForSingleAdvertisement() {}

void CustomQPushButtonForSingleAdvertisement::InitUIInformation()
{
    {
        // 自身属性
        setFixedSize(cl_current_size_);
        setCursor(Qt::PointingHandCursor);       // 手型光标
        setCheckable(true);                      // 可选中
        setFlat(true);
    }
}

QSize CustomQPushButtonForSingleAdvertisement::sizeHint() const
{
    return cl_current_size_;
}

void CustomQPushButtonForSingleAdvertisement::InitMember()
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

void CustomQPushButtonForSingleAdvertisement::InitConnect()
{
    // 尺寸
    connect(cl_size_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_current_size_ = value.value<QSize>();
                setFixedSize(cl_current_size_);
                updateGeometry();
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
            cl_size_anim_->setKeyValues({{0.0, cl_default_size_}, {1.0, cl_checked_size_}});
            cl_color_anim_->setKeyValues({{0.0, cl_default_color_}, {1.0, cl_checked_color_}});

        } else {
            cl_size_anim_->setKeyValues({{0.0, cl_checked_size_}, {1.0, cl_default_size_}});
            cl_color_anim_->setKeyValues({{0.0, cl_checked_color_}, {1.0, cl_default_color_}});
        }
        // 启动动画
        cl_parallel_group_->start();
    });
}

void CustomQPushButtonForSingleAdvertisement::drawCheckedStyle(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(cl_current_color_);
    painter.drawRoundedRect(QRect(QPoint(rect().center().x() - cl_current_size_.width() / 2, 0),
                                  cl_current_size_),
                            1,
                            1);

    painter.restore();
}

void CustomQPushButtonForSingleAdvertisement::drawUnCheckedStyle(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(cl_current_color_);
    painter.drawRoundedRect(QRect(QPoint(rect().center().x() - cl_current_size_.width() / 2, 0),
                                  cl_current_size_),
                            1,
                            1);
    painter.restore();
}

void CustomQPushButtonForSingleAdvertisement::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取按钮的状态（按下、悬停、选中、禁用等）
    bool is_Pressed = isDown();
    bool is_Hovered = underMouse();
    bool is_Checked = isCheckable() && isChecked();
    bool is_Enabled = this->isEnabled();

    // QColor bgColor;
    // if (!is_Enabled) {
    //     bgColor = QColor(200, 200, 200);
    // } else if (is_Pressed) {
    //     bgColor = QColor(100, 100, 100);
    // } else if (is_Hovered) {
    //     bgColor = QColor(180, 180, 180);
    // } else {
    //     bgColor = QColor(220, 220, 220);
    // }

    // painter.setPen(Qt::NoPen);
    // painter.setBrush(bgColor);
    // painter.drawRoundedRect(rect(), 8, 6);

    // painter.setPen(QPen(QColor(150, 150, 150), 1));
    // painter.setBrush(Qt::NoBrush);
    // painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 6, 6);

    // 选中状态
    if (is_Checked) {
        drawCheckedStyle(painter);
    } else {
        drawUnCheckedStyle(painter);
    }
}
