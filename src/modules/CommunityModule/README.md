# CommunityModule — 社区方案广场

## 概述

社区方案广场模块，支持方案广场 / 大神分享 / 官方预设三个 Tab 的方案浏览、点赞/踩、收藏、下载、分享、评论标签交互。与个人中心（已上传/已点赞列表）通过 `DataSyncCoordinator` 双向联动。

模块代码从 **WidgetCMake** 项目迁移而来，采用五层架构；其网络栈（HttpClient/ServerRouter/AuthStore）自 2026-08-04 起为**全项目唯一网络层**（旧 ServerAPI 网络栈已删除，接口契约引用 `data/`）。

## 五层架构

```
ui/              ← 视图层：QAbstractItemView + QStyledItemDelegate + QAbstractListModel（view 体系，逻辑不动）
service/         ← 业务层：乐观更新 + 异步确认 + 失败回滚
```

**2026-08-04 标准目录迁移后**（详见 `doc/reports/directory_unification_plan_20260804.md`）：
- `repository/` → 已迁至 `src/repository/`（paginated_repository / user_config_repository / scheme_repository / **ranking_helper**（新增，排行榜数据获取，点赞/下载 × 月度/总榜，仅耳机））
- `infrastructure/network/` → 已迁至 `src/network/`（HttpClient/ServerRouter/AuthStore/RequestOptions/DownloadRepository/AvatarCache——**全项目唯一网络层**；`infrastructure/rendering/avatar_cache` 随迁至 `src/network/avatar_cache`）
- `entity/`（user_config.h / scheme.h）+ `shared/`（common_fields.h）→ 已删除；DTO 统一由 `src/data/` 提供（接口契约层，如 `GetPublicConfigurationListResponse::ListItem`）；`community_item_data.h` → `src/model/`（UI 展示模型）
- 模块内仅保留 `ui/`、`service/` 及 `infrastructure/` 的 logger / qt_compat；依赖方向：`ui` → `service` → `src/repository` → `src/network` + `src/data`

## 网络栈（全项目统一）

CommunityModule 不依赖旧网络栈（`DeSheng::BaseClient` / `DeSheng::ApiConfig` 已于 2026-08-04 删除），网络组件即全项目统一新栈（位于 `src/network/`）：

| 组件 | 类 | 说明 |
|------|-----|------|
| HTTP 客户端 | `HttpClient` | 饿汉单例，Builder 模式（`RequestOptions`），get/post/upload |
| 鉴权存储 | `AuthStore` | 线程安全单例，`tokenChanged` / `tokenExpired` 信号 |
| 服务器路由 | `ServerRouter` | 三级优先级：请求级 `serverKey` > Tag 路由 > 前缀路由 > 全局默认 |
| 请求选项 | `RequestOptions` | C++17 链式 API（`withQuery`/`withBody`/`withTag`/`noAuth` 等） |
| 文件下载 | `DownloadRepository` | 文件下载到临时目录 |
| 头像缓存 | `AvatarCache` | URL → QPixmap 缓存（排行榜前三名头像/社区头像共用） |

### 使用示例

```cpp
#include "network/http_client.h"
#include "network/request_options.h"

auto &cli = HttpClient::instance();

// 简单 GET
auto *r = cli.get("/user-configs");

// GET + query + tag 路由（指定业务 Tag 决定服务器）
auto *r = cli.get("/user-configs", RequestOptions{}
    .withQuery({{"is_official_tag", "true"}, {"page", "1"}})
    .withTag("schemes_official"));

// POST + 指定服务器 + 不带 token（登录接口）
auto *r = cli.post("/user/login", RequestOptions{}
    .withBody(QJsonDocument(body).toJson())
    .withServer("domestic-t")
    .noAuth());
```

### ServerRouter 路由配置

```cpp
#include "network/server_router.h"

auto &r = ServerRouter::instance();
r.registerServer("domestic",   "https://hubsys.xiberia.net/api/v1");
r.registerServer("domestic-t", "https://hubsystest.xiberia.net/api/v1");
r.setDefaultServer("domestic");

// Tag 路由：三个方案 Tab 独立控制
r.setTagDefault("schemes_expert",   "domestic");
r.setTagDefault("schemes_official", "domestic-t");

// 前缀路由：firmware/drive 走海外
r.setPrefixDefault("/firmware/", "overseas");
```

## 实体层 (entity/) — 已删除

