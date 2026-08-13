# resources/ — 资源目录分类说明

本目录存放 XIBERIA X HUB 的全部 Qt 资源文件（图片/图标），通过 `res.qrc` 注册后以 `:/` 前缀在代码中引用（如 `:/Resources/Skin/Images/nav/logo.png`）。

## 目录总览

```
resources/
├── res.qrc                  ← 资源清单（658 条目，rcc 编译）
├── README.md                ← 本说明
└── Skin/
    ├── Images/              ← K03 原有资源体系（按功能分类，22 个子目录）
    └── icon/                ← 社区模块图标体系（WidgetCMake 迁移）
```

> **两个体系并存的原因**：`Images/` 是 K03 从 V1 迭代至今的资源；`icon/` 是 2026-08 WidgetCMake 社区模块迁入时带来的独立图标集。未来社区模块若扩展图标，优先放入 `icon/` 保持隔离。

---

## Skin/Images/ — K03 原有资源（按功能分类）

| 子目录 | 用途 |
|--------|------|
| `Community/` | 社区排行榜、点赞/下载按钮图标（K03 社区页） |
| `DevSel/` | 设备选择页：机型图、颜色变体、二维码、首页设备图、左上角状态图标、更多设置页图标 |
| `GeneralIcon/` | 通用操作图标（点赞/踩/下载/分享，含 darkBlue 社区版） |
| `Headphones/` | 耳机特效图标：EQ 滤波器、分类图标、标签、编辑（按效果类型分子目录） |
| `Login/` | 登录/注册页：输入框、按钮、验证码（`v_2_0/` 为第二版） |
| `Mic/` | 麦克风设置图标 |
| `Popup/` | 弹窗通用图标（关闭、确认/取消等） |
| `SelfSet/` | 自定义设置图标 |
| `Slider/` | 滑块控件图标（thumb、刻度） |
| `Tip/` | 提示图标 |
| `Tool.ico` | 应用图标（.pro `RC_ICONS`） |
| `cBox/` | 下拉框（ComboBox）箭头/选项图标 |
| `close/` | 关闭按钮 |
| `home/` | 首页快捷入口图标 |
| `homePage/` | 首页背景/顶部图标 |
| `listen/` | 试听相关 |
| `more/` | 更多设置页图标（个人中心、系统等子页面） |
| `nav/` | 左侧导航栏图标（首页/EQ/麦克风/更多，含选中态 `-se`/未选中 `-no`） |
| `search/` | 搜索图标 |
| `soundTest/` | 试听测试（游戏音效测试）图标 |
| `system/` | 系统头像（`system_avatar/`）等 |
| `userFeedback/` | 用户反馈页图标 |

## Skin/icon/ — 社区模块图标体系（WidgetCMake 迁移）

| 路径 | 内容 |
|------|------|
| `generalIcon/` | 预留（当前为空，社区操作图标三态后续放这里） |
| `modules/community/` | 社区徽章背景图：`god.png`（大神）、`host.png`（主播）、`official.png`（官方）、`professional.png`（职业） |

> 社区模块 delegate 中的图标路径引用位于 `src/modules/CommunityModule/ui/community/community_delegate.cpp` 的 `kStreamerBadgeBgPath` 等常量（`:/icon/...` 前缀）。

---

## res.qrc 注册机制

- 单文件 `res.qrc`，`prefix="/Resources"` 前缀下挂 `Skin/...`，另有一个 `prefix="/"` 段挂 `translations/`（qm，用 alias 保持 `:/LanguageDemo_*.qm` 资源名）
- 新增资源后**必须**在 `res.qrc` 添加 `<file>` 条目，否则运行时报资源不存在
- 资源名 = prefix + 文件路径：`:/Resources/Skin/Images/nav/logo.png`
- qrc 按顶层目录分组 + 字母序排列（保持整洁）

## 新资源放置规范

1. **K03 功能图标** → `Skin/Images/<功能子目录>/`，按现有命名（`xxx-no.png` 未选中 / `xxx-se.png` 选中 / `xxx-ho.png` 悬停 / `xxx-ch.png` 选中态）
2. **社区模块图标** → `Skin/icon/` 对应子目录
3. **三态操作图标**（normal/hover/checked）→ 社区按 `xxx_normal_darkBlue.png` 等后缀命名
4. 放置后同步在 `res.qrc` 注册
