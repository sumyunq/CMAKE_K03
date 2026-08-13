# CustomRadioButton — 自定义单选按钮

## 文件

| 文件 | 类 | 基类 | 用途 |
|------|-----|------|------|
| `NewRadioBtn.h/cpp` (97 + 687 行) | `NewRadioBtn` | QRadioButton | 方案卡片单选按钮（收藏/编辑/勾选/设备标签/描述） |
| `NewRadioBtnText.h/cpp` (34 + 196 行) | `NewRadioBtnText` | QRadioButton | 当前预设方框（指示器图标内显示文字，文本过长换行） |
| `AutoResizeLabel.h` (31 行) | `AutoResizeLabel` | QLabel | 自动宽度标签（文本变化时自适应 `fontMetrics` + 8px 边距），header-only |

## NewRadioBtn

方案库页面中的方案卡片。每个卡片是一个复杂的 QRadioButton，包含：

### 子控件

| 控件 | 类型 | 说明 |
|------|------|------|
| `AllpBt_fav` | QPushButton* | 收藏按钮（切换收藏状态，checkable） |
| `AllpBt_edit` | QPushButton* | 编辑按钮（弹出右键菜单：修改/复制/删除/移动到） |
| `AllpBt_check` | QPushButton* | 勾选框（批量操作模式，默认隐藏） |
| `lab1` | AutoResizeLabel* | 设备标签（如 "K03S"，支持多设备悬浮 ToolTip） |
| `lab2` | AutoResizeLabel* | 场景标签（如 "游戏"） |
| `lab_name` | QLabel* | 方案名称 |
| `lab_description` | QLabel* | 方案描述（超长省略，最多 5 字符 + "..."） |
| `container` | QWidget* | 标签容器（含 HLayout_label 水平布局） |
| `PlanDescription` | QWidget* | 方案描述容器（含 HLayout_description 水平布局） |
| `eMenu` / `A_rename` / `A_copy` / `A_del` / `A_move` | QMenu* / QAction* | 编辑右键菜单及其四个动作 |

### 关键方法

| 方法 | 说明 |
|------|------|
| `updateAllPlanValue(PlanVal)` | 从 PlanVal 更新全部 EQ 参数 |
| `getAllPlanValue()` | 回写全部 EQ 参数到 PlanVal |
| `setIsAddedEn(bool, int)` | 设置"已添加"（收藏）标记及收藏 ID |
| `setIsLoad(bool)` | 设置加载完成标记 |
| `setLabel2(const QString&)` | 设置场景标签文字，同时更新 QRadioButton::indicator 图标 |
| `setLabDevsOne(dev)` / `setLabDevs(devs)` | 设置设备列表（单设备 / 多设备） |
| `getLabDevs()` | 获取设备列表 |
| `setLab1Style(DeviceName)` | 设置设备标签样式（单设备用背景色 + 文字，多设备用图标） |
| `updateElidedText(fullText, PlanName)` | 更新方案名称和描述（描述截断到 5 字符 + "..."） |
| `GetPlanPageSel()` | 返回方案所属分类页索引 |
| `GetDataVisibleEn()` | 返回方案数据是否可见（`PlanVal::DataVisibleEn`） |
| `setStyle(bool IsAdded)` | 收藏样式切换（当前为空实现，预留扩展） |

### 公共数据

| 字段 | 类型 | 说明 |
|------|------|------|
| `IsAdded` | bool | 是否已添加到收藏 |
| `favIdx` | int | 收藏后的 ID |
| `IsSys` | bool | 是否为系统预设 |
| `IsLoad` | bool | 是否已完成加载 |
| `PlanPageSel` | int | 方案所属页面索引（0=所有预设, 1=我的预设, 2-8=各分类） |
| `ShareCodeId` | QString | 分享方案文件 ID（空则需上传，非空表示服务器已有） |
| `ShareCode` | QString | 分享码 |

### 事件处理

