# PersonalCenterSettings -- 个人中心

## 概述

用户个人资料页面。左侧头像/信息面板，右侧方案收藏网格（双 Tab：已上传 / 已点赞）。支持个人资料编辑、头像选择和等级显示。方案列表使用 CommunityPanel + CommunityModel（MV 渲染，已替代旧版 WidgetPool 网格）。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `personal_center_settings_main_page.h/cpp/ui` | `PersonalCenterSettingsMainPage` | 主容器。左侧用户信息 + 右侧方案网格（双 Tab + 无限滚动分页） |
| `PersonalCenterSettingsCustomUI/custom_QWidget_user_info_settings.h/cpp/ui` | `CustomQWidgetUserInfoSettings` | 只读用户信息面板（ID/头像/昵称/签名/等级/编辑退出按钮） |
| `PersonalCenterSettingsCustomUI/custom_QWidget_user_info_change.h/cpp/ui` | `CustomQWidgetUserInfoChange` | 个人信息编辑表单（昵称/签名/头像）+ 字数计数器 |
| `PersonalCenterSettingsCustomUI/custom_QWidget_avatar_selection.h/cpp/ui` | `CustomQWidgetAvatarSelection` | 头像选择弹窗（4 列 x 15 系统头像，双击选择） |
| `PersonalCenterSettingsCustomUI/custom_QWidget_grade_status.h/cpp` | `CustomQWidgetGradeStatus` | 等级控件（Lv.X + 进度条 + 经验值） |

## 用户信息面板布局

```
+---------------------------------+
|  CustomQWidgetUserInfoSettings  |
|  +------+  ID: 12345678        |
|  | 头像 |  昵称: Player       |
|  +------+  [Lv.3 ----]      |
|  角色/头衔: [主播] [大神]     |
|  签名: "Hello World"           |
|  [编辑资料]  [退出登录]        |
+---------------------------------+
          <-> 编辑模式切换
+---------------------------------+
|  CustomQWidgetUserInfoChange    |
|  昵称: [________] 5/10         |
|  签名: [________] 12/50        |
|  [选择头像]                    |
|  [取消]  [确认]                |
+---------------------------------+
```

> 2026-08-05 新增：**角色/头衔标签**（`setRoles`/`setTitles`，数据源 `g_user_information.network.roles/titles`；角色如 streamer/professional，经 `kRoleStreamer`/`kRoleProfessional` 常量判定，展示于用户信息面板；登录后 `UpdatePersonalCenterSettingsUIInformation` 刷新）。

## 架构：MV 渲染（替代 WidgetPool）

方案网格不再使用旧版 WidgetPool 动态创建/回收 widget，改为 CommunityPanel + CommunityModel（Qt Model/View）：

| 组件 | 类型 | 说明 |
|------|------|------|
| `clp_uploaded_panel_` | `CommunityPanel*` | 已上传方案面板 |
| `clp_uploaded_model_` | `CommunityModel*` | 已上传数据 model |
| `clp_liked_panel_` | `CommunityPanel*` | 已点赞方案面板 |
| `clp_liked_model_` | `CommunityModel*` | 已点赞数据 model |
| `clp_scheme_svc_` | `SchemeService*` | 注入的共享方案服务 |
| `clp_config_repo_` | `UserConfigRepository*` | 注入的共享配置仓库 |

## 依赖注入

```
CommunityMainPage 创建 SchemeService + UserConfigRepository
  -> MainWindow 转发
    -> PersonalCenterSettingsMainPage::injectServices(svc, repo)
      -> initPlansPanels()
        -> new CommunityModel + CommunityPanel
        -> connect(UserConfigRepository::myConfigsReady / myLikesReady)
        -> connect(CommunityPanel::loadMoreRequested -> 翻页)
```

## 双 Tab + 无限滚动分页

- Tab 切换：`QButtonGroup` (`clp_tab_button_group_`) 控制 `stackedWidget_plans`
- 分页参数：**50 条/页**，`UserConfigRepository::getMyConfigs(page, 50, {})` / `getMyLikes(page, 50, {})`
- 滚动触发：`CommunityPanel::loadMoreRequested` -> `fetchUploadedPage(++page)` / `fetchLikedPage(++page)`
- 首次加载：`page <= 1` 时 `CommunityModel::replaceAll()`，后续 `page > 1` 时 `CommunityModel::addItems()`
- 数据转换：静态函数 `configToItems(QList<UserConfigInfo>) -> QList<CommunityItemData>`（与社区页一致）

| 状态变量 | 说明 |
|----------|------|
| `cl_uploaded_page_` | 已上传当前页码 |
| `cl_liked_page_` | 已点赞当前页码 |
| `cl_uploaded_has_more_` | 已上传是否还有下一页（list.size() >= 50） |
| `cl_liked_has_more_` | 已点赞是否还有下一页 |

## 网络请求

使用 `DeSheng::BaseClient::instance()` + `ApiServerSwitch::serverKey()` 统一模式：

| 操作 | 实现 |
|------|------|
| 获取用户等级 | `BaseClient::get(ApiPaths::kUserLevel, serverKey)` -> `GetUserLevelResponse` |
| 已上传方案分页 | `UserConfigRepository::getMyConfigs(page, 50, filters)` |
| 已点赞方案分页 | `UserConfigRepository::getMyLikes(page, 50, filters)` |

非网络操作：
- `fetchUserLevel()`：更新 `CustomQWidgetGradeStatus`（Lv.X / 进度条 / 经验值）
- `resetData()`：清空 MV 数据 + 重置分页状态 + 切回默认 tab（退出登录时调用）

## 信号链

```
编辑按钮 -> 切换 CustomQWidgetUserInfoSettings <-> CustomQWidgetUserInfoChange
确认 -> PUT /user -> 成功 -> emit userInfoUpdated()
  -> UserSettingMainPage -> MainWindow::refreshUserDisplay()
头像选择 -> CustomQWidgetAvatarSelection::avatarSelected(int index)
  -> 下载所选头像 PNG -> 更新 UI
```

## 命名规范

- **完全遵循新式规范**（`cl_`/`clp_`/`t_`）
- `UploadPlanOk` (`UploadPlanSuccess*`) 和 `UploadMyplans` (`UploadMyPlans*`) 为旧式命名

## API 变更（2026-07-20）

- 本地 `QNetworkAccessManager` 删除 -> 统一 `BaseClient`
- 5 个 fetch/update/upload 方法迁移：get/post/put + serverKey
- 移除 `createRequest` / `user_token` / `clp_network_manager_`
- 补 `<QNetworkReply>` + `<QNetworkRequest>` include
- **2026-08 架构变更**：方案网格从 WidgetPool 池化 -> CommunityPanel + CommunityModel MV 渲染；新增 `injectServices()` 依赖注入
