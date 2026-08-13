# GeneralCustomUI — 通用自定义控件库

## 概述

跨页面共享的通用 UI 控件集合。所有控件均遵循 wbliu 编码规范。

## 控件清单

| 控件 | 基类 | 用途 | 主题 | 国际化 |
|------|------|------|:--:|:--:|
| `CustomQWidgetNotification` | QWidget(.ui) | 单按钮通知弹窗（提示文字 + "我知道了"），16px 圆角 + 阴影 + ApplicationModal | ✅ | ❌ |
| `CustomQDialogGeneralTips` | QDialog(纯代码) | 双按钮确认弹窗（标题 + 取消/确认），绝对定位 | ✅ | `tr()` |
| `CustomQWidgetFunctionButtonWithDisplayLabel` | QWidget(.ui) | 图标功能按钮 + 计数标签（Like/Unlike/Download/Share） | ✅ | ❌ |
| `CustomQWidgetSinglePlans` | QWidget(.ui) | 方案卡片（4 区：顶+标签+信息+按钮栏） | ✅ | ❌ |
| `CustomQWidgetPlanInfo` | QWidget | 方案信息（图标+名称），3 种尺寸模式 | ❌ | ❌ |
| `CustomQWidgetComments` | QWidget | 标签流式布局，折叠态 2 行 + 展开/收起 | ✅ | ❌ |
| `CustomQWidgetTagLabel` | QWidget | 单个标签（文字+数字），选中/未选中双态 | ✅ | ❌ |
| `CustomQPushButtonFunctionalClassificationButton` | QPushButton | 功能分类按钮（hover 时图标上移 + 文字淡入） | ❌ | ❌ |
| `CustomQScrollAreaGeneralLayout` | QScrollArea(.ui) | 通用滚动容器（Grid/SingleColumn/SingleLine 三种布局），含动态列数计算 | ❌ | ❌ |
| `CustomQWidgetDownloadProgressRing` | QWidget(自绘) | 下载进度圆环（14x14，背景弧+进度弧 12 点方向顺时针），固定尺寸 | ✅ | ❌ |
| `CustomQWidgetLoading` | QWidget(自绘) | 加载中动画（旋转弧线 + 可选文字），全参数可配 | ✅ | ❌ |
| `CustomQWidgetTextBadge` | QWidget(自绘) | 文字徽章（圆角背景 + 文字），尺寸自适应，颜色/圆角/边距可设，支持火焰动画背景 | ✅ | ❌ |
| `CustomQWidgetTextBadgeContainer` | QWidget(布局) | 徽章容器（水平排列 + 右侧弹簧左挤），addBadge() 返回指针链式配置 | ✅ | ❌ |

## 通用模式

### Init 三方法

所有控件遵循：
```cpp
InitUIInformation(int theme)  // 创建子控件、样式、默认值
InitMember()                   // 非 UI 对象（动画、定时器等）
InitConnect()                  // 信号槽连接
```

### 命名

- `cl_` 值类型 / `clp_` 指针类型 + `_` 后缀
- 局部变量 `t_` 前缀
- 枚举用 `enum class`，首个值 `= 0`

### 主题

有 `applyTheme` 的控件多数 theme=0 为空占位（待实现）。仅 `CustomQDialogGeneralTips` 和 `CustomQWidgetNotification` 有实际主题样式。

## 关键交互

### CustomQWidgetSinglePlans → Comments 高度联动

```
CustomQWidgetTagLabel::mousePressEvent
  → toggle 选中态 → 数字 ±1
CustomQWidgetComments::mousePressEvent (展开/收起区域)
  → setCl_expanded → doLayout() → expandedChanged(bool)
    → CustomQWidgetSinglePlans 重新计算固定高度
```

### CustomQWidgetFunctionButtonWithDisplayLabel 互斥

Like/Unlike 在 `CustomQWidgetSinglePlans::InitConnect()` 中实现互斥逻辑。

### WidgetStateCache — 方案卡片状态缓存（2026-07-27）

