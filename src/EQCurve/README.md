# EQCurve — 参数均衡器曲线可视化

## 概述

自定义绘制的参数均衡器频率响应曲线控件，支持 10 个 EQ 频段的拖拽编辑。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `EQCurveWidget.h/cpp` | `EQCurveWidget` | 频响曲线主控件。完全自绘（~320 行 `paintEvent`，总 1317 行） |
| `EditPanelTip.h/cpp/ui` | `EditPanelTip` | 浮动编辑面板（频率/增益/Q 值三输入框 + 滤波器选择按钮） |
| `FilterPopupWidget.h/cpp/ui` | `FilterPopupWidget` | 六选一滤波器类型弹出菜单（Peaking EQ / High Pass / Low Pass / High Shelving / Low Shelving / Notch Filter） |

## EQCurveWidget

### 绘制要素

- **频率范围**：20Hz – 20kHz（对数尺度，26 个特定频率标记）
- **增益范围**：-12dB 到 +12dB（6dB 步进网格线）
- **三色频率高亮区**：
  - 🟡 黄色 `#fed259`：60-500Hz（脚步声）
  - 🟠 橙色 `#ff753f`：800-4000Hz（环境音）
  - 🔵 青色 `#4ee8ff`：6k-10kHz（枪声）
- **10 个可拖拽频点**：10 种不同颜色圆圈 + 选中态三圆视觉（左右旋钮表示 Q 值）
- **频响曲线**：白色实线（总曲线）+ 虚线（单频段贡献）
- **禁用覆盖层**：半透明黑色

### 频响计算

使用 Audio EQ Cookbook 双二阶滤波器数学：
- 滤波器类型：Peaking / LowShelf / HighShelf / LowPass / HighPass
- 采样率：48kHz
- 采样点：500 点（对数间隔）

### 信号

| 信号 | 参数 | 说明 |
|------|------|------|
| `bandGainChanged` | `int index, double gain` | 频段增益变化 |
| `bandFrequencyChanged` | `int index, double freq` | 频段频率变化 |
| `bandQChanged` | `int index, double q` | 频段 Q 值变化 |
| `bandFilterTypeChanged` | `int index, int type` | 滤波器类型变化 |

### 交互

- **点击**：选中频点（显示编辑面板 `EditPanelTip`）
- **拖拽**：上下调增益 / 左右调频率 / Shift+拖拽调 Q 值
- **滚轮**：在频点上滚轮调增益

## EditPanelTip

浮动于选中频点上方的编辑面板：
- 频率输入框（Hz / KHz 自动格式化）
- 增益输入框（dB）
- Q 值输入框
- `editingFinished` 后发出对应信号连回 EQCurveWidget

## 依赖

- `UndoRedo/EqBandUndoCommands.h`（4 个 QUndoCommand 子类：增益/频率/Q 值/滤波器类型）

## 已知问题

- **P2 频率映射不一致**：`xToFreq()`（`EQCurveWidget.cpp:558`）使用 `MAX_FREQ=20000` 做像素→频率逆映射，而 `freqToX()` 绘制时使用 `PLOT_MAX_FREQ=21000` 作为 X 轴上界。二者不一致导致拖拽获取的频率值与实际绘制范围偏差约 5%。

## 命名规范

- 旧式命名（`m_` 前缀），非 wbliu 规范
