# AdvertisementSelectionPage — 广告轮播

## 概述

首页广告横幅轮播，显示在登录页和主页面。通过 `BaseClient` 异步获取广告列表，递归下载图片到本地缓存 `advertisement_cache/`，全部就绪后发射 `advertisementListReady()` 信号作为登录门控。自动 2 秒轮播，底部导航圆点指示器。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `advertisement_selection_main_page.h/cpp/ui` | `AdvertisementSelectionMainPage` | 广告轮播主页面 |
| `AdvertisementSelectionPageCustomUI/custom_QPushbutton_for_single_advertisement.h/cpp` | `CustomQPushButtonForSingleAdvertisement` | 底部导航圆点（默认 8x8 / 选中 24x8，动画切换） |
| `AdvertisementSelectionPageCustomUI/custom_QScrollarea_for_advertisement_pushbutton.h/cpp/ui` | `CustomQScrollAreaForAdvertisementPushButton` | 底部按钮条（stretch 居中布局） |

## 轮播机制

```
cl_change_timer_ (QTimer, 2s)
  → timeout → QButtonGroup 选中下一个按钮
    → cl_current_index_++ (atomic)
    → updateAdvertisementIndex() → load 缓存图片 → update() 重绘
```

- **鼠标进入**：停止定时器（暂停轮播）
- **鼠标离开**：启动定时器（恢复轮播）
- **鼠标移动到按钮条区域**：恢复定时器（按钮条区域不做暂停）
- **点击广告图**：通过 `BaseClient` POST 广告点击 API

## 图片渲染

`paintEvent` 中自绘：
- 圆角 15px 裁剪（`QPainterPath::addRoundedRect` + `setClipPath`）
- QPixmap 缩放到 `rect()` 尺寸（`Qt::IgnoreAspectRatio, Qt::SmoothTransformation`）
- 图片从本地缓存加载：`cl_cache_path_ + img_url.section("/", -1)`

## 网络请求

所有请求通过新栈 `ApiClient::instance()` 异步发起（`modules/CommunityModule/infrastructure/network/api_client.h`），URL 路由由 `ServerRouter` 按 tag "ad" 解析。

| 接口 | 方式 | 说明 |
|------|------|------|
| `GET /advertisements` | `BaseClient::get()` | 获取广告列表。query: `scene=home_banner`, `device_type=headset`。**异步**，finished 信号中处理响应 |
| `GET {img_url}` | `QNetworkAccessManager::get()` | 递归下载广告图片到本地缓存。每张下载完成后调下一张 |
| `POST /advertisements/{id}/click` | `BaseClient::post()` | 记录广告点击（异步，fire-and-forget） |

### 图片递归下载

```
updateAdvertisementList()
  → BaseClient::get() 异步获取列表
    → sort by sort_order 降序
    → downloadAdImages(0)
      → QNetworkAccessManager::get(img_url) → 保存到 cl_cache_path_
        → downloadAdImages(idx + 1)  // 递归下一张
          → … 全部完成
            → 创建导航按钮 → emit advertisementListReady()
```

## 缓存路径

```cpp
cl_cache_path_ = QStandardPaths::AppDataLocation + "/ProgramData/advertisement_cache/";
// 典型路径: C:/Users/<user>/AppData/Local/XIBERIA X HUB/ProgramData/advertisement_cache/
```

每次 `updateAdvertisementList()` 调用时先删除旧缓存目录再重建。

## 登录门控

`advertisementListReady()` 信号在所有广告图片下载完成后发射。MainWindow 连接此信号，收到后才进入登录初始化流程 —— 确保首页渲染时广告图片已就绪。

| 信号 | 说明 |
|------|------|
| `advertisementListReady()` | 广告列表及全部图片下载完成（登录门控） |

## 命名规范

- **完全遵循新式规范**（`cl_`/`clp_`/`t_`）
- 无 `applyTheme()` 或 `LanguageSet()`（硬编码暗色主题）

## 依赖

- `ApiClient`（`modules/CommunityModule/infrastructure/network/api_client.h`）：广告列表获取 + 点击上报
- `QNetworkAccessManager`：图片下载
- `QStandardPaths`：本地缓存路径
- `QTimer`：2s 轮播定时器
