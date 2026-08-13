#ifndef CUSTOM_QWIDGET_MICROPHONE_ADJUSTMENT_H
#define CUSTOM_QWIDGET_MICROPHONE_ADJUSTMENT_H

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "SwitchPbtC/custom_pushbutton.h"   ///< 子部件： 自定义开关按键
#include "Popup/CustomTipPopup/NewCustomToolTip.h"           ///< 自定义提示控件

namespace Ui {
class CustomQWidgetMicrophoneAdjustment;
}

///
/// \brief The CustomQWidgetMicrophoneAdjustment class
/// 麦克风调节（人声调节）
/// 子部件：
///     三段文字label
///     一个麦克风图标
///     两个自定义开关按键 CustomPushButton
class CustomQWidgetMicrophoneAdjustment : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetMicrophoneAdjustment(QWidget *parent = nullptr);
    ~CustomQWidgetMicrophoneAdjustment();
    void retranslateTexts();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember(); ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

public:
    /******************** UI ********************/
    QLabel *cl_text_1_ = nullptr; ///< 文字：麦克风
    QSize cl_text_1_min_size_ = QSize(42,20);
    QPoint cl_text_1_point_ = QPoint(25,18);

    QLabel *cl_icon_ = nullptr; ///< 图标：麦克风图标
    QSize cl_icon_min_size_ = QSize(13,18);
    QPoint cl_icon_point_ = QPoint(128,20);


    QLabel *cl_text_2_ = nullptr; ///< 文字：人声清晰
    QSize cl_text_2_min_size_ = QSize(48, 17);
    QPoint cl_text_2_point_ = QPoint(25,58);

    QLabel *cl_text_3_ = nullptr; ///< 文字：人声浑厚
    QSize cl_text_3_min_size_ = QSize(48, 17);
    QPoint cl_text_3_point_ = QPoint(25,96);

    CustomPushButton *cl_pushButton_clear_voices_ = nullptr;      ///< 人声清晰 开关按键
    QSize cl_pushButton_clear_voices_min_size_ = QSize(46, 22);
    QPoint cl_pushButton_clear_voices_point_ = QPoint(99,55);

    CustomPushButton *cl_pushButton_deepPowerful_voice_ = nullptr; ///< 人声浑厚 开关按键
    QSize cl_pushButton_deepPowerful_voice_min_size_ = QSize(46, 22);
    QPoint cl_pushButton_deepPowerful_voice_point_ = QPoint(99,93);

    QPushButton *cl_pBt_explain_ = nullptr;                ///< 说明按键
    NewCustomToolTip *clp_tip_explain_ = nullptr;          ///< 自定义提示控件
    QPoint cl_pBt_explain_default_point_ = QPoint(71, 24); ///< 说明按键 默认位置
    QSize cl_pBt_explain_default_size_ = QSize(13, 13);    ///< 说明按键 默认大小

private:
    Ui::CustomQWidgetMicrophoneAdjustment *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QWIDGET_MICROPHONE_ADJUSTMENT_H
