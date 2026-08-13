#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QTimer>
#include <QtMath>

CustomQWidgetTextBadge::CustomQWidgetTextBadge(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQWidgetTextBadge::~CustomQWidgetTextBadge() {}

void CustomQWidgetTextBadge::InitUIInformation(int theme)
{
    {
        setObjectName("CustomQWidgetTextBadge");
        cl_cfg_.font = font();
        cl_cfg_.font.setFamily("Noto Sans S Chinese");
        cl_cfg_.font.setPixelSize(10);
        cl_cfg_.font.setWeight(QFont::Medium);
        setFont(cl_cfg_.font);
        updateSizeFromText();
    }
    applyTheme(theme);
}

void CustomQWidgetTextBadge::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    case 1: {
    } break;
    case 2: {
    } break;
    }
}

void CustomQWidgetTextBadge::InitMember()
{
    clp_fire_timer_ = new QTimer(this);
    clp_fire_timer_->setTimerType(Qt::PreciseTimer);
    clp_fire_timer_->setInterval(50); // 默认 50ms ≈ 20fps
}

void CustomQWidgetTextBadge::InitConnect()
{
    connect(clp_fire_timer_, &QTimer::timeout, this, &CustomQWidgetTextBadge::onFireTick);
}

void CustomQWidgetTextBadge::updateSizeFromText()
{
    if (cl_cfg_.text.isEmpty()) {
        setFixedSize(0, 0);
        return;
    }

    const QFontMetrics t_fm(cl_cfg_.font);

    const int t_text_w = t_fm.horizontalAdvance(cl_cfg_.text);
    const int t_text_h = t_fm.height();

    const int t_w = t_text_w + cl_cfg_.padding_left + cl_cfg_.padding_right;
    const int t_h = t_text_h + cl_cfg_.padding_top + cl_cfg_.padding_bottom;

    setFixedSize(t_w, t_h);
}

// ── Getters ──

QString CustomQWidgetTextBadge::cl_text() const       { return cl_cfg_.text; }
QColor  CustomQWidgetTextBadge::cl_bg_color() const   { return cl_cfg_.bg_color; }
QColor  CustomQWidgetTextBadge::cl_text_color() const { return cl_cfg_.text_color; }
bool    CustomQWidgetTextBadge::cl_fire_enabled() const { return cl_fire_enabled_; }

// ── Setters ──

void CustomQWidgetTextBadge::setCl_text(const QString &text)
{
    if (cl_cfg_.text == text) return;
    cl_cfg_.text = text;
    updateSizeFromText();
    update();
}

void CustomQWidgetTextBadge::setCl_bg_color(const QColor &color)
{
    if (cl_cfg_.bg_color == color) return;
    cl_cfg_.bg_color = color;
    update();
}

void CustomQWidgetTextBadge::setCl_text_color(const QColor &color)
{
    if (cl_cfg_.text_color == color) return;
    cl_cfg_.text_color = color;
    update();
}

void CustomQWidgetTextBadge::setCl_font(const QFont &font)
{
    cl_cfg_.font = font;
    setFont(font);
    updateSizeFromText();
    update();
}

void CustomQWidgetTextBadge::setCl_radius(qreal radius)
{
    if (qFuzzyCompare(cl_cfg_.radius, radius)) return;
    cl_cfg_.radius = radius;
    update();
}

void CustomQWidgetTextBadge::setCl_padding(int left, int top, int right, int bottom)
{
    cl_cfg_.padding_left = left;
    cl_cfg_.padding_top = top;
    cl_cfg_.padding_right = right;
    cl_cfg_.padding_bottom = bottom;
    updateSizeFromText();
    update();
}

void CustomQWidgetTextBadge::setCl_alignment(Qt::Alignment align)
{
    if (cl_cfg_.alignment == align) return;
    cl_cfg_.alignment = align;
    update();
}

void CustomQWidgetTextBadge::setCl_fire_enabled(bool en)
{
    if (cl_fire_enabled_ == en) return;
    cl_fire_enabled_ = en;
    if (en && isVisible())
        clp_fire_timer_->start();
    else
        clp_fire_timer_->stop();
    update();
}

void CustomQWidgetTextBadge::setCl_fire_speed(int ms)
{
    clp_fire_timer_->setInterval(qMax(16, ms)); // 最低 16ms ≈ 60fps 上限
}

// ── Size Hints ──

QSize CustomQWidgetTextBadge::sizeHint() const
{
    return size();
}

QSize CustomQWidgetTextBadge::minimumSizeHint() const
{
    return QSize(0, 0);
}

// ── Events ──

void CustomQWidgetTextBadge::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateSizeFromText();
        update();
    }
    QWidget::changeEvent(event);
}

void CustomQWidgetTextBadge::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (cl_fire_enabled_)
        clp_fire_timer_->start();
}

