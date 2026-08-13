# CustomSlider — 自定义滑块

## 文件

| 文件 | 类 | 基类 | 用途 |
|------|-----|------|------|
| `NewHSlider.h/cpp` (80 + 569 行) | `NewHSlider` | QSlider | 通用水平滑块（4 种绘制 Style，颜色动画，悬浮数值提示，点击跳转） |
| `GearSlider.h/cpp` (37 + 393 行) | `GearSlider` | QSlider | 块状离散滑块（"齿轮"样式），完全自绘 |

## NewHSlider

通用水平滑块，支持 4 种绘制风格、颜色动画、悬浮/点击数值 ToolTip。

### Q_PROPERTY（支持 QPropertyAnimation）

| 属性 | 类型 | 读写 | 说明 |
|------|------|------|------|
| `hover` | bool | `isHover` / `setHover` | 鼠标悬停手柄状态，触发 handle pixmap 变化 |
| `pressed` | bool | `isPressed` / `setPressed` | 鼠标按下状态，触发 handle 颜色变化 |
| `fillColor` | QColor | `fillColor` / `setFillColor` | 已填充轨道颜色（默认 `#009FEF`） |
| `handleColor` | QColor | `handleColor` / `setHandleColor` | 滑块手柄基础颜色（默认 `#FFFFFF`） |

### 4 种绘制 Style（`setType(type, bgTrackH, fgTrackH, anim, tooltip)`）

| styleType | 说明 | 填充轨道行为 |
|-----------|------|-------------|
| 0 | 基础平铺 | 使用 `m_fullPixmap`（1x8px 纯色）直接 `drawTiledPixmap`，无圆角 |
| 1 | 圆角 + 左右 margin | 前景轨道比背景更细（`fgTrackHeight`），左右各留 `margin` px，带圆角 |
| 2 | 纯 QSS | 不自定义绘制，完全走 QSlider 默认 paintEvent + 样式表 |
| 3 | 圆角 + 无 margin | 前景轨道更细，左右不留间距，带圆角 |

### 关键方法

| 方法 | 说明 |
|------|------|
| `setType(int type, int bgTrackH, int fgTrackH, bool anim, bool tooltip)` | 设置绘制风格、背景/前景轨道高度、动画开关、悬浮 ToolTip 开关 |
| `setMargin(int value)` | 设置 styleType=1 时的左右边距（默认 3px） |
| `animateFillColor(from, to, duration)` | 动画过渡填充轨道颜色（QPropertyAnimation on `fillColor`） |
| `animateHandleColor(from, to, duration)` | 动画过渡滑块手柄颜色（QPropertyAnimation on `handleColor`） |
| `setFillColor(color)` / `setHandleColor(color)` | 直接设置颜色 |

### 悬浮数值 ToolTip（`ShowTooltipEn = true`）

当鼠标悬停或点击滑块手柄时，在 `m_parentWidget` 上方显示一个 30x22px 的无边框 ToolTip 窗口，内容为当前 `sliderPosition()` 值。样式：`#12161D` 背景、4px 圆角、白色 12px 文字。

### 点击跳转

`mousePressEvent` 中区分点击手柄（走 QSlider 默认拖动逻辑）与点击轨道（计算比例跳转到对应值）。`wheelEvent` 仅在 `wheelEnabled = true` 时响应（focusIn 开启、focusOut 关闭）。

### 动画手柄

当 `AnimationEn = true` 时，手柄使用自绘 `generateHandlePixmap(color, hover, pressed)`：
- 普通：6x18px，圆角 3
- 悬浮/按下：10x26px，圆角 6
- 按下时颜色变为 `#6DD3FF`

### 事件处理

| 重载 | 说明 |
|------|------|
| `paintEvent` | 绘制背景轨道（`#4D000000` 圆角矩形）、填充轨道（按 styleType 分支）、手柄（QSS 或自绘） |
| `mousePressEvent` | 区分手柄拖动与轨道点击跳转；ToolTip 显示 |
| `mouseMoveEvent` | 实时检测鼠标是否在手柄区域，更新 hover 状态和 ToolTip 位置 |
| `mouseReleaseEvent` | 清除 pressed 状态 |
| `leaveEvent` | 删除 ToolTip，清除 hover 状态 |
| `wheelEvent` | focusIn 时允许滚轮调节，否则忽略 |
| `focusInEvent` / `focusOutEvent` | 切换 `wheelEnabled` |

## GearSlider

完全自绘的块状离散滑块。每个可选项渲染为圆角矩形块，选中块用蓝色渐变填充 + 4 层发光阴影。

### 特性

- **块渲染**：`setBlockHeight(qreal)` 设置块高度（默认 12px），块间距 5px
- **混合正负范围**：支持最小值 < 0 < 最大值，零点处 16px 间隙 + 蓝色竖线标记（渐变 `#3CBEFF → #0091DA` + 发光阴影）
- **选中块渐变**：`#39B6F5 → #0091DA` 线性渐变
- **选中块阴影**：4 层辉光阴影（alpha s=4→5, s=3→13, s=2→26, s=1→38），内浓外淡
- **未选中块**：黑色 `rgba(0,0,0,51)`
- **禁用态渐变**：`#128BC9 → #046FA5`
- **点击切换**：点击块 toggle on/off；点击最右侧亮块递减熄灭

### 关键方法

| 方法 | 说明 |
|------|------|
| `setBlockHeight(qreal height)` | 设置块高度，触发 update |
| `valueFromPos(const QPoint&)` | 根据鼠标位置计算最近块的逻辑值（含 toggle 逻辑） |

### 事件处理

| 重载 | 说明 |
|------|------|
| `paintEvent` | 完全自绘所有块、阴影、零点标记 |
| `mousePressEvent` | 左键点击计算 `valueFromPos` 并 setValue |
| `mouseMoveEvent` | 空实现（不支持拖动选择） |
| `wheelEvent` | focusIn 时 +/-1 步进，否则忽略 |
| `focusInEvent` / `focusOutEvent` | 切换 `wheelEnabled` |

## 使用示例

```cpp
#include "CustomControl/CustomSlider/NewHSlider.h"
#include "CustomControl/CustomSlider/GearSlider.h"

// NewHSlider：圆角 + 左右 margin，开启动画和 ToolTip
auto* slider = new NewHSlider(parent);
slider->setRange(0, 100);
slider->setType(1, 8, 4, true, true); // styleType=1, bgH=8, fgH=4, 动画, ToolTip
slider->setMargin(3);
slider->animateFillColor(QColor("#009FEF"), QColor("#FF5500"), 300);

// GearSlider：块状离散滑块（-3 ~ +6，含零点线）
auto* gear = new GearSlider(parent);
gear->setRange(-3, 6);
gear->setBlockHeight(12);
gear->setValue(3);
```

实际使用：`NewHSlider` 广泛用于 EQ 频段增益、音量调节等场景；`GearSlider` 在 `CustomQWidgetSingleAlgorithmSetting` 中用于算法效果步进选择。

## 命名规范

- 旧式命名（`m_` 前缀），非 wbliu 规范
