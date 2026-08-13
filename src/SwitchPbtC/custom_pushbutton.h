/*#ifndef CUSTOM_PUSHBUTTON_2_H
#define CUSTOM_PUSHBUTTON_2_H


#include <QWidget>
#include <QPushButton>
#include <QPen>
#include <QPixmap>
#include <QBrush>
#include <QPainter>
#include <QColor>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QPoint>

enum class SystemStyle {
    SIMPLE,   /// 简单
    ANIMATION /// 动画
};


class CustomPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(double cl_sliderOffset_ READ sliderOffset WRITE setSliderOffset)

public:
    explicit CustomPushButton(QWidget *parent = nullptr);
    ~CustomPushButton();
    void InitUIInformation();
    void InitMember();
    void InitConnect();

    void setOpen(bool isOpen);

    void setCl_system_style(SystemStyle newCl_system_style);
    void setCl_pixmapOn(const QPixmap &newCl_pixmapOn);
    void setCl_pixmapOff(const QPixmap &newCl_pixmapOff);

    void setZoomFactor(double factor);  // 新增：设置缩放因子

signals:
    void cl_toggled(bool open);

private:
    SystemStyle cl_system_style_ = SystemStyle::ANIMATION;
    bool cl_is_Open_ = false;

    QPropertyAnimation *cl_animation_ = nullptr;
    double cl_sliderOffset_ = 0.0; // 0.0=关, 1.0=开

    QPixmap cl_pixmapOn_;
    QPixmap cl_pixmapOff_;

    // 设计稿的基础尺寸（基准值）
    static constexpr int BASE_WIDTH = 46;
    static constexpr int BASE_HEIGHT = 22;
    static constexpr int BASE_RADIUS = 8;       // 内部小圆半径
    static constexpr int BASE_MARGIN = 3;       // 内部小圆边距

    // 运行时根据缩放因子动态计算的实际值
    double cl_zoom_factor_ = 1.0;    // 缩放因子
    int cl_actualRadius_ = BASE_RADIUS;
    int cl_actualMargin_ = BASE_MARGIN;

    // 重新计算内部参数
    void recalculateSizes();

private:
    void drawAnimationStyle(QPainter &painter);
    void drawSimpleStyle(QPainter &painter);

    double sliderOffset() const;
    void setSliderOffset(double offset);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_PUSHBUTTON_2_H
*/
#ifndef CUSTOM_PUSHBUTTON_2_H
#define CUSTOM_PUSHBUTTON_2_H

#include <QWidget>
#include <QPushButton>
#include <QPen>
#include <QPixmap>
#include <QBrush>
#include <QPainter>
#include <QColor>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QPoint>

enum class SystemStyle {
    SIMPLE,   /// 简单
    ANIMATION /// 动画
};

class CustomPushButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(double cl_sliderOffset_ READ sliderOffset WRITE setSliderOffset)

public:
    explicit CustomPushButton(QWidget *parent = nullptr);
    ~CustomPushButton();
    void InitUIInformation();
    void InitMember();
    void InitConnect();

    void setCl_system_style(SystemStyle newCl_system_style);
    void setCl_pixmapOn(const QPixmap &newCl_pixmapOn);
    void setCl_pixmapOff(const QPixmap &newCl_pixmapOff);

    void setZoomFactor(double factor);  // 设置缩放因子

public:
    void setChecked(bool checked);   // 重写基类的虚函数

signals:
    void cl_toggled(bool open);         // 保留原有信号，供外部使用

private slots:
    void onToggled(bool checked);       // 响应按钮 checked 状态变化

private:
    SystemStyle cl_system_style_ = SystemStyle::ANIMATION;

    QPropertyAnimation *cl_animation_ = nullptr;
    double cl_sliderOffset_ = 0.0;      // 0.0=关, 1.0=开

    QPixmap cl_pixmapOn_;
    QPixmap cl_pixmapOff_;

    // 设计稿的基础尺寸（基准值）
    static constexpr int BASE_WIDTH = 46;
    static constexpr int BASE_HEIGHT = 22;
    static constexpr int BASE_RADIUS = 8;       // 内部小圆半径
    static constexpr int BASE_MARGIN = 3;       // 内部小圆边距

    // 运行时根据缩放因子动态计算的实际值
    double cl_zoom_factor_ = 1.0;    // 缩放因子
    int cl_actualRadius_ = BASE_RADIUS;
    int cl_actualMargin_ = BASE_MARGIN;

    // 重新计算内部参数
    void recalculateSizes();

private:
    void drawAnimationStyle(QPainter &painter);
    void drawSimpleStyle(QPainter &painter);

    double sliderOffset() const;
    void setSliderOffset(double offset);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_PUSHBUTTON_2_H
