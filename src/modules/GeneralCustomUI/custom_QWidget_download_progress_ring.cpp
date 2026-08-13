#include "modules/GeneralCustomUI/custom_QWidget_download_progress_ring.h"

#include <QPainter>
#include <QPaintEvent>

CustomQWidgetDownloadProgressRing::CustomQWidgetDownloadProgressRing(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQWidgetDownloadProgressRing::~CustomQWidgetDownloadProgressRing() {}

void CustomQWidgetDownloadProgressRing::InitUIInformation(int theme)
{
    {
        setObjectName("CustomQWidgetDownloadProgressRing_ring");
        setFixedSize(cl_min_size_);
    }

    applyTheme(theme);
}

void CustomQWidgetDownloadProgressRing::InitMember() {}

void CustomQWidgetDownloadProgressRing::InitConnect() {}

void CustomQWidgetDownloadProgressRing::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    }
}

void CustomQWidgetDownloadProgressRing::paintEvent(QPaintEvent * /*event*/)
{
    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing);

    const int t_w = width();
    const int t_h = height();
    const int t_margin = cl_ring_width_;
    const QRectF t_rect(t_margin, t_margin,
                        t_w - 2 * t_margin, t_h - 2 * t_margin);

    {
        // 背景弧（完整圆形基底）
        QPen t_pen(cl_bg_color_, cl_ring_width_, Qt::SolidLine, Qt::RoundCap);
        t_painter.setPen(t_pen);
        t_painter.drawArc(t_rect, 0, 360 * 16);
    }

    if (cl_progress_ > 0) {
        // 进度弧（12点方向顺时针）
        int t_span = -cl_progress_ * 360 / 100 * 16; ///< 负号 = 顺时针
        QPen t_pen(cl_ring_color_, cl_ring_width_, Qt::SolidLine, Qt::RoundCap);
        t_painter.setPen(t_pen);
        t_painter.drawArc(t_rect, 90 * 16, t_span);  ///< 90° = 12点方向
    }
}

int CustomQWidgetDownloadProgressRing::cl_progress() const
{
    return cl_progress_;
}

void CustomQWidgetDownloadProgressRing::setCl_progress(int percent)
{
    percent = qBound(0, percent, 100);
    if (cl_progress_ == percent)
        return;
    cl_progress_ = percent;
    update();
}

void CustomQWidgetDownloadProgressRing::reset()
{
    cl_progress_ = 0;
    update();
}
