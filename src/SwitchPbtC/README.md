# SwitchPbtC — 动画拨动开关

## 文件

| 文件 | 类 | 基类 | 用途 |
|------|-----|------|------|
| `custom_pushbutton.h/cpp` | `CustomPushButton` | QPushButton | 动画拨动开关（toggle switch） |

## CustomPushButton

`src/SwitchPbtC/` 目录下的自定义控件，**部分遵循 wbliu 规范**。

### 两种渲染模式

| 模式 | 说明 |
|------|------|
| **ANIMATION** | 自定义 `paintEvent`：胶囊形圆角矩形 + 滑动圆心。（选中：蓝色 `#0091DA` + 浅灰 `#D8D8D8` 圆 / 未选中：`#697081` 边框 + 灰色圆） |
| **SIMPLE** | `paintEvent` 画提供的 On/Off pixmap，fallback 文字 "ON"/"OFF" |

### 动画

- `QPropertyAnimation` 驱动 `cl_sliderOffset_`（Q_PROPERTY，0.0→1.0）
- `setChecked(bool)` 重载以处理信号被 block 时的动画状态

### API

| 方法 | 说明 |
|------|------|
| `setCl_system_style(SystemStyle)` | ANIMATION / SIMPLE |
| `setCl_pixmapOn(QPixmap)` / `setCl_pixmapOff(QPixmap)` | 设置 SIMPLE 模式图片 |
| `setZoomFactor(double)` | 缩放因子 |
| `setChecked(bool)` | 重载 QAbstractButton，处理 block 信号时的动画 |

### 信号

| 信号 | 说明 |
|------|------|
| `cl_toggled(bool open)` | 转发自 QPushButton::toggled |

### 命名规范

- **部分遵循 wbliu 规范**：`cl_` 前缀 + `InitUIInformation` / `InitMember` / `InitConnect` 三方法
- 使用 `Q_PROPERTY`（规范推荐 `QVariantAnimation` 替代，但此类先于规范）
