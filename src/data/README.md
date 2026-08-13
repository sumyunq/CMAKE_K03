# api — 网络请求模块

## Overview

api 是 XIBERIA X HUB 的**接口定义层**（路径常量 + Request/Response 结构体 + 解析函数）与**历史网络栈**。
**2026-08-04 起网络栈已统一迁移至新栈**：发请求用 `ApiClient`（`modules/CommunityModule/infrastructure/network/api_client.h`）+ `RequestOptions`，URL 路由用 `ServerRouter`（4 服注册 + 13 模块 tag 绑定，见 main.cpp），token 用 `AuthStore`。`BaseClient`/`ApiConfig` 已标注 `@deprecated`，仅保留供历史代码兼容。迁移方案见 `doc/reports/api_migration_plan_20260804.md`。

> **注意**：`overseas` 和 `overseas-t` 的 URL 当前为空字符串，海外服尚未配置。共 11+2 个子模块（+googleOauth/userDevice），~43 个 API 路径常量（用户端+管理端合计 90+ 端点）。

## Core Components

> ⚠️ **本节以下为历史旧栈（BaseClient/ApiConfig/ApiServerSwitch）描述，已于 2026-08-04 退役删除**（BaseClient.h/ApiConfig.h/ApiUtils.h 已从仓库移除，调用方全部迁移至新栈 ApiClient/ServerRouter/AuthStore）。本模块当前仅保留接口定义层（`ApiPaths` 路径常量 + Request/Response 结构体 + `ProcessXxxResult`/`XxxRequestToJson`/`buildXxxQuery` 三函数），供新栈调用方引用。新代码请勿参考本节写法。

**ApiConfig** = 配置中心。只管"这条请求去哪、等多久、带什么身份"。没有网络能力。

**BaseClient** = HTTP 客户端。管"发出去"。每次发请求前先问 ApiConfig 拿 URL/超时/Token，拼好 Header 再发。

**调用关系**：

```
你调 BaseClient::get("/user/login")
  → BaseClient 内部调 ApiConfig::resolveUrl("/user/login")
    → ApiConfig 查："/user/login" 没有模块缺省 → 用全局默认 "domestic"
    → 返回 "https://hubsys.xiberia.net/api/v1/user/login"
  → BaseClient 拿这个 URL + authToken() 拼好 QNetworkRequest
  → manager()->get(request) 发出去
  → 返回 QNetworkReply* 给你
```

**ApiConfig 可以单独用**（只拼 URL，不发请求）：

```cpp
QString t_url = ApiConfig::instance().resolveUrl("/user/login");        // → domestic
QString t_url = ApiConfig::instance().resolveUrl("/user/login", "overseas"); // → overseas
```

## Directory Structure

```
api/
├── README.md
├── api_global.h/.cpp      # 聚合入口 + UserInformation + extern g_user_information
├── ApiConfig.h/.cpp          # 配置单例：多服务器 + 模块缺省 + 超时 + 鉴权
├── BaseClient.h/.cpp         # 网络客户端单例：QNetworkAccessManager 池 + CRUD
├── internal/
│   └── ApiUtils.h/.cpp       # 共享工具：processReply / buildFileUpload
├── user/ feedback/ audition/ userConfig/ schemes/ ad/ drive/ firmware/
│   userLevel/ userDeviceLog/ wechatOauth/ device/
└── 各模块 *_api.h 顶部含 ApiPaths 路径常量
```

## Usage Patterns

### 前置

```cpp
#include "api/BaseClient.h"
#include "api/internal/ApiUtils.h"
#include "api/user/user_api.h"       // ApiPaths（按需包含对应模块）

auto &cli = DeSheng::BaseClient::instance();
using namespace DeSheng;
```

### 1. 服务器环境

