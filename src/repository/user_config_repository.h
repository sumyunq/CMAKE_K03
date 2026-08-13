#ifndef USER_CONFIG_REPOSITORY_H
#define USER_CONFIG_REPOSITORY_H

#include <QPixmap>
#include <QSet>
#include <functional>

#include "data/userConfig/user_config_api.h"
#include "repository/paginated_repository.h"

class AvatarCache;
class DownloadRepository;

/// \brief 用户配置仓库 / Repository for user-config (cloud configuration) endpoints.
///
/// Covers public browsing, personal library, interactions, comments, and admin.
class UserConfigRepository : public PaginatedRepository {
  Q_OBJECT
public:
  explicit UserConfigRepository(QObject* parent = nullptr);

  // ---- User-facing: browsing ----
  /// \brief 获取公开配置列表 / Get public config list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param sort 排序方式 / Sort mode
  /// \param filters 过滤条件 / Optional filter parameters
  void getPublicConfigs(int page, int pageSize, const QString& sort,
                        const QMap<QString, QString>& filters = {});
  /// \brief 回调版（每请求独立回调，多 Tab 不互串）
  /// \param cb 成功回调（列表数据）
  /// \param errCb 失败回调（网络/业务错误原因；传空则 fallback 到 errorOccurred 信号）
  void getPublicConfigs(int page, int pageSize, const QString& sort,
                        const QMap<QString, QString>& filters,
                        std::function<void(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>&,
                                           const PaginatedResult&)> cb,
                        std::function<void(const QString&)> errCb = {});
  /// \brief 获取配置详情 / Get config detail
  /// \param id 配置 ID / Config ID
  void getConfigDetail(const QString& id);
  /// \brief 获取配置详情（回调版 — 避免 connectOnce 信号共享竞态：多个并发请求时响应互吃丢失）
  void getConfigDetail(const QString& id,
                       std::function<void(const DeSheng::GetPublicConfigurationListResponse::ListItem&)> cb);
  /// \brief 下载配置 / Download a config
  /// \param id 配置 ID / Config ID
  void downloadConfig(const QString& id);
  /// \brief 下载配置文件到本地 / Download config file to local temp dir
  /// \param url 文件下载地址 / File download URL
  /// \param configId 配置 ID / Config ID for callback
  void downloadConfigFile(const QString& url, int configId);
  /// \brief 通过分享码下载配置 / Download config by share code
  /// \param code 分享码 / Share code string
  void downloadByShareCode(const QString& code);

  // ---- User-facing: CRUD ----
  /// \brief 创建配置 / Create a config
  /// \param req 创建请求 / Create request payload
  void createConfig(const DeSheng::UserConfigsCreateRequest& req);
  /// \brief 更新配置 / Update a config
  /// \param id 配置 ID / Config ID
  /// \param req 更新请求 / Update request payload
  void updateConfig(const QString& id, const QJsonObject& req);
  /// \brief 删除配置 / Delete a config
  /// \param id 配置 ID / Config ID
  void deleteConfig(const QString& id);

  // ---- User-facing: personal library ----
  /// \brief 获取我的配置列表 / Get my config list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void getMyConfigs(int page, int pageSize, const QMap<QString, QString>& filters = {});
  /// \brief 获取指定用户的配置列表 / Get a specific user's config list
  /// \param userId 用户 ID / User ID
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void getUserConfigs(const QString& userId, int page, int pageSize,
                      const QMap<QString, QString>& filters = {});

  // ---- User-facing: interactions ----
  /// \brief 分享配置 / Share a config
  /// \param id 配置 ID / Config ID
  void shareConfig(const QString& id);
  /// \brief 点赞 / Like a config
  /// \param id 配置 ID / Config ID
  void like(const QString& id);
  /// \brief 取消点赞 / Unlike (undo like) a config
  /// \param id 配置 ID / Config ID
  void unlike(const QString& id);
  /// \brief 点踩 / Dislike a config
  /// \param id 配置 ID / Config ID
  void dislike(const QString& id);
  /// \brief 取消点踩 / Undislike (undo dislike) a config
  /// \param id 配置 ID / Config ID
  void undislike(const QString& id);
  /// \brief 收藏 / Collect a config
  /// \param id 配置 ID / Config ID
  void collect(const QString& id);
  /// \brief 取消收藏 / Uncollect a config
  /// \param id 配置 ID / Config ID
  void uncollect(const QString& id);
  /// \brief 置顶 / Pin a config
  /// \param id 配置 ID / Config ID
  void pin(const QString& id);
  /// \brief 取消置顶 / Unpin a config
  /// \param id 配置 ID / Config ID
  void unpin(const QString& id);