`WidgetStateCache`（`using WidgetStateCache = QHash<int, WidgetDisplayState>`）将方案卡片的 UI 状态（如评论区展开/收起）与 widget 生命周期解耦。widget bind 时 restore，unbind 时 save，widget 回收后状态不丢失。

```cpp
#include "modules/GeneralCustomUI/WidgetStateCache.h"

WidgetStateCache t_cache;
WidgetDisplayState t_state = t_cache.value(configId);  // widget bind 时恢复
t_cache[configId] = t_state;                            // widget unbind 时保存
```

### CustomQWidgetSinglePlans — 异步回调安全（2026-07-27）

所有网络回调（`doDownload`/`doLike`/`doUnlike`/`doDislike`/`doUndislike`/`doShare`/`refreshCounts`/
`doClickComment`/`doCancelClickComment`）均使用 `QPointer<CustomQWidgetSinglePlans>` 守卫，
池回收后回调自动跳过。评论标签回调额外使用 `QPointer<CustomQLabelTag>` 防 `clearComments` 后的 use-after-free。

## 文件命名注意

- 文件名 `custom_QWidget_*`、`custom_QPushButton_*`、`custom_QScrollArea_*`、`custom_QDialog_*` 前缀按基类区分
- `CustomQWidget` 子目录存放纯自绘控件（Loading、TextBadge、TextBadgeContainer）

## CustomQWidgetTextBadge — 自绘文字徽章

`CustomQWidgetTextBadge` 是一个纯自绘控件，在 `paintEvent` 中绘制圆角背景 + 文字。所有视觉属性可独立设置，`setCl_text()` 后自动重算尺寸。

**定位**：状态标签、分类标识、计数徽章 — 任何需要"带背景色的短文本"场景。

### 使用示例

```cpp
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge.h"

// ① 默认配置（蓝色背景 #0091DA、白色文字、4px 圆角、四周 12/6/12/6 边距）
auto *t_badge = new CustomQWidgetTextBadge(parent);
t_badge->setCl_text(tr("进行中"));

// ② 场景标签（不同颜色区分状态）
t_badge->setCl_bg_color(QColor("#FF6B35"));   // 橙色背景
t_badge->setCl_text_color(QColor("#FFFFFF")); // 白色文字
t_badge->setCl_radius(10);                     // 大圆角 = 胶囊形

// ③ 紧凑模式（通过边距控制尺寸）
t_badge->setCl_padding(8, 2, 8, 2);           // 窄上下边距
t_badge->setCl_radius(3);                      // 小圆角

// ④ 更新文字 → 控件自动 resize
t_badge->setCl_text(tr("已完成"));
// 此时 t_badge 的 size 已自动更新，父布局无需手动调 adjustSize()

// ⑤ 结合布局使用
auto *t_layout = new QHBoxLayout();
t_layout->addWidget(t_badge);
t_layout->addStretch();
```

### 关键行为

- **尺寸自适应**：`setCl_text()` 内部调用 `updateSizeFromText()`，通过 `QFontMetrics` 测量文字宽度 + 边距 → `setFixedSize()`，文字变化后控件尺寸立即生效。
- **空文字处理**：text 为空时 `setFixedSize(0, 0)`，控件在布局中收缩为零尺寸。
- **绘制顺序**：`QPainterPath::addRoundedRect` 裁剪 → `fillRect` 铺背景 → `drawText` 在 padding 区域内绘制。
- **主题占位**：`applyTheme(int)` 为 switch-case 空占位，三主题暂未定义差异样式，后续可扩展。

### 配置项一览

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `text` | QString | `""` | 显示文字 |
| `bg_color` | QColor | `#0091DA` | 背景色 |
| `text_color` | QColor | `#FFFFFF` | 文字颜色 |
| `font` | QFont | Noto Sans 12px Medium | 字体 |
| `radius` | int | `4` | 圆角半径（0=直角） |
| `padding_left/top/right/bottom` | int | `12/6/12/6` | 内边距 |
| `alignment` | Qt::Alignment | `AlignCenter` | 文字对齐 |

### 火焰模式（动画渐变背景）

