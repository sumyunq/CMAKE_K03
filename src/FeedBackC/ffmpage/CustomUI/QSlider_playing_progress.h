#ifndef QSLIDER_PLAYING_PROGRESS_H
#define QSLIDER_PLAYING_PROGRESS_H

#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QSlider>
#include <QStyle>
#include <QStyleOptionSlider>
#include <QVariantAnimation>

#include <QLabel>
#include <memory>

#include <atomic>

class QSliderPlayingProgress : public QSlider
{
    Q_OBJECT

public:
    explicit QSliderPlayingProgress(QWidget *parent = nullptr);

    void InitUIInformation();
    void InitMember();
    void InitConnect();

    void setStatus(bool status); ///使能 or 禁用

signals:
    // void userMousePress();  ///用户按下鼠标（需暂停视频，然后跳转）
    // void userReleasePress();  ///用户释放鼠标

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;

private:
    void drawGrooveRect(
        QPainter &painter, const QRectF &rect, qreal tl, qreal tr, qreal br, qreal bl); ///绘制
    void drawEnableStatus(QPainter &painter);                                           ///使能状态
    void drawDisabledStyle(QPainter &painter);                                          ///禁用状态
    void stopAllAnim();                                                                 ///停止动画

private:
    std::unique_ptr<QVariantAnimation> cl_handle_anim_;       ///滑块渐变动画
    std::unique_ptr<QVariantAnimation> cl_left_groove_anim_;  ///左侧滑槽渐变动画
    std::unique_ptr<QVariantAnimation> cl_right_groove_anim_; ///右侧滑槽渐变动画
    std::unique_ptr<QVariantAnimation> cl_groove_anim_;       ///滑槽渐变动画（滑槽背景）

    std::atomic<bool> cl_isHover_ = false; ///是否悬停在滑块上
    QColor cl_handle_current_color;        ///滑块当前颜色
    QColor cl_left_groove_current_color;   ///左侧滑槽当前颜色
    QColor cl_right_groove_current_color;  ///左侧滑槽当前颜色
    QColor cl_groove_current_color;        ///滑槽当前颜色（滑槽背景）

    std::atomic<int> cl_handle_height_ = 16; ///滑块高度
    std::atomic<int> cl_handle_width_ = 6;   ///滑块宽度
    int cl_groove_height_;                   ///整体滑槽绘制高度
    int cl_left_groove_height_;              ///左侧滑槽绘制高度
    int cl_right_groove_height_;             ///右侧滑槽绘制高度

    qreal cl_handle_radius_;       ///滑块圆角
    qreal cl_groove_radius_;       ///整体滑槽圆角
    qreal cl_left_groove_radius_;  ///左侧滑槽圆角
    qreal cl_right_groove_radius_; ///右侧滑槽圆角

    ///对内部滑槽进行限制
    int cl_left_groove_start_; ///左侧滑槽绘制起点(到左边框的距离)
    int cl_left_groove_end_;   ///左侧滑槽绘制终点(到右边框的距离)

    std::atomic<bool> cl_is_wheelEnabled_
        = false;             ///是否开启滚轮调节(在非滑槽范围内禁用，非焦点时禁用)
    QRect grooveRect_actual; ///实际绘制的滑槽,保留数据用于滚轮位置判断
    QRect handleRect_actual; ///实际绘制的滑块,保留数据用于悬停判断

    // QRect filledRect_actual; ///左侧填充滑槽
    // QRect filledRect_actual_right; ///右侧空白滑槽

    std::unique_ptr<QLabel> clp_text_label_; ///文本信息label
};

#endif // QSLIDER_PLAYING_PROGRESS_H
