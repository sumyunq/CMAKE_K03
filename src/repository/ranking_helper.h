#ifndef RANKING_HELPER_H
#define RANKING_HELPER_H

#include <functional>
#include "data/userConfig/user_config_api.h"

class QObject;

/// \brief 排行榜数据获取（点赞榜/下载榜 × 月度/总榜，仅耳机设备）
///
/// 统一封装：GET /user-configs?sort=like|download&device_type=headset&page_size=N
///   （月榜附加 start_time=本月1号 RFC3339）
/// ranking_list（完整榜）与 Community（前三名头像）共用，避免请求/解析重复。
namespace RankingHelper {

/// \brief 异步获取排行榜 TopN
/// \param sort "like"=点赞榜 "download"=下载榜
/// \param limit 条数上限
/// \param monthly true=月度榜（本月创建数据） false=总榜
/// \param ctx 完成回调执行上下文（通常传 this，自动断连）
/// \param cb 结果回调（ok=true 解析成功；失败时 list 为空）
void fetchTop(const QString& sort, int limit, bool monthly, QObject* ctx,
              std::function<void(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>&,
                                 bool ok)> cb);

}  // namespace RankingHelper

#endif  // RANKING_HELPER_H
