#ifndef CUSTOM_QWIDGET_ALGORITHM_ADJUSTMENT_SETTING_H
#define CUSTOM_QWIDGET_ALGORITHM_ADJUSTMENT_SETTING_H

#include <QWidget>
#include <QGridLayout>

#include "SwitchPbtC/custom_pushbutton.h"   ///< 子部件： 自定义开关按键
#include "Popup/CustomTipPopup/NewCustomToolTip.h"           ///< 自定义提示控件

#include "modules/HomePage/HomePageCustomUI/custom_QWidget_single_algorithm_setting.h"


namespace Ui {
class CustomQWidgetAlgorithmAdjustmentSetting;
}

///
/// \brief The CustomQWidgetAlgorithmAdjustmentSetting class
/// 算法调节页面：
///     垂直布局
///     三个近乎相同的子控件
///     一个以中间开始的子控件
///     文字label: "算法"
///     自定义按键 CustomPushButton
class CustomQWidgetAlgorithmAdjustmentSetting : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetAlgorithmAdjustmentSetting(QWidget *parent = nullptr);
    ~CustomQWidgetAlgorithmAdjustmentSetting();

    ///
    /// \brief setEditStatus    设置编辑状态
    /// \param status           true:可编辑; false:不可编辑
    ///
    void setEditStatus(bool status);
    void retranslateTexts();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember(); ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽
    void InitEventFilter(); ///< 安装禁用状态下点击事件的事件过滤器

public:
    /******************** UI ********************/
    QLabel *cl_text_label_ = nullptr; ///< 算法文字
    CustomPushButton *cl_customPushButton_ = nullptr; ///< 启用按键
    QWidget *cl_content_widget_=  nullptr;  ///< 内容控件
    QHBoxLayout *cl_hBoxLayout_ = nullptr;  ///< 水平布局

    CustomQWidgetSingleAlgorithmSetting *cl_single_algorithm_setting_1_ = nullptr; /// 单个算法页面(脚步增强)
    CustomQWidgetSingleAlgorithmSetting *cl_single_algorithm_setting_2_ = nullptr; /// 单个算法页面(枪声弱化)
    CustomQWidgetSingleAlgorithmSetting *cl_single_algorithm_setting_3_ = nullptr; /// 单个算法页面(声场控制)
    CustomQWidgetSingleAlgorithmSetting *cl_single_algorithm_setting_4_ = nullptr; /// 单个算法页面(清晰度)

    QPushButton *cl_pBt_explain_ = nullptr;                ///< 说明按键
    NewCustomToolTip *clp_tip_explain_ = nullptr;          ///< 自定义提示控件
    QSize cl_pBt_explain_default_size_ = QSize(13, 13);    ///< 说明按键 默认大小

    // QObject interface
public:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::CustomQWidgetAlgorithmAdjustmentSetting *ui;

    QList<QWidget *> cl_need_checked_;  ///< 需要在禁用状态下响应点击事件的控件（算法加减按钮/滑块）
};

#endif // CUSTOM_QWIDGET_ALGORITHM_ADJUSTMENT_SETTING_H
