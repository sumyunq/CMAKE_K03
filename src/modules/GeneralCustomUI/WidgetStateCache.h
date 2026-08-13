#ifndef WIDGET_STATE_CACHE_H
#define WIDGET_STATE_CACHE_H

#include <QHash>

///
/// \brief Widget 展示状态（独立于 widget 生命周期，widget 回收后状态仍保留）
struct WidgetDisplayState
{
    bool is_expanded = false; ///< 评论区是否展开
};

///
/// \brief Widget 状态缓存，key = config_id
/// widget bind 时 restore，unbind 时 save，widget 销毁/回收后状态不丢失
using WidgetStateCache = QHash<int, WidgetDisplayState>;

#endif // WIDGET_STATE_CACHE_H