void CustomQWidgetTextBadge::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    clp_fire_timer_->stop();
}

void CustomQWidgetTextBadge::onFireTick()
{
    cl_fire_phase_ += 0.15; // 每帧相位增量（不 wrap，qSin 可处理任意值，防止乘系数后跳变）
    update();
}

// ── Drawing ──

void CustomQWidgetTextBadge::drawSolidBg(QPainter &t_painter, const QRect &t_rect)
{
    t_painter.fillRect(t_rect, cl_cfg_.bg_color);
}

void CustomQWidgetTextBadge::drawFireBg(QPainter &t_painter, const QRect &t_rect)
{
    const int t_w = t_rect.width();
    const int t_h = t_rect.height();
    const qreal t_phase = cl_fire_phase_;

    // 正弦波叠加 — 四条波合成火焰边缘形状
    // 频率递增：低频 = 大朵火焰，高频 = 小朵火焰细节
    auto flameHeight = [&](qreal x01) -> qreal {
        const qreal PI = M_PI;
        return 0.55   * qSin(x01 * PI * 1.8  + t_phase)
             + 0.25   * qSin(x01 * PI * 3.3  + t_phase * 1.4  + 1.2)
             + 0.13   * qSin(x01 * PI * 6.1  + t_phase * 2.1  + 2.8)
             + 0.07   * qSin(x01 * PI * 11.0 + t_phase * 3.0);
    };

    // 火焰区域：底部到 "峰顶"
    // 用波形控制每个 x 处的火焰上界（0=控件底部，1=控件顶部）
    const int t_steps = t_w + 1;
    QPolygonF t_flame_region;
    t_flame_region.reserve(t_steps + 4); // 上边缘 + 两个底角

    // 上边缘 — 从左到右沿正弦波位移
    for (int t_i = 0; t_i <= t_w; ++t_i) {
        const qreal t_x01 = static_cast<qreal>(t_i) / t_w;
        const qreal t_dy = flameHeight(t_x01);             // -1~+1
        const qreal t_flame_top = t_dy * t_h * 0.75;      // 映射到像素
        t_flame_region << QPointF(t_rect.x() + t_i,
                                   t_rect.bottom() - (t_h * 0.4) + t_flame_top);
    }

    // 闭合到底部
    t_flame_region << QPointF(t_rect.right(), t_rect.bottom());
    t_flame_region << QPointF(t_rect.left(), t_rect.bottom());

    // 渐变 — 黄→橙→红→暗红，模拟火焰色
    QLinearGradient t_grad(QPointF(0, t_rect.top()), QPointF(0, t_rect.bottom()));
    const QColor t_flame_yellow   = QColor("#FFD700"); // 金黄
    const QColor t_flame_orange   = QColor("#FF6600"); // 橙
    const QColor t_flame_red      = QColor("#DD2200"); // 红
    const QColor t_flame_dark     = QColor("#661100"); // 暗红
    t_grad.setColorAt(0.0,  t_flame_yellow);
    t_grad.setColorAt(0.25, t_flame_orange);
    t_grad.setColorAt(0.55, t_flame_red);
    t_grad.setColorAt(1.0,  t_flame_dark);

    t_painter.setBrush(t_grad);
    t_painter.setPen(Qt::NoPen);
    t_painter.drawPolygon(t_flame_region);
}

void CustomQWidgetTextBadge::paintEvent(QPaintEvent * /*event*/)
{
    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing);

    const QRect t_rect = rect();

    // 背景 — 圆角裁剪
    {
        QPainterPath t_path;
        t_path.addRoundedRect(t_rect, cl_cfg_.radius, cl_cfg_.radius);
        t_painter.setClipPath(t_path);

        if (cl_fire_enabled_)
            drawFireBg(t_painter, t_rect);
        else
            drawSolidBg(t_painter, t_rect);
    }

    // 文字 — 火焰模式下叠加阴影提高可读性
    if (!cl_cfg_.text.isEmpty()) {
        const QRect t_text_rect(
            t_rect.x() + cl_cfg_.padding_left,
            t_rect.y() + cl_cfg_.padding_top,
            t_rect.width() - cl_cfg_.padding_left - cl_cfg_.padding_right,
            t_rect.height() - cl_cfg_.padding_top - cl_cfg_.padding_bottom);

        t_painter.setFont(cl_cfg_.font);

        if (cl_fire_enabled_) {
            // 火焰模式：暗色阴影 + 白色文字，保证可读性
            t_painter.setPen(QColor(0, 0, 0, 80));
            t_painter.drawText(t_text_rect.adjusted(0, 1, 0, 1), cl_cfg_.alignment, cl_cfg_.text);
        }
        t_painter.setPen(cl_cfg_.text_color);
        t_painter.drawText(t_text_rect, cl_cfg_.alignment, cl_cfg_.text);
    }
}
