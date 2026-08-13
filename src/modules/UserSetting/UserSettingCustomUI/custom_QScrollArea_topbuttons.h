#ifndef CUSTOM_QSCROLLAREA_TOPBUTTONS_H
#define CUSTOM_QSCROLLAREA_TOPBUTTONS_H

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QWidget>

#include "modules/UserSetting/UserSettingCustomUI/custom_QPushButton_single_settings_type.h" ///< 子控件：自定义按键

///
/// \brief The CustomQScrollAreaTopButtons class
/// 设置界面，顶部按键组，
/// 包括：
///     个人中心、系统设置、界面设置、版本升级、联系我们
class CustomQScrollAreaTopButtons : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(QRect cl_indicatorRect READ indicatorRect WRITE setIndicatorRect)

public:
    CustomQScrollAreaTopButtons(QWidget *parent = nullptr);
    ~CustomQScrollAreaTopButtons();

    void updateLayoutView(int defaultIndex = 0); ///< 更新布局显示,可指定默认显示项

    void onSettingsTypeClicked(int index); ///< 处理对应点击的按钮（ 切换至指定设置界面 ）

    void LanguageSet();

signals:
    void changeSettingsType(int index); ///< 切换至指定设置界面

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    QRect indicatorRect() const { return cl_indicator_rect_; }
    void setIndicatorRect(const QRect &r);
    void updateIndicatorForButton(QAbstractButton *btn, bool animated); ///< 更新指示器到指定按钮
    QRect calcIndicatorRect(QAbstractButton *btn) const;               ///< 计算按钮对应的指示器矩形

public:
    QList<CustomQPushButtonSingleSettingsType *>
        cl_all_settings_type_; ///< 所有的设置类型(个人中心、系统设置、界面设置、版本升级、联系我们)
    QButtonGroup *clp_all_settings_type_buttonGroup_ = nullptr; ///< 设置类型按键组
private:
    QWidget *clp_content_widget_ = nullptr; ///< 内容显示区域
    QHBoxLayout *clp_hBoxLayout_ = nullptr; ///< 水平布局

    /// 选中指示器（浮动在 viewport 上）
    QWidget *clp_indicator_widget_ = nullptr;
    QPropertyAnimation *clp_indicator_anim_ = nullptr;
    QRect cl_indicator_rect_; ///< 指示器目标矩形（viewport 坐标），作为 Q_PROPERTY 供动画驱动

    ///布局属性
    int cl_left_margin_ = 4;
    int cl_top_margin_ = 4;
    int cl_right_margin_ = 4;
    int cl_bottom_margin_ = 4;
    int cl_spacing_ = 4; ///< 内部部件间距

protected:
    void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QSCROLLAREA_TOPBUTTONS_H
