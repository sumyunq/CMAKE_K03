#include "modules/CommunityModule/ui/community/community_model.h"

#include <QJsonArray>
#include <algorithm>

#include "modules/CommunityModule/infrastructure/logger/logger.h"

CommunityModel::CommunityModel(QObject* parent) : QAbstractListModel(parent) {}

int CommunityModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;
  return cl_items_.size();
}

QVariant CommunityModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= cl_items_.size()) return {};

  const auto& item = cl_items_.at(index.row());

  switch (role) {
    case Qt::DisplayRole:
    case NameRole:          return item.name;
    case NicknameRole:      return item.nickname;
    case AvatarRole:        return QVariant::fromValue(item.avatar);
    case UserIdRole:        return item.userId;
    case AuthorLevelRole:   return item.authorLevel;
    case IsOfficialRole:    return item.isOfficial;
    case IsExpertRole:      return item.isExpert;
    case IsStreamerRole:       return item.isStreamer;
    case IsProfessionalRole:   return item.isProfessional;
    case PlanNameRole:         return item.planName;
    case DescriptionRole:   return item.description;
    case TagsRole:          return item.tags;
    case DeviceNameRole:    return item.deviceName;
    case DeviceTypeRole:    return item.deviceType;
    case LanguageRole:      return item.language;
    case LikeCountRole:     return item.likeCount;
    case DislikeCountRole:  return item.dislikeCount;
    case DownloadCountRole: return item.downloadCount;
    case ShareCountRole:    return item.shareCount;
    case CollectCountRole:  return item.collectCount;
    case HotScoreRole:      return item.hotScore;
    case IsLikedRole:       return item.isLiked;
    case IsDislikedRole:    return item.isDisliked;
    case IsCollectedRole:   return item.isCollected;
    case ExpandedRole:      return item.expanded;
    case CommentsRole:      return QVariant::fromValue(item.comments);
    case DriveVersionRole:  return item.driveVersion;
    case FirmwareVersionRole: return item.firmwareVersion;
    case StatusRole:        return item.status;
    case VisibilityRole:    return item.visibility;
    case IsPinnedRole:      return item.isPinned;
    case CreatedAtRole:     return item.createdAt;
    default:               return {};
  }
}

bool CommunityModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if (!index.isValid() || index.row() < 0 || index.row() >= cl_items_.size()) return false;
  // 委托 applyField — 唯一权威的 role → 字段映射
  if (!applyField(cl_items_[index.row()], role, value)) return false;
  emit dataChanged(index, index, {role});
  return true;
}

void CommunityModel::replaceAll(const QList<CommunityItemData>& items) {
  beginResetModel();
  cl_items_ = items;
  endResetModel();
}

void CommunityModel::addItem(const CommunityItemData& item) {
  const int row = cl_items_.size();
  beginInsertRows(QModelIndex(), row, row);
  cl_items_.append(item);
  endInsertRows();
}

void CommunityModel::addItems(const QList<CommunityItemData>& items) {
  if (items.isEmpty()) return;
  const int first = cl_items_.size();
  const int last = first + items.size() - 1;
  beginInsertRows(QModelIndex(), first, last);
  cl_items_.append(items);
  endInsertRows();
}

void CommunityModel::removeItem(int userId) {
  const int i = indexOfUser(userId);
  if (i < 0) return;
  beginRemoveRows(QModelIndex(), i, i);
  cl_items_.removeAt(i);
  endRemoveRows();
}

void CommunityModel::updateItem(int userId, const CommunityItemData& item) {
  const int i = indexOfUser(userId);
  if (i < 0) return;
  cl_items_[i] = item;
  emit dataChanged(index(i), index(i));
}

void CommunityModel::toggleExpanded(int userId) {
  const auto opt = findById(userId);  // Item 28: 值拷贝，不持有内部指针
  if (!opt.has_value()) return;
  setField(userId, ExpandedRole, !opt->expanded);
}

