# VersionSettings -- 版本升级

## 概述

软件更新和耳机固件 OTA 升级管理页面。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `version_settings_main_page.h/cpp/ui` | `VersionSettingsMainPage` | 主页面。左侧：驱动版本 + "检查更新"按钮；右侧：固件版本 + 回退/升级按钮 |

## 功能

### 驱动器/软件更新

```
检查更新 -> HTTP GET 服务器 API
  -> QVersionNumber::fromString 版本比较（serverVer > currentVer）
    -> UpdateSoftWareFind 弹窗（下载进度）
      -> startDownload(url) -> QSaveFile 原子写入
        -> ShellExecuteExW 运行下载的 .exe
```

**QVersionNumber**（`<QtCore>` / `#include "QtCore"`）：`QVersionNumber::fromString(verStr)` 解析版本号，`operator>` 比较。

**QSaveFile**：软件更新下载使用 `QSaveFile(DfilePath)` 确保原子写入（下载成功才 commit，失败自动丢弃）。

**ShellExecuteExW**：下载完成后 `ShellExecuteExW(&sei)` 以管理员权限启动安装程序。

### 固件 OTA 升级

```
RX/TX 固件:
  检查更新 -> HTTP GET OTA 服务器
    -> 版本比较
      -> UpdateOTAFind 弹窗
        -> UpdateOTA 弹窗（状态机）:
          Idle -> WaitDevice -> BurnFirmware -> Done/Failed
```

### TP1 中介升级

部分旧版设备需先升级到 TP1 中间版本，再升级到最新版。通过文件级静态变量控制：
- `UpdateTP1` (bool)：是否先升级为 TP1 中介版本
- `TP1TxVer` / `TP1RxVer` (QString)：TP1 发射器/耳机版本号
- `UpdateTP1En` (bool)：是否已弹窗告知用户 TP1 信息
- `UpdateRxDone` / `UpdateTxDone` (bool)：RX/TX 升级完成标记

### 固件回退

类似升级流程，但下载回退版本而非最新版本。`OTARollbackEn` 标记控制。

## 文件级静态全局变量（OTA 状态传递）

| 变量 | 类型 | 说明 |
|------|------|------|
| `UpdateTP1` | `bool` | 先升级为 TP1 中介版本 |
| `TP1TxVer` | `QString` | TP1 发射器版本号 |
| `TP1RxVer` | `QString` | TP1 耳机版本号 |
| `UpdateTP1En` | `bool` | 是否已弹窗告知 TP1 信息 |
| `UpdateRxDone` | `bool` | RX 升级是否完成 |
| `UpdateTxDone` | `bool` | TX 升级是否完成 |
| `readlastVer_RX` | `QString` | 上次读取的 RX 版本（如 "260302"） |
| `readlastVer_TX` | `QString` | 上次读取的 TX 版本（如 "260305"） |
| `iDevIndex` | `int` | 设备索引 |
| `lastVerStr_RX` | `QString` | 最新 RX 版本字符串 |
| `lastVerStr_TX` | `QString` | 最新 TX 版本字符串 |

注：`SoftWareVer`、`DongleVer[31]`、`EarVer[31]` 定义在 `user_setting_main_page.cpp`，不属于本模块。

## 更新弹窗（Popup/Update/）

| 弹窗 | 用途 |
|------|------|
| `UpdateOTA` | 主 OTA 升级对话框（状态机 + 进度条 + 重试） |
| `UpdateOTAFind` | 发现新版本通知（4 秒倒计时） |
| `UpdateOTASuccess` | 升级完成 |
| `UpdateError` | 升级失败（4 种类型切换） |
| `UpdateSoftWareFind` | 软件更新通知 + 下载进度 |
| `UpdateVersion` | 已是最新版本通知 |

## 关键方法

| 方法 | 说明 |
|------|------|
| `startGenericDownload(url, filePtr, filePath, onSuccess, isOta)` | 通用下载（固件/软件共用） |
| `startBinDownload(url, filePtr, filePath, finishCallback)` | 固件 .bin 下载 |
| `startOTADownloadTX / startOTADownloadRX(url)` | TX/RX 固件专用下载 |
| `cancelCurrentDownload()` | 取消当前下载 |
| `runDownloadedFile(filePath)` | ShellExecuteExW 运行安装程序 |
| `getVersionUIText()` | 获取界面显示的版本号文本 |
| `SetOTAEn(bool)` | 设置固件升级是否可用 |
| `setDeviceIconPixmap(QPixmap)` | 设置设备图标 |

`QPointer<QNetworkReply> SoftWareLoad_Reply` 用于安全追踪软件下载 reply 生命周期。

## 命名规范

- **旧式命名为主**：`m_manager`、`uota`、`DfilePath`、`file_RX`、`file_TX`、`OTARollbackEn` 等
- 仅 `clp_target_user_setting_main_page_` 遵循新规范
- 无 `applyTheme()` / `cl_theme_`

## 国际化

- `LanguageSet()`：调用 `ui->retranslateUi(this)`

## API 变更（2026-07-20）

- `m_baseUrl` 硬编码 URL -> `ApiConfig::resolveUrl + serverKey`
- drive 模块 -> `g_api_server_switch.drive`，firmware 模块 -> `g_api_server_switch.firmware`
