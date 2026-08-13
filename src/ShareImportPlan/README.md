# ShareImportPlan — 方案分享导入

## QDialogCustom

通过分享码导入方案的模态对话框。支持两种分享码格式。

### 导入流程

```
1. 用户粘贴或输入分享码（支持格式化文本 或 纯码）
2. 粘贴过滤器自动提取纯码（解析 + 分隔符，兼容 - 旧格式）
3. 确定前缀路由：
   ys / 8位短码 → GET /api/v1/schemes/resolve/{code}
   sq / 34位Base64码 → GET /api/v1/user-configs/share/{code}/download
4. 成功 → 从返回 URL 下载方案 INI 文件
5. 校验文件格式（LoadPlan/Name）→ 导入方案 → accept()
```

### 公共接口

| 方法 | 签名 | 说明 |
|------|------|------|
| `setTitle` | `void setTitle(QString title)` | 设置对话框标题文本 |
| `set_btn_ok_text` | `void set_btn_ok_text(QString text)` | **【P1 Bug】** 实际设置的是 `cl_cancelBtn_`（取消按钮）文本，非确认按钮 |
| `set_btn_cancel_text` | `void set_btn_cancel_text(QString text)` | **【P1 Bug】** 实际设置的是 `cl_okBtn_`（确认按钮）文本，非取消按钮 |

> **已知 P1 Bug**（QDialog_custom.cpp:26-33）：`set_btn_ok_text` 和 `set_btn_cancel_text` 的实现互换了目标按钮。
> - `set_btn_ok_text(text)` → `cl_cancelBtn_->setText(text)` —— 写入了取消按钮
> - `set_btn_cancel_text(text)` → `cl_okBtn_->setText(text)` —— 写入了确认按钮
>
> 调用方如果按方法名字面意思使用，确认/取消按钮文案将颠倒显示。

### API 路由

| 前缀 | 接口 | 响应 |
|------|------|------|
| `ys` | `schemes/resolve/{code}` | `{ data.url, data.status, data.device_name }` |
| `sq` | `user-configs/share/{code}/download` | `{ data.config_url }` |

### 错误码

- **sq 路由** 6 种：`unauthorized` / `share_code_format_error` / `share_code_decode_error` / `share_code_invalid` / `share_code_not_found` / `request_rate_limited`
- **预设库路由** 3 种：`unauthorized` / `invalid_param` / `status != "active"`（配置已失效）

### 关键变更（2026-07-20）

- 网络请求从本地 `QNetworkAccessManager` 迁移至 `BaseClient::get(path, key)` + `ApiServerSwitch`
- 分享码格式支持 `名称+机型+场景+ys/sq{码}`，从右向左逐 `+` 搜索分隔符
- 验证正则 `[A-Za-z0-9+/=._-]+` 兼容 Base64 RawURL（大小写敏感）
- `textChanged` 输入过滤不再转大写、不限制 8 字符，上限 64 字符
- 错误码区分：6 种 sq 错误码 / 预设库错误码分别提示
- 下载后校验文件格式：`QSettings::IniFormat` + `LoadPlan/Name`
