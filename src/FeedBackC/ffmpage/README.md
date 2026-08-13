# ffmpage — FFmpeg 视频播放器

## 概述

基于 FFmpeg C API（5.1）的多线程视频播放器。架构是 `ffplay` 参考实现到 Qt/C++ 的移植：解封装 → 解码 → 同步 → 渲染/播放。音频输出使用 Qt `QAudioOutput` 推模式（Push Mode），`FFmpegIODevice`（拉模式）已实现但未接入。

## 架构（5+1 线程管线）

5 个自定义 `QThread` + 1 个 QAudioOutput 内部 OS 回调线程：

```
[媒体文件]
    │
    ▼
FFmpegUnpackageThread (解封装)                    — QThread #1
    │  av_read_frame → AVPacketQueue_video + AVPacketQueue_audio
    ▼
FFmpegDecoderThread ×2 (音/视解码)               — QThread #2, #3
    │  avcodec_send_packet → AVFrameQueue_video + AVFrameQueue_audio
    ▼
FFmpegSyncThread ×2 (音/视同步)                   — QThread #4, #5
    ├─ 视频同步(startSyncVideo):
    │    对比 Master Clock → 计算延迟 delay → QThread::msleep
    │    → sws_scale RGB24 → emit frameReady(QImage)
    │    → FFmpegMainPage::paintEvent 渲染(主线程)
    │
    └─ 音频同步(startSyncAudio):
        对比 Master Clock → swr_convert 重采样
        → QAudioOutput(QIODevice*)::write() 推模式直接写入
        → 更新音频主时钟
```

第 6 个线程：QAudioOutput 内部 OS 回调线程（非自定义 QThread）。音频设备 `initAudio()` 在 `FFmpegGlobal::startAllThread()` 中于**主线程**同步调用，确保 WASAPI/COM 初始化正确。

### QAudioOutput 两种模式对比

| 模式 | 机制 | 当前状态 |
|------|------|----------|
| **Push Mode**（推模式） | `QAudioOutput::start()` → 返回内部 `QIODevice*`，工作线程直接 `write()` | **实际使用。** `initAudio()` 获取 `targetAudioDevice` 指针，音频同步线程调用 `targetAudioDevice->write()` |
| **Pull Mode**（拉模式） | `QAudioOutput::start(QIODevice*)` → QAudioOutput 回调 `QIODevice::readData()` | **未接入。** `FFmpegIODevice` 已实现 `readData()` 含缓冲、重采样、时钟更新，但未被任何 `QAudioOutput::start()` 传入 |

## 目录结构

| 目录/文件 | 职责 |
|-----------|------|
| `ffmpeg_main_page.h/cpp/ui` | 播放器 UI：视频渲染 `paintEvent` + 进度条 + 控制按钮（4 个 QPushButton） |
| `VideoModule/ffmpeg_global.h/cpp` | 全局协调器（FFmpegGlobal）：拥有全部线程、队列、Clock、编解码上下文 |
| `VideoModule/ffmpeg_iodevice.h/cpp` | FFmpegIODevice：拉模式 `readData()`（已实现，未接入） |
| `VideoModule/ffmpeg_target_audio_device.h/cpp` | FFmpegTargetAudioDevice：QAudioOutput 设备初始化和托管 |
| `ThreadModule/` | 3 种工作线程（解封装、解码、同步），各 .h/.cpp 对 |
| `publicStruct/` | 数据队列（AVPacketQueue / AVFrameQueue）+ Clock + Decoder + MyAVFrame |
| `CustomUI/` | 自定义进度条 QSliderPlayingProgress |
| `public_space.h/cpp` | Windows 工具 + 时间格式化辅助 |

## FFmpegMainPage

### API

