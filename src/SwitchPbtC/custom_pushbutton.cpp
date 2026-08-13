#include "SwitchPbtC/custom_pushbutton.h"

CustomPushButton::CustomPushButton(QWidget *parent)
    : QPushButton(parent)
{
    InitUIInformation();
    InitMember();
    InitConnect();
}

CustomPushButton::~CustomPushButton() {}

void CustomPushButton::InitUIInformation()
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(0, 0);          // 允许缩小到 0
    setCheckable(true);           // 使用 QPushButton 自带的 checked 状态
}

void CustomPushButton::InitMember()
{
    cl_animation_ = new QPropertyAnimation(this, "cl_sliderOffset_");
    cl_animation_->setDuration(200);
    cl_animation_->setEasingCurve(QEasingCurve::InOutCubic);
}

void CustomPushButton::InitConnect()
{
    // checked 状态切换时启动动画并转发信号
    connect(this, &QPushButton::toggled, this, &CustomPushButton::onToggled);
}

void CustomPushButton::setZoomFactor(double factor)
{
    if (qFuzzyCompare(cl_zoom_factor_, factor)) return;
    cl_zoom_factor_ = factor;
    recalculateSizes();
    update();
}

void CustomPushButton::recalculateSizes()
{
    double widthRatio = static_cast<double>(width()) / BASE_WIDTH;
    double heightRatio = static_cast<double>(height()) / BASE_HEIGHT;
    double scale = qMin(widthRatio, heightRatio) * cl_zoom_factor_;

    cl_actualRadius_ = qRound(BASE_RADIUS * scale);
    cl_actualMargin_ = qRound(BASE_MARGIN * scale);
}

void CustomPushButton::onToggled(bool checked)
{
    if (cl_system_style_ == SystemStyle::ANIMATION && cl_animation_) {
        cl_animation_->stop();
        cl_animation_->setStartValue(cl_sliderOffset_);
        cl_animation_->setEndValue(checked ? 1.0 : 0.0);
        cl_animation_->start();
    } else {
        cl_sliderOffset_ = checked ? 1.0 : 0.0;
        update();
    }
    emit cl_toggled(checked);   // 对外转发信号
}
void CustomPushButton::setChecked(bool checked) {
    // 调用基类实现，改变按钮状态
    QAbstractButton::setChecked(checked);

    // 当信号被阻塞时，手动同步动画状态
    if (signalsBlocked()) {
        // 动画模式
        if (cl_system_style_ == SystemStyle::ANIMATION && cl_animation_) {
            // 停止当前正在运行的动画
            if (cl_animation_->state() == QAbstractAnimation::Running) {
                cl_animation_->stop();
            }
            // 设置动画的起始值和结束值，并启动动画
            cl_animation_->setStartValue(cl_sliderOffset_);
            cl_animation_->setEndValue(checked ? 1.0 : 0.0);
            cl_animation_->start();
        } else {
            // 非动画模式，直接设置
            cl_sliderOffset_ = checked ? 1.0 : 0.0;
            update();
        }
    }
}
void CustomPushButton::setCl_system_style(SystemStyle newCl_system_style)
{
    cl_system_style_ = newCl_system_style;
    update();
}

void CustomPushButton::setCl_pixmapOn(const QPixmap &newCl_pixmapOn)
{
    cl_pixmapOn_ = newCl_pixmapOn;
    update();
}

void CustomPushButton::setCl_pixmapOff(const QPixmap &newCl_pixmapOff)
{
    cl_pixmapOff_ = newCl_pixmapOff;
    update();
}

void CustomPushButton::drawAnimationStyle(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    int w = width();
    int h = height();

    if (w <= 0 || h <= 0) return;

    double outerRadius = h / 2.0;

    // 直接使用 QPushButton 的 isChecked() 判断开关状态
    if (isChecked()) {
        painter.setBrush(QColor("#0091DA"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rect(), outerRadius, outerRadius);

        double offset = cl_sliderOffset_ * (w - 2 * cl_actualMargin_ - 2 * cl_actualRadius_);
        int cx = cl_actualMargin_ + cl_actualRadius_ + static_cast<int>(offset);
        int cy = h / 2;

        painter.setBrush(QColor("#D8D8D8"));
        painter.drawEllipse(QPoint(cx, cy), cl_actualRadius_, cl_actualRadius_);
    } else {
        painter.setPen(QPen(QColor("#697081"), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), outerRadius, outerRadius);

        double offset = cl_sliderOffset_ * (w - 2 * cl_actualMargin_ - 2 * cl_actualRadius_);
        double cx = cl_actualMargin_ + cl_actualRadius_ + static_cast<int>(offset) + 0.5;
        int cy = h / 2;

        painter.setBrush(QColor("#697081"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(cx, cy + 0.5), cl_actualRadius_, cl_actualRadius_);
    }
    painter.restore();
}

void CustomPushButton::drawSimpleStyle(QPainter &painter)
{
    const QPixmap &pix = isChecked() ? cl_pixmapOn_ : cl_pixmapOff_;
    if (!pix.isNull()) {
        painter.drawPixmap(rect(), pix);
    } else {
        painter.fillRect(rect(), isChecked() ? QColor("#0091c6") : QColor("#697081"));
        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, isChecked() ? "ON" : "OFF");
    }
}

double CustomPushButton::sliderOffset() const
{
    return cl_sliderOffset_;
}

void CustomPushButton::setSliderOffset(double offset)
{
    cl_sliderOffset_ = offset;
    update();
}

void CustomPushButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    switch (cl_system_style_) {
    case SystemStyle::ANIMATION:
        drawAnimationStyle(painter);
        break;
    case SystemStyle::SIMPLE:
    default:
        drawSimpleStyle(painter);
        break;
    }
}

void CustomPushButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    recalculateSizes();
    update();
}
