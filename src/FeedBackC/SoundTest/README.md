# SoundTest — 音效试听页面

## 概述

游戏场景音效试听功能。从服务器拉取视频列表（按游戏类型分场景），在网格中展示，用户点击后在嵌入的 FFmpeg 播放器中播放。

## 文件

| 目录/文件 | 职责 |
|-----------|------|
| `sound_test_main_page.h/cpp/ui` | 顶层页面。QStackedWidget 切换视频列表 / 播放 |
| `SoundTestCustomUI/` | 自定义控件：6 个 widget（游戏类型按钮、视频卡片、下载动画等），共 7 .cpp + 7 .h + 4 .ui |
| `SoundTestDataStruct/video_data.h/cpp` | 数据模型类 `VideoData`，内部持有 `DeSheng::videoConfig`（含 `xhub_videos_grouped_` 和 `videosType`） |

## 架构

```
SoundTestMainPage (QStackedWidget)
  ├─ page_videos_viewing
  │   ├─ GameTypeSelectedScrollArea (游戏类型横向滚动按钮栏)
  │   │   └─ QPushButtonSingleGameType × N (场景按钮，从 scene_keys_ 动态创建)
  │   ├─ VideoScrollArea × N（GridDisplay 网格布局 + SingleColumnDisplay 单列布局）
  │   │   └─ SingleVideoInfo (卡片：封面 + 标题 + 下载状态)
  │   │       ├─ QWidgetVideoInfo (封面 + hover 缩放动画 QVariantAnimation，1.0→1.08)
  │   │       └─ QWidgetVideoMask (半透明遮罩 + 下载/播放按钮)
  │   └─ DownLoadVideoPending (下载进度动画 QMovie GIF)
  │
  └─ page_video_playing
      └─ widget_for_ffmpegPlaying
          └─ FFmpegMainPage (ffmpage 视频播放器，经由 QStackedWidget 切换到播放页)
```

## 10 场景键

硬编码在 `scene_keys_` 中：`xhub_01` ~ `xhub_10`。构造函数启动时对每个场景键发送一次 `AuditionsListRequest`，通过 `echoes_number_`（`std::atomic<int>`）计数回显。10 次回显全部完成后触发 `checkLocalConfigurationFile()` → 以服务端为准更新本地配置 → `updateUIFirst()` 渲染 UI。

## 已知问题

- **`netWorkFinished` 10 段重复代码（~463 行，L790--L1253）**：每个场景键（`xhub_01` ~ `xhub_10`）的 URL 匹配、JSON 解析、数据合并逻辑完全相同，仅 scene key 字符串不同。应该提取为一个私有辅助函数，传入 `scene` 参数，消除 10 段重复。

## 数据流

```
HTTP GET /api/v1/auditions?scene=xhub_01&device_type=headset
  → AuditionsListResponse.data.list (QList<VideoItem>)
  → VideoData::cl_video_config_netWork_.xhub_videos_grouped_[scene] 填充
  → echoes_number_ 递增，全部完成后 checkLocalConfigurationFile()
  → 网络数据 merge 到本地配置 → updateUIFirst()
  → 按场景键分组 → cl_video_scrollArea_grid_map_ / cl_video_scrollArea_vBox_map_
  → 动态创建 SingleVideoInfo 卡片
```

## VideoItem 结构（`DeSheng::VideoItem`，定义于 `api/`）

| 字段 | 类型 | 说明 |
|------|------|------|
| id | int | 视频 ID |
| title | QString | 标题 |
| scene / sceneName | QString | 场景键 / 场景名 |
| imgUrl / videoUrl | QString | 封面 / 视频 URL |
| localStatus | VideoStatus | 本地下载状态（UnDownloaded/Downloading/Downloaded） |
| localPath / coverLocalPath | QString | 本地缓存路径 |

## VideoData（数据管理类）

`VideoData : QObject`，定义于 `SoundTestDataStruct/video_data.h`。持有两份配置：

| 成员 | 类型 | 说明 |
|------|------|------|
| `cl_video_config_local_` | `DeSheng::videoConfig` | 本地持久化视频配置 |
| `cl_video_config_netWork_` | `DeSheng::videoConfig` | 网络拉取的视频配置 |

## 自定义控件

| 控件 | 基类 | 用途 |
|------|------|------|
| `GameTypeSelectedScrollArea` | QScrollArea | 水平滚动按钮栏 + QButtonGroup 互斥 |
| `VideoScrollArea` | QScrollArea | 网格/单列自适应视频列表 |
| `QPushButtonSingleGameType` | QPushButton | 游戏类型切换按钮（checkable） |
| `SingleVideoInfo` | QWidget | 单视频卡片（封面+标题+下载） |
| `QWidgetVideoInfo` | QWidget | 封面图 + hover 缩放动画（QVariantAnimation） |
| `QWidgetVideoMask` | QWidget | 半透明覆盖层（下载/播放/小窗按钮） |
| `DownLoadVideoPending` | QWidget | 下载进度动画（QMovie GIF） |

## 依赖

- `api/api_global.h`：`AuditionsListRequest`、`AuditionsListResponse`、`VideoItem`、`VideoStatus`、`videoConfig`
- `FeedBackC/ffmpage/ffmpeg_main_page.h`：`FFmpegMainPage`（视频播放）
- `FeedBackC/SoundTest/SoundTestDataStruct/video_data.h`：`VideoData`
- `QNetworkAccessManager`：视频列表下载 + 视频文件下载
- `QStandardPaths`：本地缓存路径

## 关键 include 路径示例

```cpp
#include "FeedBackC/SoundTest/sound_test_main_page.h"
#include "FeedBackC/SoundTest/SoundTestCustomUI/game_type_selected_scrollarea.h"
#include "FeedBackC/SoundTest/SoundTestCustomUI/video_scroll_area.h"
#include "FeedBackC/SoundTest/SoundTestDataStruct/video_data.h"
#include "FeedBackC/ffmpage/ffmpeg_main_page.h"
```

## API 变更（2026-07-20）
- `cl_AuditionsList_url_` 默认值移除 → 构造函数中 `resolveUrl + serverKey`
- 本地 `QNetworkAccessManager` 独立实例 → 专用于试听视频列表请求
- `netWorkFinished` 作为 `QNetworkReply::finished` 的统一槽函数，通过 URL 匹配分发到 10 个场景分支
