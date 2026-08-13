#ifndef DEVICE_SELECTION_MAIN_PAGE_H
#define DEVICE_SELECTION_MAIN_PAGE_H

#include <QEvent>
#include <QFile>
#include <QStandardPaths>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QWheelEvent>
#include <QWidget>

#include "data/api_global.h"
#include "modules/Common/DeviceRegistry.h" ///< DeSheng::DeviceRegistry

#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QWidget_single_device_information.h" ///< 子控件，单个设备显示

#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QPushButton_roundbutton.h" ///
#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QScrollArea_device_selection.h" ///< 子控件，设备选择 网格滚动区域
#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QScrollArea_roundbutton.h" ///< 子控件，设备选择 右侧按键区域

namespace Ui {
class DeviceSelectionMainPage;
}

/// \brief 设备选择页面
/// 子控件：
///     - CustomQScrollAreaDeviceSelection: 设备选择网格区域
///     - CustomQScrollAreaRoundbutton: 右侧行号导航按键
class DeviceSelectionMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceSelectionMainPage(QWidget *parent = nullptr);
    ~DeviceSelectionMainPage();

    void syncRowButtons(); ///< 根据设备选择区域行数同步右侧行号按键

    DeSheng::DeviceInfo cl_selected_device_information() const;
    void setCl_selected_device_information(
        const DeSheng::DeviceInfo &newCl_selected_device_information);

private:
    void InitUIInformation();                                 ///< 初始化UI的默认信息
    void InitMember();                                        ///< 初始化内部成员
    void InitConnect();                                       ///< 连接默认的信号槽
    void smoothScrollBy(QScrollArea *scrollArea, int pixels); ///< 通过动画平滑滚动指定像素(+下/-上)
    void smoothScrollTo(QScrollArea *scrollArea, int targetValue); ///< 通过动画滚动到绝对位置
    void switchButton(int delta);                                  ///< 滑轮滚动,更新右侧按键显示
public:
    CustomQScrollAreaDeviceSelection *clp_scrollArea_device_selection_ = nullptr; ///< 设备选择区域

    CustomQScrollAreaRoundbutton *clp_scrollArea_roundbutton_ = nullptr; ///< 右侧按键控制

    DeSheng::DeviceInfo
        cl_selected_device_information_; ///< 选择设备后， cl_selected_device_information_ 更新为选择的设备的基础信息

private:
    int cl_wheel_accumulated_angle_ = 0; ///< 滚轮累积角度，达到 ±120 触发一次滚动

private:
    Ui::DeviceSelectionMainPage *ui; ///< .ui 界面指针

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // DEVICE_SELECTION_MAIN_PAGE_H
