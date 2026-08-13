# Common

> 全局可复用的通用模块 — 单例、工具类、共享资源缓存。

## Overview

`src/Common/` 存放跨模块共享的通用类，与具体业务页面解耦，任何层级的代码均可直接引用。

## Modules

### AppImageCache

#### What is it?

全局图片缓存**单例**。存放壁纸预缩放缓存等运行时渲染资源，避免各模块重复加载、解码、缩放同一张图片。

**设计动机**：壁纸缓存原本散落在 `MainWindow` 的私有成员中，其他模块（HomePage、弹窗、设置预览）需要背景图时必须穿透 `MainWindow` 取数据，架构不合理。提取为全局单例后，任何模块通过 `AppImageCache::instance()` 直接访问。

#### Quick Start

```cpp
#include "modules/Common/AppImageCache.h"

// 写入缓存
auto &t_cache = AppImageCache::instance();
t_cache.cl_background_pixmap_ = QPixmap("/path/to/wallpaper.png");
t_cache.updateBackgroundCache(mainWindow->size());

// 读取缓存（paintEvent 等高频路径，零缩放开销）
void SomeWidget::paintEvent(QPaintEvent *event) {
    QPainter t_painter(this);
    auto &t_cache = AppImageCache::instance();
    if (!t_cache.cl_background_scaled_cache_.isNull()) {
        t_painter.drawPixmap(0, 0, t_cache.cl_background_scaled_cache_);
    }
}
```

#### API Reference

| Member | Type | Description |
|--------|------|-------------|
| `instance()` | `static AppImageCache&` | 单例入口 |
| `updateBackgroundCache(const QSize&)` | `void` | 按给定尺寸预缩放全部壁纸缓存，调用后可直接绘制 |
| `cl_background_pixmap_` | `QPixmap` | 用户壁纸原图，空 = 未设置 |
| `cl_background_scaled_cache_` | `QPixmap` | 用户壁纸缩放缓存，`paintEvent` 中零开销绘制 |
| `cl_default_background_cache_` | `QPixmap` | 默认底部背景缩放缓存，始终有效 |

#### Design Notes

- **单例模式**：C++11 Magic Static，线程安全，无需显式析构。
- **预缩放策略**：缩放仅在 `updateBackgroundCache()` 中执行一次，`paintEvent` 等高频路径直接绘制缓存，零 `scaled()` 调用。
- **DPI 感知**：缩放目标 = `size * devicePixelRatio`，输出设置 `setDevicePixelRatio(dpr)`，逻辑尺寸居中。
- **非持久化**：缓存为纯运行时数据，不参与序列化 / 反序列化。

### FrostedPanel

#### What is it?

磨砂玻璃面板，在自身区域显示窗口背景的模糊效果 + stylesheet tint 叠加。17 个面板已接入，覆盖 7 个页面。

#### Architecture（2026-07-27 重构）

内部使用独立的 `FrostedBackgroundLayer` 子控件承载模糊 + tint。业务子控件与背景层是**平级兄弟**关系，不依赖父级 `paintEvent`，子控件背景不会与面板 tint 额外叠加。

```
FrostedPanel (paintEvent 空)
  ├─ FrostedBackgroundLayer  ← blur + tint，lower() 确保最底层
  ├─ widget_01               ← 业务子控件，自然绘制于背景层之上
  └─ ...
```

#### Quick Start

```cpp
#include "modules/Common/FrostedPanel.h"

// Qt Designer: 将 QWidget 提升为 FrostedPanel
// 代码中设置圆角（与 stylesheet border-radius 对齐）
auto *t_panel = new FrostedPanel(parent);
t_panel->setCornerRadius(10);
t_panel->setStyleSheet(R"(
    #panel {
        border-radius: 10px;
        background-color: rgba(81, 96, 122, 0.2);
    }
)");
```

#### API Reference

| Member | Type | Description |
|--------|------|-------------|
| `setCornerRadius(qreal)` | `void` | 四角统一圆角 |
| `setCornerRadius(tl, tr, br, bl)` | `void` | 四角独立圆角 |
| `cornerRadius()` | `qreal` | 获取圆角（返回左上角值） |
| `setShapePath(QPainterPath)` | `void` | 自定义裁剪路径（异形面板） |
| `shapePath()` | `QPainterPath` | 获取当前裁剪路径 |