原 `entity/user_config.h`（`UserConfigInfo` / `CommentItem`）与 `entity/scheme.h`（`SchemeInfo` / `SchemeCreateRequest`）已随标准目录迁移删除，DTO 统一由 `src/data/` 提供：

| 原实体 | 现位置 | 说明 |
|--------|--------|------|
| `UserConfigInfo` | `data/userConfig/user_config_api.h` → `DeSheng::GetPublicConfigurationListResponse::ListItem`（含嵌套 `Author` / `Comment`） | 方案列表 DTO；个人库/详情另有 `GetMyConfigurationListResponse::ListItem`、`GetConfigurationDetailsResponse` 等 |
| `SchemeInfo` / `SchemeCreateRequest` | `data/userConfig/user_config_api.h` → `DeSheng::UserConfigsCreateRequest` 等 | 分享码/创建请求 DTO |
| `CommunityItemData` | `src/model/community_item_data.h` | UI 展示模型；`configToItems()`（community_page_widget.cpp）由 `ListItem` 映射 |

### ListItem 关键字段（对应原 UserConfigInfo）

- 用户信息：`author`（user_id / username / avatar / nickname / level / roles / titles）
- 方案字段：`title` / `description` / `language` / `userTags` / `deviceName` / `deviceType` / `visibility`
- 互动计数：`likeCount` / `dislikeCount` / `downloadCount` / `shareCount` / `collectCount` / `hotScore` / `likeDislikeScore`
- 互动状态：`isLiked` / `isDisliked` / `isCollected` / `comments`（`Comment`：id / comment_text / count / is_clicked）
- 标签：`isOfficialTag` / `isExpertTag`（新 DTO 无 `isPinned` 字段，接口未返回，模型保持默认 false）
- 版本：`driveVersion` / `firmwareVersion`

### CommunityItemData 四区布局（与 CustomQWidgetSinglePlans 一致）

```
widget_00: 置顶条（可选，置顶方案显示；开启后 widget_01~04 经 pinnedOffset 整体下移）
widget_01: 头像+昵称+等级+更多（顶部信息区）
widget_02: 评论区（折叠态 2 行标签 + 展开/收起）
widget_03: 方案信息（图标+名称+描述+徽章）
widget_04: 操作栏（点赞/踩/下载/分享 + 计数）
```

> 2026-08-05 新增：**置顶**（`is_pinned`，置顶条 widget_00 + `pinnedOffset` 布局下移；`setPinnedBarEnabled` **默认关闭**，仅个人中心"已上传"显式开启并按服务器 `is_pinned` 渲染；社区广场/大神/官方与"已点赞"不显示）、**可见性**（`visibility` public/private，private 图标展示）。

## 共享字段组 (shared/) — 已删除

原 `shared/common_fields.h`（`EntityMeta` / `AuthorBrief` / `DeviceBinding` / `InteractionCounts` / `VersionInfo` / `DownloadSource` / `AdminControl` 字段组 + `json_util::` 辅助函数）已删除；字段组聚合模式取消，DTO 结构体直接内联定义全部字段。原 `json_util` 辅助函数（`jsonString` / `jsonInt` / `jsonBool` / `jsonStringList` / `jsonChild`）在 `src/repository/user_config_repository.cpp` 内联保留（见该文件注释）。

## 基础设施层 (infrastructure/)

模块内 `infrastructure/` 仅剩两个组件（网络/头像缓存组件已随迁移移出）：

### 日志 (`infrastructure/logger/`)

基于 **spdlog** 的异步日志系统。后台线程池 + 无锁队列，同时输出到控制台和每日滚动文件。

> **迁移残留**：旧 WidgetCMake 项目的日志文件名为 `widgetcmake.log`，迁移后使用 `QCoreApplication::applicationName()` 动态命名（实际文件名为 `XIBERIA X HUB_debug.log` / `XIBERIA X HUB_release.log`）。

```cpp
#include "modules/CommunityModule/infrastructure/logger/logger.h"

Logger::init();                           // 默认路径 + 保留 30 天
Logger::init("/custom/path", 60);         // 自定义路径 + 保留天数
LOG_INFO("Server: {}", url);
LOG_ERROR("Request failed: {}", reason);
Logger::shutdown();                       // 退出前
```

**日志格式**：`YYYY-MM-DD HH:MM:SS.ms [LEVEL] [文件:行号 函数名] 消息`