  // ---- User-facing: collections & counts ----
  /// \brief 获取我的收藏列表 / Get my collection list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void getMyCollects(int page, int pageSize, const QMap<QString, QString>& filters = {});
  /// \brief 获取我的点赞列表 / Get my like list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void getMyLikes(int page, int pageSize, const QMap<QString, QString>& filters = {});
  /// \brief 获取当日配置数量 / Get today's config count
  /// \param deviceName 设备名称 / Device name
  void getTodayCount(const QString& deviceName);
  /// \brief 获取置顶配置数量 / Get pinned config count
  /// \param deviceType 设备类型 / Device type
  void getPinnedCount(const QString& deviceType);

  // ---- File upload ----
  /// \brief 上传文件 / Upload file (multipart/form-data)
  /// \param filePath 本地文件路径 / Local file path
  void uploadUserFile(const QString& filePath);

  // ---- User-facing: comments ----
  /// \brief 获取配置的评论列表 / Get comments for a config
  /// \param configId 配置 ID / Config ID
  void getComments(const QString& configId);
  /// \brief 点赞评论 / Like a comment
  /// \param configId 配置 ID / Config ID
  /// \param commentId 评论 ID / Comment ID
  void clickComment(const QString& configId, const QString& commentId);
  /// \brief 取消点赞评论 / Unlike a comment
  void unclickComment(const QString& configId, const QString& commentId);
  /// \brief 下载头像 / Fetch avatar pixmap from URL
  /// \param userId 用户 ID（配置 ID，用于信号回传）/ config/user id for callback
  /// \param avatarUrl 头像图片 URL / avatar image URL
  void fetchAvatar(int userId, const QString& avatarUrl);

  // ---- Admin ----
  /// \brief 管理员获取配置列表 / Admin: get config list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void adminGetConfigs(int page, int pageSize, const QMap<QString, QString>& filters = {});
  /// \brief 管理员获取配置详情 / Admin: get config detail
  /// \param id 配置 ID / Config ID
  void adminGetConfigDetail(const QString& id);
  /// \brief 管理员更新配置状态 / Admin: update config status
  /// \param id 配置 ID / Config ID
  /// \param status 新状态 / New status string
  void adminUpdateStatus(const QString& id, const QString& status);
  /// \brief 管理员删除配置 / Admin: delete a config
  /// \param id 配置 ID / Config ID
  void adminDeleteConfig(const QString& id);
  /// \brief 管理员设置标签 / Admin: set official/expert tags
  /// \param id 配置 ID / Config ID
  /// \param isOfficial 是否为官方方案 / Whether the config is official
  /// \param isExpert 是否为专家方案 / Whether the config is expert
  void adminSetTags(const QString& id, bool isOfficial, bool isExpert);

