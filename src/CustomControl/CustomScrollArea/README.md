# CustomScrollArea — 自定义滚动区域

## 废弃声明

**SnapScrollArea 当前零引用**（未被 `src/` 下任何其他源文件 include 或使用），疑似废弃。建议在确认无外部依赖后从项目移除。

## 文件

| 文件 | 类 | 基类 | 用途 | 状态 |
|------|-----|------|------|------|
| `SnapScrollArea.h/cpp` (23 + 26 行) | `SnapScrollArea` | QScrollArea | 垂直滚动吸附到整数倍行高（78px） | 零引用，疑似废弃 |

## SnapScrollArea

重载 `scrollContentsBy` 使垂直滚动吸附到最近的 `m_rowHeight` 倍数。滚动条范围限制为 0-1（二值滚动）。

```
滚动值 → 量化到 round(value / m_rowHeight) * m_rowHeight
```

构造函数默认 `m_rowHeight = 78`，隐藏水平滚动条，垂直滚动条 range 设为 `[0, 1]`，singleStep/pageStep 均为 1。

### 使用示例（仅供参考——当前无实际引用）

```cpp
#include "CustomControl/CustomScrollArea/SnapScrollArea.h"

auto* scrollArea = new SnapScrollArea(parent);
// m_rowHeight 默认为 78px，硬编码不可外部配置
```

## 命名规范

- 旧式命名（`m_` 前缀），非 wbliu 规范