**便捷宏**：`LOG_TRACE` / `LOG_DEBUG` / `LOG_INFO` / `LOG_WARN` / `LOG_ERROR`，自动携带文件、行号、函数名。

**敏感信息脱敏（强制约定）**：Release 构建（NDEBUG）下敏感值统一打码为 `***`，Debug 构建原样输出（调试期全量）。凡含 token/密码/激活码/分享码等敏感数据的日志参数**必须**走 `LOG_REDACT`，禁止裸打：

```cpp
LOG_INFO("token={}", LOG_REDACT(token).toStdString());  // Release → "***"；Debug → 原值
```

### Qt 兼容层 (`infrastructure/compat/`)

`qt_compat.h` 提供 `connectOnce()` 模板函数，模拟 Qt 6.0 的 `Qt::SingleShotConnection`，在 Qt 5.15 上实现单次信号连接。

```cpp
#include "modules/CommunityModule/infrastructure/compat/qt_compat.h"

connectOnce(repo, &UserConfigRepository::configLiked, this, [this, configId] {
    // 只执行一次，自动断开
});
```

## 仓库层 (repository/)

已迁至 **`src/repository/`**（仓库层为全项目共享，个人中心等模块同样使用），include 相对 `src/` 根：

| 文件 | 类 | 说明 |
|------|-----|------|
| `paginated_repository.h/.cpp` | `PaginatedRepository` | 分页基类：`parsePaginated()` + `buildPageQuery()` |
| `user_config_repository.h/.cpp` | `UserConfigRepository` | 方案 CRUD + 互动 + 评论 + 管理后台（60+ 信号） |
| `scheme_repository.h/.cpp` | `SchemeRepository` | 分享码 CRUD + 管理后台 |
| `ranking_helper.h/.cpp` | `RankingHelper` | **2026-08-04 新增**：排行榜数据获取（点赞榜/下载榜 × 月度/总榜，仅耳机）；`fetchTop()` 异步拉取 + 回调（ranking_list 与 Community 前三名头像共用） |

```cpp
#include "repository/user_config_repository.h"
```

### PaginatedResult

```cpp
struct PaginatedResult {
    QJsonArray items;   // 分页数据项
    int page = 1;       // 当前页码
    int pageSize = 20;  // 每页大小
    int total = 0;      // 总条目数
    bool hasMore() const; // page * pageSize < total
};
```

### UserConfigRepository 操作分类

| 分类 | 操作 |
|------|------|
| 浏览 | `getPublicConfigs` / `getConfigDetail` / `downloadConfig` / `downloadByShareCode` |
| CRUD | `createConfig` / `updateConfig` / `deleteConfig` |
| 个人库 | `getMyConfigs` / `getUserConfigs` / `getMyCollects` / `getMyLikes` |
| 互动 | `like` / `unlike` / `dislike` / `undislike` / `collect` / `uncollect` / `pin` / `unpin` |
| 评论 | `getComments` / `clickComment` / `unclickComment` |
| 计数 | `getTodayCount` / `getPinnedCount` |
| 文件 | `uploadUserFile` / `downloadConfigFile` / `fetchAvatar` |
| 管理后台 | `adminGetConfigs` / `adminUpdateStatus` / `adminDeleteConfig` / `adminSetTags` / `admin*Comments` |

## 服务层 (service/)

`SchemeService` — 三合一业务服务：方案广场 / 大神分享 / 官方预设。封装 `UserConfigRepository`，对 UI 层暴露统一的分页查询 + 互动操作接口。

```cpp
#include "modules/CommunityModule/service/scheme_service.h"

auto *svc = new SchemeService(parent);
svc->init(repo);  // 注入 UserConfigRepository

// Tab 分页加载
svc->fetchSquare(1, "new", "headset", "T10");   // 方案广场
svc->fetchExpert(1, "headset");                  // 大神分享（is_expert_tag=true, sort=hot）
svc->fetchOfficial(1, "headset");                // 官方预设（is_official_tag=true, sort=hot）

// 互动操作（乐观更新 + 异步确认 + 失败回滚）
svc->toggleLike(configId, true);
svc->toggleDislike(configId, false);
svc->toggleCollect(configId, true);
svc->download(configId);
svc->share(configId);
svc->toggleCommentClick(configId, commentId, true);
```

### 乐观更新模式

