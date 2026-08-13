#include "modules/CommunityModule/service/scheme_service.h"

#include <QMap>
#include <QPointer>
#include <QString>

#include "data/userConfig/user_config_api.h"
#include "modules/CommunityModule/infrastructure/compat/qt_compat.h"
#include "repository/user_config_repository.h"

SchemeService::SchemeService(QObject* parent) : QObject(parent) {}

void SchemeService::init(UserConfigRepository* repo) { clp_repo_ = repo; }

QMap<QString, QString> SchemeService::buildFilters(const QString& deviceType,
                                                   const QString& deviceName,
                                                   const QString& keyword,
                                                   const QString& userTag) {
  QMap<QString, QString> filters;
  if (!deviceType.isEmpty()) {
    filters.insert(QStringLiteral("device_type"), deviceType);
  }
  if (!deviceName.isEmpty()) {
    filters.insert(QStringLiteral("device_name"), deviceName);
  }
  if (!keyword.isEmpty()) {
    filters.insert(QStringLiteral("keyword"), keyword);
  }
  if (!userTag.isEmpty()) {
    filters.insert(QStringLiteral("user_tag"), userTag);
  }
  return filters;
}

void SchemeService::fetchSquare(int page, const QString& sort, const QString& deviceType,
                                const QString& deviceName, const QString& keyword,
                                const QString& userTag) {
  if (!clp_repo_) return;
  const int requestSeq = ++cl_square_fetch_seq_;
  auto filters = buildFilters(deviceType, deviceName, keyword, userTag);
  clp_repo_->getPublicConfigs(page, kDefaultPageSize, sort, filters,
                              [this, requestSeq](
                                  const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                                  const PaginatedResult& pg) {
                                if (requestSeq != cl_square_fetch_seq_) return;
                                emit squareDataReady(list, pg);
                              },
                              [this, requestSeq](const QString& reason) {
                                if (requestSeq != cl_square_fetch_seq_) return;
                                // fetch 失败：请求级错误通道 → 页面错误态覆盖层
                                emit errorOccurred(QStringLiteral("fetch-square"), reason);
                              });
}

void SchemeService::fetchExpert(int page, const QString& sort, const QString& deviceType,
                                const QString& deviceName, const QString& userTag,
                                const QString& keyword) {
  if (!clp_repo_) return;
  const int requestSeq = ++cl_expert_fetch_seq_;
  auto filters = buildFilters(deviceType, deviceName, keyword, userTag);
  filters.insert(QStringLiteral("is_expert_tag"), QStringLiteral("true"));
  clp_repo_->getPublicConfigs(page, kDefaultPageSize, sort, filters,
                              [this, requestSeq](
                                  const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                                  const PaginatedResult& pg) {
                                if (requestSeq != cl_expert_fetch_seq_) return;
                                emit expertDataReady(list, pg);
                              },
                              [this, requestSeq](const QString& reason) {
                                if (requestSeq != cl_expert_fetch_seq_) return;
                                emit errorOccurred(QStringLiteral("fetch-expert"), reason);
                              });
}

void SchemeService::fetchOfficial(int page, const QString& sort, const QString& deviceType,
                                  const QString& deviceName, const QString& userTag,
                                  const QString& keyword) {
  if (!clp_repo_) return;
  const int requestSeq = ++cl_official_fetch_seq_;
  auto filters = buildFilters(deviceType, deviceName, keyword, userTag);
  filters.insert(QStringLiteral("is_official_tag"), QStringLiteral("true"));
  clp_repo_->getPublicConfigs(page, kDefaultPageSize, sort, filters,
                              [this, requestSeq](
                                  const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                                  const PaginatedResult& pg) {
                                if (requestSeq != cl_official_fetch_seq_) return;
                                emit officialDataReady(list, pg);
                              },
                              [this, requestSeq](const QString& reason) {
                                if (requestSeq != cl_official_fetch_seq_) return;
                                emit errorOccurred(QStringLiteral("fetch-official"), reason);
                              });
}

void SchemeService::toggleLike(int configId, bool liked) {
  if (!clp_repo_) return;

  const QString id = QString::number(configId);

  // 1. Optimistic update — emit the new (toggled) state
  const bool newState = !liked;
  cl_like_ok_ = false;
  emit likeToggled(configId, newState);

  if (liked) {
    connectOnce(clp_repo_, &UserConfigRepository::configUnliked, this, [this, configId] {
      cl_like_ok_ = true;
      refreshCounts(configId);
    });
    clp_repo_->unlike(id);
  } else {
    connectOnce(clp_repo_, &UserConfigRepository::configLiked, this, [this, configId] {
      cl_like_ok_ = true;
      refreshCounts(configId);
    });
    clp_repo_->like(id);
  }

  // 3. Error → rollback ONLY if the like/unlike itself failed (not refreshCounts)
  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this,
              [this, configId, liked](const QString& reason) {
                if (cl_like_ok_) return;  // like succeeded; this error is from refreshCounts
                emit likeToggled(configId, liked);  // revert to original state
                emit errorOccurred(QStringLiteral("toggleLike"), reason);
              });
}

