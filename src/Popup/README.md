# Popup — 弹窗组件

## 概述

应用程序级别弹窗集合，共 19 个组件。除 `NewCustomToolTip` 继承 `QWidget` 外，其余均继承 `QDialog`，模态运行。

## 弹出窗口清单

### 通用

| 类 | 文件 | 用途 |
|----|------|------|
| `FactoryReset` | `FactoryReset.h/cpp/ui` | 恢复出厂确认。4 秒倒计时安全机制 |
| `bExitDirectly` | `bExitDirectly.h/cpp/ui` | 关闭窗口行为选择（最小化到托盘 / 直接退出） |
| `UserGuide` | `UserGuide/UserGuide.h/cpp/ui` | 用户引导页弹窗 |
| `WeChatCode` | `WeChatQRCode/WeChatCode.h/cpp/ui` | 微信扫码登录弹窗（内嵌 QWebEngineView） |
| `RestartPrompt` | `RestartPrompt/RestartPrompt.h/cpp/ui` | 重启应用提示弹窗 |
| `NewCustomToolTip` | `CustomTipPopup/NewCustomToolTip.h/cpp` | 自定义气泡提示（继承 QWidget）。`setLabelStyle(idx)` 三种样式：聊天气泡（带三角箭头, 0）、2.4G 链接提示（1）、方案描述弹窗（2）。挂载方式：`AddToolTip(target, text, align)` + `showToolTipBelow()` |

### 方案管理（Plans/）

| 类 | 文件 | 用途 |
|----|------|------|
| `DelReset` | `Plans/DelReset.h/cpp/ui` | 删除/重置方案确认。`editText(idx)` 切换 7 种文案（0:删除方案 / 1:重置均衡器 / 2:重置算法 / 3:重置空间音频 / 4:删除分类 / 5:删除所有选中方案 / 6:删除已上传方案） |
| `EditPlansType` | `Plans/EditPlansType.h/cpp/ui` | 新建/重命名方案分类。`EditTitle(idx)` 切换标题，`ShowEditName(id, txt)` 编辑已有分类名 |
| `MovePlan` | `Plans/MovePlan.h/cpp/ui` | 移动方案到其他分类（NewComboBox 下拉）。`addType/delType/delAllType/rnameType/showType` 管理分类列表 |
| `PlanCopy` | `Plans/PlanCopy.h/cpp/ui` | 复制/新建方案（名称/描述/设备标签/目标分类/场景）。`showCurName/showSysName/ShowDev` 初始化数据 |
| `EqualizerHidden` | `Plans/EqualizerHidden.h/cpp/ui` | 均衡器隐藏提示。挂载 `NewCustomToolTip` 显示提示气泡，`GetEqShowEn()` 返回用户确认状态 |
| `UploadMyPlans` | `Plans/UploadMyPlans.h/cpp/ui` | 上传方案到社区。搜索/分类/描述/设备标签，`writeExportPlanIni` 导出方案 INI，`planUploaded(configId)` 信号通知上传结果 |
| `UploadPlanSuccess` | `Plans/UploadPlanSuccess.h/cpp/ui` | 上传方案成功提示。`ShowUploadPlanCnt(cnt)` 显示当日已上传数量 |

### 固件/软件更新（Update/）

| 类 | 文件 | 用途 |
|----|------|------|
| `UpdateOTA` | `Update/UpdateOTA.h/cpp/ui` | OTA 固件升级主弹窗。状态机：Idle → WaitDevice → BurnFirmware → TP1_UpdateDone → WaitAfterBurn → Done/Failed。`runOTAFile(path, type, ver)` 启动升级流程 |
| `UpdateOTAFind` | `Update/UpdateOTAFind.h/cpp/ui` | 发现 OTA 更新/回退版本通知。`updateTitle(theme, type, version)` 设置标题，4 秒倒计时自动关闭 |
| `UpdateOTASuccess` | `Update/UpdateOTASuccess.h/cpp/ui` | OTA 更新/回退成功。`UpdateTitle(type)` 切换"升级"/"回退"标题 |
| `UpdateError` | `Update/UpdateError.h/cpp/ui` | 更新错误通知。`setTitle(type)` 四态文案 |
| `UpdateSoftWareFind` | `Update/UpdateSoftWareFind.h/cpp/ui` | 驱动/软件更新通知 + 下载进度。`setShowPage(idx)` 切换通知/进度页面，信号 `startUpdate()` |
| `UpdateVersion` | `Update/UpdateVersion.h/cpp/ui` | "已是最新版本" / "无回退版本" 通知。`UpdateVer(ver)` 显示版本号，`UpdateTitle(type)` 切换标题 |

## 命名规范

- **全部使用旧式命名**（`m_` 前缀、`ui->pBt_` 风格）
- 无一遵循 wbliu `cl_`/`clp_` 规范
- 使用 Qt `QMetaObject::connectSlotsByName` 自动连接

## 共同模式

- 全部通过 stylesheet 设置背景/圆角/阴影
- 大部分有 `setTheme_*` 方法支持主题切换（值为 0/1/2，但通常只有 theme=0 实现）
- `exec()` 模态运行

## 已知问题

### UpdateOTA — CreateThread 句柄泄漏 + GUI 线程 msleep 阻塞
- `runOTAFile()`（UpdateOTA.cpp:524）：通过 `CreateThread` 创建 `DownFWThread` 工作线程，返回的 `HANDLE hThread` 未调用 `CloseHandle`，每次 OTA 升级泄漏一个线程句柄。
- `waitForDeviceReady()`（UpdateOTA.cpp:377/398/405）：在 GUI 线程直接调用 `QThread::msleep(10000)` / `QThread::msleep(40000)`，阻塞事件循环，导致升级期间 UI 假死无响应。

### DelReset — case 5 fall-through 导致文案覆盖
- `editText()`（DelReset.cpp:72-78）：case 5（删除所有选中方案）末尾缺少 `break;`，执行完 case 5 的文案设置后 fall-through 到 case 6（删除已上传方案），覆盖为 case 6 的提示文案 `"删除仅下架社区，预设库预设保留不变"`。用户看到的删除选中方案确认弹窗将显示错误提示。

## API 变更（2026-07-20）
- `UploadMyPlans`：multipart → `buildRequest+manager()->post`，POST → `BaseClient::post`
- 删除 `createRequest` 死函数