1. UI 立即更新（乐观假设成功）
2. 异步确认：API 成功 → 调 `refreshCounts()` 纠正计数
3. 失败回滚：`errorOccurred` 信号通知 UI 恢复原值

## UI 层 (ui/)

### CommunityMainPage — 自包含入口

不依赖 `MainWindow` / `AppBootstrap`，可直接嵌入任何 `QWidget` 容器。主界面社区 Tab 的 `src/Community` 容器（含顶部排行榜入口按钮）即内嵌本页。

```cpp
#include "modules/CommunityModule/ui/community/community_main_page.h"

auto *page = new CommunityMainPage(parent);
page->setServer("domestic", "https://hubsys.xiberia.net/api/v1");  // ① 目标服务器
page->setAuthToken(token);                                          // ② 登录 token
page->loadInitialData();                                            // ③ 加载数据
```

### CommunityPageWidget — 页面容器

封装 Panel + 筛选栏（关键词搜索/排序/上传/国内海外）+ Service + 分页。自包含 Widget，三个 Tab 通过 `TabState` 结构体管理状态，消除 `switch(cl_active_tab_)` 分支。

**筛选栏功能**（2026-08-04 交互样式）：
- 关键词搜索输入框 254×32，`padding-left: 0`（左侧 15×15 搜索图标紧跟文字）
- 排序按钮 30×32：`Filter-no.png` / `Filter-ho.png` 图标，点击弹出排序 Dialog（最新/热门/下载/点赞/收藏/得分），选择后刷新当前 Tab
- 刷新/回顶按钮：工具栏版 30×30（`Refresh-no/ho.png` / `BackTop-no/ho.png`，滚动条在顶部=0 时显示）；右下角浮动版 40×40（滚动离开顶部时显示）——**各 tab 独立显隐**（滚动守卫：仅当前活动 tab 的滚动条 `valueChanged` 驱动显隐；切 tab 时调 `updateScrollButtons()` 立即刷新）
- 上传方案按钮 104×30：`confirm-no/ho.png` 图（与个人中心 `pushButton_upload_plan` 一致）；点击先做今日计数预检（≥10 提示上限），再弹 `UploadMyPlans` + `UploadPlanSuccess`
- 国内/海外服务器开关：国内 104×30、海外 58×32 圆角 19，互斥切换；海外服务器未上线，暂隐藏
- 设备类型固定"耳机"（设备下拉已移除）
- 排行榜入口：由外层容器 `src/Community` 提供（顶部排行榜入口按钮 + `modules/CommunityModule/ui/ranking/`（RankingList）弹窗），数据源 `RankingHelper::fetchTop`（`src/repository/ranking_helper.h`）

### CommunityPanel — 3-View Facade

`QStackedWidget` 切换三个 View（方案广场/大神分享/官方预设），每个 View 组合 `CommunityFlowView` + `CommunityDelegate` + `CommunityModel`。

### CommunityModel — 数据模型

`QAbstractListModel` 子类，K03 全字段（30+ role）：

```cpp
enum CommunityRole {
    AvatarRole = Qt::UserRole + 1,  // QPixmap
    UserIdRole, NameRole, NicknameRole, AuthorLevelRole,
    IsOfficialRole, IsExpertRole, IsStreamerRole, IsProfessionalRole,
    PlanNameRole, DescriptionRole, TagsRole, DeviceNameRole, DeviceTypeRole, LanguageRole,
    LikeCountRole, DislikeCountRole, DownloadCountRole, ShareCountRole, CollectCountRole, HotScoreRole,
    IsLikedRole, IsDislikedRole, IsCollectedRole,
    ExpandedRole, CommentsRole,
    DriveVersionRole, FirmwareVersionRole,
    StatusRole, VisibilityRole, IsPinnedRole, CreatedAtRole
};
```

### CommunityDelegate — 自绘卡片委托

