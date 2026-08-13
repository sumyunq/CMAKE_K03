# InterfaceSettings -- 界面设置

## 概述

管理语言切换、主题选择、壁纸切换、背景透明度和面板透明模糊度的界面设置页面。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `interface_settings_main_page.h/cpp/ui` | `InterfaceSettingsMainPage` | 主页面：语言/主题下拉框 + 透明度/模糊滑块 + 壁纸区 |
| `InterfaceSettingCustomUI/custom_QWidget_background_images.h/cpp` | `CustomQWidgetBackgroundImages` | 单个壁纸卡片（4 种形态：默认/系统/自定义/添加） |
| `InterfaceSettingCustomUI/custom_QScrollArea_background_images_component.h/cpp/ui` | `CustomQScrollAreaBackgroundComponent` | 壁纸网格滚动容器（header/list/tail 三段式） |

## 页面布局

```
+-----------------------------------------+
|  语言: [中文 v]   主题: [深蓝 v]       |
+-----------------------------------------+
|  背景透明度: [=======o======]           |
|  面板模糊度: [====o==========]           |
+-----------------------------------------+
|  +------+ +------+ +------+ +------+  |
|  |默认   | |壁纸1 | |壁纸2 | |壁纸3 |  |
|  +------+ +------+ +------+ +------+  |
|  +------+ +------+ +------+ +------+  |
|  |壁纸4 | |壁纸5 | |壁纸6 | |自定义|  |
|  +------+ +------+ +------+ +------+  |
|         ... [+ 添加背景]                |
+-----------------------------------------+
```

## 语言切换（QTranslator）

全局文件级变量 `QTranslator tran`（定义在 `interface_settings_main_page.cpp` line 12），头文件 include `<QTranslator>`。

```
cBox_language::currentIndexChanged(int index)
  -> onLanguageChanged(index)
    -> qApp->removeTranslator(&tran)   // 先移除旧翻译
    -> tran.load(":/LanguageDemo_zh_CN.qm")  // index 0 简体中文
    -> tran.load(":/LanguageDemo_zh_TC.qm")  // index 1 繁体中文
    -> tran.load(":/LanguageDemo_en_US.qm")  // index 2 英语
    -> qApp->installTranslator(&tran)  // 安装新翻译
    -> emit languageChange()           // 通知 MainWindow 刷新全部页面 LanguageSet()
```

三份 `.qm` 文件以 Qt 资源形式嵌入（`:/` 前缀）。

## 壁纸数据模型

三张 `QSharedPointer<QMap<int, QSharedPointer<WallpaperEntry>>>`：
- `default_wallpaper_map`：index 0 = DefaultTheme（不持久化）
- `system_wallpaper_map`：index 1-6 = SystemTheme（扫描 exe 目录）
- `custom_wallpaper_map`：自定义壁纸（按 `WallpaperStorageScope` 持久化）

当前临时需求下，`WallpaperStorageScope::AppLocalJson` 为默认值：
- 壁纸选择、自定义壁纸列表、背景透明度、面板模糊度统一保存到应用本地 `Config/wallpaper_config.json`
- 自定义壁纸文件保存到应用本地 `Config/CustomBackground/`
- 后续如果客户要求重新跟随登录用户，可将存储范围切回 `WallpaperStorageScope::UserLocal`

## 完整信号链

```
CustomQScrollAreaBackgroundComponent
  -> backgroundChanged(path) / defaultBackgroundRestored()
    -> InterfaceSettingsMainPage (前向)
      -> MainWindow
        -> AppImageCache::updateBackgroundCache()
        + scheduleBlurRebuild()
          -> updateBlurredBackdrop()
        -> paintEvent 重绘
```

`InterfaceSettingsMainPage` 自身信号（转发到 MainWindow）：
- `backgroundTransparencyChanged(qreal opacity)` -- 背景透明度变化
- `panelBlurChanged(qreal radius)` -- 面板模糊度变化
- `backgroundChanged(const QString &path)` -- 壁纸变更
- `defaultBackgroundRestored()` -- 恢复默认背景
- `languageChange()` -- 语言切换请求

## 滑块优化

- **valueChanged**：只更新内存值 + 触发实时重绘
- **sliderReleased**：才调用 `saveWallpaperConfigAsync()` 持久化背景表现配置

`syncSlidersFromModel()`：轻量方法，仅同步滑块值（无壁纸刷新），供 `UpdateAllSubPageUIInformation(All)` 路径使用，避免卡顿。

## 命名规范

- **主页面遵循新式规范**（`cl_`/`clp_`），含 `cl_theme_` 成员
- `NoIconEffectDelegate` 为局部辅助类（`QStyledItemDelegate` 子类），旧式命名，定义在头文件内
- 语言/主题下拉框 ComboBox 使用 `NewComboBox`（`#include "CustomControl/NewComboBox.h"`）
