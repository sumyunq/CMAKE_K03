# src/Community — 社区入口与排行榜（旧容器）

> 2026-08-05 更新：排行榜弹窗已迁入 `modules/CommunityModule/ui/ranking/`（Model/View 架构，见下文「排行榜」一节）。
> 2026-08-04 更新：排行榜已服务器化（原硬编码 100 条测试数据），入口容器逻辑保留。

## 定位

旧式社区容器（`Community`）：主界面社区 Tab 的入口 Widget，内嵌新社区模块（`modules/CommunityModule` 的 CommunityMainPage）+ 顶部排行榜入口按钮 + 排行榜弹窗（`RankingList`，本体已迁至 `modules/CommunityModule/ui/ranking/`，本目录仅保留入口壳）。

## 排行榜（RankingList，Model/View）

弹窗本体已迁至 `modules/CommunityModule/ui/ranking/`（`RankingModel` + `RankingDelegate` + `RankingList` + `RankingListView`，QAbstractListModel / QListView / 自绘委托，详见 `doc/design/ranking_modelview_20260805.md`）。`Community` 仅保留入口壳：show/hide/定位逻辑不变。

- 数据源：`RankingHelper::fetchTop`（`src/repository/ranking_helper`）——`GET /user-configs?sort=like|download&device_type=headset&page_size=100`，月度榜附加 `start_time=本月1号`（RFC3339，`+` 编码 `%2B`，时区动态）
- 四象限：点赞月榜 / 点赞总榜 / 下载月榜 / 下载总榜（Tab 点赞/下载 + 下拉月度/总榜切换，均触发刷新）
- 每次打开弹窗（showEvent）自动刷新；空态"暂无数据"/失败"加载失败，点击重试"
- 排名图标：点赞榜 Like 系列（Like-No1~No3 / Like-dis）、下载榜 DL 系列（DL-No1~No3 / DL-no）——`setRankingType` 切换
- **行为变化（2026-08-05 迁移 + 对齐社区）**：
  - 点赞与服务器同步（与社区一致）：点赞榜点 Like → 不本地翻转 → `SchemeService::toggleLike` → 服务器 `refreshCounts` 真实计数应用整行；请求进行中防连点；失败仅解锁（UI 无本地改动可回滚）
  - 下载真实下载（与社区一致）：下载榜点 DL → 防重复点击 → 行内按钮变**排名色圆环进度**（替代图标，`DownloadProgressRole`）→ `SchemeService::download` → `MainWindow::importDownloadedPlan` 导入方案库 + 真实计数刷新；失败 toast"下载失败，请检查网络"（2 秒自动隐藏）
  - 方案卡片完整填充：点击方案名 → `CustomQWidgetSinglePlans` 卡片以 ListItem 数据填满（方案名/描述/标签/作者/等级/头像），修复旧版空卡片问题
  - 头像已接入：`UserConfigRepository::fetchAvatar` 异步下载 + 圆形裁剪显示
  - hover 反馈：行 hover 高亮、方案名/用户名/点赞按钮 hover 变色、头像 hover 遮罩

## 前三名头像（Community 顶部按钮，2026-08-07 更新）

- `btn_Like_No1~3` / `btn_download_No1~3`：`RankingHelper::fetchTop(sort, 3)` → `author.avatar` → `AvatarCache` 下载 → `AvatarButton::setAvatarPixmap`
- **三态处理**：榜单数据不足（<3 人）→ 多余头像位隐藏（容器 widget_2 固定 59×452 保持高度）；有数据但 avatar 字段空 → 默认第一个系统头像（`system_avatar_2x_01.png`）；非空 → 下载真实头像后显示
- **点击弹窗**：`AvatarButton::clicked()` 信号（左键释放于按钮内）→ 六头像位绑定用户资料（作者 user_id/昵称/等级/roles 徽章，`cl_top_three_users_`）→ 点击弹 `UserUploadsDialog`（与社区 item 点击头像行为一致）
- 旧占位图 `profile_picture.png`（文件不存在）已删除

## 文件

| 文件 | 职责 |
|------|------|
| `Community.{h,cpp}` | 容器：嵌入 CommunityMainPage + 排行榜入口 + 前三名头像数据（位于 `src/` 根） |
| `AvatarButton.{h,cpp}` | 头像按钮（圆形裁剪 + 边框 + 角标，`setAvatarPixmap` 支持网络头像） |

排行榜弹窗文件已迁至 `modules/CommunityModule/ui/ranking/`：

| 文件 | 职责 |
|------|------|
| `ranking_model.{h,cpp}` | 数据模型（QAbstractListModel，DTO 直存 + 8 role，服务器覆盖/头像回写/下载状态） |
| `ranking_delegate.{h,cpp}` | 行委托（360×55 自绘：排名/头像/方案名/用户名/热度/点赞按钮 + hover 反馈 + 下载中置灰） |
| `ranking_list.{h,cpp,ui}` | 弹窗容器（榜单拉取/四态/头像下载/点赞同步/下载导入/方案卡片/toast） |
| `ranking_list_view.h` | 暴露 protected `setViewportMargins` 的 QListView 子类（.ui 提升用） |
