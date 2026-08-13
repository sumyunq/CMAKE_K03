# FeedBackC — 反馈与媒体子系统

## 概述

FeedBackC 包含 4 个子模块：固件升级工具（FirmwareTool）、音效试听（SoundTest）、用户反馈（UserFeedBack）、FFmpeg 视频播放器（ffmpage）。

## 子模块

| 模块 | 目录 | 职责 |
|------|------|------|
| FirmwareTool | `FirmwareTool/` | USB HID 固件升级 DLL 封装（`UsbCliBridge.dll`）。`openFWFile()` 为零调用死代码。 |
| SoundTest | `SoundTest/` | 游戏场景试听页面（10 场景视频列表 + FFmpeg 播放）。`netWorkFinished` ~463 行 10 段重复代码为已知问题。 |
| UserFeedBack | `UserFeedBack/` | 用户反馈表单 + 多图上传（最多 3 张）。Content-Type 硬编码 `image/png` 为已知问题。 |
| ffmpage | `ffmpage/` | FFmpeg 5.1 视频播放器（5+1 线程管线）。音频同步线程跨线程写 QAudioOutput、seek 3--15s 卡顿、FFmpegIODevice 拉模式未接入为已知高风险问题。 |

## 与主应用的交互

- `SoundTestMainPage` 被 `SpeakerListen`（旧版监听页）内嵌使用
- `FeedbackMainPage` 被 `ContactSettingsMainPage`（新版联系我们页）内嵌使用
- `FirmwareTool` 被 `FeedbackScrollareaWidget` 和 `VersionSettings` 使用
- `FFmpegMainPage` 被 `SoundTestMainPage` 和 `VideoHover`（迷你窗口）内嵌使用

## 线程模型

| 模块 | 线程模型 |
|------|----------|
| FirmwareTool | 主线程，同步阻塞（`Sleep(50)` 等 DLL 响应） |
| SoundTest | 主线程（`QNetworkAccessManager` 异步），FFmpeg 子线程（视频解码） |
| UserFeedBack | 主线程（网络异步 + `QMutex` 保护图传计数 `cl_upload_images_mutex_`） |
| ffmpage | **5 自定义 QThread**（解封装 ×1 + 解码 ×2 + 同步 ×2）+ **1 QAudioOutput 内部 OS 回调线程**。音频设备初始化（`initAudio()`）在主线程同步执行，音频同步线程通过 Push Mode 直接 `write()` 写入音频设备。 |
