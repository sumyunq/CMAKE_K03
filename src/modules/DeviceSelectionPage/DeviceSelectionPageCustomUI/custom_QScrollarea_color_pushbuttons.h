#ifndef CUSTOM_QSCROLLAREA_COLOR_PUSHBUTTONS_H
#define CUSTOM_QSCROLLAREA_COLOR_PUSHBUTTONS_H

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTimer>

#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QPushButton_single_device_color.h" ///< 子控件：单个按键

namespace Ui {
class CustomQScrollAreaColorPushButtons;
}

///
/// \brief The CustomQScrollAreaColorPushButtons class
/// 颜色按键水平布局
/// 子控件：
///     各种颜色按键 CustomQPushButtonSingleDeviceColor
class CustomQScrollAreaColorPushButtons : public QScrollArea
{
    Q_OBJECT

public:
    explicit CustomQScrollAreaColorPushButtons(QWidget *parent = nullptr);
    ~CustomQScrollAreaColorPushButtons();

    void updateView();  ///< 根据 cl_all_color_pushButton_list_ 更新显示

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QList<CustomQPushButtonSingleDeviceColor *> cl_all_color_pushButton_list_; ///< 按键 集合
    QButtonGroup *cl_all_color_pushButton_buttonGroup_ = nullptr;              ///< 颜色 按键组

    QWidget *cl_content_widget_ = nullptr; ///< 内容显示区域
    QHBoxLayout *cl_hBoxLayout_ = nullptr; ///< 水平布局

private:
    ///布局属性
    int left_margin_ = 0;
    int top_margin_ = 0;
    int right_margin_ = 0;
    int bottom_margin_ = 0;
    int spacing_ = 10; ///内部部件见间距

private:
    Ui::CustomQScrollAreaColorPushButtons *ui;
};

#endif // CUSTOM_QSCROLLAREA_COLOR_PUSHBUTTONS_H
