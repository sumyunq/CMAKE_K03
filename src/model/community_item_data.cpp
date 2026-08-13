#include "model/community_item_data.h"

#include "modules/Common/DeviceRegistry.h"  ///< shortDisplayName 设备名映射

/// \brief ListItem(DTO) → CommunityItemData 统一映射（社区主页面 / 个人中心 / 上传弹窗共用，避免三处拷贝漂移）
QList<CommunityItemData> configToItems(
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list) {
  QList<CommunityItemData> items;
  items.reserve(list.size());
  for (const auto& c : list) {
    CommunityItemData item;
    item.userId = c.id;
    item.authorUserId = c.author.user_id;  ///< 作者用户 ID（头像弹窗入口用）
    item.name = c.author.username;
    item.nickname = c.author.nickname.isEmpty() ? c.author.username : c.author.nickname;
    item.authorLevel = c.author.level;
    // 徽章判定：主播/官方/职业 = 作者身份（roles），大神 = 方案标签（is_expert_tag，管理端精选）
    item.isOfficial = c.author.roles.contains(DeSheng::kRoleOfficial);
    item.isExpert = c.is_expert_tag;
    item.isStreamer = c.author.roles.contains(DeSheng::kRoleStreamer);
    item.isProfessional = c.author.roles.contains(DeSheng::kRoleProfessional);
    item.planName = c.title;
    item.description = c.description;
    item.tags = c.user_tags;
    item.deviceName = DeSheng::DeviceRegistry::shortDisplayName(c.device_name);
    item.deviceType = c.device_type;
    item.language = c.language;
    item.likeCount = c.like_count;
    item.dislikeCount = c.dislike_count;
    item.downloadCount = c.download_count;
    item.shareCount = c.share_count;
    item.collectCount = c.collect_count;
    item.hotScore = c.hot_score;
    item.isLiked = c.is_liked;
    item.isDisliked = c.is_disliked;
    item.isCollected = c.is_collected;
    item.driveVersion = c.drive_version;
    item.firmwareVersion = c.firmware_version;
    item.status = c.status;
    item.visibility = c.visibility;
    item.isPinned = c.is_pinned;
    item.createdAt = c.created_at;
    item.comments = c.comments;
    items.append(item);
  }
  return items;
}
