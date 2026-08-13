#include "modules/Common/FrostedPanel.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOption>

#include "modules/Common/AppImageCache.h"

namespace {
/// \brief 按四角半径构造圆角矩形裁剪路径（顺时针，从左上角起）
QPainterPath buildRoundedRectPath(const QRectF &t_rect, qreal t_tl, qreal t_tr,
                                  qreal t_br, qreal t_bl)
{
    // 半径不超过短边一半，避免相互穿插
    const qreal t_max = qMin(t_rect.width(), t_rect.height()) / 2.0;
    t_tl = qBound(0.0, t_tl, t_max);
    t_tr = qBound(0.0, t_tr, t_max);
    t_br = qBound(0.0, t_br, t_max);
    t_bl = qBound(0.0, t_bl, t_max);

    QPainterPath t_path;
    t_path.moveTo(t_rect.left() + t_tl, t_rect.top());
    t_path.lineTo(t_rect.right() - t_tr, t_rect.top());
    if (t_tr > 0.0)
        t_path.arcTo(t_rect.right() - 2 * t_tr, t_rect.top(), 2 * t_tr, 2 * t_tr, 90, -90);
    t_path.lineTo(t_rect.right(), t_rect.bottom() - t_br);
    if (t_br > 0.0)
        t_path.arcTo(t_rect.right() - 2 * t_br, t_rect.bottom() - 2 * t_br,
                     2 * t_br, 2 * t_br, 0, -90);
    t_path.lineTo(t_rect.left() + t_bl, t_rect.bottom());
    if (t_bl > 0.0)
        t_path.arcTo(t_rect.left(), t_rect.bottom() - 2 * t_bl, 2 * t_bl, 2 * t_bl, 270, -90);
    t_path.lineTo(t_rect.left(), t_rect.top() + t_tl);
    if (t_tl > 0.0)
        t_path.arcTo(t_rect.left(), t_rect.top(), 2 * t_tl, 2 * t_tl, 180, -90);
    t_path.closeSubpath();
    return t_path;
}

} // namespace

/// \brief 底层背景 widget — 承载毛玻璃模糊 + stylesheet tint
/// 作为 FrostedPanel 的第一个子控件，确保始终在其他子控件之下绘制
/// 必须在全局作用域定义，以匹配头文件中 friend struct FrostedBackgroundLayer 的声明
struct FrostedBackgroundLayer : public QWidget {
    FrostedPanel *cl_panel_ = nullptr;

    using QWidget::QWidget;

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        if (!cl_panel_) return;

        QPainter t_painter(this);
        t_painter.setRenderHint(QPainter::Antialiasing, true);
        t_painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

        // 裁剪形状：自定义路径优先，否则用圆角，否则不裁剪（矩形）
        // 背景层与 FrostedPanel 同尺寸，本地坐标一致，直接使用
        const QPainterPath &t_shape = cl_panel_->shapePath();
        if (!t_shape.isEmpty()) {
            t_painter.setClipPath(t_shape);
        } else if (cl_panel_->cornerRadius() > 0.0
                   || cl_panel_->cornerRadiusTR() > 0.0
                   || cl_panel_->cornerRadiusBR() > 0.0
                   || cl_panel_->cornerRadiusBL() > 0.0) {
            t_painter.setClipPath(buildRoundedRectPath(
                rect(), cl_panel_->cornerRadius(), cl_panel_->cornerRadiusTR(),
                cl_panel_->cornerRadiusBR(), cl_panel_->cornerRadiusBL()));
        }

        // ① 模糊背景切片
        const QPixmap &t_blur = AppImageCache::instance().cl_background_blurred_cache_;
        if (!t_blur.isNull()) {
            QWidget *t_top = window();
            if (t_top) {
                const QPoint t_tl = cl_panel_->mapTo(t_top, QPoint(0, 0));
                t_painter.save();
                t_painter.setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑放大，避免 4× 降采样颗粒
                t_painter.translate(-t_tl);
                t_painter.drawPixmap(QRect(0, 0, t_top->width(), t_top->height()), t_blur);
                t_painter.restore();
            }
        }

        // ② 叠加 stylesheet tint（仅作用于背景层，不影响兄弟控件）
        QStyleOption t_opt;
        t_opt.initFrom(cl_panel_);
        style()->drawPrimitive(QStyle::PE_Widget, &t_opt, &t_painter, cl_panel_);
    }
};

/// \brief 构造函数
FrostedPanel::FrostedPanel(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true); // 确保 stylesheet 背景由 style 绘制

    // 底层背景：承载毛玻璃模糊 + tint，始终位于所有子控件之下
    cl_background_layer_ = new FrostedBackgroundLayer(this);
    cl_background_layer_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto *t_bg = static_cast<FrostedBackgroundLayer *>(cl_background_layer_);
    t_bg->cl_panel_ = this;
    cl_background_layer_->lower(); // 确保 z-order 最低
}

/// \brief 统一圆角（四角相同）
void FrostedPanel::setCornerRadius(qreal t_radius)
{
    setCornerRadius(t_radius, t_radius, t_radius, t_radius);
}

/// \brief 四角独立圆角（左上、右上、右下、左下）
void FrostedPanel::setCornerRadius(qreal t_topLeft, qreal t_topRight,
                                   qreal t_bottomRight, qreal t_bottomLeft)
{
    if (cl_radius_tl_ == t_topLeft && cl_radius_tr_ == t_topRight
        && cl_radius_br_ == t_bottomRight && cl_radius_bl_ == t_bottomLeft)
        return;
    cl_radius_tl_ = t_topLeft;
    cl_radius_tr_ = t_topRight;
    cl_radius_br_ = t_bottomRight;
    cl_radius_bl_ = t_bottomLeft;
    cl_shape_path_ = QPainterPath(); // 清除自定义路径，回退到圆角模式
    update();
    if (cl_background_layer_) cl_background_layer_->update();
}

/// \brief 自定义裁剪路径（非空时优先于圆角，空时回退到圆角模式）
/// \note 路径使用控件本地坐标。resize 后外部需重新设置
void FrostedPanel::setShapePath(const QPainterPath &t_path)
{
    cl_shape_path_ = t_path;
    update();
}

QPainterPath FrostedPanel::shapePath() const
{
    return cl_shape_path_;
}

/// \brief 左上角圆角
qreal FrostedPanel::cornerRadius() const
{
    return cl_radius_tl_;
}

/// \brief 底层背景层随面板同步缩放
void FrostedPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (cl_background_layer_)
        cl_background_layer_->setGeometry(rect());
}

/// \brief 面板自身不再绘制模糊/tint，由 cl_background_layer_ 作为独立子控件绘制
void FrostedPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // 模糊 + tint 已全部移至 cl_background_layer_，它作为子控件会自然绘制在所有
    // 业务子控件之下。面板自身无需额外绘制。
}
