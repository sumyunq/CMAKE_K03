# SSL — 证书管理

## 概述

确保应用程序在系统证书存储损坏的 Windows 机器上仍能正常发起 HTTPS 请求。

## 文件

| 文件 | 职责 |
|------|------|
| `SslCertManager.h/cpp` | SSL/TLS 证书探测、本地 PEM 加载、cacert.pem 自动下载 |

## SslCertManager

### API

| 方法 | 说明 |
|------|------|
| `SslCertManager(QObject*)` | 构造函数 |
| `initialize(certFileName)` | 三阶段初始化，**同步阻塞**（探测 5s + 下载每次 15s，HTTPS→HTTP 降级最长 30s）。返回 `true` = SSL 可用。 |

### 初始化流程

```
SslCertManager::initialize(certFileName)
  ├─ 1. probeSystemCertificates()
  │     → GET https://www.google.com（同步 QEventLoop，5s 超时）
  │     → 成功：系统证书可用，直接返回 true
  │     → 失败：进入阶段 2
  │
  ├─ 2. checkAndUpdateCertBundle(certPath)
  │     → 检查本地 PEM 文件年龄（30 天阈值）
  │     → 过期/不存在：从 curl.haxx.se/ca/cacert.pem 下载
  │       （HTTPS 先试，失败 fallback HTTP）
  │
  └─ 3. loadLocalCertificates(certPath)
        → 加载 PEM → QSslConfiguration::defaultConfiguration() 的 CA 证书
```

### 使用方法

在 `MainWindow::LoginAndInit()` 中调用：
```cpp
#include "SSL/SslCertManager.h"
// ...
SslCertManager mgr(this);
if (mgr.initialize("cacert.pem")) {
    // SSL 可用，继续网络初始化
}
```

### 依赖

- `QNetworkAccessManager`（HTTPS 探测 + 下载）
- `QSslConfiguration`（设置 CA 证书）
- `QFile` / `QFileInfo`（本地 PEM 读写 + 年龄检测）