`QStyledItemDelegate` 子类，330x300 卡片，与 `CustomQWidgetSinglePlans` 布局一致。支持：
- **HoverZone 命中检测**：头像/名称/方案名/展开栏/点赞/踩/下载/分享/更多 等 10 个区域
- **评论标签流式布局**：折叠态 2 行 + 展开/收起，`hitTest` 精确到标签 ID
- **操作按钮三态图**：normal / hover / checked，`QSS` 资源图切换
- **下载进度环**：`setDownloadProgress(userId, percent)` 在下载按钮上绘制环
- **徽章系统**：官方/大神/主播/职业 四种徽章，背景图 + 文字，可独立开关；**判定规则**（2026-08-06 确认）：主播/官方/职业 = 作者身份（`author.roles` 含 `kRoleStreamer`/`kRoleOfficial`/`kRoleProfessional`），大神 = 方案标签 `is_expert_tag`（管理端精选，`is_official_tag`/`is_expert_tag` 同时用于大神分享/官方预设 Tab 筛选）
- **置顶条（widget_00，2026-08-05）**：`setPinnedBarEnabled(bool)` 开关 + `pinnedOffset()` 置顶时 widget_01~04 整体下移
- **可见性（2026-08-05）**：`visibility` public/private 图标展示
- **布局常量**：全部硬编码为 `static constexpr`，含颜色/尺寸/间距/字体

### CommunityFlowView — 流式布局视图

`QAbstractItemView` 子类，自定义多垂直列并列布局。根据视口宽度动态计算列数（最小卡片宽 342px），所有布局缓存在 `QHash<int, FlowItemLayout>` 中。覆盖全部必须实现的虚函数（`visualRect` / `indexAt` / `scrollTo` / `moveCursor` 等）。

### DataSyncCoordinator — 5-Model 同步协调器

社区操作与个人中心"已上传/已点赞"列表双向联动。管理 5 个 `CommunityModel`：

| Model | 说明 |
|-------|------|
| `clp_left_models_[0]` | 方案广场（社区左侧 Tab 1） |
| `clp_left_models_[1]` | 大神分享（社区左侧 Tab 2） |
| `clp_left_models_[2]` | 官方预设（社区左侧 Tab 3） |
| `clp_uploaded_model_` | 个人中心已上传 |
| `clp_liked_model_` | 个人中心已点赞 |

```cpp
#include "modules/CommunityModule/ui/data_sync_coordinator.h"

auto *coordinator = new DataSyncCoordinator(parent);
coordinator->init(svc, repo, leftModels, uploadedModel, likedModel,
                  uploadedPanel, likedPanel);
```

**核心功能**：
- **头像同步**：`setupAvatarFetch()` — 任一 model 新增 item 时触发头像下载，下载完成后广播到所有 5 个 model
- **数据同步**：`setupDataSync()` — 社区点赞/踩/下载/收藏/删除操作后同步更新个人中心对应列表
- **取消点赞缓存**：`cl_removed_liked_cache_` 缓存已取消点赞的 item，再次点赞时从缓存恢复（零重新拉取）
- **下载进度分发**：`cl_download_panels_` 按 `configId` 分发下载进度到对应面板（避免 `disconnect-all` 竞态）

### SchemeFilterPopup — 条件筛选弹窗（2026-08-05 迁移自 WidgetCMake）

分组式筛选浮层（非模态、可拖动、点击外部关闭），替代原模态排序 Dialog。文件：`ui/community/scheme_filter_popup.{h,cpp}`（`.ui` 仅设计参考，不参与构建）。

- **状态唯一来源**：筛选值存在组件内 `QHash<key, value>`，页面不镜像，请求前现取 `value(key)`
- **默认三组**：`sort`（排序依据：最新发布/最多点赞/最多下载/最多分享 → `new`/`like`/`download`/`share`）、`scene`（场景分类 → `user_tag`）、`model`（机型分类 → `device_name`）
- **信号**：`filterChanged(key, value)`（仅实际值变化时发射）/ `filtersReset()` / `closed()`
- **已知坑（勿重蹈）**：`Qt::Popup` 上 `WA_TranslucentBackground` 被忽略（四角露白）→ 用 `Qt::Dialog`+事件过滤器；顶层 QSS 背景不可靠 → 背景画在子容器 shell 上；`QHash::value(缺失key)` 返回空串 → 守卫必须带 `contains`；重建必须递归清布局；按钮选中按 `filterValue` 动态属性匹配；重复点击已选项需恢复选中态
- **筛选应用范围**：`filterChanged`/`filtersReset` → 刷新**当前 Tab**；`sort`/`scene`/`model`/关键词搜索 三个 Tab 均生效（`fetchExpert`/`fetchOfficial` 的 `sort` 默认 `hot` 保持旧行为，页面始终传弹窗选中值）
- **机型英文名映射**：弹窗机型选项 = 中文短名（CMake 原样），请求时经 `DeviceRegistry::deviceNameParam()` 转为 API `device_name` 英文全名（如 `T10有线` → `XIBERIA T10G`；`T7 GT` → `CROCIRIS T7 GT`，英文名待确认）

