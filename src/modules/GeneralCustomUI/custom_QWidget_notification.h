#ifndef CUSTOM_QWIDGET_NOTIFICATION_H
#define CUSTOM_QWIDGET_NOTIFICATION_H

#include <QWidget>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>

namespace Ui {
class CustomQWidgetNotification;
}

///
/// \brief 通用通知提示部件
/// 子控件：
///     - label_notification: 提示文字
///     - pushButton_accept: 确认按钮
class CustomQWidgetNotification : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetNotification(const QString &text = QString(),
                                         const QString &btnText = QString(),
                                         QWidget *parent = nullptr,
                                         int theme = 0);
    ~CustomQWidgetNotification();

    void setNotificationText(const QString &text);     ///< 设置提示文字
    void setAcceptButtonText(const QString &text);     ///< 设置确认按钮文字
    QString notificationText() const;                  ///< 获取当前提示文字

signals:
    void accepted(); ///< 点击确认按钮时发射

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽
    void applyTheme(int theme);        ///< 按主题更新样式

private:
    Ui::CustomQWidgetNotification *ui;
    int cl_theme_ = 0; ///< 当前主题
    QGraphicsDropShadowEffect *clp_shadow_ = nullptr; ///< 阴影效果
};

#endif // CUSTOM_QWIDGET_NOTIFICATION_H
