# HomePage — 首页

## 概述

登录后的主首页，由 `HomePageMainPage` 作为顶层容器。通过 6 个 FrostedPanel 面板组成仪表盘布局：
产品展示 / 扬声器 / 麦克风 / 算法调节 / 方案选择 / 人声调节。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `home_page_main_page.h/cpp/ui` | `HomePageMainPage` | 顶层容器，组装所有子面板 |
| `HomePageCustomUI/custom_QWidget_product_display.h/cpp/ui` | `CustomQWidgetProductDisplay` | 产品展示区（设备图 + 动画 + 遮罩按钮） |
| `HomePageCustomUI/custom_QWidet_speaker_setting.h/cpp/ui` | `CustomQWidetSpeakerSetting` | 扬声器设置面板（ComboBox + 开关 + 音量滑块） |
| `HomePageCustomUI/custom_QWidget_microphone_setting.h/cpp/ui` | `CustomQWidgetMicrophoneSetting` | 麦克风设置面板（ComboBox + 开关 + 电平滑块） |
| `HomePageCustomUI/custom_QWidget_microphone_adjustment.h/cpp/ui` | `CustomQWidgetMicrophoneAdjustment` | 人声调节（清晰度/深度拨动开关） |
| `HomePageCustomUI/custom_QWidget_algorithm_adjustment_setting.h/cpp/ui` | `CustomQWidgetAlgorithmAdjustmentSetting` | 算法调节（4 个场景 EQ：脚步声/枪声/声场/清晰度） |
| `HomePageCustomUI/custom_QWidget_single_algorithm_setting.h/cpp/ui` | `CustomQWidgetSingleAlgorithmSetting` | 单个算法滑块（图标 + 名称 + GearSlider + 增减按钮） |
| `HomePageCustomUI/custom_QWidget_plans_selection.h/cpp/ui` | `CustomQWidgetPlansSelection` | 方案预设选择（2x2 游戏场景按钮 + QButtonGroup） |
| `HomePageCustomUI/custom_QPushButton_single_plan.h/cpp` | `CustomQPushButtonSinglePlan` | 单个方案预设按钮（3 态图标 + 自绘） |
| `HomePageCustomUI/custom_QPushbutton_hover_leftward.h/cpp` | `CustomQPushButtonHoverLeftward` | 悬停左向展开按钮（32x32 → 99x32 动画） |

## 架构

```
HomePageMainPage (QWidget, FrostedPanel 圆角=10)
  ├─ CustomQWidgetProductDisplay         ← 继承 CumtomQWidgetGlobalBase
  │   ├─ CustomQPushButtonHoverLeftward x3 (选择型号/声音设置/用户指南)
  │   ├─ flyLabel (动画浮层)
  │   └─ NewCustomToolTip (信息提示)
  ├─ CustomQWidetSpeakerSetting          ← FrostedPanel, 圆角=10
  │   ├─ NewComboBox (设备选择)
  │   ├─ CustomPushButton (开关)
  │   └─ NewHSlider (音量)
  ├─ CustomQWidgetMicrophoneSetting      ← FrostedPanel, 圆角=10
  │   ├─ NewComboBox
  │   ├─ CustomPushButton
  │   └─ NewHSlider
  ├─ CustomQWidgetAlgorithmAdjustmentSetting ← FrostedPanel, 圆角=10
  │   ├─ CustomPushButton (总开关)
  │   └─ CustomQWidgetSingleAlgorithmSetting x4
  │       └─ GearSlider (步进滑块 0-6)
  ├─ CustomQWidgetPlansSelection         ← FrostedPanel, 圆角=10
  │   ├─ CustomPushButton (总开关)
  │   └─ CustomQPushButtonSinglePlan x4 (2x2 网格)
  └─ CustomQWidgetMicrophoneAdjustment   ← FrostedPanel, 圆角=10
      ├─ CustomPushButton x2 (清晰度/深度)
      └─ NewCustomToolTip
```

## 关键信号链

### 算法滑块 → APO ExtendEq

4 个算法通道分别映射到 APO 扩展 EQ 通道 2-5：

| 算法通道 | AlgorithmType 枚举 | ExtendEq idx |
|----------|-------------------|:------------:|
| 脚步增强 | `FootstepEnhance` | 2 |
| 枪声弱化 | `GunshotWeakening` | 3 |
| 声场控制 | `SoundFieldControl` | 4 |
| 清晰度 | `Clarity` | 5 |

每个通道的完整调用链：
```
GearSlider::valueChanged → HomePageMainPage lambda
  ├─ requestSetExtendEqState(idx, on/off)          // 值==0 关闭, >0 打开
  ├─ requestSetExtendEqualizerCenterFrequencyEx()   // 设置频点
  ├─ requestSetExtendEqualizerBandQualityEx()       // 设置 Q 值
  └─ requestSetExtendEqualizerGainEx()              // 设置 Gain（7 档预设表）
```

### 算法值 → 全局状态

- `HomePageExtraEQValue`（`QList<int>`，长度 4）：存储 4 个算法通道的当前滑块值，定义于 `mainwindow.cpp`，`extern` 声明于 `home_page_main_page.h`。
- `HomePageExtraEQOpen`（`bool`）：算法调节总开关状态。
- 滑块值变更后触发 `HomePageMainPage::HomePageEQValueChange()` 信号，通知外部（如 EQ 曲线图）刷新。

### 方案选择 → 全局状态

- `QButtonGroup::idClicked` → 更新全局变量 `currentPlanVal`、`HomePagePlansOpen`
- `HomePagePlansOpen`（`bool`）：方案预设总开关状态。

### 语言切换

- `LanguageSet()` → `ui->retranslateUi()` + 各子面板 `retranslateTexts()`

## 全局变量

声明于 `modules/HomePage/home_page_main_page.h`，定义于 `mainwindow.cpp`：

| 变量 | 类型 | 说明 |
|------|------|------|
| `HomePageExtraEQValue` | `QList<int>` | 4 个算法通道的当前值（索引 0-3 对应 ExtendEq idx 2-5） |
| `HomePageExtraEQOpen` | `bool` | 算法调节总开关 |
| `HomePagePlansOpen` | `bool` | 方案预设总开关 |

## 命名规范

- 全部遵循 wbliu 规范（`cl_`/`clp_` 前缀 + `_` 后缀）
- 注意：`CustomQWidetSpeakerSetting` 有拼写错误（"Widet" → "Widget"）
- `CustomQPushButtonHoverLeftward` 文件名 "Pushbutton" 首字母小写不一致
