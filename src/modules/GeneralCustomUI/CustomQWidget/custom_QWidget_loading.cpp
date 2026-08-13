#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_loading.h"

#include <QConicalGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QTimer>

CustomQWidgetLoading::CustomQWidgetLoading(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQWidgetLoading::~CustomQWidgetLoading() {}

void CustomQWidgetLoading::InitUIInformation(int theme)
{
    {
        setObjectName("CustomQWidgetLoading");
        setMinimumSize(20, 20);
    }
    applyTheme(theme);
}

void CustomQWidgetLoading::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    }
}

void CustomQWidgetLoading::InitMember()
{
    clp_timer_ = new QTimer(this);
    clp_timer_->setTimerType(Qt::PreciseTimer);
}

void CustomQWidgetLoading::InitConnect()
{
    connect(clp_timer_, &QTimer::timeout, this, &CustomQWidgetLoading::tick);
}

void CustomQWidgetLoading::setCl_config(const CustomQWidgetLoadingConfig &cfg)
{
    cl_cfg_ = cfg;
    updateGeometry();
    update();
}

CustomQWidgetLoadingConfig CustomQWidgetLoading::cl_config() const
{
    return cl_cfg_;
}

void CustomQWidgetLoading::setCl_text(const QString &text)
{
    if (cl_cfg_.text == text) return;
    cl_cfg_.text = text;
    updateGeometry();
    update();
}

void CustomQWidgetLoading::setCl_arc_color(const QColor &color)
{
    cl_cfg_.arc_head_color = color;
    QColor t_tail = color;
    t_tail.setAlpha(0);
    cl_cfg_.arc_tail_color = t_tail;
    update();
}

void CustomQWidgetLoading::start()
{
    cl_angle_ = 0;
    clp_timer_->start(cl_cfg_.period_ms / 20);
}

void CustomQWidgetLoading::stop()
{
    clp_timer_->stop();
}

bool CustomQWidgetLoading::cl_running() const
{
    return clp_timer_->isActive();
}

QSize CustomQWidgetLoading::sizeHint() const
{
    if (cl_cfg_.arc_rect.isValid() || cl_cfg_.text_rect.isValid()) {
        QRect t_r = cl_cfg_.arc_rect.isValid() ? cl_cfg_.arc_rect : QRect(0, 0, 24, 24);
        QRect t_tr = cl_cfg_.text_rect.isValid() ? cl_cfg_.text_rect : QRect();
        return QRect(t_r | t_tr).size();
    }
    int t_d = cl_cfg_.radius > 0 ? cl_cfg_.radius * 2 : 24;
    return QSize(t_d, t_d);
}

void CustomQWidgetLoading::tick()
{
    if (cl_cfg_.clockwise)
        cl_angle_ -= 18.0;
    else
        cl_angle_ += 18.0;
    if (cl_angle_ < 0.0)    cl_angle_ += 360.0;
    if (cl_angle_ >= 360.0) cl_angle_ -= 360.0;
    update();
}

void CustomQWidgetLoading::paintEvent(QPaintEvent * /*event*/)
{
    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing);

    // 裁剪
    if (cl_cfg_.clip_radius > 0) {
        QPainterPath t_path;
        t_path.addRoundedRect(rect(), cl_cfg_.clip_radius, cl_cfg_.clip_radius);
        t_painter.setClipPath(t_path);
    }

    // 背景
    if (cl_cfg_.bg_color.alpha() > 0)
        t_painter.fillRect(rect(), cl_cfg_.bg_color);

    const int t_w = width();
    const int t_h = height();
    const int t_margin = static_cast<int>(cl_cfg_.arc_width);
    const bool t_has_text = cl_cfg_.text_visible && !cl_cfg_.text.isEmpty();

    QRectF t_arc_rect;
    QRectF t_text_rect;

    if (!t_has_text) {
        t_arc_rect = QRectF(t_margin, t_margin,
                            t_w - 2 * t_margin, t_h - 2 * t_margin);
    } else {
        t_text_rect = cl_cfg_.text_rect.isValid()
                          ? QRectF(cl_cfg_.text_rect)
                          : QRectF(0, 0,
                                   fontMetrics().horizontalAdvance(cl_cfg_.text) + 4, t_h);

        QRectF t_ar = cl_cfg_.arc_rect.isValid()
                          ? QRectF(cl_cfg_.arc_rect)
                          : QRectF(t_text_rect.right() + 4, 0,
                                   qMin(t_w - static_cast<int>(t_text_rect.right()) - 4, t_h),
                                   qMin(t_w - static_cast<int>(t_text_rect.right()) - 4, t_h));

        t_arc_rect = QRectF(t_ar.x() + t_margin, t_ar.y() + t_margin,
                            t_ar.width() - 2 * t_margin, t_ar.height() - 2 * t_margin);
    }

    // 弧线
    if (t_arc_rect.width() > 0) {
        QConicalGradient t_grad(t_arc_rect.center(), cl_angle_ + 90);
        t_grad.setColorAt(0.0, cl_cfg_.arc_head_color);
        qreal t_ratio = cl_cfg_.arc_span / 360.0;
        t_grad.setColorAt(t_ratio, cl_cfg_.arc_tail_color);
        t_grad.setColorAt(1.0, cl_cfg_.arc_tail_color);

        QPen t_pen(QBrush(t_grad), cl_cfg_.arc_width, Qt::SolidLine, Qt::RoundCap);
        t_painter.setPen(t_pen);
        int t_span = cl_cfg_.arc_span * 16;
        if (cl_cfg_.clockwise) t_span = -t_span;

        t_painter.drawArc(t_arc_rect, static_cast<int>(cl_angle_ * 16), t_span);
    }

    // 文字
    if (t_has_text) {
        t_painter.setPen(cl_cfg_.text_color);
        t_painter.drawText(t_text_rect, Qt::AlignCenter, cl_cfg_.text);
    }
}
