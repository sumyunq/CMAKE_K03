#ifndef HOME_PAGE_MAIN_PAGE_H
#define HOME_PAGE_MAIN_PAGE_H

#include <QDebug>
#include <QSlider>
#include <QWidget>
#include <QtConcurrent>

#include "modules/HomePage/HomePageCustomUI/custom_QWidet_speaker_setting.h"     /// 扬声器设置 头文件
#include "modules/HomePage/HomePageCustomUI/custom_QWidget_microphone_setting.h" /// 麦克风设置 头文件
#include "modules/HomePage/HomePageCustomUI/custom_QWidget_product_display.h"    /// 产品展示页面 头文件

#include "modules/HomePage/HomePageCustomUI/custom_QWidget_algorithm_adjustment_setting.h" /// 算法调节页面 头文件

#include "modules/HomePage/HomePageCustomUI/custom_QWidget_microphone_adjustment.h" /// 麦克风调节（人声调节） 头文件
#include "modules/HomePage/HomePageCustomUI/custom_QWidget_plans_selection.h"       /// 方案选择 头文件

extern QList<int> HomePageExtraEQValue; ///< 首页 额外eq
extern bool HomePageExtraEQOpen;    ///<  首页 额外eq 开关
extern bool HomePagePlansOpen;   ///< 首页预设方案开关

namespace Ui {
class HomePageMainPage;
}

///
/// \brief The HomePageMainPage class
/// 登录成功后的软件主页
/// 子部件：
///     产品展示页面
///     音量调节页
///     麦克风调节页
///     算法调节页面
///     人声调节页面
class HomePageMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageMainPage(QWidget *parent = nullptr);
    ~HomePageMainPage();

    void updateUIInfo();
    void LanguageSet();



signals:
    void HomePageEQValueChange();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    /************************************* UI *************************************/
    CustomQWidgetProductDisplay *cl_product_display_ = nullptr; ///< 产品展示页面
    QPoint cl_product_display_point_;                           /// 产品展示页面 坐标
    QSize cl_product_display_size_;                             /// 产品展示页面 尺寸

    CustomQWidetSpeakerSetting *cl_speaker_setting_ = nullptr; ///< 声音设置页面

    CustomQWidgetMicrophoneSetting *cl_microphone_setting_ = nullptr; ///< 麦克风设置页面

    CustomQWidgetAlgorithmAdjustmentSetting *cl_algorithm_adjustment_setting
        = nullptr; ///< 算法调节页面

    CustomQWidgetPlansSelection *cl_plans_selection_ = nullptr;             ///< 方案选择页面
    CustomQWidgetMicrophoneAdjustment *cl_microphone_adjustment_ = nullptr; ///< 麦克风调节（人声调节）

    // QButtonGroup *cl_button_group_
    //     = nullptr; ///< 互斥按键组，针对 方案选择页面 cl_plans_selection_->cl_customPushButton_ 和 算法调节页面 cl_algorithm_adjustment_setting->cl_customPushButton_

private:
    Ui::HomePageMainPage *ui;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;   ///< 每次显示刷新算法值/预设选中（恢复时机全覆盖）
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // HOME_PAGE_MAIN_PAGE_H
