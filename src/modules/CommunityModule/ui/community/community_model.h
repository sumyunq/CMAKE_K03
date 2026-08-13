#ifndef COMMUNITY_MODEL_H
#define COMMUNITY_MODEL_H

#include <QAbstractListModel>

#include <optional>

#include "model/community_item_data.h"
#include "data/userConfig/user_config_api.h"  ///< Comment 元类型注册（原 entity/user_config.h 已删）

// QVariant 需在编译期注册 Comment/QList<Comment>（CommentsRole 跨 model 传输用）
Q_DECLARE_METATYPE(DeSheng::GetPublicConfigurationListResponse::Comment)
Q_DECLARE_METATYPE(QList<DeSheng::GetPublicConfigurationListResponse::Comment>)

/// \brief 社区数据模型 — K03 全字段 / Community list model with full K03 fields
class CommunityModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum CommunityRole {
    // widget_01: 头像+昵称+等级+更多
    AvatarRole = Qt::UserRole + 1,   ///< 头像 / avatar (QPixmap)
    UserIdRole,                      ///< 配置 ID / config id (int)
    NameRole,                        ///< 用户名 / username (QString)
    NicknameRole,                    ///< 展示昵称 / display nickname (QString)
    AuthorLevelRole,                 ///< 作者等级 / author level (int)
    IsOfficialRole,                  ///< 官方标签 / official tag (bool)
    IsExpertRole,                    ///< 专家标签 / expert tag (bool)
    IsStreamerRole,                  ///< 主播标签 / streamer tag (bool)
    IsProfessionalRole,              ///< 职业标签 / professional tag (bool)
    // widget_03: 方案信息
    PlanNameRole,                    ///< 方案标题 / plan title (QString)
    DescriptionRole,                 ///< 方案描述 / plan description (QString)
    TagsRole,                        ///< 用户标签 / user tags (QStringList)
    DeviceNameRole,                  ///< 设备名称 / device name (QString)
    DeviceTypeRole,                  ///< 设备类型 / device type (QString)
    LanguageRole,                    ///< 语言 / language (QString)
    // widget_04: 操作栏
    LikeCountRole,                   ///< 点赞数 / like count (int)
    DislikeCountRole,                ///< 踩数 / dislike count (int)
    DownloadCountRole,               ///< 下载数 / download count (int)
    ShareCountRole,                  ///< 分享数 / share count (int)
    CollectCountRole,                ///< 收藏数 / collect count (int)
    HotScoreRole,                    ///< 热度 / hot score (int)
    IsLikedRole,                     ///< 当前用户已点赞 / liked by current (bool)
    IsDislikedRole,                  ///< 当前用户已踩 / disliked by current (bool)
    IsCollectedRole,                 ///< 当前用户已收藏 / collected by current (bool)
    // widget_02: 评论区
    ExpandedRole,                    ///< 展开状态 / expand state (bool)
    CommentsRole,                    ///< 评论标签列表 / comments list (QList<DeSheng::GetPublicConfigurationListResponse::Comment>)
    // 版本
    DriveVersionRole,                ///< 驱动版本 / drive version (QString)
    FirmwareVersionRole,             ///< 固件版本 / firmware version (QString)
    // 元数据（预留，暂不绘制）
    StatusRole,                      ///< 状态 / status (QString)
    VisibilityRole,                  ///< 可见性 / visibility (QString)
    IsPinnedRole,                    ///< 置顶 / pinned (bool)
    CreatedAtRole                    ///< 创建时间 / created at (QString)
  };

  explicit CommunityModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;

  void replaceAll(const QList<CommunityItemData>& items);
  void addItem(const CommunityItemData& item);
  void addItems(const QList<CommunityItemData>& items);
  void removeItem(int userId);
  void updateItem(int userId, const CommunityItemData& item);
  void toggleExpanded(int userId);
  void clear();

  void setField(int userId, int role, const QVariant& value);
  void batchSetField(const QList<int>& userIds, int role, const QVariant& value);

  std::optional<CommunityItemData> findById(int userId) const;
  int count() const;
  bool isEmpty() const;

 private:
  int indexOfUser(int userId) const;
  bool applyField(CommunityItemData& item, int role, const QVariant& value);
  QList<CommunityItemData> cl_items_;
};

#endif  // COMMUNITY_MODEL_H
