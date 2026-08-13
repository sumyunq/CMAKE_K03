#include "modules/CommunityModule/ui/ranking/ranking_model.h"

// 完整命名空间前缀（规范：禁止 using 指令）
typedef DeSheng::GetPublicConfigurationListResponse::ListItem RankingListItem;

RankingModel::RankingModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int RankingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return cl_items_.size();
}

QVariant RankingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= cl_items_.size())
        return {};

    const RankingListItem &t_item = cl_items_.at(index.row());

    switch (static_cast<RankingRole>(role)) {
    case RankingRole::RankRole:
        return index.row() + 1;
    case RankingRole::PlanNameRole:
        return t_item.title;
    case RankingRole::UserNameRole:
        return t_item.author.nickname.isEmpty() ? t_item.author.username
                                                : t_item.author.nickname;
    case RankingRole::HeatCountRole:
        return cl_heat_is_like_ ? t_item.like_count : t_item.download_count;
    case RankingRole::AvatarRole: {
        const auto t_it = cl_avatars_.constFind(index.row());
        return t_it != cl_avatars_.constEnd() ? QVariant::fromValue(t_it.value()) : QVariant();
    }
    case RankingRole::PlanIdRole:
        return t_item.id;
    case RankingRole::IsLikedRole:
        return t_item.is_liked;
    case RankingRole::DownloadingRole:
        return cl_downloading_ids_.contains(t_item.id);
    case RankingRole::DownloadProgressRole:
        return cl_download_progress_.value(t_item.id, -1);
    default:
        return {};
    }
}

void RankingModel::replaceAll(const QList<RankingListItem> &items)
{
    beginResetModel();
    cl_items_ = items;
    cl_avatars_.clear();
    cl_downloading_ids_.clear();
    cl_download_progress_.clear();
    endResetModel();
}

void RankingModel::clear()
{
    beginResetModel();
    cl_items_.clear();
    cl_avatars_.clear();
    cl_downloading_ids_.clear();
    cl_download_progress_.clear();
    endResetModel();
}

void RankingModel::setAvatar(int row, const QPixmap &pm)
{
    if (row < 0 || row >= cl_items_.size() || pm.isNull())
        return;
    cl_avatars_.insert(row, pm);
    const QModelIndex t_index = index(row);
    emit dataChanged(t_index, t_index,
                     QVector<int>{static_cast<int>(RankingRole::AvatarRole)});
}

void RankingModel::setDownloading(int configId, bool downloading)
{
    if (downloading)
        cl_downloading_ids_.insert(configId);
    else {
        cl_downloading_ids_.remove(configId);
        cl_download_progress_.remove(configId);  // 下载结束：进度一并清除
    }
    const int t_row = rowOfConfigId(configId);
    if (t_row >= 0) {
        const QModelIndex t_index = index(t_row);
        emit dataChanged(t_index, t_index,
                         QVector<int>{static_cast<int>(RankingRole::DownloadingRole)});
    }
}

void RankingModel::setDownloadProgress(int configId, int percent)
{
    if (!cl_downloading_ids_.contains(configId))
        return;  // 仅下载中有效
    cl_download_progress_.insert(configId, percent);
    const int t_row = rowOfConfigId(configId);
    if (t_row >= 0) {
        const QModelIndex t_index = index(t_row);
        emit dataChanged(t_index, t_index,
                         QVector<int>{static_cast<int>(RankingRole::DownloadProgressRole)});
    }
}

void RankingModel::setHeatField(bool isLike)
{
    if (cl_heat_is_like_ == isLike)
        return;
    cl_heat_is_like_ = isLike;
    if (!cl_items_.isEmpty()) {
        const QModelIndex t_first = index(0);
        emit dataChanged(t_first, index(cl_items_.size() - 1),
                         QVector<int>{static_cast<int>(RankingRole::HeatCountRole)});
    }
}

void RankingModel::applyServerItem(int row, const RankingListItem &item)
{
    if (row < 0 || row >= cl_items_.size())
        return;
    cl_items_[row] = item;
    const QModelIndex t_index = index(row);
    emit dataChanged(t_index, t_index);
}

int RankingModel::rowOfConfigId(int configId) const
{
    for (int t_i = 0; t_i < cl_items_.size(); ++t_i) {
        if (cl_items_.at(t_i).id == configId)
            return t_i;
    }
    return -1;
}

void RankingModel::removeByConfigId(int configId)
{
    const int t_row = rowOfConfigId(configId);
    if (t_row < 0)
        return;

    beginRemoveRows(QModelIndex(), t_row, t_row);
    cl_items_.removeAt(t_row);
    endRemoveRows();

    // 头像缓存按行号索引：移除中间行后，后续行号整体左移
    if (!cl_avatars_.isEmpty()) {
        QHash<int, QPixmap> t_new;
        for (auto t_it = cl_avatars_.constBegin(); t_it != cl_avatars_.constEnd(); ++t_it) {
            const int t_key = t_it.key();
            t_new.insert(t_key > t_row ? t_key - 1 : t_key, t_it.value());
        }
        cl_avatars_ = t_new;
    }
    cl_downloading_ids_.remove(configId);
    cl_download_progress_.remove(configId);
}

std::optional<RankingListItem> RankingModel::itemAt(int row) const
{
    if (row < 0 || row >= cl_items_.size())
        return std::nullopt;
    return cl_items_.at(row);
}
