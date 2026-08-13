# ContactSettings -- 联系我们

## 概述

"联系我们"页面。左侧：用户反馈表单，右侧：联系方式（电话/官网链接/社区/二维码）。

## 文件

| 文件 | 类 | 职责 |
|------|-----|------|
| `contact_settings_main_page.h/cpp/ui` | `ContactSettingsMainPage` | 主页面。左侧反馈表单 + 右侧联系信息 |

## 子控件

| 控件 | 类型 | 说明 |
|------|------|------|
| `clp_feedBackPage_` | `FeedbackMainPage*` | 反馈提交表单（`#include "FeedBackC/UserFeedBack/feedback_main_page.h"`） |
| `clp_feed_back_return_page` | `CustomQWidgetFeedBackReturnPage*` | 提交结果页，3 态（提交中 / 成功 / 失败），`#include "FeedBackC/UserFeedBack/custom_QWidget_feedback_return_page.h"` |

## 反馈提交流程

```
填表 -> 点"提交"
  -> FeedbackMainPage::FeedBackSubmitting
    -> 切换到 clp_feed_back_return_page（显示 GIF "提交中"）
  -> 成功：FeedBackSubmitSucceed -> 成功图标 + 工单号
  -> 失败：FeedBackSubmitFail -> 失败图标 + "重试"按钮
  -> actionButtonClicked -> 回到反馈表单
```

## 联系信息（右侧面板）

- 电话号码
- 官网链接（`eventFilter` 拦截 `QEvent::MouseButtonPress` -> `QDesktopServices::openUrl(QUrl("https://www.xiberia.net"))`）
- 社区信息
- 二维码图片（`setQrCodePixmap(QPixmap)` 缩放至 `label_QR_code` 尺寸，`Qt::KeepAspectRatio + Qt::SmoothTransformation`）

## 网络请求

- 无直接网络请求（委托 `FeedbackMainPage` 处理）
- `UpdateContactSettingsUIInformation()` 通过 `clp_target_user_setting_main_page_` 调用父级：
  - `DevGetVersion()` -> HID 读取耳机版本信息
  - `SoftGetVersion()` -> 获取驱动版本号
  - 然后 `clp_feedBackPage_->showOutInfo()` 填充反馈表单设备信息
  - `clp_feedBackPage_->setCl_access_token(access_token)` 设置用户 token

## 命名规范

- **完全遵循新式规范**（`cl_`/`clp_`/`t_`）
- `applyTheme(int)` 存在但函数体为空占位（`cl_theme_ = theme;`）
- `clp_target_user_setting_main_page_` 反向持有父级指针（用于调用 `DevGetVersion` / `SoftGetVersion`）

## 国际化

- `LanguageSet()`：调用 `ui->retranslateUi(this)`
