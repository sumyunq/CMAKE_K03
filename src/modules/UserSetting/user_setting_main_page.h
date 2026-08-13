#ifndef USER_SETTING_MAIN_PAGE_H
#define USER_SETTING_MAIN_PAGE_H

#include <QWidget>

#include "modules/GeneralCustomUI/custom_QDialog_general_tips.h" ///< 通用的 QDialog 自定义控件

#include "modules/UserSetting/UserSettingCustomUI/custom_QScrollArea_topbuttons.h" ///< 顶部设置按键组

#include "modules/UserSetting/UserSettingSubModule/ContactSettings/contact_settings_main_page.h"     ///< 联系我们 页面
#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/interface_settings_main_page.h" ///< 界面设置 页面
#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/personal_center_settings_main_page.h" ///< 个人中心设置 页面
#include "modules/UserSetting/UserSettingSubModule/SystemSettings/system_settings_main_page.h"   ///< 系统设置 页面
#include "modules/UserSetting/UserSettingSubModule/VersionSettings/version_settings_main_page.h" ///< 版本升级 页面


namespace Ui {
class UserSetMainPage;
}

/// \brief 用户设置主页面
/// 子控件：
///     - CustomQScrollAreaTopButtons: 顶部设置类型按键组
///     - stackedWidget: 多页切换（个人中心/系统/界面/版本/联系我们）
class UserSettingMainPage : public QWidget
{
    Q_OBJECT

public:
    enum SubPage {
        PersonalCenter = 0, ///< 个人中心
        SystemSettings,     ///< 系统设置
        InterfaceSettings,  ///< 界面设置
        VersionSettings,    ///< 版本升级
        ContactSettings,    ///< 联系我们
        All                 ///< 全部
    };

    explicit UserSettingMainPage(QWidget *parent = nullptr);
    ~UserSettingMainPage();

    ///< 子对象公共函数
    void DevGetVersion();                         ///<  获取耳机版本信息
    void SoftGetVersion();                        ///<  获取软件版本信息
    void saveIniValue(int &Language, int &Theme); ///< 保存 配置文件 index 值
    void readIniValue(int Language, int Theme);   ///< 读取 配置文件 index 值
    void UpdateAllSubPageUIInformation(SubPage page = All); ///< 更新子页面 UI 信息
    void LanguageSet();                              ///< 刷新翻译文本
    void ResetDefaultSetting();                   ///< 恢复默认设置
    void CloseReceiveTimer();   //关闭与设备之间进行信息传输的定时器

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    CustomQScrollAreaTopButtons *clp_topButtons_ = nullptr; ///< 顶部按键组

    PersonalCenterSettingsMainPage *clp_personal_center_settings_main_page_
        = nullptr; ///< 个人中心设置

    /// \brief 获取个人中心页（供 MainWindow 注入共享 Service）
    PersonalCenterSettingsMainPage *personalCenterPage() const
    { return clp_personal_center_settings_main_page_; }

    SystemSettingsMainPage *clp_system_settings_mainPage_ = nullptr; ///< 系统设置

    InterfaceSettingsMainPage *clp_interface_settings_main_page_ = nullptr; ///< 界面设置

    VersionSettingsMainPage *clp_version_settings_main_page_ = nullptr; ///< 版本升级

    ContactSettingsMainPage *clp_contact_settings_main_page_ = nullptr; ///< 联系我们

    CustomQDialogGeneralTips *clp_dialog_tips_ = nullptr; ///< 通用提示弹窗

private:
    Ui::UserSetMainPage *ui;

signals:

    void CloseReceiveTimer_S();//关闭与设备之间进行信息传输的定时器
};

#endif // USER_SETTING_MAIN_PAGE_H
