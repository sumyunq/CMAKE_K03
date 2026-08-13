# src/repository — 数据访问层

> 2026-08-04 标准目录统一后自 `src/modules/CommunityModule/repository/` 迁入。
> 网络/数据统一方案见 `doc/reports/directory_unification_plan_20260804.md`。

## 定位

业务数据访问层：调用 `src/network/`（HttpClient）发请求，用 `src/data/` 的 DTO + `ProcessXxxResult` 解析，经 `dto_mapper` 转为 `src/model/` 的 UI 展示模型（CommunityItemData）。

## 文件

| 文件 | 职责 |
|------|------|
| `ranking_helper.{h,cpp}` | 排行榜数据获取（点赞/下载 × 月度/总榜，仅耳机；ranking_list 与 Community 前三头像共用） |
| `paginated_repository.{h,cpp}` | 分页基类（parsePaginated + buildPageQuery） |
| `user_config_repository.{h,cpp}` | 用户配置全量 API（浏览/CRUD/互动/评论/钉选/管理端，60+ 方法） |
| `scheme_repository.{h,cpp}` | 分享码仓库（创建/解析/更新 + 管理端） |
| `dto_mapper.{h,cpp}` | DTO → CommunityItemData 转换（待建） |

## 依赖

```
repository → src/network（HttpClient/ServerRouter/AuthStore）
           → src/data（DTO + ProcessXxxResult）
           → src/model（CommunityItemData）
```

## 使用

```cpp
#include "repository/user_config_repository.h"
#include "network/http_client.h"
#include "network/request_options.h"
#include "data/api_global.h"
```