| 重载 | 说明 |
|------|------|
| `resizeEvent` | 调用 `updateButtonPosition()` 重新布局子控件 |
| `event` | 预留 tooltip 自定义显示逻辑（当前注释掉，改用 NewCustomToolTip） |
| `leaveEvent` | 清理自定义 tooltip 引用 |
| `eventFilter` | 监听 `lab1` 的 Enter/Leave 事件，在多设备时显示悬浮框 |

### 内部私有成员

| 字段 | 类型 | 说明 |
|------|------|------|
| `m_planValue` | PlanVal | 内部保存的方案数据 |
| `m_customTooltip` | QLabel* | 方案描述悬浮提示（已废弃，改用 `tip_des`） |
| `lab_devs` | QStringList | 关联设备列表 |
| `m_tooltipWidget` / `m_tooltipLayout` | QWidget* / QVBoxLayout* | 多机型悬浮框及布局 |
| `tip_des` | NewCustomToolTip* | 方案描述自定义 ToolTip（`setLabelStyle(2)`） |

## NewRadioBtnText

当前预设页面的方案选项卡。QRadioButton 指示器图标内部左下角叠加显示文字，文字过长时自动省略。

### 关键方法

| 方法 | 说明 |
|------|------|
| `setIndicatorText(text, label)` | 设置指示器内显示文字，同时更新指示器图标（根据 label 映射图片） |
| `getIndicatorText()` | 返回当前指示器文字 |
| `setThemeAndPanelTransparency(idx, PValue)` | 设置主题（0=深蓝/1=白/2=黑）和面板透明度 |
| `updateStyleSheet()` | 根据主题、透明度、场景标签重新生成并应用 QSS |

### 信号

| 信号 | 说明 |
|------|------|
| `SetITextSignal(const QString &text, const QString &label)` | 当 `setIndicatorText` 被调用时发射，通知其他界面当前方案变更 |

### 重载事件

| 重载 | 说明 |
|------|------|
| `paintEvent` | 完全自绘：先调用 QStyle 绘制默认 RadioButton，再在指示器矩形内绘制 `indicatorText`（10px 字体，左下角对齐，ElideRight 省略） |

### 内部私有成员

| 字段 | 类型 | 说明 |
|------|------|------|
| `indicatorText` | QString | 指示器内显示文本 |
| `m_baseName` | QString | 场景标签映射的图片基础名（如 "game", "music"） |
| `m_Themeidx` | int | 主题索引（0/1/2） |
| `m_PanelTransparency` | double | 面板透明度（0.0-1.0） |

## AutoResizeLabel

```cpp
void setText(const QString &text) override {
    QLabel::setText(text);
    adjustWidth();  // 根据 fontMetrics 自动计算宽度 + 左右各 8px 边距
}
```

header-only，构造函数中设置 `setContentsMargins(8, 0, 8, 0)`，`setText` 时自动调用 `adjustWidth()` 计算 `fontMetrics().horizontalAdvance(text()) + left + right`。

## 使用示例

```cpp
#include "CustomControl/CustomRadioButton/NewRadioBtn.h"
#include "CustomControl/CustomRadioButton/NewRadioBtnText.h"
#include "CustomControl/CustomRadioButton/AutoResizeLabel.h"

// NewRadioBtn：构造时传入 PlanVal
PlanVal plan = /* ... */;
auto* card = new NewRadioBtn(plan, parent);
card->setLabel2("游戏");
card->setLabDevsOne("K03S");
card->updateElidedText("自定义方案描述文字", "我的方案");

// NewRadioBtnText：当前预设选项卡
auto* planTab = new NewRadioBtnText(parent);
planTab->setIndicatorText("方案名称", "游戏");
planTab->setThemeAndPanelTransparency(0, 20); // 深蓝主题，透明度 0.2

// AutoResizeLabel：自适应宽度标签
auto* label = new AutoResizeLabel(parent);
label->setText("K03S"); // 自动计算宽度
```

## 命名规范

- 旧式命名（`m_` 前缀、`lab_` 无前缀混用），非 wbliu 规范
- 无 Init 三方法模式，构造函直接做全部初始化

## 依赖

- `NewCustomToolTip`：方案描述悬浮提示
- `PlanVal`（`GlobalDefinition.h`）：EQ 方案数据结构
