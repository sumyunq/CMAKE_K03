#include "modules/CommunityModule/ui/data_sync_coordinator.h"

#include <QApplication>
#include <QClipboard>
#include <QTimer>

#include "modules/CommunityModule/infrastructure/compat/qt_compat.h"
#include "model/community_item_data.h"
#include "data/userConfig/user_config_api.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"
#include "repository/user_config_repository.h"
#include "modules/CommunityModule/service/scheme_service.h"
#include "modules/CommunityModule/ui/community/community_delegate.h"
#include "modules/CommunityModule/ui/community/community_model.h"
#include "modules/CommunityModule/ui/community/community_panel.h"
#include "modules/CommunityModule/ui/community_page_widget.h"
#include "LoadLib.h" ///< extern MainWindow *m（importDownloadedPlan 导入方案库）

DataSyncCoordinator::DataSyncCoordinator(QObject* parent) : QObject(parent) {}

void DataSyncCoordinator::init(SchemeService* svc, UserConfigRepository* repo,
                               CommunityModel* const (&leftModels)[3],
                               CommunityModel* uploadedModel, CommunityModel* likedModel,
                               QWidget* uploadedPanel, QWidget* likedPanel) {
  clp_svc_ = svc;
  clp_right_repo_ = repo;
  for (int i = 0; i < 3; ++i) clp_left_models_[i] = leftModels[i];
  clp_uploaded_model_ = uploadedModel;
  clp_liked_model_ = likedModel;
  clp_uploaded_panel_widget_ = uploadedPanel;
  clp_liked_panel_widget_ = likedPanel;

  setupAvatarFetch();
  setupDataSync();
  setupPersonalCenterInteractions();
}

// ── 头像下载 → 5 model ──

void DataSyncCoordinator::setupAvatarFetch() {
  connect(clp_right_repo_, &UserConfigRepository::avatarReady, this,
          [this](int userId, const QPixmap& pm) {
            for (auto* m : clp_left_models_)
              if (m) m->setField(userId, CommunityModel::AvatarRole, QVariant::fromValue(pm));
            if (clp_uploaded_model_)
              clp_uploaded_model_->setField(userId, CommunityModel::AvatarRole, QVariant::fromValue(pm));
            if (clp_liked_model_)
              clp_liked_model_->setField(userId, CommunityModel::AvatarRole, QVariant::fromValue(pm));
          });
}

// ── 5-model 查找/移除 ──

std::optional<CommunityItemData> DataSyncCoordinator::findItemFromAll(int userId) const {
  for (auto* m : clp_left_models_)
    if (m) if (auto opt = m->findById(userId)) return opt;
  if (clp_uploaded_model_) if (auto opt = clp_uploaded_model_->findById(userId)) return opt;
  if (clp_liked_model_) if (auto opt = clp_liked_model_->findById(userId)) return opt;
  return {};
}

void DataSyncCoordinator::rmFromAll(int userId) {
  for (auto* m : clp_left_models_) if (m) m->removeItem(userId);
  if (clp_uploaded_model_) clp_uploaded_model_->removeItem(userId);
  if (clp_liked_model_) clp_liked_model_->removeItem(userId);
}

// ── 操作 → 5-model 精准同步 ──

