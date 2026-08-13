#ifndef USER_UPLOADS_DIALOG_H
#define USER_UPLOADS_DIALOG_H

#include <QDialog>
#include <QList>
#include <QPointer>
#include <QPixmap>
#include <QString>

#include "data/userConfig/user_config_api.h"  ///< ListItem（嵌套类型无法前置声明）

struct PaginatedResult;  ///< 前置声明（repository/paginated_repository.h）

class QFont;
class QLabel;
class QPropertyAnimation;
class QPushButton;
class QVBoxLayout;
class QWidget;
class CommunityDelegate;
class CommunityFlowView;
class CommunityModel;
class SchemeService;
class UserConfigRepository;
class UserRepository;

/// \brief 弹窗展示的用户资料 / User profile shown in the dialog
struct UserProfile {
  int userId = -1;      ///< 用户 ID / user id
  QString username;     ///< 用户名 / username
  QString nickname;     ///< 展示昵称 / display nickname
  QString bio;          ///< 个性签名 / bio
  QPixmap avatar;       ///< 头像 / avatar pixmap
  int level = 0;        ///< 用户等级 / user level
  bool isOfficial = false;      ///< 官方徽章 / official badge
  bool isExpert = false;        ///< 大神徽章（GET /user/:id 的 titles 含 expert）/ expert badge
  bool isStreamer = false;      ///< 主播徽章 / streamer badge
  bool isProfessional = false;  ///< 职业徽章 / professional badge
};

/// \brief 用户上传方案弹窗 — 点击社区用户头像后展示其资料 + 公开上传方案列表
///
/// 固定 382×640，无边框 + 半透明背景（圆角靠 shell 子控件绘制）。
/// 列表可滚动分页加载，支持点赞/踩/下载/分享/评论标签互动；
/// 滚动超过阈值折叠资料区（资料区直接隐藏，tabRow/view 瞬间定位，资料按钮顶部平移滑入）。
class UserUploadsDialog : public QDialog {
  Q_OBJECT

public:
  /// \brief 构造弹窗 / Construct dialog
  /// \param profile 用户资料（等级/徽章取列表 item，bio/大神头衔由 GET /user/:id 补充）
  /// \param parent 父窗口（showEvent 时居中）
  explicit UserUploadsDialog(const UserProfile& profile, QWidget* parent = nullptr);
  /// \brief 默认析构（子对象全部 QObject 父子链管理，动画随 this 析构安全）
  ~UserUploadsDialog() override = default;

protected:
  /// \brief 显示时居中于父窗口 / Center on parent when shown
  void showEvent(QShowEvent* event) override;

private:
  // ── 布局构建 ──
  void initUi();
  QWidget* createProfileWidget();  ///< 资料区（自然高度 219）
  QWidget* createTabRow();         ///< "上传"标签行（含状态文本）
  void initConnections();
  void updateProfileInfo();        ///< 昵称/头像/等级徽章/签名全量刷新
  void updateBadges();             ///< 重建等级胶囊 + 动态徽章（先删旧）
  void updateProfileAvatar(const QPixmap& pm);  ///< 头像 + 资料按钮图标（空图 → 绿底占位）
  void setStatusText(const QString& text);      ///< 状态文本（折叠态隐藏）

  // ── 数据加载 ──
  void loadPage(int page);
  void applyConfigs(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                    const PaginatedResult& page);
  void fetchAvatarsForList(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list);

  // ── 折叠/展开（资料区直接隐藏/显示；资料按钮顶部平移滑入/滑出） ──
  void updateHeaderForScroll(int value);
  void animateHeader(bool collapsed);

  // ── 互动操作 ──
  void onLikeClicked(int userId, bool liked);
  void onDislikeClicked(int userId, bool disliked);
  void onDownloadClicked(int userId);
  void onShareClicked(int userId);

  // ── 绘制 ──
  static QPixmap roundedAvatar(const QPixmap& pixmap, int size);   ///< 圆形头像（空图绿底占位）
  static QPixmap roundedBadgePixmap(const QPixmap& pixmap, int w, int h);  ///< 徽章 3px 圆角 clip