```cpp
auto &cfg = ApiConfig::instance();

// 预注册 4 个服务器（构造时自动注册，无需手动调 registerServer）
// "domestic"   → https://hubsys.xiberia.net/api/v1          国内正式服（默认）
// "domestic-t" → https://hubsystest.xiberia.net/api/v1    国内测试服
// "overseas"   → (空字符串，尚未配置)                        海外正式服
// "overseas-t" → (空字符串，尚未配置)                        海外测试服

// 全局切到测试服
cfg.setDefaultServer("domestic-t");

// 模块级缺省：firmware/drive 默认走海外
cfg.setModuleDefault("/firmware/", "overseas");
cfg.setModuleDefault("/drive/",    "overseas");

// 运行时动态注册新服务器
cfg.registerServer("staging", "https://staging.example.com/api/v1");
```

### 2. GET（默认服务器，异步）

```cpp
QNetworkReply *r = cli.get(ApiPaths::kUserMe);
connect(r, &QNetworkReply::finished, this, [r]() {
    int status; QJsonDocument doc;
    if (!processReply(r, doc, status)) { /* 网络错误 */ }
    else { /* doc 可用 */ }
    r->deleteLater();
});
```

### 3. GET + Query 参数

```cpp
QUrlQuery q;
q.addQueryItem("page", "1");
QNetworkReply *r = cli.get(ApiPaths::kAdList, q);
```

### 4. POST JSON Body

```cpp
DeSheng::UserLoginRequest t_req;
t_req.email = "u@x.com"; t_req.password = "****";
QByteArray t_body = QJsonDocument(DeSheng::UserLoginRequestToJson(t_req)).toJson();
QNetworkReply *r = cli.post(ApiPaths::kUserLogin, t_body);
```

### 5. PUT JSON Body

```cpp
QByteArray t_body = QJsonDocument(DeSheng::UpdateUserRequestToJson(t_req)).toJson();
QNetworkReply *r = cli.put(ApiPaths::kUserUpdate, t_body);
```

### 6. DELETE

```cpp
cli.del(ApiPaths::kUserLogout);                                    // 无参数
cli.del(QString(ApiPaths::kConfigDetail).arg(42));                 // 子路径含 ID
```

### 7. 指定服务器（请求级 / 全局 / 模块缺省）

```cpp
// 请求级：单请求切到海外 / 测试服
cli.get(ApiPaths::kFirmwareInfo, "overseas");
cli.post(ApiPaths::kConfigBase, body, "domestic-t");
cli.del(QString(ApiPaths::kConfigDetail).arg(42), "overseas-t");

// 模块级：firmware / drive 模块默认走海外（启动时配一次）
cfg.setModuleDefault("/firmware/", "overseas");
cfg.setModuleDefault("/drive/",    "overseas");
// 此后 cli.get(ApiPaths::kFirmwareInfo) 自动走 overseas

// 全局级：所有请求切到测试服
cfg.setDefaultServer("domestic-t");

// 运行时动态注册新服
cfg.registerServer("staging", "https://staging.example.com/api/v1");
```

### 8. 并行访问国内/海外

```cpp
// 同时向国内和海外发同一请求，互不阻塞
auto *r1 = cli.get(ApiPaths::kUserMe, "domestic");   // → 国内
auto *r2 = cli.get(ApiPaths::kUserMe, "overseas");   // → 海外

// lambda 捕获直接区分结果来源
connect(r1, &QNetworkReply::finished, this, [r1]() {
    qDebug() << "国内结果";
    r1->deleteLater();
});
connect(r2, &QNetworkReply::finished, this, [r2]() {
    qDebug() << "海外结果";
    r2->deleteLater();
});

### 9. 子路径拼接（含 ID）

​```cpp
QString t_p1 = QString(ApiPaths::kConfigDetail).arg(42);   // → "/user-configs/42"
QString t_p2 = QString(ApiPaths::kConfigLike).arg(42);     // → "/user-configs/42/like"
QString t_p3 = QString(ApiPaths::kConfigComments).arg(42); // → "/user-configs/42/comments"