void DataSyncCoordinator::setupDataSync() {
  // 点赞 → liked.addItem  /  取消点赞 → liked.removeItem（乐观，零重拉）
  connect(clp_svc_, &SchemeService::likeToggled, this, [this](int uid, bool newState) {
    if (!clp_liked_model_) return;
    if (newState) {
      // 优先从"取消点赞缓存"恢复（数据可能只存在于已点赞列表，其他 model 找不到）
      auto t_it = cl_removed_liked_cache_.find(uid);
      if (t_it != cl_removed_liked_cache_.end()) {
        t_it->isLiked = true;
        clp_liked_model_->addItem(t_it.value());
        cl_removed_liked_cache_.erase(t_it);
        return;
      }
      if (auto item = findItemFromAll(uid)) {
        item->isLiked = true;
        clp_liked_model_->addItem(*item);
      }
    } else {
      // 取消点赞：缓存 item（供再次点赞恢复），再移除
      if (auto opt = clp_liked_model_->findById(uid))
        cl_removed_liked_cache_.insert(uid, *opt);
      clp_liked_model_->removeItem(uid);
    }
  });

  // 踩 → liked.removeItem（缓存供"重新点赞"恢复）
  // 取消踩 → 不自动恢复（服务端未重新点赞，isLiked 仍为 false — 恢复会导致列表出现未赞方案）
  // 用户想回列表需重新点赞 → likeToggled(true) → 走缓存恢复
  connect(clp_svc_, &SchemeService::dislikeToggled, this,
          [this](int uid, bool newState) {
            if (!clp_liked_model_ || !newState) return;
            if (auto opt = clp_liked_model_->findById(uid))
              cl_removed_liked_cache_.insert(uid, *opt);
            clp_liked_model_->removeItem(uid);
          });

  // 删除确认（API 层面；model 移除已在 deleteRequested 处理器中完成）
  connect(clp_right_repo_, &UserConfigRepository::configDeleted, this,
          [] { LOG_DEBUG("[DataSync] configDeleted OK"); });

  // 置顶/取消置顶完成 → 已上传 model 同步 IsPinnedRole（置顶条显示/隐藏）
  auto syncPinned = [this](const QString& id, bool pinned) {
    if (!clp_uploaded_model_) return;
    const int uid = id.toInt();
    if (clp_uploaded_model_->findById(uid))
      clp_uploaded_model_->setField(uid, CommunityModel::IsPinnedRole, pinned);
  };
  connect(clp_right_repo_, &UserConfigRepository::configPinned, this,
          [syncPinned](const QString& id) { syncPinned(id, true); });
  connect(clp_right_repo_, &UserConfigRepository::configUnpinned, this,
          [syncPinned](const QString& id) { syncPinned(id, false); });

  // 可见性更新完成 → 已上传 model 同步 VisibilityRole（PUT /user-configs/:id）
  connect(clp_right_repo_, &UserConfigRepository::configUpdated, this,
          [this](const DeSheng::GetPublicConfigurationListResponse::ListItem& info) {
            if (!clp_uploaded_model_) return;
            const int uid = info.id;
            if (clp_uploaded_model_->findById(uid))
              clp_uploaded_model_->setField(uid, CommunityModel::VisibilityRole, info.visibility);
          });

  // 错误回滚 → 已上传列表重拉（一致性）
  // 下载失败且配置已被服务端删除 → 从全部 model 移除（当前视图数据即时刷新）
  connect(clp_svc_, &SchemeService::downloadFailed, this,
          [this](int configId, const QString& reason) {
            if (!reason.contains(QStringLiteral("不存在"))) return;
            for (auto* m : clp_left_models_) if (m) m->removeItem(configId);
            if (clp_uploaded_model_) clp_uploaded_model_->removeItem(configId);
            if (clp_liked_model_) clp_liked_model_->removeItem(configId);
          });

  // 评论标签失败回滚 → 5 model 翻转恢复（与 delegate 乐观更新对称）
  connect(clp_svc_, &SchemeService::commentClickReverted, this,
          [this](int configId, int commentId) {
            auto upd = [&](CommunityModel* m) {
              if (!m) return;
              auto opt = m->findById(configId);
              if (!opt.has_value()) return;
              for (auto& c : opt->comments) {
                if (c.id == commentId) {
                  c.is_clicked = !c.is_clicked;
                  c.count += c.is_clicked ? 1 : -1;
                  m->setField(configId, CommunityModel::CommentsRole,
                              QVariant::fromValue(opt->comments));
                  break;
                }
              }
            };
            for (auto* m : clp_left_models_) upd(m);
            upd(clp_uploaded_model_);
            upd(clp_liked_model_);
          });

  // countsSynced → 5 model（6 字段同步）
  connect(clp_svc_, &SchemeService::countsSynced, this,
          [this](const DeSheng::GetPublicConfigurationListResponse::ListItem& info) {
            const int id = info.id;
            auto upd = [&](CommunityModel* m) {
              if (!m) return;
              m->setField(id, CommunityModel::LikeCountRole, info.like_count);
              m->setField(id, CommunityModel::DislikeCountRole, info.dislike_count);
              m->setField(id, CommunityModel::DownloadCountRole, info.download_count);
              m->setField(id, CommunityModel::ShareCountRole, info.share_count);
              m->setField(id, CommunityModel::IsLikedRole, info.is_liked);
              m->setField(id, CommunityModel::IsDislikedRole, info.is_disliked);
            };
            for (auto* m : clp_left_models_) upd(m);
            upd(clp_uploaded_model_);
            upd(clp_liked_model_);
          });
}

// ── 个人中心交互（与社区一致的信号流） ──

