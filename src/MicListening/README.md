# MicListening — WASAPI 麦克风监听

## 概述

基于 Windows WASAPI 的麦克风实时环回（Mic Monitoring / Sidetone）。从默认麦克风捕获音频，**直通到默认扬声器播放**。

## 文件

| 文件 | 职责 |
|------|------|
| `WASAPIPipeline.h/cpp` | WASAPI Capture → Render 直通管线 |

## WASAPIPipeline

### API

| 方法 | 说明 |
|------|------|
| `WASAPIPipeline()` | 构造函数，`CoInitializeEx(COINIT_MULTITHREADED)` |
| `initialize()` | 协商通用格式 → 创建 IAudioClient(capture + render) → 获取 IAudioCaptureClient / IAudioRenderClient |
| `start()` | 启动两个 audio client → `std::thread` 运行 `audioLoop()` |
| `stop()` | 设置 atomic 标志 → join thread → stop clients |

### 格式协商

尝试 4 种格式（16-bit PCM）：
1. 2ch / 44100 Hz
2. 2ch / 48000 Hz
3. 1ch / 44100 Hz
4. 1ch / 48000 Hz

使用 `AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM` 自动转换不匹配格式。

### 音频环回

- 缓冲区长度：10ms
- 处理：`memcpy` capture buffer → render buffer（无任何处理）
- 线程：`std::thread`（非 Qt 线程）

## 线程安全

- `m_running` 为 `std::atomic<bool>`
- 无其他线程同步机制
- **未集成 Qt 信号槽系统**

## 已知限制

- 无延迟管理 / 无 buffer underrun 处理
- 设备运行时被拔出无错误恢复
- COM 在构造函数以 `COINIT_MULTITHREADED` 初始化，适用于独立线程
