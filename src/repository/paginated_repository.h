#ifndef PAGINATED_REPOSITORY_H
#define PAGINATED_REPOSITORY_H

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUrlQuery>

/// \brief 共享分页结果 / Shared pagination result.
///
/// All list endpoints return {list, total, page, page_size}
struct PaginatedResult {
  QJsonArray items;   ///< 分页数据项 / Paginated data items
  int page = 1;       ///< 当前页码 / Current page number
  int pageSize = 20;  ///< 每页大小 / Items per page
  int total = 0;      ///< 总条目数 / Total item count

  /// \brief 是否还有下一页 / Whether more pages are available
  bool hasMore() const { return page * pageSize < total; }
};

/// \brief 分页仓库基类 / Base class providing shared pagination helpers for all list repositories.
class PaginatedRepository : public QObject {
  Q_OBJECT
public:
  explicit PaginatedRepository(QObject* parent = nullptr) : QObject(parent) {}

protected:
  /// \brief 解析分页 JSON 响应 / Parse paginated response JSON into PaginatedResult.
  /// \param response API 返回的 JSON 对象 / JSON object from API response
  /// \return 解析后的分页结果 / Parsed paginated result
  ///
  /// Handles both {"list": [...], "total": N, "page": P, "page_size": S}
  /// and {"items": [...], "total": N, "page": P, "pageSize": S}.
  static PaginatedResult parsePaginated(const QJsonObject& response);

  /// \brief 构建带标准分页键的查询 / Build a QUrlQuery with standard pagination keys plus optional
  /// extras.
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param extra 额外查询参数 / Extra query parameters
  /// \return 构建好的 QUrlQuery / Assembled QUrlQuery
  static QUrlQuery buildPageQuery(int page, int pageSize, const QMap<QString, QString>& extra = {});

signals:
  /// \brief 请求失败时触发 / Emitted when any request fails
  /// \param error 错误描述 / Error description string
  void errorOccurred(const QString& error);
};

#endif  // PAGINATED_REPOSITORY_H
