# UserFeedBack — 用户反馈

## 概述

用户反馈提交流程：文本表单 + 图片上传（最多 3 张，`FeedbackImagesWidget::maxSize = 3`） → 服务器 POST。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `feedback_main_page.h/cpp/ui` | `FeedbackMainPage` | 顶层页面。表单 + 提交 + QStackedWidget 状态页切换 |
| `feedback_scrollarea_widget.h/cpp/ui` | `FeedbackScrollareaWidget` | 可滚动表单：设备信息（自动填充）、类型下拉、标题、描述、图片区 |
| `feedback_images_widget.h/cpp/ui` | `FeedbackImagesWidget` | 图片缩略图横排容器（`maxSize = 3`，`InitMember()` 中初始化） |
| `feedback_images_label.h/cpp` | `FeedbackImagesLabel` | 单个缩略图 (80×80)：左键添加图片、右键删除、大小校验 |
| `custom_QWidget_feedback_return_page.h/cpp/ui` | `CustomQWidgetFeedBackReturnPage` | 三态结果页：提交中（GIF 动画）/ 成功 / 失败 |

## FeedbackMainPage

### 提交流程

```
1. 用户填表 → 点"提交"
2. requiredFieldVerify() 必填校验
3. 发送 FeedBackSubmitting 信号 → 显示结果页（GIF 动画 "提交中"）
4. 若无图片 → 直接 onAllImagesUploaded() 跳文本提交
5. 多图上传：QHttpMultiPart + BaseClient::postMultipart("/user/uploads", ...)，每图独立 lambda 回调
6. 上传回调计数 → 全部完成 → onAllImagesUploaded()
7. 构建 UserFeedBacksRequest → sendFeedback() → BaseClient::post("/feedbacks", json, t_key)
8. 成功：FeedBackSubmitSucceed + 工单号 ticket_no
9. 失败：FeedBackSubmitFail + 错误提示
```

### 线程与锁

| 组件 | 机制 |
|------|------|
| 上传计数器 | `cl_upload_images_mutex_`（QMutex）保护 `cl_uploadImagesResponse_` 列表的 append / size / clear |
| 网络回调 | lambda 内联，通过 `reply->deleteLater()` 回收 |

### 关键变更（2026-07-20）

- 本地 QNetworkAccessManager 删除 → 统一使用 `DeSheng::BaseClient`
- `onReplyFinished` 全局回调删除 → 每请求独立 lambda
- `cl_feedback_url_` (QUrl) → `cl_feedback_path_` (QString)，路由在 BaseClient 层
- `sendFeedback`：手动 Auth+Header → `BaseClient::post`
- multipart 上传：`QHttpMultiPart` + `BaseClient::postMultipart(path, multiPart, key)`
- 服务端路由通过 `ApiServerSwitch::serverKey(g_api_server_switch.feedback, ...)`

## 已知问题

- **Content-Type 硬编码 `image/png`**（`feedback_main_page.cpp:142--143`）：上传图片时 `ContentTypeHeader` 固定为 `"image/png"`，不根据实际文件扩展名（JPEG、BMP 等）动态设置。如果用户选择非 PNG 图片，服务端按 `image/png` 解析可能出现偏差。

## 命名规范

- 主要遵循 `cl_`/`clp_` 约定
- `CustomQWidgetFeedBackReturnPage` 有 `applyTheme(int)` 和 `cl_theme_`
- 图片容器用 `maxSize`（非 `cl_` 前缀），初始化为 `3`

## 关键 include 路径示例

```cpp
#include "FeedBackC/UserFeedBack/feedback_main_page.h"
#include "FeedBackC/UserFeedBack/feedback_scrollarea_widget.h"
#include "FeedBackC/UserFeedBack/feedback_images_widget.h"
#include "FeedBackC/UserFeedBack/feedback_images_label.h"
#include "FeedBackC/UserFeedBack/custom_QWidget_feedback_return_page.h"
#include "api/api_global.h"  // BaseClient + ApiServerSwitch
```