### CommunityStateOverlay — 空态/错误态覆盖层（图标 + 刷新按钮）

`CommunityPanel` 内置居中覆盖层（文件 `ui/community/community_state_overlay.{h,cpp}`），5 个列表视图共用（社区 3 Tab 共享一个面板级覆盖层，已上传/已点赞各一）：

- **签名**：`showState(text, retryEnabled, iconPath)`——iconPath 非空 → 图标 + 文字（@2x 图按像素一半显示），retryEnabled → 文字下方 24px 显示"刷新"按钮（confirm 图 104×30，点击触发 retryClicked）
- **图标语义**：社区搜索空 → `searchPlanEmpty` + "未搜索到该方案"；Tab 空 → `PlanEmpty` + "暂无数据"；加载失败 → `NetError` + "预设加载失败，请检查您的网络"；个人中心空 → `PlanEmpty` + "这里什么也没有，快去上传方案吧~"/"快去给心仪的方案点赞吧~"，失败 → `NetError`
- **文字规格**：16px、固定宽 500（换行排版）、居中；图标与文字间距 8px
- **接线**：首屏空列表（page==1）→ 空态；拉取失败（首屏/已有数据均覆盖层）→ 错误态；有数据/刷新起点/切 Tab → 隐藏
- **竞态防护**：覆盖层面板级共享，DataReady 处理器仅在回包属于当前 Tab 时碰覆盖层（切 Tab 后旧请求返回不覆盖）；切 Tab 显式清理
- **预留扩展**：`clp_icon_label_` 图标位（隐藏）+ QSS objectName（背景图）

### 已知问题（2026-08-05 记录，待确认）

- **短页无法触发加载更多（待确认）**：筛选后第一页结果不足一屏时，`CommunityFlowView` 的"底部弹簧"（`cl_content_height_ < vpHeight` 时撑满）导致滚动条 max=0，无法滚动到底触发 `loadMoreRequested` → 后续页加载不出来。候选修复：内容不足视口且行数 > 0 时自动发一次 `loadMoreRequested`（服务端每页 50 条足够填满，不会死循环）。**是否真为用户遇到场景未确认，先记录**。

## 目录结构

```
CommunityModule/
├── infrastructure/
│   ├── compat/
│   │   └── qt_compat.h             ← connectOnce()（Qt6 → 5.15 兼容）
│   └── logger/
│       ├── logger.h                ← spdlog 异步日志（宏定义）
│       └── logger.cpp              ← 每日滚动文件 + 控制台双 sink
├── service/
│   ├── scheme_service.h            ← 三合一业务服务
│   └── scheme_service.cpp
├── ui/
│   ├── community_page_widget.h/.cpp ← 页面容器（筛选栏+分页）
│   ├── data_sync_coordinator.h/.cpp ← 5-Model 同步协调器
│   └── community/
│       ├── community_main_page.h/.cpp  ← 自包含入口
│       ├── community_delegate.h/.cpp   ← 自绘卡片委托（330×300）
│       ├── community_flow_view.h/.cpp  ← 流式布局视图
│       ├── community_model.h/.cpp      ← 数据模型（30+ role）
│       └── community_panel.h/.cpp      ← 3-View Facade
└── README.md
```

模块外部依赖（全项目共享，非本模块目录）：

```
src/repository/   ← 仓库层：paginated_repository / user_config_repository / scheme_repository / ranking_helper
src/network/      ← 网络层：HttpClient / ServerRouter / AuthStore / RequestOptions / DownloadRepository / AvatarCache
src/data/         ← 接口契约 + DTO：api_global.h 聚合、userConfig/user_config_api.h 等
src/model/        ← UI 展示模型：community_item_data.h
```

## 依赖

- Qt 5.15+（`widgets` `network` `gui`）
- **spdlog**（header-only，异步日志）
- 网络层自含（`src/network/`：HttpClient/ServerRouter/AuthStore/RequestOptions/DownloadRepository/AvatarCache，全项目统一）；接口契约引用 `data/` 路径常量（`api_global.h` 聚合各子模块 `DeSheng` 命名空间）
- DTO 来自 `src/data/`、UI 展示模型来自 `src/model/`（纯数据结构，不依赖任何 Qt Widget）