| 方法/槽 | 说明 |
|----------|------|
| `open(const char *url)` | 打开并播放媒体文件（内部调用 `FFmpegGlobal::open()`） |
| `seek(double seconds)` | 跳转到指定位置（`std::atomic` CAS 防重入） |
| `pause()` | 切换暂停/播放（通过 `FFmpegGlobal::pause()` 翻转 `cl_is_pause_` + 时钟暂停 + `cl_pause_Cond_.wakeAll()`） |
| `updateMainPage_Frame(const QImage &)` | 槽：接收同步后解码帧并 `update()` 触发 `paintEvent` |
| `onSliderDrag(int)` | 槽：进度条拖动释放 → seek |
| `onMinWidgetSlots(bool)` | 槽：启用/禁用迷你窗模式 |

### 信号

| 信号 | 说明 |
|------|------|
| `fullScreen(bool)` | 全屏切换（被 VideoHover 迷你窗连接） |
| `EnableSmallWindowMode(bool)` | 启用/禁用迷你窗模式（被 SoundTestMainPage 连接） |

### UI 控件

- `QSliderPlayingProgress`：自定义进度条。左半填充（蓝色动画 `#006184→#0091DA`）、右半透明白、hover 放大（6×16→10×26）、点击跳转、滚轮微调
- 4 个 QPushButton：`pBn_min_widget_`（迷你窗）、`pBn_request_pause_`（暂停/播放）、`pBn_full_screen_`（全屏）、`pBn_exit_full_screen_`（退出全屏），均为样式表 + 图片皮肤（`:/Skin/Images/soundTest/`）
- `label_play_time_`：时间显示

### 内部状态

| 成员 | 类型 | 说明 |
|------|------|------|
| `transitioning_` | `std::atomic<bool>` | 视频切换过渡期标志，拒绝旧帧（防止前一个视频的延迟信号污染画面） |
| `is_slider_dragging_` | `bool` | 进度条拖拽中标志 |
| `saved_playback_frame_` | `QImage` | 拖拽前保存播放画面，释放时恢复 |
| `loop_check_timer_` | `QTimer*` | 循环播放检测定时器 |
| `thumbnail_cache_` | `QMap<int, QPixmap>` | 缩略图缓存（key: 毫秒数，间隔 1000ms） |

## QSliderPlayingProgress

`CustomUI/QSlider_playing_progress.h/cpp`，自定义绘制的水平进度条：

- **左 groove**：填充色 3 个 `QVariantAnimation` 淡入淡出（`#006184` → `#0091DA`）
- **右 groove**：半透明白（`#66FFFFFF`）
- **handle**：hover 放大 `6×16 → 10×26`，颜色动画
- **点击 groove**：直接跳转（非拖拽）
- **滚轮**：聚焦时微调

## 线程安全

- 每个 `AVFormatContext` / `AVCodecContext` / `SwsContext` / `SwrContext` 由独立 `QMutex` 保护（如 `cl_avFormatContext_Mutex_`、`cl_decoder_video_Mutex_`、`sub_convert_ctx_Mutex_`、`swr_ctx_Mutex_`）
- 包队列 / 帧队列：`QMutex` + `QWaitCondition` 有界生产者-消费者
- 控制标志：`std::atomic<bool>`（`cl_is_pause_`、`cl_is_stop_`、`cl_is_seeking_`、`has_video_stream_`、`has_audio_stream_`）
- `seek` 重入保护：`cl_is_seeking_` 通过 `compare_exchange_strong` CAS 操作
- `frameReady(QImage)` 通过 `Qt::QueuedConnection` 跨线程发到主线程
- seek 旧帧过滤：`seek_target_pts_` / `seek_serial_` 原子变量，同步线程检测并丢弃关键帧之前的帧

## Clock（A/V 同步）

```cpp
struct Clock {
    QMutex clock_mutex;
    double pts_;            // 当前 PTS
    double pts_drift_;      // PTS - 系统时间
    double last_updated_;   // 上次更新时间
    double speed_;          // 播放速度
    int serial_;            // 播放序列号（seek/切换文件时 +1）
    bool paused_;
};
```

