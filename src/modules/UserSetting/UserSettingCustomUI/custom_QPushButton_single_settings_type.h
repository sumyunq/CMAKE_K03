#ifndef CUSTOM_QPUSHBUTTON_SINGLE_SETTINGS_TYPE_H
#define CUSTOM_QPUSHBUTTON_SINGLE_SETTINGS_TYPE_H

#include <QObject>
#include <QPushButton>
#include <QWidget>

/// \brief 单个设置类型按键
/// 用于用户设置页面的顶部类型切换
class CustomQPushButtonSingleSettingsType : public QPushButton
{
    Q_OBJECT
public:
    CustomQPushButtonSingleSettingsType(QWidget *parent = nullptr);
    ~CustomQPushButtonSingleSettingsType();

    QString cl_settings_type() const;
    void setCl_settings_type(const QString &newCl_settings_type);

    bool cl_is_show() const;
    void setCl_is_show(const bool newCl_is_show);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

private:
    ///< 整体 Pushbutton 的尺寸
    QSize cl_min_size_ = QSize(91, 36);
    QSize cl_max_size_ = QSize(91*2, 36*2);

    QString cl_settings_type_ = "";        ///< 设置类型 名称,默认空
    bool cl_is_show_ = false; ///< 是否在布局中显示,默认false
};

#endif // CUSTOM_QPUSHBUTTON_SINGLE_SETTINGS_TYPE_H
