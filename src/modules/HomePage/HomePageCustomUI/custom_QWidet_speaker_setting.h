#ifndef CUSTOM_QWIDET_SPEAKER_SETTING_H
#define CUSTOM_QWIDET_SPEAKER_SETTING_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSequentialAnimationGroup>

#include "CustomControl/NewComboBox.h"   ///< 子部件： 自定义下拉框
#include "CustomControl/CustomSlider/NewHSlider.h"   ///< 子部件： 自定义水平进度条
#include "SwitchPbtC/custom_pushbutton.h"   ///< 子部件： 自定义开关按键
#include "CustomControl/TextColorAnimator/TextColorAnimator.h"   ///< 动画

namespace Ui {
class CustomQWidetSpeakerSetting;
}
///
/// \brief The CustomQWidetSpeakerSetting class
/// 扬声器设置
/// 子部件（部分自定义）：
///     扬声器图标
///     扬声器 NewComboBox
///     开关按键 CustomPushButton
///     声音大小进度条 NewHSlider
class CustomQWidetSpeakerSetting : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidetSpeakerSetting(QWidget *parent = nullptr);
    ~CustomQWidetSpeakerSetting();

    ///
    /// \brief changeTextColor
    /// \param targetStatus 目标状态 true:启用状态; false:禁用状态
    /// \param isAnimation 是否开启动画
    ///
    void changeTextColor(bool targetStatus,bool isAnimation); ///< 是否动画启动文字渐变

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    /******************** UI ********************/
    QLabel *cl_icon_ = nullptr; ///< 图标
    QSize cl_icon_min_size_= QSize(25,25);
    QPoint cl_icon_point_= QPoint(30,25);

    NewComboBox *cl_spk_ = nullptr; ///< 扬声器
    QSize cl_spk_min_size_= QSize(200,14);
    QPoint cl_spk_point_= QPoint(65,31);

    CustomPushButton *pBt_spk_switch_ = nullptr; ///< 开关按键
    QSize pBt_spk_switch_min_size_= QSize(45,22);
    QPoint pBt_spk_switch_point_= QPoint(452,26);

    NewHSlider *cl_HSlider_ = nullptr; ///< 水平进度条，表示音量大小
    QSize cl_HSlider_min_size_ = QSize(462, 50);
    QPoint cl_HSlider_point_ = QPoint(30, 62);

    TextColorAnimator *cl_animTarget_M_ = nullptr; /// 自定义 文本文字颜色变化
    QSequentialAnimationGroup *cl_sequential_animation_group_ = nullptr; ///动画组
    QPropertyAnimation *cl_animation_M_ = nullptr; ///<创建动画对象
    // 存储三色渐变
    QVector<QColor> cl_color_change_ = {QColor("#B2A1A8B3"),
                                        QColor("#FFA1A8B3"),
                                        QColor("#B2A1A8B3")};
    QGraphicsOpacityEffect *cl_icon_Effect_ = nullptr; ///< 图标 淡入淡出效果
    QPropertyAnimation *cl_icon_fadeOut_animation_ = nullptr;///< 图标 淡出动画

private:
    Ui::CustomQWidetSpeakerSetting *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QWIDET_SPEAKER_SETTING_H
