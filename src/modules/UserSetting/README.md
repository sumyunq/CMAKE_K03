# UserSetting -- 用户设置

## 概述

用户设置主容器，使用 QStackedWidget 管理 5 个子页面，顶部横向切换按钮栏。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `user_setting_main_page.h/cpp/ui` | `UserSettingMainPage` | 顶级容器：QStackedWidget + 顶部按钮栏 + 语言/主题持久化 + 子页面调度 |
| `UserSettingCustomUI/custom_QScrollArea_topbuttons.h/cpp` | `CustomQScrollAreaTopButtons` | 顶部 5 类型切换按钮组（浮动指示器动画 300ms OutCubic） |
| `UserSettingCustomUI/custom_QPushButton_single_settings_type.h/cpp` | `CustomQPushButtonSingleSettingsType` | 单个设置类型按钮 |

## 子模块

| 页面索引 | SubPage 枚举 | 目录 | 功能 |
|----------|-------------|------|------|
| 0 | `PersonalCenter` | `UserSettingSubModule/PersonalCenterSettings/` | 个人中心 |
| 1 | `SystemSettings` | `UserSettingSubModule/SystemSettings/` | 系统设置 |
| 2 | `InterfaceSettings` | `UserSettingSubModule/InterfaceSettings/` | 界面设置 |
| 3 | `VersionSettings` | `UserSettingSubModule/VersionSettings/` | 版本升级 |
| 4 | `ContactSettings` | `UserSettingSubModule/ContactSettings/` | 联系我们 |

## 页面切换

```
CustomQScrollAreaTopButtons
  -- changeSettingsType(int index)
       -> UserSettingMainPage 槽 (lambda)
         -> QStackedWidget::setCurrentIndex(index)
```

顶部按钮栏有浮动指示器动画（300ms OutCubic）。`InitConnect` 中 `Qt::UniqueConnection` 防重复连接。

## 子页面信息刷新

`UpdateAllSubPageUIInformation(SubPage page = All)` 按需刷新各个子页面。`All` 路径跳过 `InterfaceSettings` 的 `UpdateInterfaceSettingsUIInformation`（避免壁纸刷新卡顿），改为只调用 `syncSlidersFromModel()` 轻量同步滑块。

## 工厂复位流程

**注意**: `SystemSettingsMainPage::onFactoryResetClicked()` 直接在自身内部完成全部复位逻辑（与 `UserSettingMainPage::ResetDefaultSetting()` 代码重复），其 `factoryResetRequested()` 信号声明但从未 emit，父级也未连接。

`UserSettingMainPage::ResetDefaultSetting()` 的实际流程：

```
FactoryReset 确认弹窗 (QDialog::Accepted)
  -> delete globalSettings (QSettings)
  -> 删除 ProgramData 目录 (QStandardPaths::AppDataLocation)
  -> detach + delete m_sharedMemory
  -> QProcess::startDetached(applicationFilePath) 新进程
  -> QApplication::quit()
```

注：原 `// emit LoginPage();` 已注释，不走信号跳转。

## 国际化

| 方法 | 说明 |
|------|------|
| `LanguageSet()` | `retranslateUi(this)` + 逐级传递到 5 个子页面 |
| `saveIniValue(Language, Theme)` | 从 UI 读取语言/主题 index 到引用参数（供 MainWindow 持久化到 globalSettings） |
| `readIniValue(Language, Theme)` | 将语言/主题 index 设置到 UI（从 globalSettings 恢复） |

## 命名规范

- **新式规范**：`cl_`/`clp_` + `_` 后缀
- 小不一致：`clp_system_settings_mainPage_`（mainPage 而非 main_page）