// 消除重复：switch 逻辑唯一权威来源（Clean Code: 消除重复）
bool CommunityModel::applyField(CommunityItemData& item, int role, const QVariant& value) {
  switch (role) {
    case ExpandedRole:            item.expanded = value.toBool(); break;
    case CommentsRole: {
      // QVariant can carry QList<DeSheng::GetPublicConfigurationListResponse::Comment> via QMetaType registration
      item.comments = value.value<QList<DeSheng::GetPublicConfigurationListResponse::Comment>>();
      break;
    }
    case NameRole:                item.name = value.toString(); break;
    case NicknameRole:            item.nickname = value.toString(); break;
    case AvatarRole:              item.avatar = value.value<QPixmap>(); break;
    case UserIdRole:              item.userId = value.toInt(); break;
    case AuthorLevelRole:         item.authorLevel = value.toInt(); break;
    case IsOfficialRole:          item.isOfficial = value.toBool(); break;
    case IsExpertRole:            item.isExpert = value.toBool(); break;
    case IsStreamerRole:          item.isStreamer = value.toBool(); break;
    case IsProfessionalRole:      item.isProfessional = value.toBool(); break;
    case PlanNameRole:            item.planName = value.toString(); break;
    case DescriptionRole:         item.description = value.toString(); break;
    case TagsRole:                item.tags = value.toStringList(); break;
    case DeviceNameRole:          item.deviceName = value.toString(); break;
    case DeviceTypeRole:          item.deviceType = value.toString(); break;
    case LanguageRole:            item.language = value.toString(); break;
    case LikeCountRole:           item.likeCount = value.toInt(); break;
    case DislikeCountRole:        item.dislikeCount = value.toInt(); break;
    case DownloadCountRole:       item.downloadCount = value.toInt(); break;
    case ShareCountRole:          item.shareCount = value.toInt(); break;
    case CollectCountRole:        item.collectCount = value.toInt(); break;
    case HotScoreRole:            item.hotScore = value.toInt(); break;
    case IsLikedRole:             item.isLiked = value.toBool(); break;
    case IsDislikedRole:          item.isDisliked = value.toBool(); break;
    case IsCollectedRole:         item.isCollected = value.toBool(); break;
    case DriveVersionRole:        item.driveVersion = value.toString(); break;
    case FirmwareVersionRole:     item.firmwareVersion = value.toString(); break;
    case StatusRole:              item.status = value.toString(); break;
    case VisibilityRole:          item.visibility = value.toString(); break;
    case IsPinnedRole:            item.isPinned = value.toBool(); break;
    case CreatedAtRole:           item.createdAt = value.toString(); break;
    default:                      return false;
  }
  return true;
}

void CommunityModel::setField(int userId, int role, const QVariant& value) {
  const int i = indexOfUser(userId);
  if (i < 0) return;
  if (!applyField(cl_items_[i], role, value)) return;
  emit dataChanged(index(i), index(i), {role});
}

void CommunityModel::batchSetField(const QList<int>& userIds, int role, const QVariant& value) {
  if (userIds.isEmpty()) return;

  int minRow = cl_items_.size();
  int maxRow = -1;

  for (int uid : userIds) {
    const int i = indexOfUser(uid);
    if (i < 0) continue;
    if (!applyField(cl_items_[i], role, value)) continue;
    minRow = (std::min)(minRow, i);
    maxRow = (std::max)(maxRow, i);
  }

  if (maxRow >= minRow) emit dataChanged(index(minRow), index(maxRow), {role});
}

void CommunityModel::clear() {
  if (cl_items_.isEmpty()) return;
  beginResetModel();
  cl_items_.clear();
  endResetModel();
}

std::optional<CommunityItemData> CommunityModel::findById(int userId) const {
  const int i = indexOfUser(userId);
  if (i < 0) return std::nullopt;
  return cl_items_.at(i);
}

int CommunityModel::count() const { return cl_items_.size(); }
bool CommunityModel::isEmpty() const { return cl_items_.isEmpty(); }

int CommunityModel::indexOfUser(int userId) const {
  // Effective STL Item 34: 算法 > 手写循环
  const auto it = std::find_if(cl_items_.cbegin(), cl_items_.cend(),
                               [userId](const CommunityItemData& d) { return d.userId == userId; });
  if (it == cl_items_.cend()) return -1;
  return static_cast<int>(std::distance(cl_items_.cbegin(), it));
}