void SchemeService::toggleDislike(int configId, bool disliked) {
  if (!clp_repo_) return;
  const QString id = QString::number(configId);

  // 1. Optimistic update
  const bool newState = !disliked;
  emit dislikeToggled(configId, newState);

  if (disliked) {
    connectOnce(clp_repo_, &UserConfigRepository::configUndisliked, this, [this, configId] {
      refreshCounts(configId);
    });
    clp_repo_->undislike(id);
  } else {
    connectOnce(clp_repo_, &UserConfigRepository::configDisliked, this, [this, configId] {
      refreshCounts(configId);
    });
    clp_repo_->dislike(id);
  }

  // Error → rollback
  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this,
              [this, configId, disliked](const QString& reason) {
                emit dislikeToggled(configId, disliked);  // revert
                emit errorOccurred(QStringLiteral("toggleDislike"), reason);
              });
}

void SchemeService::toggleCollect(int configId, bool collected) {
  if (!clp_repo_) return;

  const QString id = QString::number(configId);

  const bool newState = !collected;
  emit collectToggled(configId, newState);

  if (collected) {
    connectOnce(clp_repo_, &UserConfigRepository::configUncollected, this, [this, configId] {
      refreshCounts(configId);
    });
    clp_repo_->uncollect(id);
  } else {
    connectOnce(clp_repo_, &UserConfigRepository::configCollected, this, [this, configId] {
      refreshCounts(configId);
    });
    clp_repo_->collect(id);
  }

  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this,
              [this, configId, collected](const QString& reason) {
                emit collectToggled(configId, collected);  // rollback
                emit errorOccurred(QStringLiteral("toggleCollect"), reason);
              });
}

void SchemeService::download(int configId) {
  if (!clp_repo_) return;
  const QString id = QString::number(configId);

  // Step 1: GET /user-configs/:id/download → config_url
  connectOnce(clp_repo_, &UserConfigRepository::configDownloadUrlReady, this,
              [this, configId](const QString& configUrl) {
    // Step 2: 委托 Repository 下载文件（DIP：Service 不直接调 ApiClient）
    // 进度和保存信号通过 Repository 信号转发到 Service
    QPointer<SchemeService> self(this);
    connectOnce(clp_repo_, &UserConfigRepository::downloadProgress, this,
                [self, configId](int cid, int pct) {
      if (self) self->downloadProgress(cid, pct);
    });
    connectOnce(clp_repo_, &UserConfigRepository::downloadFileSaved, this,
                [self](int cid, const QString& p) {
      if (self) self->downloadFileSaved(cid, p);
    });
    connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this,
                [self, configId](const QString& r) {
      if (self) {
        self->errorOccurred(QStringLiteral("download"), r);
        self->downloadFailed(configId, r);
        self->downloadProgress(configId, -1);
      }
    });
    clp_repo_->downloadConfigFile(configUrl, configId);
  });

  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this, [this, configId](const QString& r) {
    emit errorOccurred(QStringLiteral("download"), r);
    emit downloadFailed(configId, r);
    emit downloadProgress(configId, -1);
  });

  clp_repo_->downloadConfig(id);
}

void SchemeService::share(int configId) {
  if (!clp_repo_) return;

  const QString id = QString::number(configId);

  connectOnce(clp_repo_, &UserConfigRepository::configShared, this,
              [this](const QString& shareCode, int shareCount) {
                emit shareCodeReady(shareCode, shareCount);
              });

  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this, [this](const QString& reason) {
    emit errorOccurred(QStringLiteral("share"), reason);
  });

  clp_repo_->shareConfig(id);
}

void SchemeService::clickComment(int configId, int commentId) {
  if (!clp_repo_) return;

  const QString cfgId = QString::number(configId);
  const QString cmtId = QString::number(commentId);

  connectOnce(clp_repo_, &UserConfigRepository::commentClicked, this,
              [this, configId, commentId] {});

  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this, [this](const QString& reason) {
    emit errorOccurred(QStringLiteral("clickComment"), reason);
  });

  clp_repo_->clickComment(cfgId, cmtId);
}

void SchemeService::toggleCommentClick(int configId, int commentId, bool clicked) {
  if (!clp_repo_) return;
  const QString cfgId = QString::number(configId);
  const QString cmtId = QString::number(commentId);

  connectOnce(clp_repo_, &UserConfigRepository::commentClicked, this,
              [this, configId, commentId] {
                refreshCounts(configId);  ///< 服务端确认后同步真实计数（评论标签计数跨 model 同步）
              });
  connectOnce(clp_repo_, &UserConfigRepository::commentUnclicked, this,
              [this, configId, commentId] {
                refreshCounts(configId);
              });
  // 评论失败 → 精确回滚信号（repository 已按操作区分，不会误触发）
  connectOnce(clp_repo_, &UserConfigRepository::commentClickFailed, this,
              [this, configId, commentId](int, int) {
                emit commentClickReverted(configId, commentId);
              });
  connectOnce(clp_repo_, &UserConfigRepository::errorOccurred, this, [this](const QString& reason) {
    emit errorOccurred(QStringLiteral("toggleCommentClick"), reason);
  });

  if (clicked) {
    clp_repo_->clickComment(cfgId, cmtId);
  } else {
    clp_repo_->unclickComment(cfgId, cmtId);
  }
}

void SchemeService::refreshCounts(int configId) {
  if (!clp_repo_) return;
  const QString id = QString::number(configId);
  // 回调版 — 避免 connectOnce(configDetailReady) 共享竞态：
  // 多个 refreshCounts 并发时，一个响应会触发所有 connectOnce，后续响应丢失（计数不同步）
  clp_repo_->getConfigDetail(id, [this](const DeSheng::GetPublicConfigurationListResponse::ListItem& info) {
    emit countsSynced(info);
  });
}
