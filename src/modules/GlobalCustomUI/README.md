# GlobalCustomUI — 全局基类控件

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `cumtom_QWidget_global_base.h/cpp` | `CumtomQWidgetGlobalBase` | 全局 Widget 基类，提供所有实例统一的透明度/模糊控制 |

> ⚠️ 类名有拼写错误："Cumtom" 应为 "Custom"，编码中继承此类的子类都使用了错误的名称。

## CumtomQWidgetGlobalBase

### 设计模式

**静态注册表 + 批量更新**：每个实例构造时自动注册到静态 `s_g_instances_list_`，析构时移除。全局透明度/模糊变更时遍历全部实例统一更新。

### 静态 API

| 方法 | 说明 |
|------|------|
| `s_g_Opacity()` / `setS_g_Opacity(double)` | 全局透明度（0.0–1.0, atomic） |
| `s_g_BlurRadius()` / `setS_g_BlurRadius(qreal)` | 全局模糊半径（0.0–25.0, atomic） |
| `updateAllInstances()` | 遍历全部实例调用 `applyEffects()` |

### 实例结构

```
CumtomQWidgetGlobalBase (QWidget)
  ├─ cl_background_widget_ (QWidget*)  ← 背景层，接收 QGraphicsBlurEffect
  └─ cl_content_widget_ (QWidget*)     ← 内容层（透明度效果代码已注释）
```

### 当前状态

- **模糊效果**：已实现，通过 `QGraphicsBlurEffect` 应用到 `cl_background_widget_`
- **透明度效果**：代码已写但被注释（`cl_opacityEffect_` + `QGraphicsOpacityEffect`）
- **唯一子类**：`CustomQWidgetProductDisplay`（`src/modules/HomePage/HomePageCustomUI/custom_QWidget_product_display.h`，HomePage 产品展示区）

### 命名规范

- `cl_` 前缀 + `_` 后缀
- 静态成员用 `s_` / `s_g_` 前缀
- Init 三方法模式
