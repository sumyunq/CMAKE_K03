# DeviceSelectionPage — 设备选择页面

## Overview

应用启动时的设备选择/机型选择界面。展示所有支持的设备型号及颜色变体，用户可以浏览并选择当前连接的设备。选中后通知 MainWindow 进入主页面。

## Directory Structure

```
src/modules/DeviceSelectionPage/
├── README.md                                           # ← 本文件
├── device_selection_main_page.h/.cpp/.ui               # 主页面
└── DeviceSelectionPageCustomUI/
    ├── custom_QPushButton_roundbutton.h/.cpp            # 圆形导航按键（右侧行号）
    ├── custom_QPushButton_single_device_color.h/.cpp    # 颜色变体选择按键
    ├── custom_QScrollArea_device_selection.h/.cpp/.ui   # 设备网格滚动区域
    ├── custom_QScrollArea_roundbutton.h/.cpp/.ui        # 右侧按键滚动容器
    ├── custom_QScrollarea_color_pushbuttons.h/.cpp/.ui  # 颜色按键滚动容器
    └── custom_QWidget_single_device_information.h/.cpp/.ui # 单个设备信息卡片
```

## 旧壳套新核

`DeviceSelectionMainPage` 是纯 UI 页面（不含设备列表构建逻辑）。设备列表的构建由旧代码 `src/DeviceSel.cpp` 完成：

```
DeviceSel::InitUIInformation()
  → new DeviceSelectionMainPage(ui->widget_device_section_page)  // 创建新页面

DeviceSel::DevSelInitialization()
  → UpdateDeviceSelectionMainPage()
    → 遍历全局设备集合 ERdevs
    → 根据 VID/PID/GUID 匹配 DeviceRegistry 中的 DeviceInfo
    → AddTargetDeviceInfo() 填充设备网格
```

这种模式保持了旧页面外壳（`DeviceSel` QDialog）与 UI 文件不变，内部实际显示的是新版 `DeviceSelectionMainPage`。

## Data Model

### DeviceInfo (`api/device/device_info.h`)

设备信息结构体，14 个字段按类型分块：

| 字段 | 类型 | 说明 |
|------|------|------|
| `DeviceTypeName` | QString | 设备型号（如 "T10有线"） |
| `DeviceColorName` | QString | 颜色中文名称 |
| `DeviceColorPixmapPath` | QString | 颜色对应的资源路径 |
| `DeviceColorRGB` | QString | 颜色 RGB 值 |
| `isChecked` | bool | 是否默认选中 |
| `DeviceHomePagePixmapPath` | QString | 首页产品图片 |
| `DeviceHomePageTopLeftPixmapPath_normal` | QString | 首页左上角图标（正常） |
| `DeviceHomePageTopLeftPixmapPath_abnormal` | QString | 首页左上角图标（异常） |
| `DeviceMoreSetPixmapPath` | QString | 更多设置 — 耳机图片 |
| `DeviceMoreSetQrCodePixmapPath` | QString | 更多设置 — 二维码图片 |
| `DeviceSysTypeName` | QString | 系统显示名称 |
| `DeviceGuid` | QString | Windows 设备 GUID（APO 参数） |
| `SelDev_VID` | unsigned short | USB VID |
| `SelDev_PID` | unsigned short | USB PID |

### 全局设备表 DeviceRegistry

由 `modules/Common/DeviceRegistry` 饿汉单例管理，`main()` 启动早期调 `DeviceRegistry::instance().init()` 初始化。

```cpp
#include "modules/Common/DeviceRegistry.h"
auto &t_reg = DeSheng::DeviceRegistry::instance();
auto t_info = t_reg.find("T10有线", 0);
```

## Key Functions

### DeviceRegistry（`src/modules/Common/DeviceRegistry.h/.cpp`）

| 函数 | 说明 |
|------|------|
| `instance()` | 饿汉单例，main() 前构造 |
| `init()` | 启动时调用：读磁盘 → 不存在则调 `defaultConfig()` → 持久化 |
| `defaultConfig()` | 写入默认设备表（23 个变体，逐字段命名赋值） |
| `save(path, mode)` | 持久化：-1=异步加密，0=同步 XOR+Base64 |
| `load(path)` | 从磁盘恢复，版本不匹配则放弃 |
| `find(type, index)` | 按键查找，无则返回 nullptr |
| `deviceMap()` | 只读访问完整 `QMap<{机型,序号}, shared_ptr<DeviceInfo>>` |
| `deviceSysTypeName(vid, pid)` | 根据 VID/PID 返回设备简写（T10 / K03S / …） |
| `deviceLabel(deviceId, isTest)` | 查硬编码表返回设备标签名称 |
| `shortDisplayName(fullName)` | 截去 "XIBERIA " 前缀返回简称 |
| `configFilePath()` | 静态，返回 `ProgramData/deviceInfo/DevInfo.dat` |
| `kConfigVersion` | 配置文件版本号 = 1 |

### DeviceSelectionMainPage

| 函数 | 说明 |
|------|------|
| `syncRowButtons()` | 根据设备网格行数同步右侧圆形导航按键 |

### 选中设备流程

```
用户点击设备卡片
  → CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo(deviceInfo)
    → DeviceSelectionMainPage::setCl_selected_device_information(deviceInfo)
      → 通知 MainWindow 进入主页
```

## Current Devices

| 机型 | 颜色数 | SysTypeName |
|------|:--:|------|
| T10有线 | 4 (玄墨黑/蚀金黑/凌空灰/皓月银) | T10 |
| T10无线 | 3 (玄墨黑/香槟金/皓月银) | T10Wireless |
| K03S | 4 (武士黑/熊猫白/极光粉/宝石蓝) | K03S |
| K03有线版 | 1（全空值预留） | — |
| K03S超竞版 | 4 (武士黑/熊猫白/极光粉/宝石蓝) | K03SGC |
| K06S | 2 (黑色/云贝白) | K06S |
| T7 | 5 (玄墨黑/香槟金/雪山粉/卡布里蓝/赤芒红) | T7 |
| T7 GT | —（新增 2026-08-07，PID 0xF015/0xF009） | T7 GT |

> DeviceSel 构造卡片时传第三参数（按机型显示方案开关）：K03有线版/T7 = true，其余 false；T7 GT 为新增支持机型

## Conventions

- `DeviceInfo` 使用逐字段命名赋值（`t_info->FieldName = value;`），不使用位置初始化。
- 新增机型时在 `defaultConfig()` 中追加条目，并更新 `kConfigVersion`。
- 颜色序号从 0 递增，每个机型的默认颜色（`isChecked = true`）固定在序号 0。
- 资源路径统一放在 `:/Resources/Skin/Images/DevSel/` 下，按 `selectionPage` / `homePageDevice` / `leftTopIcon` 分子目录。
