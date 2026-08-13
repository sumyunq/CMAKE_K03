# DrawingArea — 透明叠加绘制

## 文件

| 文件 | 类 | 基类 | 用途 |
|------|-----|------|------|
| `TransparentWidget.h` | `TransparentWidget` | QWidget | 透明覆盖层 Widget，在三控件之间填充彩色圆角区域 |

## TransparentWidget

### 用途

填补水平排列的 3 个控件（左控件 + 中间按钮 + 右控件）之间的视觉空隙，用半透明色 `rgba(81,96,122,51)` 渲染圆角过渡区域。

### API

```cpp
void setup(QWidget *w2, QPushButton *btn, QWidget *w3);
void setButtonBottomRadius(int radius);
void setSideRadius(int w2Right, int w3Left);
```

### 渲染

`paintEvent` 使用 `QPainterPath` 组合矩形和圆弧扇形，填充控件下方间隙。用于布局装饰，非交互控件。

## 命名规范

- 旧式命名：`w2_`、`btn_`、`w3_`、`bottomY_` 等（有 `_` 后缀但无 `cl_` 前缀）

## 已知问题

- **Header-only**：无 `.cpp` 文件，全部实现在 `TransparentWidget.h` 头文件中。
- **零引用死代码**：`TransparentWidget` 未被项目中任何源文件 `#include` 或实例化，仅自身 README 提及。
