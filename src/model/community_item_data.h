#ifndef COMMUNITY_ITEM_DATA_H
#define COMMUNITY_ITEM_DATA_H

#include <QPixmap>
#include <QString>

#include "data/userConfig/user_config_api.h"  ///< Comment + ListItem（DTO 唯一来源）

/// \brief 社区卡片数据 — K03 全字段 / Full K03 card data
struct CommunityItemData {
  // ── widget_01: 头像+昵称+等级+更多 ──
  QPixmap avatar;         ///< Avatar 40×40 / 头像
  int userId = -1;        ///< Config ID (API int) / 配置 ID
  int authorUserId = -1;  ///< 作者用户 ID（author.user_id；头像弹窗/用户主页用——注意 userId 语义为配置 ID，勿混用）
  QString name;           ///< Username / 用户名
  QString nickname;       ///< Display nickname / 展示昵称
  int authorLevel = 0;    ///< Author level / 作者等级
  bool isOfficial = false;///< Official tag / 官方标签
  bool isExpert = false;  ///< Expert tag / 专家标签（大神徽章）
  bool isStreamer = false;     ///< 从 author.roles 派生
  bool isProfessional = false; ///< 从 author.roles 派生
  // ── widget_03: 方案信息 ──
  QString planName;       ///< Plan title / 方案标题
  QString description;    ///< Plan description / 方案描述
  QStringList tags;       ///< User tags / 用户标签
  QString deviceName;     ///< Device name / 设备名称
  QString deviceType;     ///< Device type (mouse/keyboard/headset) / 设备类型
  QString language;       ///< Language (zh/en) / 语言
  // ── widget_04: 操作栏 ──
  int likeCount = 0;      ///< Like count / 点赞数
  int dislikeCount = 0;   ///< Dislike count / 踩数
  int downloadCount = 0;  ///< Download count / 下载数
  int shareCount = 0;     ///< Share count / 分享数
  int collectCount = 0;   ///< Collection count / 收藏数
  int hotScore = 0;       ///< Hot score / 热度
  bool isLiked = false;   ///< Current user liked / 当前用户已点赞
  bool isDisliked = false;///< Current user disliked / 当前用户已踩
  bool isCollected = false;///< Current user collected / 当前用户已收藏
  // ── widget_02: 评论区 ──
  bool expanded = false;           ///< Expand state / 展开状态
  QList<DeSheng::GetPublicConfigurationListResponse::Comment> comments; ///< Comment tag list / 快捷评论标签列表
  // ── 版本 ──
  QString driveVersion;   ///< Drive version / 驱动版本
  QString firmwareVersion;///< Firmware version / 固件版本
  // ── 元数据(暂不绘制,预留) ──
  QString status;         ///< active/rejected / 状态
  QString visibility;     ///< public/private / 可见性
  bool isPinned = false;  ///< Pinned / 置顶
  QString createdAt;      ///< Create time / 创建时间
};

/// \brief ListItem(DTO) → CommunityItemData 映射 / API DTO to UI entity
QList<CommunityItemData> configToItems(
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list);

#endif  // COMMUNITY_ITEM_DATA_H
