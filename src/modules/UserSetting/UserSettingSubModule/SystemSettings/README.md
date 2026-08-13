# SystemSettings -- 系统设置

## 概述

系统级设置：Windows 开机自启、关闭主界面行为、恢复出厂设置。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `system_settings_main_page.h/cpp/ui` | `SystemSettingsMainPage` | 主页面：三个设置区 + 语言翻新 + TaskScheduler COM 管理 |

## 功能

### 开机自启

```
pBt_SelfStart::toggled
  -> onAutoStartToggled(bool checked)
    -> Windows Task Scheduler COM API (ITaskService)
      -> CreateScheduledTask / DeleteScheduledTask

pBt_SelfStart_showWidget::toggled
  -> onAutoStartShowToggled(bool checked)
    -> 控制开机时是否显示主窗口 (g_user_system_settings_config_info.is_auto_start_show_widget)
```

- 基于 Windows 任务计划程序（`ITaskService` COM 接口），非注册表 Run 键
- 头文件引入：`<taskschd.h>`, `<Lmcons.h>`, `<comdef.h>`，链接 `taskschd.lib` + `Advapi32.lib`
- `pBt_SelfStart_showWidget` 受 `pBt_SelfStart` 状态控制（自启关闭时禁用子控件）

#### TaskScheduler COM 接口方法

| 方法 | 说明 |
|------|------|
| `InitializeTaskService(ITaskService **ppService)` | 初始化 COM 任务计划服务 |
| `CreateScheduledTask(ITaskService*, taskName, appPath, userName)` | 创建开机自启动任务 |
| `DeleteScheduledTask(ITaskService*, taskName)` | 删除开机自启动任务 |
| `RunScheduledTask(ITaskService*, taskName)` | 立即执行任务 |
| `IsTaskExists(ITaskService*, taskName)` | 检测任务是否存在 |
| `NormalizePath(path)` | 规范化路径（去引号/斜杠） |
| `IsTaskPathMatch(ITaskService*, taskName, expectedPath)` | 判断任务路径是否匹配当前程序 |
| `UpdateTaskPath(ITaskService*, taskName, newPath)` | 更新任务中的启动路径 |

### 关闭行为

```
radioButton_minimize / radioButton_exit
  -> onExitModeMinimizeToggled / onExitModeExitToggled
    -> g_user_system_settings_config_info.is_exit_directly
```

- 最小化到系统托盘 / 直接退出
- "关闭前询问"复选框 `checkBox_close_ask` -> `is_remember_choice`（取反存储）

### 恢复出厂

```
pBt_reset::clicked
  -> onFactoryResetClicked()
    -> FactoryReset 确认弹窗
      -> delete globalSettings
      -> 删除 ProgramData 目录
      -> detach + delete m_sharedMemory
      -> QProcess::startDetached 新进程
      -> QApplication::quit()
```

**重要**: `onFactoryResetClicked()` 在 `SystemSettingsMainPage` 内部直接完成全部复位逻辑，代码与 `UserSettingMainPage::ResetDefaultSetting()` 重复。`factoryResetRequested()` 信号声明在头文件（line 72），但**从未 emit，父级 `UserSettingMainPage` 也未连接**——该信号是死代码。

## 数据存储

- `g_user_system_settings_config_info`：4 个 `std::atomic<bool>`（内存）
  - `is_auto_start` — 开机自启
  - `is_auto_start_show_widget` — 自启时显示窗口
  - `is_exit_directly` — 关闭时直接退出
  - `is_remember_choice` — 记住关闭选择
- `saveToDisk()`：持久化到 `globalSettings` QSettings INI

## 命名规范

- **混合风格**：COM 接口用原始指针（`pService`/`ppService`），UI 成员用 `cl_` 前缀
- `saveToDisk()` 方法和槽命名遵循现代模式
- 无 `applyTheme()` 和 `cl_theme_`（硬编码暗色主题）

## 国际化

- `LanguageSet()`：调用 `ui->retranslateUi(this)`
