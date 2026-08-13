#ifndef PERSONAL_CENTER_SETTINGS_MAIN_PAGE_H
#define PERSONAL_CENTER_SETTINGS_MAIN_PAGE_H

#include <QButtonGroup>
#include <QWidget>

#include <QList>
#include <memory>

#include "data/api_global.h" ///< 统一 API 入口

#include "modules/Common/DeviceRegistry.h" ///< deviceLabel() 设备标签匹配
#include "modules/GeneralCustomUI/custom_QWidget_notification.h" ///< 自定义弹窗:提示用户相关信息
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_loading.h" ///< 加载动画

#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_avatar_selection.h" ///< 子控件:头像编辑页面
#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_user_info_change.h" ///< 子控件:左侧用户信息变更
#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_user_info_settings.h" ///< 子控件:左侧用户信息

#include "modules/CommunityModule/ui/community/community_model.h"   ///< 已上传/已点赞 MV model
#include "modules/CommunityModule/ui/community/community_panel.h"   ///< 已上传/已点赞 MV panel
#include "modules/CommunityModule/service/scheme_service.h"        ///< 共享方案服务
#include "repository/user_config_repository.h" ///< 共享配置仓库

#include "Popup/Plans/UploadMyPlans.h"
#include "Popup/Plans/UploadPlanSuccess.h"

class QLabel;
class UserSettingMainPage;

namespace Ui {
class PersonalCenterSettingsMainPage;
}

/// \brief 更多设置 — 个人中心设置
/// 子控件：
///     CustomQWidgetUserInfoSettings      左侧用户信息
///     CustomQWidgetUserInfoChange         左侧用户信息变更
///     CustomQScrollAreaGridLayoutView     右侧方案网格
class PersonalCenterSettingsMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalCenterSettingsMainPage(QWidget *parent = nullptr,
                                            UserSettingMainPage *targetObject = nullptr,
                                            int theme = 0); ///< 构造函数
    ~PersonalCenterSettingsMainPage();                      ///< 析构函数

    void UpdatePersonalCenterSettingsUIInformation(); ///< 更新个人中心页面 UI 信息（供父级调用）
    void applyTheme(int theme);                       ///< 按主题更新样式
    void fetchUserLevel();                            ///< 获取用户等级信息
    void fetchMyUploadedConfigPage(int page = 1);     ///< 获取已上传配置（page=1 全量刷新）
    void fetchMyLikedConfigPage(int page = 1);        ///< 获取已点赞配置
    void LanguageSet();                               ///< 刷新翻译文本
    void resetData();                                 ///< 清空列表缓存（退出登录时调用）
    /// \brief 注入共享 Service（CommunityMainPage 创建，MainWindow 转发）
    void injectServices(SchemeService *svc, UserConfigRepository *repo);

    /// \brief 已上传 MV model（DataSyncCoordinator 同步用）
    CommunityModel *uploadedModel() const { return clp_uploaded_model_; }
    /// \brief 已点赞 MV model
    CommunityModel *likedModel() const { return clp_liked_model_; }
    /// \brief 已上传 MV panel（交互接线用）
    CommunityPanel *uploadedPanel() const { return clp_uploaded_panel_; }
    /// \brief 已点赞 MV panel
    CommunityPanel *likedPanel() const { return clp_liked_panel_; }

signals:
    void userInfoUpdated(); ///< 用户信息已更新，通知 MainWindow 刷新显示

private:
    void InitUIInformation(int theme); ///< 初始化 UI 默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认信号槽

    /// \brief 创建已上传/已点赞 CommunityPanel + Model（injectServices 时调用）
    void initPlansPanels();
    /// \brief 拉取已上传第 page 页（50/页）
    void fetchUploadedPage(int page);
    /// \brief 拉取已点赞第 page 页
    void fetchLikedPage(int page);

private:
    void showLoading();  ///< 显示加载中图标
    void hideLoading();  ///< 隐藏加载中图标
    void setPlansLoadingText(const QString &text); ///< 设置底部加载文字与图标布局
    void showUploadedNoMoreStatus(); ///< 显示已上传无更多数据提示
    void hideUploadedNoMoreStatus(); ///< 隐藏已上传无更多数据提示
    void updateUploadedNoMoreStatus(); ///< 按已上传滚动位置更新无更多数据提示

public:
    CustomQWidgetUserInfoSettings *clp_user_info_ = nullptr;           ///< 左侧用户信息
    CustomQWidgetUserInfoChange *clp_user_info_change_page_ = nullptr; ///< 左侧用户信息变更页
    CustomQWidgetAvatarSelection *clp_avatar_selection_ = nullptr;     ///< 头像编辑页面(需要时构造)

    CommunityPanel *clp_uploaded_panel_ = nullptr; ///< 已上传（MV）
    CommunityModel *clp_uploaded_model_ = nullptr; ///< 已上传 model
    CommunityPanel *clp_liked_panel_ = nullptr;    ///< 已点赞（MV）
    CommunityModel *clp_liked_model_ = nullptr;    ///< 已点赞 model
    CustomQWidgetLoading *clp_plans_loading_ = nullptr; ///< 已上传/已点赞底部加载动画
    QLabel *clp_uploaded_no_more_label_ = nullptr; ///< 已上传触底无更多数据提示

    SchemeService *clp_scheme_svc_ = nullptr;      ///< 注入的共享方案服务
    UserConfigRepository *clp_config_repo_ = nullptr; ///< 注入的共享配置仓库

    int cl_uploaded_page_ = 0;   ///< 已上传当前页码（MV 版分页）
    int cl_liked_page_ = 0;      ///< 已点赞当前页码
    bool cl_uploaded_fetching_ = false; ///< 已上传请求中（错误态覆盖层用）
    bool cl_liked_fetching_ = false;    ///< 已点赞请求中（错误态覆盖层用）
    bool cl_uploaded_has_more_ = true; ///< 已上传是否还有下一页
    bool cl_liked_has_more_ = true;    ///< 已点赞是否还有下一页


public:
    QString cl_selected_avatar_path_; ///< 当前选中的头像资源路径
    int cl_theme_ = 0; ///< 当前主题

private slots:
    void on_pushButton_upload_plan_clicked();

private:
    Ui::PersonalCenterSettingsMainPage *ui; ///< .ui 界面指针

private:
    UserSettingMainPage *clp_target_user_setting_main_page_ = nullptr;
    QButtonGroup *clp_tab_button_group_ = nullptr;

    UploadPlanSuccess *UploadPlanOk = nullptr;
    UploadMyPlans *UploadMyplans = nullptr;

    int GetUplodPlanCnt();
};

#endif // PERSONAL_CENTER_SETTINGS_MAIN_PAGE_H