cli.get(t_p1);                          // GET  /user-configs/42
cli.post(t_p2, body);                   // POST /user-configs/42/like
cli.del(t_p1);                          // DELETE /user-configs/42
```

### 10. 文件上传（Multipart）

```cpp
QHttpMultiPart *t_mp = buildFileUpload("C:/avatar.png", "file");
if (!t_mp) return;
QNetworkReply *r = cli.postMultipart(ApiPaths::kUserUploads, t_mp);
t_mp->setParent(r);  // reply 析构时自动清理 multipart
```

### 11. 自定义 URL（第三方服务）

```cpp
auto *t_mgr = cli.manager();
QNetworkRequest t_req(QUrl("https://external.example.com/data"));
QNetworkReply *r = t_mgr->get(t_req);
// 此类请求不带 Authorization / Content-Type，须自行设置
```

### 12. 同步等待（仅非 GUI 线程）

```cpp
QNetworkReply *r = cli.get(ApiPaths::kUserMe);
QEventLoop loop;
connect(r, &QNetworkReply::finished, &loop, &QEventLoop::quit);
loop.exec();
// 处理...
r->deleteLater();
```

### 13. 并发请求

```cpp
auto *r1 = cli.get(ApiPaths::kUserMe);
auto *r2 = cli.get(ApiPaths::kUserLevel);
auto *r3 = cli.get(ApiPaths::kAdList);
// 同时发出，互不阻塞，各自独立回调
```

### 14. 请求取消

```cpp
r->abort();
```

### 错误处理（统一模式）

```cpp
int t_status = 0;
QJsonDocument t_doc;
if (!processReply(r, t_doc, t_status)) {       // ① 网络层
    showError(tr("网络请求失败")); return;
}
DeSheng::XxxResponse t_resp;
if (!DeSheng::ProcessXxxResult(t_resp, t_doc)) { // ② 解析层
    showError(tr("数据异常")); return;
}
if (t_resp.code != "success") {                  // ③ 业务层
    showError(t_resp.message); return;
}
// ④ 成功
```

---

## API Reference

### ApiConfig（饿汉单例，线程安全）

| Method | Description |
|--------|-------------|
| `instance()` | 获取单例引用 |
| `registerServer(key, url)` | 注册服务器（运行时动态添加） |
| `setDefaultServer(key)` | 全局默认服务器 |
| `setModuleDefault(prefix, key)` | 模块级缺省：指定路径前缀的默认服务器（最长前缀匹配），key 为空 = 清除 |
| `resolveUrl(path, key)` | 解析完整 URL。key 为空 → 模块缺省 → 全局缺省 |
| `timeoutMs()` / `setTimeoutMs(ms)` | 超时（默认 60000） |
| `authToken()` | Bearer token（从 `g_user_information` 现取） |
| `setAuthToken(t)` | 设置 Bearer token（线程安全） |

**预注册服务器**（构造时自动）：

| Key | URL |
|-----|-----|
| `domestic` | `https://hubsys.xiberia.net/api/v1`（默认） |
| `domestic-t` | `https://hubsystest.xiberia.net/api/v1` |
| `overseas` | `""`（空字符串，尚未配置） |
| `overseas-t` | `""`（空字符串，尚未配置） |

### BaseClient（饿汉单例，线程安全）

| Method | Description |
|--------|-------------|
| `instance()` | 获取单例引用 |
| `manager()` | `QNetworkAccessManager *`（首次调用懒创建，`QMutex` 保护） |
| `buildRequest(path)` | 构造请求，默认服务器 |
| `buildRequest(path, key)` | 构造请求，指定服务器 |
| `buildRequest(QUrl)` | 构造请求，已有完整 URL |
| `get(path)` / `get(path, query)` | GET |
| `post(path, body)` | POST |
| `put(path, body)` | PUT |
| `del(path)` / `del(path, query)` | DELETE |
| `postMultipart(path, mp)` | POST Multipart |
| 以上均含 `(..., key)` 重载 | 指定服务器 |

所有方法返回 `QNetworkReply *`，调用方负责 `deleteLater()`。

### ApiUtils（自由函数，线程安全）

| Function | Description |
|----------|-------------|
| `processReply(reply, doc, status)` | 统一解析：网络检查 → HTTP 状态 → JSON。`true` = 成功 |
| `buildFileUpload(path, fieldName)` | 构建单文件上传 `QHttpMultiPart`，失败返回 `nullptr` |