void DataSyncCoordinator::setupPersonalCenterInteractions() {
  auto* svc = clp_svc_;
  if (!svc) return;

  // 下载进度 — 常驻连接 + 按 configId 查表分发面板（避免 disconnect-all 竞态：快速多下载各显各的）
  connect(svc, &SchemeService::downloadProgress, this,
          [this](int cid, int pct) {
            if (auto* p = cl_download_panels_.value(cid, nullptr))
              p->setDownloadProgress(cid, pct);
          });

  auto wirePanel = [this, svc](QWidget* panelWidget, bool isUploaded) {
    auto* rp = qobject_cast<CommunityPanel*>(panelWidget);
    if (!rp) return;

    // 点赞 → svc（乐观更新）
    connect(rp, &CommunityPanel::likeToggled, this,
            [svc](int uid, bool liked) { svc->toggleLike(uid, !liked); });

    // 踩 → svc（移除/恢复统一由 setupDataSync 的 dislikeToggled 处理：缓存 + 移除，保证可恢复）
    connect(rp, &CommunityPanel::dislikeToggled, this,
            [svc](int uid, bool disliked) { svc->toggleDislike(uid, !disliked); });

    // 评论标签
    connect(rp, &CommunityPanel::commentTagClicked, this,
            [svc](int cid, int cmtId, bool clicked) { svc->toggleCommentClick(cid, cmtId, clicked); });

    // 下载 — 登记面板到查表（进度由常驻连接按 configId 分发）
    connect(rp, &CommunityPanel::downloadRequested, this, [this, svc, rp](int uid) {
      cl_download_panels_[uid] = rp;
      connectOnce(svc, &SchemeService::downloadFileSaved, this,
                  [this, rp, uid](int, const QString& t_filePath) {
                    cl_download_panels_.remove(uid);
                    rp->setDownloadProgress(uid, 100);
                    // 导入方案库 + 清理临时文件（importDownloadedPlan 内部 remove）
                    if (m) m->importDownloadedPlan(t_filePath);
                    // 拉取详情刷新下载计数（与社区页 downloadFileSaved 处理一致）
                    clp_svc_->refreshCounts(uid);
                    QTimer::singleShot(500, [rp, uid] { rp->setDownloadProgress(uid, -1); });
                  });
      connectOnce(svc, &SchemeService::errorOccurred, this,
                  [this, rp, uid](const QString&, const QString&) {
                    cl_download_panels_.remove(uid);
                    rp->setDownloadProgress(uid, -1);
                  });
      rp->setDownloadProgress(uid, 0);
      svc->download(uid);
    });

    // 分享 → svc + shareCodeReady → 复制分享码 + toast（与社区页一致）
    connect(rp, &CommunityPanel::shareRequested, this,
            [this, svc](int uid) {
              connectOnce(svc, &SchemeService::shareCodeReady, this,
                          [this, uid](const QString& shareCode, int shareCount) {
                            QString shareText = "sq" + shareCode;
                            for (auto* m : {clp_uploaded_model_, clp_liked_model_}) {
                              if (!m) continue;
                              auto opt = m->findById(uid);
                              if (opt.has_value()) {
                                shareText = opt->planName + "+" + opt->deviceName + "+" +
                                            opt->tags.join("+") + "+sq" + shareCode;
                                m->setField(uid, CommunityModel::ShareCountRole, shareCount);
                                break;
                              }
                            }
                            if (clp_svc_) clp_svc_->refreshCounts(uid);
                            QApplication::clipboard()->setText(shareText);
                            if (g_shareCodeCopyHint) {
                              g_shareCodeCopyHint->setText(tr("分享码已复制"));
                              g_shareCodeCopyHint->show();
                              QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                            }
                          });
              connectOnce(svc, &SchemeService::errorOccurred, this,
                          [](const QString&, const QString&) {});
              svc->share(uid);
            });

    // 删除（仅已上传面板）→ 5 model 移除 + API
    if (isUploaded) {
      connect(rp, &CommunityPanel::deleteRequested, this,
              [this](int userId) {
                rmFromAll(userId);
                clp_right_repo_->deleteConfig(QString::number(userId));
              });

      // 置顶/取消置顶（仅已上传面板）→ repo API（POST/DELETE /user-configs/:id/pin）
      // 成功回调 setupDataSync 中更新 IsPinnedRole；失败走 errorOccurred → 列表重拉（一致性）
      connect(rp, &CommunityPanel::pinRequested, this,
              [this](int userId, bool pin) {
                if (pin)
                  clp_right_repo_->pin(QString::number(userId));
                else
                  clp_right_repo_->unpin(QString::number(userId));
              });

      // 仅自己可见/设为公开（仅已上传面板）→ PUT /user-configs/:id 更新 visibility
      // 成功回调 setupDataSync 中更新 VisibilityRole；失败走 errorOccurred → 列表重拉（一致性）
      connect(rp, &CommunityPanel::visibilityRequested, this,
              [this](int userId, bool privateOnly) {
                QJsonObject t_req;
                t_req[QStringLiteral("visibility")] =
                    privateOnly ? DeSheng::kVisibilityPrivate : DeSheng::kVisibilityPublic;
                clp_right_repo_->updateConfig(QString::number(userId), t_req);
              });
    }
  };

  wirePanel(clp_uploaded_panel_widget_, true);
  wirePanel(clp_liked_panel_widget_, false);
}
