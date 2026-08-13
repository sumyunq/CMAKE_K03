#include "repository/paginated_repository.h"

#include <QJsonArray>

PaginatedResult PaginatedRepository::parsePaginated(const QJsonObject& response) {
  PaginatedResult result;

  const QJsonObject data = response.value("data").toObject();

  // Handle both "list" and "items" key conventions
  if (data.contains("list")) {
    result.items = data.value("list").toArray();
  } else if (data.contains("items")) {
    result.items = data.value("items").toArray();
  } else {
    result.items = data.value("data").toArray();
  }

  result.page = data.value("page").toInt(1);
  result.total = data.value("total").toInt(0);

  // Handle both "page_size" and "pageSize"
  if (data.contains("page_size")) {
    result.pageSize = data.value("page_size").toInt(20);
  } else if (data.contains("pageSize")) {
    result.pageSize = data.value("pageSize").toInt(20);
  } else {
    result.pageSize = 20;
  }

  return result;
}

QUrlQuery PaginatedRepository::buildPageQuery(int page, int pageSize,
                                              const QMap<QString, QString>& extra) {
  QUrlQuery query;
  query.addQueryItem("page", QString::number(page));
  query.addQueryItem("page_size", QString::number(pageSize));

  for (auto it = extra.cbegin(); it != extra.cend(); ++it) {
    query.addQueryItem(it.key(), it.value());
  }

  return query;
}
