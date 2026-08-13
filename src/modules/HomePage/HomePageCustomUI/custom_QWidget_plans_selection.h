#ifndef CUSTOM_QWIDGET_PLANS_SELECTION_H
#define CUSTOM_QWIDGET_PLANS_SELECTION_H

#include <QButtonGroup>
#include <QGridLayout>
#include <QPair>
#include <QWidget>

#include "SwitchPbtC/custom_pushbutton.h" ///< 子部件： 自定义开关按键
#include "Popup/CustomTipPopup/NewCustomToolTip.h"           ///< 自定义提示控件
#include "modules/HomePage/HomePageCustomUI/custom_QPushButton_single_plan.h"      /// 子部件 方案库按键 头文件

#include "GlobalVariable.h" /// 全局变量 头文件 取方案内容

typedef struct DeviceInfo
{
    QString SelDev_device_name_; ///< 设备名称
    unsigned short SelDev_vid_;  ///< 设备vid
    unsigned short SelDev_pid_;  ///< 设备pid
} DeviceInfo;

namespace Ui {
class CustomQWidgetPlansSelection;
}

///
/// \brief The CustomQWidgetPlansSelection class
/// 子部件：
///     四个方案库按键
///     文字label: "算法"
///     自定义按键  CustomPushButton
/// 布局：
///     水平布局
///     网格布局(整体)
class CustomQWidgetPlansSelection : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetPlansSelection(QWidget *parent = nullptr);
    ~CustomQWidgetPlansSelection();

    void updatePlansUIInfo(DeviceInfo sel_device_info); /// 根据机型 更新首页 预设 UI 信息

    void updatePlansUIInfo(
        QPair<QString, QString>
            target_plan_Key); /// 所有按键置为非选中状态 根据机预设键值 找到对应的方案，使其为选中状态
    void retranslateTexts();
    NewRadioBtn *findHomePlan(const QPair<QString, QString> &planKey) const;
    NewRadioBtn *firstAvailableHomePlan() const;
    bool isCurrentPlanHomePlan() const;

signals:
    void updatePlan();                                  //方案已变化,通知
    void requestHomePlanSelected(const QPair<QString, QString> &planKey);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    QPair<QString, QString> normalizePlanKey(const QPair<QString, QString> &planKey) const;

public:
    /******************** UI ********************/
    QList<std::shared_ptr<CustomQPushButtonSinglePlan>> cl_plans_; ///<
    int cl_size_ = 4;                                              ///< 默认四个

    QButtonGroup *cl_plans_buttonGroup_ = nullptr;    ///< 方案按键组
    QGridLayout *cl_gridLayout_ = nullptr;            ///< 网格布局
    QLabel *cl_text_label_ = nullptr;                 ///< 方案预设文字
    CustomPushButton *cl_customPushButton_ = nullptr; ///< 启用按键
    QHBoxLayout *cl_hBoxLayout_ = nullptr;            ///< 水平布局
    QWidget *cl_top_content_widget_ = nullptr;        ///< 顶部内容，内部为  cl_hBoxLayout_

    QPushButton *cl_pBt_explain_ = nullptr;                ///< 说明按键
    NewCustomToolTip *clp_tip_explain_ = nullptr;          ///< 自定义提示控件
    QSize cl_pBt_explain_default_size_ = QSize(13, 13);    ///< 说明按键 默认大小

    const QString icon_path_valorant_normal_ = ":/Skin/Images/homePage/plansIcons/VAL-normal.png";
    const QString icon_path_valorant_hover_ = ":/Skin/Images/homePage/plansIcons/VAL-hover.png";
    const QString icon_path_valorant_checked_ = ":/Skin/Images/homePage/plansIcons/VAL-checked.png";
    const QString icon_path_CSGO_normal_ = ":/Skin/Images/homePage/plansIcons/CSGO-normal.png";
    const QString icon_path_CSGO_hover_ = ":/Skin/Images/homePage/plansIcons/CSGO-hover.png";
    const QString icon_path_CSGO_checked_ = ":/Skin/Images/homePage/plansIcons/CSGO-checked.png";
    const QString icon_path_delta_normal_ = ":/Skin/Images/homePage/plansIcons/DF-normal.png";
    const QString icon_path_delta_hover_ = ":/Skin/Images/homePage/plansIcons/DF-hover.png";
    const QString icon_path_delta_checked_ = ":/Skin/Images/homePage/plansIcons/DF-checked.png";
    const QString icon_path_PUBG_normal_ = ":/Skin/Images/homePage/plansIcons/PUBG-normal.png";
    const QString icon_path_PUBG_hover_ = ":/Skin/Images/homePage/plansIcons/PUBG-hover.png";
    const QString icon_path_PUBG_checked_ = ":/Skin/Images/homePage/plansIcons/PUBG-checked.png";

private:
    Ui::CustomQWidgetPlansSelection *ui;
};

#endif // CUSTOM_QWIDGET_PLANS_SELECTION_H