开启后背景替换为动态火焰效果——四条正弦波叠加模拟火焰边缘形状，黄→橙→红→暗红渐变填充，QTimer 驱动逐帧相位推进。

**开关火焰**：
```cpp
t_badge->setCl_fire_enabled(true);   // 启动火焰动画
t_badge->setCl_fire_speed(40);       // 调整帧率（默认 50ms，40ms = 25fps）
t_badge->setCl_text_color(QColor("#FFD700")); // 推荐：金黄色文字与火焰搭配
```

**细节**：
- 显示时自动启动定时器，隐藏时自动停止（`showEvent`/`hideEvent`）
- 火焰模式下文字自动叠加暗色阴影（`#00000050` 下偏移 1px），确保在火焰背景上的可读性
- `setCl_bg_color` 在火焰模式下暂不使用（火焰色为固定的暖色调：黄→橙→红→暗红）

## CustomQWidgetTextBadgeContainer — 徽章容器

水平排列多个 `CustomQWidgetTextBadge`，右侧弹簧把所有徽章往左挤。`addBadge()` 返回 `CustomQWidgetTextBadge*` 供链式配置。

### 使用示例

```cpp
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge_container.h"

auto *t_tags = new CustomQWidgetTextBadgeContainer(parent);
t_tags->setCl_spacing(8);                                // 徽章间距 8px（默认 6）
t_tags->setCl_margin(0, 4, 0, 4);                       // 上下 4px 外边距

// 添加徽章 — 返回指针，可链式配置
t_tags->addBadge(tr("CSGO"))->setCl_radius(8);
t_tags->addBadge(tr("热门"))->setCl_fire_enabled(true);  // 热门标签加火焰
t_tags->addBadge(tr("官方"))->setCl_bg_color(QColor("#FF6B35"));

// 移除
t_tags->clearBadges(); // 清空
```

### 关键行为

- **弹簧机制**：`QHBoxLayout::addStretch()` 将弹簧固定在末尾，新徽章 `insertWidget(count - 1)` 始终插在弹簧之前
- **尺寸自适应**：容器尺寸由子徽章 + 间距 + 外边距自动撑开，无需手动 `setFixedSize`
- **徽章生命周期**：`removeBadge()` 调 `deleteLater()`，`clearBadges()` 批量清理

## CustomQWidgetLoading — 自绘加载动画

纯自绘旋转弧线 + 可选文字。通过 `QVariantAnimation` 驱动 `cl_angle_` 变化 → `update()` 重绘。

### 使用示例

```cpp
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_loading.h"

auto *t_loading = new CustomQWidgetLoading(parent);
t_loading->setFixedSize(24, 24);
t_loading->start();   // 开始旋转
t_loading->stop();    // 停止

// 或带文字
t_loading->setCl_text(tr("加载中…"));
t_loading->setCl_arc_color(QColor("#0091DA"));
```

所有视觉参数通过 `CustomQWidgetLoadingConfig` 配置结构体统一管理，`setCl_config(cfg)` 批量应用。

## API 变更

## 目录结构

```
GeneralCustomUI/
├── CustomQWidget/                      ← 纯自绘控件子目录
│   ├── custom_QWidget_loading.h/.cpp            ← 加载动画
│   ├── custom_QWidget_text_badge.h/.cpp          ← 文字徽章
│   └── custom_QWidget_text_badge_container.h/.cpp ← 徽章容器
├── custom_QDialog_general_tips.h/.cpp
├── custom_QLabel_tag.h/.cpp
├── custom_QPushButton_functional_classification_button.h/.cpp
├── custom_QScrollArea_general_layout.h/.cpp/.ui
├── custom_QWidget_comments.h/.cpp
├── custom_QWidget_download_progress_ring.h/.cpp
├── custom_QWidget_function_button_with_display_label.h/.cpp/.ui
├── custom_QWidget_notification.h/.cpp/.ui
├── custom_QWidget_plan_info.h/.cpp/.ui
├── custom_QWidget_single_plans.h/.cpp/.ui
├── WidgetStateCache.h                  ← 方案卡片状态缓存（header-only）
└── README.md
```