  // ── 常量 ──
  static constexpr int kDialogContentW = 382;    ///< 内容 shell 宽 / content shell width
  static constexpr int kDialogContentH = 640;    ///< 内容 shell 高 / content shell height
  static constexpr int kShadowPadding = 20;      ///< 阴影透明边距 / shadow transparent padding
  static constexpr int kPageSize = 50;           ///< 每页条数 / page size
  static constexpr int kHideProfileScroll = 80;  ///< 滚动超过此值触发折叠 / collapse threshold
  // 折叠态手动定位坐标（窗口坐标，shell 充满全窗）/ Collapsed coords (window coords)
  static constexpr int kTabRowCollapsedY = 36;   ///< 折叠态 tabRow Y / collapsed tab row Y
  static constexpr int kTabRowCollapsedW = 91;   ///< 折叠态 tabRow 宽 / collapsed tab row width
  static constexpr int kViewCollapsedY = 75;     ///< 折叠态 view Y / collapsed view Y
  static constexpr int kViewCollapsedH = 543;    ///< 折叠态 view 高 / collapsed view height
  static constexpr int kProfileBtnX = 143;       ///< 资料按钮 X（折叠态）/ profile btn X
  static constexpr int kProfileBtnY = 29;        ///< 资料按钮 Y（折叠态）/ profile btn Y
  static constexpr int kProfileBtnW = 97;        ///< 资料按钮宽 / profile btn width
  static constexpr int kProfileBtnH = 37;        ///< 资料按钮高 / profile btn height

  // ── 状态 ──
  UserProfile cl_profile_;            ///< 用户资料 / user profile
  bool cl_profile_collapsed_ = false; ///< 资料区折叠状态 / profile collapsed flag
  bool cl_loading_ = false;           ///< 分页加载中 / loading flag
  bool cl_has_more_ = true;           ///< 是否还有下一页 / has more pages
  int cl_page_ = 1;                   ///< 当前页 / current page

  // ── UI ──
  QWidget* clp_shell_ = nullptr;               ///< 圆角背景容器 / rounded shell
  QVBoxLayout* clp_shell_layout_ = nullptr;    ///< shell 主布局 / shell main layout
  QWidget* clp_tab_row_ = nullptr;             ///< "上传"标签行（折叠时移出布局手动定位）
  QPropertyAnimation* clp_profile_button_anim_ = nullptr;  ///< 资料按钮顶部平移滑入 / profile btn slide-in
  QWidget* clp_profile_widget_ = nullptr;       ///< 资料区容器 / profile container
  QLabel* clp_avatar_label_ = nullptr;          ///< 81×81 圆头像 / avatar label
  QLabel* clp_name_label_ = nullptr;            ///< 昵称 / nickname label
  QWidget* clp_badges_row_ = nullptr;           ///< 等级胶囊 + 徽章行 / level & badges row
  QLabel* clp_level_label_ = nullptr;           ///< 等级胶囊 Lv.x / level capsule
  QList<QPointer<QLabel>> clp_badge_labels_;    ///< 动态徽章（QPointer 防悬挂）/ badge labels
  QLabel* clp_bio_label_ = nullptr;             ///< 个性签名 / bio label
  QLabel* clp_upload_underline_ = nullptr;      ///< "上传"文字下等宽蓝线 / upload underline
  QLabel* clp_status_label_ = nullptr;          ///< 状态文本 / status label
  QPushButton* clp_profile_button_ = nullptr;   ///< 折叠态资料按钮（头像+昵称）/ profile button
  QPushButton* clp_close_button_ = nullptr;     ///< 关闭按钮 / close button

  // ── 数据层 ──
  CommunityModel* clp_model_ = nullptr;         ///< 列表模型 / list model
  CommunityFlowView* clp_view_ = nullptr;       ///< 瀑布流视图 / flow view
  CommunityDelegate* clp_delegate_ = nullptr;   ///< 卡片委托 / card delegate
  UserConfigRepository* clp_repo_ = nullptr;    ///< 配置仓库 / config repository
  SchemeService* clp_scheme_svc_ = nullptr;     ///< 方案服务（独立实例）/ scheme service
  UserRepository* clp_user_repo_ = nullptr;     ///< 用户仓库（GET /user/:id）/ user repository
};

#endif  // USER_UPLOADS_DIALOG_H
