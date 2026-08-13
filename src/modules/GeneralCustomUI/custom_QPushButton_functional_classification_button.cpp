#include "modules/GeneralCustomUI/custom_QPushButton_functional_classification_button.h"

#include <QFontMetrics>

/// \brief 构造函数
CustomQPushButtonFunctionalClassificationButton::CustomQPushButtonFunctionalClassificationButton(
    QWidget *parent)
    : QPushButton(parent)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

/// \brief 初始化UI的默认信息
void CustomQPushButtonFunctionalClassificationButton::InitUIInformation()
{
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

/// \brief 初始化内部成员
void CustomQPushButtonFunctionalClassificationButton::InitMember()
{
    setMinimumSize(cl_min_size_);
    cl_icon_current_point_ = cl_icon_default_point_;
    cl_icon_size_prop_ = cl_icon_default_size_.width();
    cl_bg_current_ = cl_bg_default_;

    // 图标位移动画
    cl_icon_anim_ = new QPropertyAnimation(this, "cl_icon_point", this);
    cl_icon_anim_->setDuration(200);
    cl_icon_anim_->setEasingCurve(QEasingCurve::OutQuad);

    // 文字透明度动画
    cl_text_anim_ = new QPropertyAnimation(this, "cl_text_opacity", this);
    cl_text_anim_->setDuration(200);
    cl_text_anim_->setEasingCurve(QEasingCurve::OutQuad);

    // 图标尺寸动画
    cl_icon_size_anim_ = new QPropertyAnimation(this, "cl_icon_size_prop", this);
    cl_icon_size_anim_->setDuration(200);
    cl_icon_size_anim_->setEasingCurve(QEasingCurve::OutQuad);

    // 背景色渐变动画
    cl_bg_anim_ = new QPropertyAnimation(this, "cl_bg_current", this);
    cl_bg_anim_->setDuration(200);
    cl_bg_anim_->setEasingCurve(QEasingCurve::OutQuad);
}

/// \brief 连接默认的信号槽
void CustomQPushButtonFunctionalClassificationButton::InitConnect() {}

void CustomQPushButtonFunctionalClassificationButton::setCl_classification_name(
    const QString &name)
{
    cl_classification_name_ = name;
    update();
}

QString CustomQPushButtonFunctionalClassificationButton::cl_classification_name() const
{
    return cl_classification_name_;
}

void CustomQPushButtonFunctionalClassificationButton::setCl_pixmap(const QPixmap &pixmap)
{
    cl_pixmap_original_ = pixmap;
    cl_pixmap_ = pixmap.scaled(cl_icon_target_size_, Qt::KeepAspectRatio,
                               Qt::SmoothTransformation);
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_size(const QSize &defaultSize,
                                                                        const QSize &targetSize)
{
    setCl_icon_default_size(defaultSize);
    setCl_icon_target_size(targetSize);
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_point(const QPoint &defaultPoint,
                                                                        const QPoint &targetPoint)
{
    setCl_icon_default_point(defaultPoint);
    setCl_icon_target_point(targetPoint);
}

void CustomQPushButtonFunctionalClassificationButton::setCl_border_radius(int radius)
{
    cl_border_radius_ = radius;
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_text_spacing(int spacing)
{
    cl_icon_text_spacing_ = spacing;
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_min_size(const QSize &size)
{
    cl_min_size_ = size;
    setMinimumSize(size);
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_default_size(const QSize &size)
{
    cl_icon_default_size_ = size;
    cl_icon_size_prop_ = size.width();
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_target_size(const QSize &size)
{
    cl_icon_target_size_ = size;
    if (!cl_pixmap_original_.isNull()) {
        cl_pixmap_ = cl_pixmap_original_.scaled(size, Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation);
    }
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_default_point(const QPoint &point)
{
    cl_icon_default_point_ = point;
    cl_icon_current_point_ = point;
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_icon_target_point(const QPoint &point)
{
    cl_icon_target_point_ = point;
    update();
}

/// \brief 颜色叠加：将 overlay 叠在 base 上
QColor CustomQPushButtonFunctionalClassificationButton::blendColors(const QColor &base,
                                                                     const QColor &overlay) const
{
    int a1 = overlay.alpha();
    int a0 = 255 - a1;
    int r = (overlay.red()   * a1 + base.red()   * a0) / 255;
    int g = (overlay.green() * a1 + base.green() * a0) / 255;
    int b = (overlay.blue()  * a1 + base.blue()  * a0) / 255;
    return QColor(r, g, b, qMin(255, base.alpha() + a1 * (255 - base.alpha()) / 255));
}

void CustomQPushButtonFunctionalClassificationButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing, true);

    // 背景色（动画过渡）
    t_painter.setPen(Qt::NoPen);
    t_painter.setBrush(cl_bg_current_);
    t_painter.drawRoundedRect(rect(), cl_border_radius_, cl_border_radius_);

    // 遮罩层（背景之上、图标/文字之下，不覆盖图标和文字）
    // 20% 黑遮罩
    {
        const QColor t_mask_color(0, 0, 0, 51);
        t_painter.setBrush(t_mask_color);
        const int t_mask_size = 72;
        QRect t_mask_rect(0, 0, t_mask_size, t_mask_size);
        t_mask_rect.moveCenter(rect().center());
        t_painter.drawRoundedRect(t_mask_rect, cl_border_radius_, cl_border_radius_);
    }

    // 图标 — 动态尺寸
    int t_icon_w = cl_icon_size_prop_;
    int t_icon_h = cl_icon_default_size_.height();
    int t_width_delta = cl_icon_target_size_.width() - cl_icon_default_size_.width();
    if (t_width_delta > 0) {
        t_icon_h += (cl_icon_target_size_.height() - cl_icon_default_size_.height())
                    * (t_icon_w - cl_icon_default_size_.width()) / t_width_delta;
    }
    QRect t_icon_rect(0, 0, t_icon_w, t_icon_h);
    t_icon_rect.moveTopLeft(cl_icon_current_point_);

    if (!cl_pixmap_.isNull()) {
        t_painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        t_painter.drawPixmap(t_icon_rect, cl_pixmap_);
    }

    // 文字 — 始终水平居中，位于图标下方
    if (!cl_classification_name_.isEmpty() && cl_text_opacity_ > 0.01) {
        QFont t_font("Noto Sans S Chinese", 10);
        t_painter.setFont(t_font);
        t_painter.setOpacity(cl_text_opacity_);

        QFontMetrics t_fm(t_font);
        int t_text_w = t_fm.horizontalAdvance(cl_classification_name_) + 10;
        QRect t_text_rect(0, 0, t_text_w, 16);
        t_text_rect.moveTopLeft(QPoint(rect().center().x() - t_text_rect.width() / 2,
                                        t_icon_rect.bottom() + cl_icon_text_spacing_));

        t_painter.setPen(QColor("#A1A8B3"));
        t_painter.drawText(t_text_rect, Qt::AlignCenter, cl_classification_name_);
        t_painter.setOpacity(1.0);
    }
}

void CustomQPushButtonFunctionalClassificationButton::enterEvent(QEvent *event)
{
    cl_icon_anim_->stop();
    cl_icon_anim_->setStartValue(cl_icon_current_point_);
    cl_icon_anim_->setEndValue(cl_icon_target_point_);
    cl_icon_anim_->start();

    cl_icon_size_anim_->stop();
    cl_icon_size_anim_->setStartValue(cl_icon_size_prop_);
    cl_icon_size_anim_->setEndValue(cl_icon_target_size_.width());
    cl_icon_size_anim_->start();

    cl_text_anim_->stop();
    cl_text_anim_->setStartValue(cl_text_opacity_);
    cl_text_anim_->setEndValue(1.0);
    cl_text_anim_->start();

    cl_bg_anim_->stop();
    cl_bg_anim_->setStartValue(cl_bg_current_);
    cl_bg_anim_->setEndValue(blendColors(cl_bg_default_, cl_bg_hover_));
    cl_bg_anim_->start();

    QPushButton::enterEvent(event);
}

void CustomQPushButtonFunctionalClassificationButton::leaveEvent(QEvent *event)
{
    cl_icon_anim_->stop();
    cl_icon_anim_->setStartValue(cl_icon_current_point_);
    cl_icon_anim_->setEndValue(cl_icon_default_point_);
    cl_icon_anim_->start();

    cl_icon_size_anim_->stop();
    cl_icon_size_anim_->setStartValue(cl_icon_size_prop_);
    cl_icon_size_anim_->setEndValue(cl_icon_default_size_.width());
    cl_icon_size_anim_->start();

    cl_text_anim_->stop();
    cl_text_anim_->setStartValue(cl_text_opacity_);
    cl_text_anim_->setEndValue(0.0);
    cl_text_anim_->start();

    cl_bg_anim_->stop();
    cl_bg_anim_->setStartValue(cl_bg_current_);
    cl_bg_anim_->setEndValue(cl_bg_default_);
    cl_bg_anim_->start();

    QPushButton::leaveEvent(event);
}

void CustomQPushButtonFunctionalClassificationButton::setCl_bg_default_color(
    const QColor &color)
{
    cl_bg_default_ = color;
    cl_bg_current_ = color;
    update();
}

void CustomQPushButtonFunctionalClassificationButton::setCl_bg_hover_color(const QColor &color)
{
    cl_bg_hover_ = color;
    update();
}

//设置背景色
void CustomQPushButtonFunctionalClassificationButton::setBackground(QColor colorStr)
{
    cl_bg_default_ = colorStr; ///< 默认背景色
    cl_bg_current_ = colorStr; ///< 当前背景色（动画过渡）
    repaint();//重绘
}
