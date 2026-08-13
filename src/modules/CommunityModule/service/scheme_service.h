#ifndef SERVICE_SCHEME_SERVICE_H
#define SERVICE_SCHEME_SERVICE_H

#include <QList>
#include <QObject>

#include "data/userConfig/user_config_api.h"  ///< ListItem（原 entity/user_config.h 的 UserConfigInfo）

class UserConfigRepository;
struct PaginatedResult;

/// \brief 方案广场 / 大神分享 / 官方预设 三合一 service
///
/// Wraps UserConfigRepository to provide three curated browsing tabs:
/// - 方案广场 (public configs, sorted by new, no tag filter)
/// - 大神分享 (expert-tagged configs, sorted by hot)
/// - 官方预设 (official-tagged configs, sorted by hot)
///
/// Action methods (like, collect, download, share, comment) use
/// optimistic-update + async-confirm + rollback-on-failure.
class SchemeService : public QObject {
  Q_OBJECT
public:
  /// \brief 构造 SchemeService / Construct SchemeService
  explicit SchemeService(QObject* parent = nullptr);

  /// \brief Inject the user-config repository dependency
  void init(UserConfigRepository* repo);

  /// \brief Tab 1: 方案广场 — public configs, default sort=new, no tag filter
  /// \param page 页码 / page number
  /// \param sort 排序方式 "new"/"hot" / sort mode
  /// \param deviceType 设备类型过滤 / device type filter
  /// \param deviceName 设备名称过滤 / device name filter
  /// \param keyword 关键词搜索 / keyword search
  void fetchSquare(int page = 1, const QString& sort = QStringLiteral("new"),
                   const QString& deviceType = {}, const QString& deviceName = {},
                   const QString& keyword = {}, const QString& userTag = {});

  /// \brief Tab 2: 大神分享 — is_expert_tag=true，排序跟随筛选弹窗
  /// \param page 页码 / page number
  /// \param sort 排序方式（筛选弹窗取值，默认 hot 保持原行为）
  /// \param deviceType 设备类型过滤 / device type filter
  /// \param deviceName 设备名称过滤 / device name filter
  /// \param userTag 场景标签过滤 / user tag filter
  /// \param keyword 关键词搜索 / keyword search
  void fetchExpert(int page = 1, const QString& sort = QStringLiteral("hot"),
                   const QString& deviceType = {}, const QString& deviceName = {},
                   const QString& userTag = {}, const QString& keyword = {});

  /// \brief Tab 3: 官方预设 — is_official_tag=true，排序跟随筛选弹窗
  /// \param page 页码 / page number
  /// \param sort 排序方式（筛选弹窗取值，默认 hot 保持原行为）
  /// \param deviceType 设备类型过滤 / device type filter
  /// \param deviceName 设备名称过滤 / device name filter
  /// \param userTag 场景标签过滤 / user tag filter
  /// \param keyword 关键词搜索 / keyword search
  void fetchOfficial(int page = 1, const QString& sort = QStringLiteral("hot"),
                     const QString& deviceType = {}, const QString& deviceName = {},
                     const QString& userTag = {}, const QString& keyword = {});

  /// \brief Toggle like state with optimistic UI update
  void toggleLike(int configId, bool liked);
  /// \brief Toggle dislike state — POST/DELETE /api/v1/user-configs/:id/dislike
  void toggleDislike(int configId, bool disliked);
  /// \brief K03 refreshCounts — API 成功后拉取详情纠正乐观更新计数
  void refreshCounts(int configId);
  /// \brief Toggle collect state with optimistic UI update
  void toggleCollect(int configId, bool collected);

  /// \brief Request download URL for a config
  /// \param configId 方案配置 ID / config id
  void download(int configId);

  /// \brief Request share code for a config
  /// \param configId 方案配置 ID / config id
  void share(int configId);

  /// \brief Click a predefined comment on a config
  /// \param configId 方案配置 ID / config id
  /// \param commentId 预设评论 ID / predefined comment id
  void clickComment(int configId, int commentId);
  /// \brief Toggle comment click — POST or DELETE /api/v1/user-configs/:id/comments
  /// \param configId 方案配置 ID
  /// \param commentId 预设评论 ID
  /// \param clicked true=点击, false=取消点击
  void toggleCommentClick(int configId, int commentId, bool clicked);

signals:
  /// \brief 方案广场数据加载完成 / Square tab data loaded
  void squareDataReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& items,
                       const PaginatedResult& page);
  /// \brief 大神分享数据加载完成 / Expert tab data loaded
  void expertDataReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& items,
                       const PaginatedResult& page);
  /// \brief 官方预设数据加载完成 / Official tab data loaded
  void officialDataReady(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& items,
                         const PaginatedResult& page);
  /// \brief 点赞状态切换确认 / Like state confirmed after toggle
  void likeToggled(int configId, bool newState);
  /// \brief 踩状态切换确认 / Dislike state confirmed after toggle
  void dislikeToggled(int configId, bool newState);
  /// \brief 收藏状态切换确认 / Collect state confirmed after toggle
  void collectToggled(int configId, bool newState);
  /// \brief 下载链接准备就绪 / Download URL ready
  void downloadUrlReady(const QString& url);
  /// \brief 分享码生成完毕 / Share code generated
  void shareCodeReady(const QString& shareCode, int shareCount);
  /// \brief 文件下载进度 0~100 / File download progress
  void downloadProgress(int configId, int percent);
  /// \brief 文件已保存到临时目录 / File saved to temp dir
  void downloadFileSaved(int configId, const QString& filePath);
  /// \brief 下载失败（带 configId，供精准处理，如服务端已删除配置） / Download failed
  void downloadFailed(int configId, const QString& reason);
  /// \brief 服务端真实计数同步 / Server counts synced after refreshCounts
  void countsSynced(const DeSheng::GetPublicConfigurationListResponse::ListItem& info);
  /// \brief 评论标签点击失败回滚（UI 乐观更新需翻转恢复）
  void commentClickReverted(int configId, int commentId);
  /// \brief 任意操作发生错误 / An error occurred during an action
  void errorOccurred(const QString& action, const QString& reason);

private:
  static constexpr int kDefaultPageSize = 50;  ///< default page size for list queries

  /// \brief Build filter map from device type / name / keyword / user tag
  static QMap<QString, QString> buildFilters(const QString& deviceType, const QString& deviceName,
                                             const QString& keyword, const QString& userTag = {});

  UserConfigRepository* clp_repo_ = nullptr;  ///< injected user-config repository
  bool cl_like_ok_ = false;  ///< gate: like/unlike API succeeded → suppress error-rollback during refreshCounts
  int cl_square_fetch_seq_ = 0;
  int cl_expert_fetch_seq_ = 0;
  int cl_official_fetch_seq_ = 0;
};

#endif  // SERVICE_SCHEME_SERVICE_H
