#ifndef FROSTED_PANEL_H
#define FROSTED_PANEL_H

#include <QWidget>
#include <QPainterPath>

///
/// \brief 磨砂玻璃面板（可复用）
///
/// 内部使用独立的底层背景子控件承载毛玻璃模糊 + stylesheet tint。
/// 业务子控件与背景层是平级兄弟关系，子控件背景不会与父级面板 tint 叠加。
///
/// 用法：在 Qt Designer 中把目标半透明 QWidget 提升为 FrostedPanel，
/// 保留其原有半透明 stylesheet；若有圆角，构造后调 setCornerRadius() 对齐。
class FrostedPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FrostedPanel(QWidget *parent = nullptr);

    void setCornerRadius(qreal t_radius);
    void setCornerRadius(qreal t_topLeft, qreal t_topRight,
                         qreal t_bottomRight, qreal t_bottomLeft);
    qreal cornerRadius() const;
    qreal cornerRadiusTR() const { return cl_radius_tr_; }
    qreal cornerRadiusBR() const { return cl_radius_br_; }
    qreal cornerRadiusBL() const { return cl_radius_bl_; }

    void setShapePath(const QPainterPath &t_path);
    QPainterPath shapePath() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // 底层背景子控件 — 仅 .cpp 中的 FrostedBackgroundLayer 需要访问圆角成员
    friend struct FrostedBackgroundLayer;
    QWidget *cl_background_layer_ = nullptr;

    qreal cl_radius_tl_ = 0.0;
    qreal cl_radius_tr_ = 0.0;
    qreal cl_radius_br_ = 0.0;
    qreal cl_radius_bl_ = 0.0;
    QPainterPath cl_shape_path_;
};

#endif // FROSTED_PANEL_H