  /// \brief 管理员获取评论列表 / Admin: get comment list
  /// \param page 页码 / Page number
  /// \param pageSize 每页大小 / Items per page
  /// \param filters 过滤条件 / Optional filter parameters
  void adminGetComments(int page, int pageSize, const QMap<QString, QString>& filters = {});
  /// \brief 管理员创建评论 / Admin: create a comment
  /// \param req 创建请求 / Create request payload
  void adminCreateComment(const QJsonObject& req);
  /// \brief 管理员更新评论 / Admin: update a comment
  /// \param id 评论 ID / Comment ID
  /// \param text 评论文本 / Comment text
  void adminUpdateComment(const QString& id, const QString& text);
  /// \brief 管理员删除评论 / Admin: delete a comment
  /// \param id 评论 ID / Comment ID
  void adminDeleteComment(const QString& id);
  /// \brief 管理员更新评论状态 / Admin: update comment status
  /// \param id 评论 ID / Comment ID
  /// \param status 新状态 / New status string
  void adminUpdateCommentStatus(const QString& id, const QString& status);

signals:
  // User-facing: browsing
  /// \brief 公开配置列表就绪 / Emitted when public config list is ready
  void publicConfigsReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, const PaginatedResult& page);
  /// \brief 配置详情就绪 / Emitted when config detail is ready
  void configDetailReady(const DeSheng::GetPublicConfigurationListResponse::ListItem& info);
  /// \brief 配置下载链接就绪 / Emitted when config download URL is ready
  void configDownloadUrlReady(const QString& configUrl);

  // User-facing: CRUD
  /// \brief 配置创建完成 / Emitted when a config is created
  void configCreated(const DeSheng::GetPublicConfigurationListResponse::ListItem& info);
  /// \brief 配置更新完成 / Emitted when a config is updated
  void configUpdated(const DeSheng::GetPublicConfigurationListResponse::ListItem& info);
  /// \brief 配置删除完成 / Emitted when a config is deleted
  void configDeleted();

  // User-facing: personal library
  /// \brief 我的配置列表就绪 / Emitted when my config list is ready
  void myConfigsReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, const PaginatedResult& page);
  /// \brief 指定用户配置列表就绪 / Emitted when a user's config list is ready
  void userConfigsReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, const PaginatedResult& page);

  // User-facing: interactions
  /// \brief 分享完成 / Emitted when a config is shared
  void configShared(const QString& shareCode, int shareCount);
  /// \brief 点赞完成 / Emitted when a config is liked
  void configLiked();
  /// \brief 取消点赞完成 / Emitted when a config is unliked
  void configUnliked();
  /// \brief 点踩完成 / Emitted when a config is disliked
  void configDisliked();
  /// \brief 取消点踩完成 / Emitted when a config is undisliked
  void configUndisliked();
  /// \brief 收藏完成 / Emitted when a config is collected
  void configCollected();
  /// \brief 取消收藏完成 / Emitted when a config is uncollected
  void configUncollected();
  /// \brief 置顶完成 / Emitted when a config is pinned
  /// \param id 配置 ID / Config ID
  void configPinned(const QString& id);
  /// \brief 取消置顶完成 / Emitted when a config is unpinned
  /// \param id 配置 ID / Config ID
  void configUnpinned(const QString& id);

  // User-facing: collections & counts
  /// \brief 我的收藏列表就绪 / Emitted when my collection list is ready
  void myCollectsReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, const PaginatedResult& page);
  /// \brief 我的点赞列表就绪 / Emitted when my like list is ready
  void myLikesReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, const PaginatedResult& page);
  /// \brief 当日配置数量就绪 / Emitted when today's config count is ready
  void todayCountReady(int count);
  /// \brief 置顶配置数量就绪 / Emitted when pinned config count is ready
  void pinnedCountReady(int count, int limit);

  // User-facing: comments
  /// \brief 评论列表就绪 / Emitted when the comment list is ready
  void commentsReady(const QList<DeSheng::GetPublicConfigurationListResponse::Comment>& list);
  /// \brief 文件下载进度 / File download progress 0-100
  void downloadProgress(int configId, int percent);
  /// \brief 文件下载完成 / File download saved to temp dir
  void downloadFileSaved(int configId, const QString& filePath);
  /// \brief 文件上传完成 / File upload complete
  void fileUploaded(const QString& fileUrl);
  /// \brief 头像下载完成 / Avatar pixmap ready
  void avatarReady(int userId, const QPixmap& pixmap);
  /// \brief 评论点赞完成 / Emitted when a comment is liked
  void commentClicked();
  /// \brief 取消评论点赞完成 / Emitted when a comment like is undone
  void commentUnclicked();
  /// \brief 评论标签点击失败（带参数精确关联，供乐观更新回滚）
  void commentClickFailed(int configId, int commentId);

  // Admin
  /// \brief 管理员配置列表就绪 / Emitted when admin config list is ready
  void adminConfigsReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, const PaginatedResult& page);
  /// \brief 管理员配置详情就绪 / Emitted when admin config detail is ready
  void adminConfigDetailReady(const DeSheng::GetPublicConfigurationListResponse::ListItem& info);
  /// \brief 管理员配置状态更新完成 / Emitted when admin config status is updated
  void adminConfigStatusUpdated();
  /// \brief 管理员配置删除完成 / Emitted when admin deletes a config
  void adminConfigDeleted();
  /// \brief 管理员标签设置完成 / Emitted when admin sets official/expert tags
  void adminTagsSet();

  /// \brief 管理员评论列表就绪 / Emitted when admin comment list is ready
  void adminCommentsReady(const QList<DeSheng::GetPublicConfigurationListResponse::Comment>& list, const PaginatedResult& page);
  /// \brief 管理员评论创建完成 / Emitted when admin creates a comment
  void adminCommentCreated(const DeSheng::GetPublicConfigurationListResponse::Comment& item);
  /// \brief 管理员评论更新完成 / Emitted when admin updates a comment
  void adminCommentUpdated();
  /// \brief 管理员评论删除完成 / Emitted when admin deletes a comment
  void adminCommentDeleted();
  /// \brief 管理员评论状态更新完成 / Emitted when admin comment status is updated
  void adminCommentStatusUpdated();

private:
  AvatarCache* clp_avatar_cache_ = nullptr;
  DownloadRepository* clp_download_repo_ = nullptr;
  QSet<QString> cl_inflight_my_configs_;
};

#endif  // USER_CONFIG_REPOSITORY_H
