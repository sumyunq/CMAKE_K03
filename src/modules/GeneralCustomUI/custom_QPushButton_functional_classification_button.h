#ifndef CUSTOM_QPUSHBUTTON_FUNCTIONAL_CLASSIFICATION_BUTTON_H
#define CUSTOM_QPUSHBUTTON_FUNCTIONAL_CLASSIFICATION_BUTTON_H

#include <QPainter>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QWidget>

/// \brief 功能分类按键
/// 默认状态：半透明背景 + 居中图标
/// 悬停状态：图标上移 + 文字出现 + 背景叠加悬停色
class CustomQPushButtonFunctionalClassificationButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QPoint cl_icon_point READ iconPoint WRITE setIconPoint)
    Q_PROPERTY(qreal cl_text_opacity READ textOpacity WRITE setTextOpacity)
    Q_PROPERTY(int cl_icon_size_prop READ iconSizeProp WRITE setIconSizeProp)
    Q_PROPERTY(QColor cl_bg_current READ bgCurrent WRITE setBgCurrent)

public:
    explicit CustomQPushButtonFunctionalClassificationButton(QWidget *parent = nullptr);

    void setBackground(QColor colorStr);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    void setCl_classification_name(const QString &name); ///< 设置分类名称
    QString cl_classification_name() const;              ///< 获取分类名称
    void setCl_pixmap(const QPixmap &pixmap);            ///< 设置图标
    void setCl_min_size(const QSize &size);              ///< 设置最小尺寸
    void setCl_icon_default_size(const QSize &size);     ///< 设置图标默认尺寸
    void setCl_icon_target_size(const QSize &size);      ///< 设置图标目标尺寸
    void setCl_icon_size(const QSize &defaultSize, const QSize &targetSize); ///< 便捷
    void setCl_icon_default_point(const QPoint &point);  ///< 设置图标默认位置
    void setCl_icon_target_point(const QPoint &point);   ///< 设置图标悬停目标位置
    void setCl_icon_point(const QPoint &defaultPoint, const QPoint &targetPoint); ///< 便捷
    void setCl_icon_text_spacing(int spacing);           ///< 设置图标与文字间距
    void setCl_bg_default_color(const QColor &color);    ///< 设置默认背景色
    void setCl_bg_hover_color(const QColor &color);      ///< 设置悬停叠加色
    void setCl_border_radius(int radius);                ///< 设置圆角半径

private:
    QColor blendColors(const QColor &base, const QColor &overlay) const; ///< 颜色叠加

    QPoint iconPoint() const { return cl_icon_current_point_; }
    void setIconPoint(QPoint p) { cl_icon_current_point_ = p; update(); }
    qreal textOpacity() const { return cl_text_opacity_; }
    void setTextOpacity(qreal o) { cl_text_opacity_ = o; update(); }
    int iconSizeProp() const { return cl_icon_size_prop_; }
    void setIconSizeProp(int v) { cl_icon_size_prop_ = v; update(); }
    QColor bgCurrent() const { return cl_bg_current_; }
    void setBgCurrent(QColor c) { cl_bg_current_ = c; update(); }

    QString cl_classification_name_;              ///< 分类名称
    QPixmap cl_pixmap_original_;                  ///< 图标（原始未缩放）
    QPixmap cl_pixmap_;                           ///< 图标（缓存缩放后）
    QSize cl_min_size_ = QSize(88, 88);           ///< 最小尺寸
    QSize cl_icon_default_size_ = QSize(24, 24);  ///< 图标默认尺寸
    QSize cl_icon_target_size_ = QSize(28, 28);   ///< 图标目标（悬停）尺寸
    QPoint cl_icon_default_point_ = QPoint(29, 29); ///< 图标默认位置（相对原点）
    QPoint cl_icon_target_point_ = QPoint(29, 23);  ///< 图标悬停目标位置（相对原点）
    int cl_icon_text_spacing_ = 4;                ///< 图标与文字间距
    int cl_border_radius_ = 8;                    ///< 圆角半径

    QPropertyAnimation *cl_icon_anim_ = nullptr;      ///< 图标位移动画
    QPropertyAnimation *cl_text_anim_ = nullptr;      ///< 文字透明度动画
    QPropertyAnimation *cl_icon_size_anim_ = nullptr; ///< 图标尺寸动画
    QPropertyAnimation *cl_bg_anim_ = nullptr;        ///< 背景色渐变动画

    QColor cl_bg_default_ = QColor(81, 96, 122, 51); ///< 默认背景色
    QColor cl_bg_hover_ = QColor(255, 255, 255, 25); ///< 悬停叠加色
    QColor cl_bg_current_ = QColor(81, 96, 122, 51); ///< 当前背景色（动画过渡）
    QPoint cl_icon_current_point_ = QPoint(0, 0);    ///< 图标当前位置（相对原点）
    qreal cl_text_opacity_ = 0.0;                    ///< 文字当前透明度
    int cl_icon_size_prop_ = 24;                     ///< 图标当前尺寸（动画过渡）

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

#endif // CUSTOM_QPUSHBUTTON_FUNCTIONAL_CLASSIFICATION_BUTTON_H