### ApiPaths — 路径常量（各模块头文件顶部）

| 模块 | 头文件 | 常量 |
|------|--------|------|
| user | `api/user/user_api.h` | `kUserLogin` `kUserSignUp` `kUserMe` `kUserLogout` `kUserSendCode` `kUserForgotPassword` `kUserChangePassword` `kUserUpdate` `kUserUploads` |
| firmware | `api/firmware/firmware_api.h` | `kFirmwareInfo` |
| drive | `api/drive/drive_api.h` | `kDriveInfo` |
| ad | `api/ad/ad_api.h` | `kAdList` `kAdClick` |
| audition | `api/audition/audition_api.h` | `kAuditionList` |
| feedback | `api/feedback/feedback_api.h` | `kFeedbackList` `kFeedbackUpload` |
| schemes | `api/schemes/schemes_api.h` | `kSchemeCreate` `kSchemeResolve` `kSchemeShare` |
| userConfig | `api/userConfig/user_config_api.h` | `kConfigBase` `kConfigCollection` `kConfigDetail` `kConfigLike` `kConfigDislike` `kConfigCollect` `kConfigComments` |
| userDeviceLog | `api/userDeviceLog/user_device_log_api.h` | `kDeviceLogCreate` `kDeviceLogList` |
| device | `data/device/device_info.h` | `DeSheng::DeviceInfo`（本地设备注册表数据：型号/颜色/资源路径/VID/PID/GUID + **DeviceManualUrl 说明书 URL**（2026-08-04 新增，型号级，由 DeviceRegistry MANUAL_* 常量配置）） |
| userLevel | `api/userLevel/user_level_api.h` | `kUserLevel` |
| wechatOauth | `api/wechatOauth/wechat_oauth_api.h` | `kWechatPreAuth` `kWechatLoginStatus` |

子路径含 `%1` 占位符的常量，使用 `QString(constant).arg(id)` 拼接（见 Usage §8）。

---

## URL 解析优先级

```
resolveUrl(path, key)
  │
  ├─ key 不为空 → 直接匹配 cl_servers_[key]   ← ① 请求级覆盖
  │
  └─ key 为空
       ├─ 遍历 cl_module_defaults_（最长前缀）   ← ② 模块级缺省
       │   例：path="/firmware/check" → 匹配 "/firmware/" → "overseas"
       │
       └─ 无匹配 → cl_default_key_              ← ③ 全局缺省
```

## Asynchronous Model

- 所有网络方法**天然异步**：调用立即返回，实际 I/O 由 Qt 事件循环驱动。
- 回调在**主线程**执行，可直接更新 UI。
- 支持并发请求，连接池自动排队。
- 取消：`reply->abort()`。
- 同步等待见 Usage §11。

## Error Handling Model

```
QNetworkReply
  ├─ processReply() → false ─── 网络传输层错误
  └─ processReply() → true ─── JSON 已解析
       ├─ ProcessXxxResult() → false ─── JSON 结构异常
       └─ ProcessXxxResult() → true ─── 结构正确
            ├─ code == "success" ─── 业务成功
            └─ code != "success" ─── 业务失败（见 errcode.go）
```

## Conventions

- 业务域模块仅定义数据结构 + 序列化，不持有网络状态。
- 网络调用统一通过 `BaseClient`。
- 接口路径通过 `DeSheng::ApiPaths::` 常量引用，禁止硬编码字符串。
- 服务器通过字符串 key 引用，预注册 key 见 ApiConfig 预注册表。
- 子路径含 ID 时使用 `%1` 占位 + `QString::arg()`。
- `QNetworkReply *` 必须 `deleteLater()`。
- `QNetworkAccessManager` 由 `BaseClient` 统一管理。

## Thread Safety

| 组件 | 机制 |
|------|------|
| `ApiConfig`、`BaseClient` | 饿汉单例 + `QMutex` 保护读写 |
| `ApiUtils` | 自由函数，无共享状态 |
| 业务域 API 模块 | 无共享状态，天然线程安全 |
| `QNetworkReply` | 仅 GUI 线程使用 |