#### 已接入面板（17 个，覆盖 7 个页面）

`FrostedPanel` 通过 Qt Designer 提升（promote）方式接入，各面板保留原有半透明 stylesheet + `setCornerRadius()` 对齐圆角。

### DeviceRegistry

#### What is it?

`DeSheng::DeviceRegistry` — 全局设备注册表**饿汉单例**。持有全部支持设备及其颜色变体信息（~30 SKU 硬编码），提供 VID/PID 反向查找、设备标签名称查询等功能。

**初始化链路**：`main()` -> `init()` -> 读磁盘 `DevInfo.dat` -> 回退 `defaultConfig()`。

**设备说明书 URL（2026-08-04 新增）**：`DeviceInfo::DeviceManualUrl`（型号级字段，同型号多颜色共用）。`defaultConfig()` 内以 `MANUAL_*` 常量（与二维码 `QR_*` 常量并列）按型号组赋值：T10有线 groupId=16 / T10无线 20 / K03S超竞版 24 / K03有线版二代 26 / K06S 18 / T7 22 / T7 GT 暂复用 22（8/10 换正式）；K03S 普通版未提供留空。链路：常量赋值 → 颜色按钮 toggled 拷贝 → `cl_selected_device_information()` → 首页说明书按钮 openUrl。

#### Quick Start

```cpp
#include "modules/Common/DeviceRegistry.h"

// 查找设备
auto &t_reg = DeSheng::DeviceRegistry::instance();
auto t_info = t_reg.find("T10有线", 0);

// VID/PID 反查设备简写
QString t_typeName = t_reg.deviceSysTypeName(vid, pid);  // "T10" / "K03S" / ...

// 查询设备标签名称
QString t_label = DeSheng::DeviceRegistry::deviceLabel(deviceId, isTest);

// 截去 "XIBERIA " 前缀的短名称
QString t_short = DeSheng::DeviceRegistry::shortDisplayName("XIBERIA T10 Wireless");
// → "T10 Wireless"
```

#### API Reference

| Member | Type | Description |
|--------|------|-------------|
| `instance()` | `static DeviceRegistry&` | 饿汉单例入口 |
| `deviceMap()` | `const DeviceMap&` | 只读访问设备表 `QMap<QPair<QString,int>, shared_ptr<DeviceInfo>>` |
| `find(type, colorIndex)` | `shared_ptr<DeviceInfo>` | 按设备型号+颜色索引查找 |
| `deviceSysTypeName(vid, pid)` | `QString` | VID/PID 反查设备类型简写 |
| `deviceLabel(deviceId, isTest)` | `static QString` | 查硬编码表返回设备标签名称 |
| `shortDisplayName(fullName)` | `static QString` | 截去 "XIBERIA " 前缀 |
| `init()` | `void` | 启动时初始化：读磁盘 → 回退默认表 |
| `defaultConfig()` | `void` | 写入默认设备表（硬编码 ~30 SKU） |
| `save(filePath)` / `load(filePath)` | `bool` | 持久化/恢复 |
| `configFilePath()` | `static QString` | 配置文件路径：`ProgramData/deviceInfo/DevInfo.dat` |

#### Design Notes

- **饿汉单例**：`main()` 前构造，线程安全。
- **线程安全**：写路径（init/defaultConfig/save/load）由 `cl_mutex_` 保护；读路径（find/deviceMap）调用方应在 GUI 线程使用。
- **配置持久化**：设备表序列化到 `ProgramData/deviceInfo/DevInfo.dat`，版本号 `kConfigVersion = 2`（v2：DeviceInfo 新增 `DeviceManualUrl` 说明书 URL；版本不匹配自动回退 `defaultConfig()` 重建缓存）。

## Dependencies

- Qt 5.15+ (`QtGui` / `QPixmap`, `QSize`)
- `api/device/device_info.h`（`DeSheng::DeviceInfo`）
- 无业务模块依赖
