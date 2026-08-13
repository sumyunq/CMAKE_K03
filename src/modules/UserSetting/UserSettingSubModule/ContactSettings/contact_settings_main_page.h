#ifndef CONTACT_SETTINGS_MAIN_PAGE_H
#define CONTACT_SETTINGS_MAIN_PAGE_H

#include <QWidget>

#include "FeedBackC/UserFeedBack/feedback_main_page.h" ///< 用户反馈界面
#include "FeedBackC/UserFeedBack/custom_QWidget_feedback_return_page.h" ///< 用户反馈回显界面

class UserSettingMainPage;

namespace Ui {
class ContactSettingsMainPage;
}

/// \brief 联系我们 页面
/// 子控件：
///     - widget_userFeedBack: 左侧面板 — 用户反馈界面 FeedbackMainPage
///     - widget_2: 右侧面板 — 服务电话、官网链接、社群、二维码
class ContactSettingsMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit ContactSettingsMainPage(QWidget *parent = nullptr,
                                     UserSettingMainPage *targetObject = nullptr,
                                     int theme = 0);
    ~ContactSettingsMainPage();

    void applyTheme(int theme);              ///< 按主题更新样式
    void UpdateContactSettingsUIInformation(); ///< 更新 UI 信息
    void LanguageSet();                        ///< 刷新翻译文本

    /// \brief 清除 label_QR_code 当前图片并设置新的 pixmap，缩放至图标大小
    void setQrCodePixmap(const QPixmap &t_pixmap);

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽

public:
    FeedbackMainPage *clp_feedBackPage_ = nullptr; ///< 用户反馈界面
    CustomQWidgetFeedBackReturnPage *clp_feed_back_return_page = nullptr; ///< 用户反馈回显界面
    int cl_theme_ = 0;                             ///< 当前主题

private:
    Ui::ContactSettingsMainPage *ui;
    UserSettingMainPage *clp_target_user_setting_main_page_ = nullptr; ///< 需要用到其部分通用函数

    // QWidget interface
protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override; ///< 事件过滤器（官网链接点击）
};

#endif // CONTACT_SETTINGS_MAIN_PAGE_H
