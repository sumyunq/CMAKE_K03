#ifndef UI_RANKING_RANKING_MODEL_H
#define UI_RANKING_RANKING_MODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QPixmap>
#include <QSet>
#include <optional>

#include "data/userConfig/user_config_api.h"  ///< GetPublicConfigurationListResponse::ListItem

/// \brief 排行榜数据模型 — 行 = ListItem，排行号 = 行号 + 1
class RankingModel : public QAbstractListModel {
  Q_OBJECT

 public:
  /// \brief 数据角色 / Data roles
  enum class RankingRole {
    RankRole = Qt::UserRole + 1,  ///< 排名 / rank (int)
    PlanNameRole,                 ///< 方案名 / plan title (QString)
    UserNameRole,                 ///< 展示用户名 / display user name (QString)
    HeatCountRole,                ///< 热度值 / heat count (int)
    AvatarRole,                   ///< 头像 / avatar (QPixmap)
    PlanIdRole,                   ///< 配置 ID / config id (int)
    IsLikedRole,                  ///< 当前用户已点赞 / liked by current user (bool)
    DownloadingRole,              ///< 下载进行中 / download in progress (bool)
    DownloadProgressRole,         ///< 下载进度 0-100 / download progress (int)
  };

  explicit RankingModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  /// \brief 全量替换数据（beginResetModel/endResetModel）并清空头像缓存
  void replaceAll(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &items);
  /// \brief 清空数据 / Clear all rows
  void clear();
  /// \brief 设置某行头像（发 dataChanged） / Set avatar for a row
  void setAvatar(int row, const QPixmap &pm);
  /// \brief 设置某行下载状态（发 dataChanged） / Set download-in-progress state
  void setDownloading(int configId, bool downloading);
  /// \brief 设置某行下载进度 0-100（发 dataChanged） / Set download progress
  void setDownloadProgress(int configId, int percent);
  /// \brief 设置热度取值字段：true=like_count（点赞榜） false=download_count（下载榜）
  void setHeatField(bool isLike);
  /// \brief 用服务器真实数据覆盖某行（countsSynced 回调后）
  void applyServerItem(int row, const DeSheng::GetPublicConfigurationListResponse::ListItem &item);
  /// \brief 按配置 ID 查行号，找不到返回 -1 / Find row by config id
  int rowOfConfigId(int configId) const;
  /// \brief 按配置 ID 移除行（下载失败"配置不存在"等场景；头像缓存行号左移） / Remove row by config id
  void removeByConfigId(int configId);
  /// \brief 取某行完整数据（方案卡片填充用） / Full item for plan card
  std::optional<DeSheng::GetPublicConfigurationListResponse::ListItem> itemAt(int row) const;

 private:
  QList<DeSheng::GetPublicConfigurationListResponse::ListItem> cl_items_;  ///< 数据行
  QHash<int, QPixmap> cl_avatars_;  ///< 行号 → 头像 pixmap
  QSet<int> cl_downloading_ids_;  ///< 下载进行中的配置 ID
  QHash<int, int> cl_download_progress_;  ///< 配置 ID → 下载进度 0-100
  bool cl_heat_is_like_ = true;     ///< 热度字段：true=like_count false=download_count
};

#endif  // UI_RANKING_RANKING_MODEL_H
