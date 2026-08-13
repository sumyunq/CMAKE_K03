#ifndef CUSTOM_QPUSHBUTTON_SINGLE_DEVICE_COLOR_H
#define CUSTOM_QPUSHBUTTON_SINGLE_DEVICE_COLOR_H

#include <QObject>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPushButton>
#include <QToolTip>
#include <QVariantAnimation>
#include <QWidget>

#include "data/api_global.h"

#include "Popup/CustomTipPopup/NewCustomToolTip.h"  ///< 自定义提示窗口

///
/// \brief The CustomQPushButtonSingleDeviceColor class
/// 设备选择界面 底部颜色选择按键
/// 支持
///     默认、悬停、选中三态变化
class CustomQPushButtonSingleDeviceColor : public QPushButton
{
    Q_OBJECT
public:
    explicit CustomQPushButtonSingleDeviceColor(QWidget *parent = nullptr);
    CustomQPushButtonSingleDeviceColor(QWidget *parent,
                                       const QColor &defaultColor,
                                       const QColor &hoverColor,
                                       const QColor &checkedColor); ///< 构造时 指定 颜色
    ~CustomQPushButtonSingleDeviceColor();
    QSize sizeHint() const override;

signals:
    void hoverCentrePositon(QPoint centre_point); ///< 控件中心点 全局坐标

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void drawCheckedStyle(QPainter &painter);
    void drawUnCheckedStyle(QPainter &painter);

public:
    std::shared_ptr<DeSheng::DeviceInfo> cl_device_info_; ///< 设备信息(完整)

private:
    QSize cl_default_size_ = QSize(8, 8);                  ///< 默认大小
    QSize cl_hover_size_ = QSize(10, 8);                   ///< 悬停时大小
    QSize cl_checked_size_ = QSize(24, 8);                 ///< 选中时大小
    QSize cl_current_size_ = cl_default_size_;             ///< 当前尺寸
    QColor cl_default_color_ = QColor("#FF0000");          ///< 默认目标颜色
    QColor cl_hover_color_ = QColor("#00FF00");            ///< 悬停时目标颜色
    QColor cl_checked_color_ = QColor("#0000FF");          ///< 选中时目标颜色
    QColor cl_current_color_ = cl_default_color_;          ///< 当前颜色
    std::unique_ptr<QVariantAnimation> cl_size_anim_;      ///< 大小动画
    std::unique_ptr<QVariantAnimation> cl_color_anim_;     ///< 颜色渐变动画
    QParallelAnimationGroup *cl_parallel_group_ = nullptr; ///< 并行执行动画

    NewCustomToolTip *cl_ToolTip_ = nullptr;    ///< 提示小窗口

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

#endif // CUSTOM_QPUSHBUTTON_SINGLE_DEVICE_COLOR_H