## Migration

```cpp
// 旧写法
auto *t_mgr = new QNetworkAccessManager(this);
t_mgr->setTransferTimeout(60000);
QNetworkRequest t_req(QUrl("https://hubsystest.xiberia.net/api/v1/user/login"));
t_req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
t_req.setRawHeader("Authorization", "Bearer " + token.toUtf8());
QNetworkReply *r = t_mgr->post(t_req, body);

// → 新写法
auto &cli = DeSheng::BaseClient::instance();
QNetworkReply *r = cli.post(DeSheng::ApiPaths::kUserLogin, body);
// URL / Auth / 超时 / Content-Type / 服务器全部自动
```

## Adding a New API Endpoint

以新增 `GET /user/settings` 为例，完整步骤：

### 1. ApiPaths 路径常量

在 `api/user/user_api.h` 顶部 `namespace ApiPaths` 中：

```cpp
inline constexpr const char *kUserSettings = "/user/settings";
```

### 2. 请求/响应结构体

同一个 `user_api.h`，按现有模式定义：

```cpp
/// GET /user/settings 应答
typedef struct GetUserSettingsResponse
{
    QString code;
    QString message;
    typedef struct ReturnData { /* ... */ } ReturnData;
    ReturnData data;
} GetUserSettingsResponse;
```

### 3. Process / ToJson / buildQuery 三函数

`user_api.h` 声明，`user_api.cpp` 实现：

```cpp
// user_api.h
bool ProcessGetUserSettingsResult(GetUserSettingsResponse &resp, QJsonDocument &doc);

// user_api.cpp — 按现有 Process 模板实现
bool DeSheng::ProcessGetUserSettingsResult(GetUserSettingsResponse &resp, QJsonDocument &doc) { ... }
```

### 4. api_global.h 聚合入口

`api/api_global.h` 已聚合所有子模块，无需改动。

### 5. 调用示例

```cpp
// 方式 A: BaseClient（自动 URL/Auth/超时）
auto &t_cli = DeSheng::BaseClient::instance();
QNetworkReply *t_r = t_cli.get(
    DeSheng::ApiPaths::kUserSettings, 
    ApiServerSwitch::serverKey(g_api_server_switch.user,
                               g_api_server_switch.test,
                               g_api_server_switch.overseas));
connect(t_r, &QNetworkReply::finished, [t_r]() { ... });

// 方式 B: 硬编码 URL → resolveUrl（非 BaseClient 场景）
QUrl t_url(DeSheng::ApiConfig::instance().resolveUrl(
    "/user/settings",
    ApiServerSwitch::serverKey(g_api_server_switch.user,
                               g_api_server_switch.test,
                               g_api_server_switch.overseas)));
```

### 6. 完整文件清单

| # | 文件 | 操作 |
|---|------|:--:|
| 1 | `api/{module}/{module}_api.h` | +ApiPaths 常量 + 结构体声明 + 三函数声明 |
| 2 | `api/{module}/{module}_api.cpp` | +三函数实现 |
| 3 | 调用方 `.cpp` | +BaseClient 调用 或 resolveUrl 替换硬编码 |

## API 服务器开关 (ApiServerSwitch)

### 结构

```cpp
struct ApiServerSwitch {
    bool test = false;      // 全局：Debug 默认 true
    bool overseas = false;  // 全局：false = 国内

    struct ModuleSwitch {
        bool test_override = false;      bool test_set = false;
        bool overseas_override = false;  bool overseas_set = false;
    };
    ModuleSwitch user, userConfig, schemes, firmware, drive, 
                 ad, audition, feedback, userLevel, userDeviceLog, wechatOauth;
};
```

### 使用

```cpp
// 初始化（main.cpp）
#ifdef QT_DEBUG
    g_api_server_switch.test = true;
#endif

// 获取模块对应的服务器 key
QString t_key = ApiServerSwitch::serverKey(
    g_api_server_switch.userConfig,
    g_api_server_switch.test,
    g_api_server_switch.overseas);
// → "domestic" / "domestic-t" / "overseas" / "overseas-t"
```