#ifndef SCHEME_REPOSITORY_H
#define SCHEME_REPOSITORY_H

#include "data/schemes/schemes_api.h"
#include "repository/paginated_repository.h"

/// \brief 方案 / 分享码仓库 / Repository for scheme / share-code endpoints.
class SchemeRepository : public PaginatedRepository {
  Q_OBJECT
public:
  explicit SchemeRepository(QObject* parent = nullptr);

  /// \brief 创建分享码 / Create a share code
  /// \param req 创建请求 / Create request payload
  void createShareCode(const DeSheng::CreateShareCodeRequest& req);
  /// \brief 解析分享码 / Resolve a share code
  /// \param code 分享码字符串 / Share code string
  void resolveShareCode(const QString& code);
  /// \brief 更新方案 / Update a scheme
  /// \param id 方案 ID / Scheme ID
  /// \param req 更新请求 / Update request payload
  void updateScheme(const QString& id, const QJsonObject& req);
  /// \brief 管理员获取方案列表 / Admin: get scheme list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void adminGetSchemes(int page, int pageSize, const QMap<QString, QString>& filters = {});
  /// \brief 管理员更新方案 / Admin: update a scheme
  /// \param id 方案 ID / Scheme ID
  /// \param req 更新请求 / Update request payload
  void adminUpdateScheme(const QString& id, const QJsonObject& req);
  /// \brief 管理员删除方案 / Admin: delete a scheme
  /// \param id 方案 ID / Scheme ID
  void adminDeleteScheme(const QString& id);

signals:
  /// \brief 分享码创建完成 / Emitted when a share code is created
  void shareCodeCreated(const DeSheng::CreateShareCodeResponse::ReturnData& info);
  /// \brief 分享码解析完成 / Emitted when a share code is resolved
  void shareCodeResolved(const DeSheng::CreateShareCodeResponse::ReturnData& info);
  /// \brief 方案更新完成 / Emitted when a scheme is updated
  void schemeUpdated();
  /// \brief 管理员方案列表就绪 / Emitted when admin scheme list is ready
  void adminSchemesReady(const QList<DeSheng::CreateShareCodeResponse::ReturnData>& list,
                         const PaginatedResult& page);
  /// \brief 管理员方案更新完成 / Emitted when admin scheme update completes
  void adminSchemeUpdated();
  /// \brief 管理员方案删除完成 / Emitted when admin scheme delete completes
  void adminSchemeDeleted();
};

#endif  // SCHEME_REPOSITORY_H
