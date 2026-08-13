#ifndef SYSTEM_SETTINGS_MAIN_PAGE_H
#define SYSTEM_SETTINGS_MAIN_PAGE_H

#include <QWidget>

//写任务计划程序头文件
#include <QProcess>
#include <Lmcons.h>
#include <comdef.h>
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "Advapi32.lib")

namespace Ui {
class SystemSettingsMainPage;
}

/// \brief 系统设置页面
/// 子控件：
///     - widget_power_on_setting: 开机自启动 开关
///     - widget_close_main_interface_setting: 关闭主界面 选项(最小化系统托盘 / 退出应用)
///     - widget_restore_factory_setting: 恢复出厂设置 按钮
class SystemSettingsMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSettingsMainPage(QWidget *parent = nullptr);
    ~SystemSettingsMainPage();

    void UpdateSystemSettingsUIInformation(); ///< 更新 UI 信息（供父级 UserSettingMainPage 调用）
    void LanguageSet();                      ///< 刷新翻译文本

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    /// TaskScheduler 开机自启动 — COM 接口方法
    HRESULT InitializeTaskService(ITaskService **ppService); ///< 初始化任务计划服务
    HRESULT CreateScheduledTask(ITaskService *pService,
                                const std::wstring &taskName,
                                const std::wstring &appPath,
                                const std::wstring &userName); ///< 创建开机自启动任务
    HRESULT DeleteScheduledTask(ITaskService *pService,
                                const std::wstring &taskName); ///< 删除开机自启动任务
    HRESULT RunScheduledTask(ITaskService *pService,
                             const std::wstring &taskName);                  ///< 立即执行任务
    bool IsTaskExists(ITaskService *pService, const std::wstring &taskName); ///< 检测任务是否存在
    std::wstring NormalizePath(const std::wstring &path); ///< 规范化路径(去引号/斜杠)
    bool IsTaskPathMatch(ITaskService *pService,
                         const std::wstring &taskName,
                         const std::wstring &expectedPath); ///< 判断任务路径是否匹配
    bool UpdateTaskPath(ITaskService *pService,
                        const std::wstring &taskName,
                        const std::wstring &newPath); ///< 更新任务中的启动路径

private slots:
    void onAutoStartToggled(bool checked);        ///< 开机自启动 开关切换
    void onAutoStartShowToggled(bool checked);    ///< 开机自启动 功能开关
    void onExitModeMinimizeToggled(bool checked); ///< 关闭主界面 → 最小化到托盘
    void onExitModeExitToggled(bool checked);     ///< 关闭主界面 → 退出应用
    void onFactoryResetClicked();                 ///< 恢复出厂设置 按钮点击

private:
    void saveToDisk();                          ///< 持久化到 globalSettings
    void syncAutoStartState();                  ///< 同步自启动状态到 UI
    void revertButtonCheck(bool currentState);  ///< 回弹按钮状态（阻止信号重入）

signals:
    void factoryResetRequested(); ///< 恢复出厂设置请求（由父级 UserSettingMainPage 处理）

private:
    ITaskService *clp_task_service_ = nullptr; ///< 任务计划服务 COM 接口
    std::wstring cl_task_name_;                ///< 计划任务名称
    std::wstring cl_app_path_;                 ///< 应用程序路径
    std::wstring cl_user_name_;                ///< 当前登录用户名

private:
    Ui::SystemSettingsMainPage *ui;
};

#endif // SYSTEM_SETTINGS_MAIN_PAGE_H
