#ifndef CUSTOM_QPUSHBUTTON_ROUNDBUTTON_H
#define CUSTOM_QPUSHBUTTON_ROUNDBUTTON_H

#include <QObject>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QToolTip>
#include <QWidget>

#include "data/api_global.h"

///
/// \brief The CustomQPushButtonRoundButton class
/// 自定义圆形按键
class CustomQPushButtonRoundButton : public QPushButton
{
    Q_OBJECT
public:
    explicit CustomQPushButtonRoundButton(QWidget *parent = nullptr);
    CustomQPushButtonRoundButton(QWidget *parent,
                                 const QColor &defaultColor,
                                 const QColor &hoverColor,
                                 const QColor &checkedColor); ///< 构造时 指定 颜色
    ~CustomQPushButtonRoundButton();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void drawCheckedStyle(QPainter &painter);
    void drawUnCheckedStyle(QPainter &painter);

public:
    QSize cl_default_size_ = QSize(8, 8);                  ///< 默认大小
    QSize cl_hover_size_ = QSize(8, 8);                    ///< 悬停时大小
    QSize cl_checked_size_ = QSize(8, 8);                  ///< 选中时大小
    QSize cl_current_size_ = cl_default_size_;             ///< 当前尺寸
    QColor cl_default_color_ = QColor("#33FFFFFF");        ///< 默认目标颜色
    QColor cl_hover_color_ = QColor("#33FFFFFF");          ///< 悬停时目标颜色
    QColor cl_checked_color_ = QColor("#0091DA");          ///< 选中时目标颜色
    QColor cl_current_color_ = cl_default_color_;          ///< 当前颜色
    std::unique_ptr<QVariantAnimation> cl_size_anim_;      ///< 大小动画
    std::unique_ptr<QVariantAnimation> cl_color_anim_;     ///< 颜色渐变动画
    QParallelAnimationGroup *cl_parallel_group_ = nullptr; ///< 并行执行动画

    QSize cl_pushbutton_min_size_ = QSize(8, 8); ///< 最小尺寸
    QSize cl_pushbutton_max_size_ = QSize(8, 8);

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

#endif // CUSTOM_QPUSHBUTTON_ROUNDBUTTON_H
