#ifndef DATA_SYNC_COORDINATOR_H
#define DATA_SYNC_COORDINATOR_H

#include <QHash>
#include <QObject>
#include <QList>
#include <functional>
#include <optional>

class SchemeService;
class UserConfigRepository;
class CommunityPageWidget;
class CommunityModel;
class QWidget;
struct CommunityItemData;
struct PaginatedResult;

/// \brief 数据同步协调器 — 5-model 同步（3 社区左 + 2 个人中心）+ 个人中心交互 + 头像加载
///
/// 社区操作（点赞/踩/下载/分享/删除）与个人中心"已上传/已点赞"列表双向联动。
class DataSyncCoordinator : public QObject {
  Q_OBJECT
public:
  explicit DataSyncCoordinator(QObject* parent = nullptr);

  /// \brief 注入依赖并建立所有信号连接
  /// \param leftModels 社区三 Tab model 数组
  /// \param uploadedModel 个人中心已上传 model
  /// \param likedModel 个人中心已点赞 model
  /// \param uploadedPanel 个人中心已上传 CommunityPanel（交互接线用，QWidget 基类指针）
  /// \param likedPanel 个人中心已点赞 CommunityPanel
  void init(SchemeService* svc, UserConfigRepository* repo,
            CommunityModel* const (&leftModels)[3],
            CommunityModel* uploadedModel, CommunityModel* likedModel,
            QWidget* uploadedPanel, QWidget* likedPanel);

private:
  void setupAvatarFetch();
  void setupDataSync();
  void setupPersonalCenterInteractions();

  /// \brief 在所有 5 个 model 中查找 userId 的 item
  std::optional<CommunityItemData> findItemFromAll(int userId) const;
  /// \brief 从所有 5 个 model 中移除 userId
  void rmFromAll(int userId);

  SchemeService* clp_svc_ = nullptr;
  UserConfigRepository* clp_right_repo_ = nullptr;
  CommunityModel* clp_left_models_[3] = {nullptr, nullptr, nullptr};
  CommunityModel* clp_uploaded_model_ = nullptr;  ///< 个人中心已上传
  CommunityModel* clp_liked_model_ = nullptr;     ///< 个人中心已点赞
  QWidget* clp_uploaded_panel_widget_ = nullptr;  ///< 个人中心已上传 CommunityPanel
  QWidget* clp_liked_panel_widget_ = nullptr;     ///< 个人中心已点赞 CommunityPanel
  QHash<int, struct CommunityItemData> cl_removed_liked_cache_; ///< 取消点赞缓存（再点赞恢复，零重拉）
  QHash<int, class CommunityPanel*> cl_download_panels_;        ///< configId → 下载进度面板（按 id 分发，避免 disconnect-all 竞态）
};

#endif  // DATA_SYNC_COORDINATOR_H
