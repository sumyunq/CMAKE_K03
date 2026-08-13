#ifndef CUSTOM_QSCROLLAREA_DEVICE_SELECTION_H
#define CUSTOM_QSCROLLAREA_DEVICE_SELECTION_H

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>

#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QWidget_single_device_information.h" ///< 子控件：单个设备信息

namespace Ui {
class CustomQScrollAreaDeviceSelection;
}

///
/// \brief The CustomQScrollAreaDeviceSelection class
/// 子控件
///
class CustomQScrollAreaDeviceSelection : public QScrollArea
{
    Q_OBJECT

public:
    explicit CustomQScrollAreaDeviceSelection(QWidget *parent = nullptr);
    ~CustomQScrollAreaDeviceSelection();

    void updateView(); ///< 根据 cl_all_device_list_ 更新显示

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QList<CustomQWidgetSingleDeviceInfo *> cl_all_device_list_; ///< 设备 集合

    QWidget *cl_content_widget_ = nullptr; ///< 内容显示区域
    QGridLayout *cl_gridLayout_ = nullptr; ///< 网格布局
    std::atomic<int> cl_columnCount_;      ///< 子部件列数
    std::atomic<int> cl_rowCount_;         ///< 子部件行数

private:
    ///布局属性
    int left_margin_ = 0;
    int top_margin_ = 0;
    int right_margin_ = 0;
    int bottom_margin_ = 0;
    int spacing_ = 25; ///内部部件见间距

private:
    Ui::CustomQScrollAreaDeviceSelection *ui;
};

#endif // CUSTOM_QSCROLLAREA_DEVICE_SELECTION_H
