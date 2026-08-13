#pragma once

#include <QSlider>
#include <QStyleOptionSlider>
#include <QPainter>
#include <QStyle>
#include <QPainterPath>
#include <QLabel>

class NewHSlider : public QSlider {
    Q_OBJECT

    //声明类的属性,则根据属性值自动切换对应的样式。若不声明则不生效
    Q_PROPERTY(bool hover READ isHover WRITE setHover)
    Q_PROPERTY(bool pressed READ isPressed WRITE setPressed)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor)
    Q_PROPERTY(QColor handleColor READ handleColor WRITE setHandleColor) //滑块

public:
    NewHSlider(QWidget *parent = nullptr);

    ~NewHSlider();

    void setType(int type,int bgTrackHeight,int TfgTrackHeight,bool HAnimationEn,bool ShowTooltip);//绘制样式，背景高度，数值背景高度，是否开启动画，是否当鼠标悬浮到滑块上方时显示数组
    void setMargin(int value);//设置左右两边间隔，默认值3
    // 滑动条颜色动画：from 和 to 为初始、目标颜色
    void animateFillColor(const QColor &from, const QColor &to, int duration = 100);
    // 滑块颜色动画
    void animateHandleColor(const QColor &from, const QColor &to, int duration = 100);


    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor &color);

    QColor handleColor() const{ return m_handleColor; };
    void setHandleColor(const QColor &color);


protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) override;

    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

    void changeEvent(QEvent *event) override;
    void leaveEvent(QEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *ev) override;

private:

    QPixmap m_fullPixmap;
    void updateFullPixmap();   // 根据 m_fillColor 重新生成 m_fullPixmap
    QPixmap generateHandlePixmap(const QColor &baseColor, bool hover, bool pressed) const;
    void hideDisplayLabel();
    void ensureDisplayLabel();
    void updateDisplayLabelPosition(int displayValue);
    int sliderCenterForValue(int displayValue) const;
    void cancelActiveInteraction();
    QColor m_fillColor;//滑动条
    QColor m_handleColor;          // 滑块基础颜色

    QWidget*			m_parentWidget;
    QWidget*            m_sliderDisplayLabelContainer;
    QLabel*				m_sliderDisplayLabel;

    int styleType;
    int trackHeight = 8;
    int fgTrackHeight = 4;
    int margin = 3;//显示条与左右两边间隔

    bool wheelEnabled;//是否开启滚轮调节

    bool AnimationEn = false;
    bool ShowTooltipEn = false;
    bool m_hover = false;
    bool m_pressed = false;
    bool m_emitSliderReleasedOnMouseRelease = false;
    bool isHover() const { return m_hover; }
    bool isPressed() const { return m_pressed; }
    void setHover(bool hover);
    void setPressed(bool pressed);

};
