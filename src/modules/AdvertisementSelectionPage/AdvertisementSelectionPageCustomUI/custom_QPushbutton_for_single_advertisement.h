#ifndef CUSTOM_QPUSHBUTTON_FOR_SINGLE_ADVERTISEMENT_H
#define CUSTOM_QPUSHBUTTON_FOR_SINGLE_ADVERTISEMENT_H

#include <QObject>
#include <QPainter>
#include <QPushButton>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>
#include <QWidget>

///
/// \brief The CustomQPushButtonForSingleAdvertisement class
/// 广告页面底部的单击按键
class CustomQPushButtonForSingleAdvertisement : public QPushButton
{
    Q_OBJECT
public:
    CustomQPushButtonForSingleAdvertisement(QWidget *parent = nullptr);
    ~CustomQPushButtonForSingleAdvertisement();
    QSize sizeHint() const override;


private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    void drawCheckedStyle(QPainter &painter);
    void drawUnCheckedStyle(QPainter &painter);

private:
    QSize cl_default_size_ = QSize(8, 8);              ///< 默认大小
    QSize cl_checked_size_ = QSize(24, 8);             ///< 选中时大小
    QSize cl_current_size_ = cl_default_size_;                            ///< 当前尺寸
    QColor cl_default_color_ = QColor("#33FFFFFF");    ///< 默认目标颜色
    QColor cl_checked_color_ = QColor("#0091DA");      ///< 选中时目标颜色
    QColor cl_current_color_ = cl_default_color_;                          ///< 当前颜色
    std::unique_ptr<QVariantAnimation> cl_size_anim_;  ///< 大小动画
    std::unique_ptr<QVariantAnimation> cl_color_anim_; ///< 颜色渐变动画
    QParallelAnimationGroup *cl_parallel_group_ = nullptr;  ///< 并行执行动画

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;


};

#endif // CUSTOM_QPUSHBUTTON_FOR_SINGLE_ADVERTISEMENT_H
