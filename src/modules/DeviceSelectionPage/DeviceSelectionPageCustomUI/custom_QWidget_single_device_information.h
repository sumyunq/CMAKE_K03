#ifndef CUSTOM_QWIDGET_SINGLE_DEVICE_INFORMATION_H
#define CUSTOM_QWIDGET_SINGLE_DEVICE_INFORMATION_H

#include <QHash>
#include <QLabel>
#include <QPair>
#include <QWidget>

#include "data/api_global.h"

#include "modules/Common/FrostedPanel.h"
#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QScrollarea_color_pushbuttons.h"

namespace Ui {
class CustomQWidgetSingleDeviceInfo;
}

///
/// \brief The CustomQWidgetSingleDeviceInfo class
/// 单个设备显示页面
/// 子部件：
///     QLabel 电量 最后绘制,保证其在图片之上
///     QPixmap 对应的设备图片
///     QLabel 文字 设备名称
///     QLabel 文字 2.0（由构造参数 showExtraTags 控制是否显示）
///     QLabel 文字 颜色中文名称
///     CustomQScrollAreaColorPushButtons 颜色选择 按键区域
class CustomQWidgetSingleDeviceInfo : public FrostedPanel
{
    Q_OBJECT

public:
    explicit CustomQWidgetSingleDeviceInfo(QWidget *parent = nullptr, bool showExtraTags = true);
    CustomQWidgetSingleDeviceInfo(const QString deviceTypeName, QWidget *parent = nullptr, bool showExtraTags = true);
    ~CustomQWidgetSingleDeviceInfo();

    void updatePushButtonScrollArea(); ///< 更新滚动区域 按键

    void updatePushButtonList(QString device_info_DeviceTypeName); ///< 根据设备名称去更新 按键列表

    void setCheckedDevice(int index = -1); ///< 设置当前选中项,-1: 按配置文件中设置; 其他:指定 index 按键为选中状态

signals:
    void sendSignalsDeviceInfo(const DeSheng::DeviceInfo &deviceInfo); ///< 点击时，发送该设备相关信息

protected slots:
    // ///
    // /// \brief showHintWidget
    // /// \param pBn_centre_point 目标控件中心位置的全局坐标
    // ///
    // void showHintWidget(QPoint pBn_centre_point); ///< 指定位置显示提示框 cl_hint_widget_

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void updateTextLayout();  ///< 三个文字控件宽度按文本自适应并整体水平居中

public:
    /************************************ UI ************************************/
    QLabel *cl_battery_icon_ = nullptr;             ///< 电量图标
    QSize cl_battery_icon_size_ = QSize(28, 14);    ///< 电量图标 尺寸
    QPoint cl_battery_icon_point_ = QPoint(30, 28); ///< 电量图标 位置
    std::atomic<bool> cl_battery_is_show_ = false;  ///< 电量图标是否显示

    QPixmap cl_device_pixmap_;                                     ///< 设备图片，根据颜色变化
    QLabel *cl_device_pixmap_show_label_ = nullptr; ///< 设备图片，显示设备图，避免重绘导致的模糊
    QRect cl_device_pixmap_default_rect_ = QRect(0, 44, 344, 245); ///< 设备图片,默认矩阵
    QPoint cl_device_pixmap_default_point_ = QPoint(0, 44);        ///< 设备图片,默认尺寸
    QSize cl_device_pixmap_default_size_ = QSize(344, 245);        ///< 设备图片,默认尺寸
    double cl_current_scaling_factor_ = 1.0;                       /// 当前缩放系数
    double cl_default_scaling_factor_ = 1.0;                       /// 默认缩放系数
    double cl_target_scaling_factor_ = 1.12;                       /// 目标放大系数（306，322）

    QLabel *cl_device_pixmap_label_ = nullptr; ///< 设备图片，不放图，仅作 事件监听处理
    QSize cl_device_pixmap_label_current_size_ = QSize(274, 287);  ///< 设备图片 当前尺寸 动画可变值
    QSize cl_device_pixmap_label_default_size_ = QSize(274, 287);  ///< 设备图片 默认尺寸
    QSize cl_device_pixmap_label_enlarge_size_ = QSize(306, 322);  ///< 设备图片 悬停放大尺寸
    QPoint cl_device_pixmap_label_current_point_ = QPoint(27, 16); ///< 设备图片 当前位置 动画可变值
    QPoint cl_device_pixmap_label_default_point_ = QPoint(27, 16); ///< 设备图片 默认位置
    QPoint cl_device_pixmap_label_enlarge_point_ = QPoint(11, 0);  ///< 设备图片 悬停放大位置

    QLabel *cl_text_device_info_DeviceTypeName_ = nullptr;             ///< 设备类型名称 文字靠右
    QSize cl_text_device_info_DeviceTypeName_size_ = QSize(174, 32);   ///< 设备类型名称 尺寸(仅高度有效,宽度按文本自适应)
    QPoint cl_text_device_info_DeviceTypeName_point_ = QPoint(0, 321); ///< 设备类型名称 位置(仅 y 坐标有效)

    QLabel *cl_text_device_info_ExtraTags_ = nullptr;             ///< 额外标签2.0 文字靠左
    QSize cl_text_device_info_ExtraTags_size_ = QSize(18, 17);   ///< 额外标签2.0 尺寸(仅高度有效,宽度按文本自适应)
    QPoint cl_text_device_info_ExtraTags_point_ = QPoint(178, 333); ///< 额外标签2.0 位置(仅 y 坐标有效)
    bool cl_extra_tags_is_show_ = true;                          ///< 额外标签2.0 是否显示

    QLabel *cl_text_device_info_DeviceColorName_ = nullptr;           ///< 颜色中文名称 文字靠左
    QSize cl_text_device_info_DeviceColorName_size_ = QSize(156, 17); ///< 颜色中文名称 尺寸(仅高度有效,宽度按文本自适应)
    QPoint cl_text_device_info_DeviceColorName_point_ = QPoint(210,332); ///< 颜色中文名称 位置(仅 y 坐标有效)

    CustomQScrollAreaColorPushButtons *cl_color_pushButtons_scrollArea_ = nullptr; ///< 按键滚动区域
    QSize cl_color_pushButtons_scrollArea_size_ = QSize(344, 8); ///< 按键滚动区域 尺寸
    QPoint cl_color_pushButtons_scrollArea_point_ = QPoint(0,387); ///< 按键滚动区域 位置

    QLabel *cl_hint_widget_ = nullptr; ///< 悬停提示框

    /************************************ 其他 ************************************/
    DeSheng::DeviceInfo
        cl_device_info_; ///< 该设备相关信息(在该类中，只用到了 DeviceTypeName ,UI 详细细节保存在按键中，通过按键组 点击事件触发)

    /************************************ 动画 ************************************/
    std::unique_ptr<QVariantAnimation> cl_size_anim_;           ///< 大小动画 设备图片
    std::unique_ptr<QVariantAnimation> cl_point_anim_;          ///< 位置动画 设备图片
    std::unique_ptr<QVariantAnimation> cl_scaling_factor_anim_; ///< 图片缩放系数动画 设备图片
    QParallelAnimationGroup *cl_parallel_group_ = nullptr;      ///< 并行执行动画

private:
    Ui::CustomQWidgetSingleDeviceInfo *ui;

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // CUSTOM_QWIDGET_SINGLE_DEVICE_INFORMATION_H