默认以音频为主时钟（`AVSyncType::AudioMaster`）。视频同步线程对比 `get_master_clock()` 与当前帧 PTS：
- 视频超前 > 10ms：`QThread::msleep(delay)` 等待
- 视频落后 > 100ms：丢帧跳过

## 依赖

- **FFmpeg 5.1 C API**：libavcodec / libavformat / libavutil / libswscale / libswresample
- **Qt Multimedia**：`QAudioOutput` / `QAudioDeviceInfo` / `QAudioFormat`
- **Windows API**：`<windows.h>` / `<tchar.h>` / `<winnt.h>`
- `VideoHover`（迷你窗浮层）
- `public_space.h`（Windows 工具函数 + 时间格式化）

## 关键 include 路径示例

```cpp
#include "FeedBackC/ffmpage/ffmpeg_main_page.h"
#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"
#include "FeedBackC/ffmpage/VideoModule/ffmpeg_iodevice.h"
#include "FeedBackC/ffmpage/VideoModule/ffmpeg_target_audio_device.h"
#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_unpackage_thread.h"
#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_decoder_thread.h"
#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_sync_thread.h"
#include "FeedBackC/ffmpage/publicStruct/ffmpeg_public_struct.h"
#include "FeedBackC/ffmpage/CustomUI/QSlider_playing_progress.h"
```

## 已知高风险问题

### 1. 音频同步线程跨线程写 QAudioOutput（P0）

**位置**: `ffmpeg_sync_thread.cpp:254--269` (`startSyncAudio`)

音频同步线程运行在 `clp_sync_audio_thread_`（独立 QThread）中，直接调用 `targetAudioDevice->write(outBuffer, real_bytes)`。但 `targetAudioDevice` 由 `QAudioOutput::start()`（Push Mode）在**主线程** `initAudio()` 中获取。Qt 文档要求 `QAudioOutput` 的 `write()` 调用应在创建线程或同一线程执行，跨线程写可能导致：

- `QAudioOutput::bytesFree()` 轮询在另一线程读取设备状态（数据竞争）
- WASAPI 内部状态不一致

**缓解建议**: 改用 `FFmpegIODevice`（拉模式），由 QAudioOutput 内部线程回调 `readData()`，或使用 `QMetaObject::invokeMethod` 将 PCM 数据投递到 `FFmpegTargetAudioDevice` 所在线程。

### 2. seek 3--15 秒卡顿（P1）

**位置**: `ffmpeg_global.cpp:590--736` (`FFmpegGlobal::seek`)

seek 流程：`stopAllThread()` → 刷新所有队列 → `avformat_seek_file`（阻塞 I/O）→ 刷新解码器 → 更新时钟 → `startAllThread()`。

问题在于 `stopAllThread()` 调用 `QThread::quit()` + `wait(3000)` 逐线程关闭，如果线程正在阻塞（队列 dequeue 等待 3000ms 超时），全流程耗时 = 5 线程 × 最多 3s = 最坏 15 秒。实际测量通常在 3--15 秒范围内。

**缓解建议**: seek 前先 `flush()` 所有队列（触发 dequeue 立即返回），再 `quit()` + `wait()`。

### 3. FFmpegIODevice 拉模式未接入（P2）

**位置**: `ffmpeg_iodevice.cpp:44--195` (`readData`)

`FFmpegIODevice` 实现了完整的拉模式 `readData()`：内部缓冲 + 帧出队 + swr_convert 重采样 + 音频时钟更新。但 `ffmpeg_target_audio_device.cpp:38` 中 QAudioOutput 仍然使用 Push Mode：
```cpp
targetAudioDevice = targetAudioOutput->start();  // Push Mode，未传入 FFmpegIODevice
```
应改为 `targetAudioOutput->start(cl_ffmpegIODevice_.get())` 以接入拉模式。此改动也可以解决问题 #1（跨线程写 QAudioOutput），因为 `readData()` 由 QAudioOutput 内部线程安全回调。
