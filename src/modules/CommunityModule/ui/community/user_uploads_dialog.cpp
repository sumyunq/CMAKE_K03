#include "modules/CommunityModule/ui/community/user_uploads_dialog.h"

#include <QApplication>
#include <QClipboard>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QRadialGradient>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

#include "LoadLib.h"  ///< extern MainWindow *m（importDownloadedPlan）
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots
#include "data/user/user_api.h"  ///< GetPublicUserInfoResponse
#include "data/userConfig/user_config_api.h"  ///< ListItem
#include "model/community_item_data.h"  ///< CommunityItemData + configToItems（公共实现）
#include "modules/Common/DeviceRegistry.h"  ///< shortDisplayName 设备名映射
#include "modules/CommunityModule/infrastructure/compat/qt_compat.h"  ///< connectOnce
#include "modules/CommunityModule/infrastructure/logger/logger.h"
#include "modules/CommunityModule/service/scheme_service.h"
#include "modules/CommunityModule/ui/community/community_delegate.h"
#include "modules/CommunityModule/ui/community/community_flow_view.h"
#include "modules/CommunityModule/ui/community/community_model.h"
#include "repository/user_config_repository.h"
#include "repository/user_repository.h"

// ── 构造 / 生命周期 ──

UserUploadsDialog::UserUploadsDialog(const UserProfile& profile, QWidget* parent)
    : QDialog(parent), cl_profile_(profile) {
  // 独立数据层实例（不复用主页面共享实例，避免信号串扰；随弹窗析构）
  clp_repo_ = new UserConfigRepository(this);
  clp_scheme_svc_ = new SchemeService(this);
  clp_scheme_svc_->init(clp_repo_);
  clp_user_repo_ = new UserRepository(this);

  initUi();
  initConnections();
  updateProfileInfo();
  loadPage(1);

  // 拉作者公开信息（bio + 大神头衔 titles）
  if (cl_profile_.userId > 0)
    clp_user_repo_->getPublicUserInfo(QString::number(cl_profile_.userId));
}

void UserUploadsDialog::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  // 居中于顶层应用窗口（parentWidget 是社区页子部件，geometry 仅其自身区域）
  if (QWidget* t_parent = parentWidget()) {
    const QRect t_geom = t_parent->window()->geometry();
    move(t_geom.center() - rect().center());
  }
}

// ── 布局构建 ──

void UserUploadsDialog::initUi() {
  setFixedSize(kDialogContentW + kShadowPadding * 2,
               kDialogContentH + kShadowPadding * 2);
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground, true);  // 圆角靠 shell 子控件背景绘制
  setAttribute(Qt::WA_DeleteOnClose, true);
  setModal(true);

  auto* t_root_layout = new QVBoxLayout(this);
  t_root_layout->setContentsMargins(kShadowPadding, kShadowPadding,
                                    kShadowPadding, kShadowPadding);

  // shell — 圆角背景容器
  clp_shell_ = new QWidget(this);
  clp_shell_->setFixedSize(kDialogContentW, kDialogContentH);
  clp_shell_->setObjectName(QStringLiteral("userUploadsShell"));
  clp_shell_->setAttribute(Qt::WA_StyledBackground, true);
  clp_shell_->setStyleSheet(QStringLiteral(
      "QWidget#userUploadsShell { background:#0f1620; border-radius:10px; }"
      "QLabel { color:#9aa4b2; background:transparent; }"
      "QPushButton { border:none; background:transparent; color:#9aa4b2; }"));
  auto* t_shadow = new QGraphicsDropShadowEffect(clp_shell_);
  t_shadow->setBlurRadius(20);
  t_shadow->setOffset(0, 0);
  t_shadow->setColor(QColor(0, 0, 0, 128));
  clp_shell_->setGraphicsEffect(t_shadow);
  t_root_layout->addWidget(clp_shell_);

  clp_shell_layout_ = new QVBoxLayout(clp_shell_);
  clp_shell_layout_->setContentsMargins(26, 10, 0, 22);  // 内容宽 356（右缘贴齐弹窗，列表滚动条靠弹窗右缘）
  clp_shell_layout_->setSpacing(0);

  // topBar（占位固定高，保证 profile 从 y=47 开始；关闭按钮绝对定位在其右侧）
  auto* t_top_bar = new QWidget(clp_shell_);
  t_top_bar->setFixedHeight(37);
  clp_shell_layout_->addWidget(t_top_bar);

  // 资料区（自然高度 219，折叠时直接隐藏）
  clp_profile_widget_ = createProfileWidget();
  clp_shell_layout_->addWidget(clp_profile_widget_);

  // "上传"标签行
  clp_tab_row_ = createTabRow();
  clp_shell_layout_->addWidget(clp_tab_row_);

  // 列表视图 + 模型 + 委托
  clp_model_ = new CommunityModel(this);
  clp_view_ = new CommunityFlowView(clp_shell_);
  clp_view_->setMaxContentWidth(330);  ///< 视口加宽（滚动条贴右缘）但卡片宽度保持 330 不扩张
  clp_delegate_ = new CommunityDelegate(clp_view_);
  clp_delegate_->setPinnedBarEnabled(false);  // 用户上传列表不显示置顶条（与主页面公开列表一致）
  clp_delegate_->setCardBlurEnabled(false);   // 弹窗独立窗口：卡片禁用毛玻璃（背景模糊快照不适配）
  clp_view_->setModel(clp_model_);
  clp_view_->setItemDelegate(clp_delegate_);
  clp_shell_layout_->addWidget(clp_view_, 1);

  // 关闭按钮（右上角，绝对定位）
  clp_close_button_ = new QPushButton(clp_shell_);
  clp_close_button_->setGeometry(341, 10, 31, 31);
  clp_close_button_->setCursor(Qt::PointingHandCursor);
  clp_close_button_->setStyleSheet(R"(
      QPushButton
      {
        border-radius:0px;
        border-image: url(:/Skin/Images/Popup/close-no.png);
        background:transparent;
      }
      QPushButton:hover
      {
        border-image: url(:/Skin/Images/Popup/close-ho.png);
      }
  )");

  // 资料按钮（折叠态显示：25×25 圆头像 icon + 昵称，顶部平移滑入）
  clp_profile_button_ = new QPushButton(clp_shell_);
  clp_profile_button_->setGeometry(kProfileBtnX, kProfileBtnY, kProfileBtnW, kProfileBtnH);
  clp_profile_button_->setCursor(Qt::PointingHandCursor);
  clp_profile_button_->setIconSize(QSize(25, 25));
  clp_profile_button_->setStyleSheet(QStringLiteral(
      "QPushButton{background:#1e2735;border-radius:15px;color:#dce4ef;font-size:12px;"
      "padding-left:6px;padding-right:6px;}"
      "QPushButton:hover{background:#26334a;}"));
  clp_profile_button_->hide();  // 折叠时由 animateHeader 平移滑入

  // 资料按钮平移动画：折叠滑入（顶部→目标坐标）/ 展开滑出（目标坐标→顶部）
  clp_profile_button_anim_ = new QPropertyAnimation(clp_profile_button_, "pos", this);
  clp_profile_button_anim_->setDuration(300);  // 按钮平移时长（用户指定）
  clp_profile_button_anim_->setEasingCurve(QEasingCurve::InOutQuad);
  connect(clp_profile_button_anim_, &QPropertyAnimation::finished, this, [this] {
    // 展开方向滑出完成后隐藏（折叠方向滑入完成保持显示）
    if (!cl_profile_collapsed_) clp_profile_button_->hide();
  });
}

QWidget* UserUploadsDialog::createProfileWidget() {
  auto* t_widget = new QWidget(clp_shell_);
  auto* t_layout = new QVBoxLayout(t_widget);
  t_layout->setContentsMargins(0, 0, 0, 20);  // 81+8+23+8+20+8+51+20 = 219
  t_layout->setSpacing(8);

  // 81×81 圆头像
  clp_avatar_label_ = new QLabel(t_widget);
  clp_avatar_label_->setFixedSize(81, 81);
  clp_avatar_label_->setAlignment(Qt::AlignCenter);
  t_layout->addWidget(clp_avatar_label_, 0, Qt::AlignHCenter);

  // 昵称 239×23
  clp_name_label_ = new QLabel(t_widget);
  clp_name_label_->setFixedSize(239, 23);
  clp_name_label_->setAlignment(Qt::AlignCenter);
  clp_name_label_->setStyleSheet(
      QStringLiteral("color:#dce4ef;font-size:16px;font-weight:600;"));
  t_layout->addWidget(clp_name_label_, 0, Qt::AlignHCenter);

  // 等级胶囊 + 动态徽章行（行高 20）
  clp_badges_row_ = new QWidget(t_widget);
  clp_badges_row_->setFixedHeight(20);
  auto* t_row_layout = new QHBoxLayout(clp_badges_row_);
  t_row_layout->setContentsMargins(0, 0, 0, 0);
  t_row_layout->setSpacing(6);
  clp_level_label_ = new QLabel(clp_badges_row_);
  clp_level_label_->setFixedSize(25, 13);
  clp_level_label_->setAlignment(Qt::AlignCenter);
  clp_level_label_->setStyleSheet(QStringLiteral(
      "QLabel{background:#2d9df0;color:#ffffff;border-radius:3px;font-size:10px;"
      "font-weight:600;padding:0 2px;}"));
  t_row_layout->addWidget(clp_level_label_);
  t_layout->addWidget(clp_badges_row_, 0, Qt::AlignHCenter);

  // 个性签名 240×51（3 行 wordWrap）
  clp_bio_label_ = new QLabel(t_widget);
  clp_bio_label_->setFixedSize(240, 51);
  clp_bio_label_->setAlignment(Qt::AlignCenter);
  clp_bio_label_->setWordWrap(true);
  clp_bio_label_->setStyleSheet(QStringLiteral("color:#818b9a;font-size:11px;"));
  t_layout->addWidget(clp_bio_label_, 0, Qt::AlignHCenter);

  return t_widget;
}

QWidget* UserUploadsDialog::createTabRow() {
  auto* t_row = new QWidget(clp_shell_);
  t_row->setFixedHeight(39);

  // "上传"标签 86×23
  auto* t_upload_label = new QLabel(tr("上传"), t_row);
  t_upload_label->setGeometry(1, 0, 86, 23);
  t_upload_label->setStyleSheet(
      QStringLiteral("color:#08a8ff;font-size:13px;font-weight:600;"));

  // 下划线随文本墨迹宽度自适应（中英文通用）：宽度 = 前进宽 - 首字左 bearing - 末字右 bearing，
  // 起点对齐首字形墨迹左缘；垂直位置 = 基线 + 5px 间距（基线 = 标签垂直居中下的字体基线）
  clp_upload_underline_ = new QLabel(t_row);
  const QFontMetrics t_fm(t_upload_label->font());
  const QString t_txt = t_upload_label->text();
  if (!t_txt.isEmpty()) {
    const int t_underline_x = 1 + t_fm.leftBearing(t_txt.at(0));
    const int t_underline_w = t_fm.horizontalAdvance(t_txt)
                              - t_fm.leftBearing(t_txt.at(0))
                              - t_fm.rightBearing(t_txt.at(t_txt.size() - 1));
    const int t_underline_y = (23 - t_fm.height()) / 2 + t_fm.ascent() + 5;
    clp_upload_underline_->setGeometry(t_underline_x, t_underline_y, t_underline_w, 2);
  }
  clp_upload_underline_->setStyleSheet(QStringLiteral("background:#08a8ff;"));

  // 状态文本 200×23（随"上传"行右侧显示）
  clp_status_label_ = new QLabel(t_row);
  clp_status_label_->setGeometry(94, 0, 200, 23);
  clp_status_label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  clp_status_label_->setStyleSheet(QStringLiteral("color:#9aa4b2;font-size:12px;"));

  return t_row;
}

// ── 信号连接（互动操作全表） ──

void UserUploadsDialog::initConnections() {
  connect(clp_close_button_, &QPushButton::clicked, this, [this] { close(); });

  // 折叠/展开：滚动 >80 折叠；点资料按钮/滚回顶部展开
  connect(clp_profile_button_, &QPushButton::clicked, this,
          [this] { updateHeaderForScroll(0); });
  connect(clp_view_->verticalScrollBar(), &QScrollBar::valueChanged, this,
          [this](int value) { updateHeaderForScroll(value); });
  // 触底加载下一页
  connect(clp_view_, &CommunityFlowView::loadMoreRequested, this, [this] {
    if (!cl_loading_ && cl_has_more_) loadPage(cl_page_ + 1);
  });

  // ── 数据 ──
  connect(clp_repo_, &UserConfigRepository::userConfigsReady, this,
          [this](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                 const PaginatedResult& pg) { applyConfigs(list, pg); });
  // 头像：列表 item 按 configId 回填；资料区头像为空时同步更新（列表同属该用户）
  connect(clp_repo_, &UserConfigRepository::avatarReady, this,
          [this](int userId, const QPixmap& pm) {
            clp_model_->setField(userId, CommunityModel::AvatarRole, QVariant::fromValue(pm));
            if (cl_profile_.avatar.isNull()) updateProfileAvatar(pm);
          });
  // 列表加载失败（单参数无 action：仅 cl_loading_ 期间视为列表错误）
  connect(clp_repo_, &PaginatedRepository::errorOccurred, this,
          [this](const QString&) {
            if (cl_loading_) {
              cl_loading_ = false;
              // 状态文本仅加载中提示（产品要求），失败不占用状态区
              setStatusText(QString());
            }
          });
  // GET /user/:id：bio + 大神头衔（isExpert 由 titles.contains("expert") 决定）
  connect(clp_user_repo_, &UserRepository::publicUserInfoReady, this,
          [this](const DeSheng::GetPublicUserInfoResponse& info) {
            cl_profile_.bio = info.data.bio;
            cl_profile_.isExpert = info.data.titles.contains(QStringLiteral("expert"));
            if (cl_profile_.userId > 0 && !info.data.avatar.isEmpty())
              clp_repo_->fetchAvatar(cl_profile_.userId, info.data.avatar);
            updateProfileInfo();
          });
  connect(clp_user_repo_, &UserRepository::errorOccurred, this,
          [](const QString& error) {
            LOG_WARN("[UserUploadsDialog] getPublicUserInfo failed: {}", error.toStdString());
          });

  // ── 互动 ──
  connect(clp_delegate_, &CommunityDelegate::iconClicked, this,
          [this](int userId) { clp_model_->toggleExpanded(userId); });
  // 点击列表内头像 → 滚回顶部展开资料区
  connect(clp_delegate_, &CommunityDelegate::avatarClicked, this, [this] {
    clp_view_->verticalScrollBar()->setValue(0);
    updateHeaderForScroll(0);
  });
  connect(clp_delegate_, &CommunityDelegate::likeClicked, this,
          &UserUploadsDialog::onLikeClicked);
  connect(clp_delegate_, &CommunityDelegate::dislikeClicked, this,
          &UserUploadsDialog::onDislikeClicked);
  connect(clp_delegate_, &CommunityDelegate::commentTagClicked, this,
          [this](int configId, int commentId, bool nowClicked) {
            clp_scheme_svc_->toggleCommentClick(configId, commentId, nowClicked);
          });
  connect(clp_delegate_, &CommunityDelegate::downloadClicked, this,
          &UserUploadsDialog::onDownloadClicked);
  connect(clp_delegate_, &CommunityDelegate::shareClicked, this,
          &UserUploadsDialog::onShareClicked);

  // 下载进度 → 委托 + 重绘
  connect(clp_scheme_svc_, &SchemeService::downloadProgress, this,
          [this](int configId, int percent) {
            clp_delegate_->setDownloadProgress(configId, percent);
            clp_view_->viewport()->update();
          });
  // 下载失败（常驻连接，具体原因提示）
  connect(clp_scheme_svc_, &SchemeService::downloadFailed, this,
          [this](int configId, const QString&) {
            clp_delegate_->setDownloadProgress(configId, -1);
            clp_view_->viewport()->update();
            // 状态文本仅加载中提示（产品要求），下载失败不占用状态区
            setStatusText(QString());
          });
  // API 后纠正计数（乐观更新收敛）
  connect(clp_scheme_svc_, &SchemeService::countsSynced, this,
          [this](const DeSheng::GetPublicConfigurationListResponse::ListItem& info) {
            const int t_id = info.id;
            clp_model_->setField(t_id, CommunityModel::LikeCountRole, info.like_count);
            clp_model_->setField(t_id, CommunityModel::DislikeCountRole, info.dislike_count);
            clp_model_->setField(t_id, CommunityModel::DownloadCountRole, info.download_count);
            clp_model_->setField(t_id, CommunityModel::ShareCountRole, info.share_count);
            clp_model_->setField(t_id, CommunityModel::IsLikedRole, info.is_liked);
            clp_model_->setField(t_id, CommunityModel::IsDislikedRole, info.is_disliked);
          });
  // 评论标签失败回滚（与 delegate 乐观更新对称）
  connect(clp_scheme_svc_, &SchemeService::commentClickReverted, this,
          [this](int configId, int commentId) {
            auto t_opt = clp_model_->findById(configId);
            if (!t_opt.has_value()) return;
            for (auto& t_comment : t_opt->comments) {
              if (t_comment.id == commentId) {
                t_comment.is_clicked = !t_comment.is_clicked;
                t_comment.count += t_comment.is_clicked ? 1 : -1;
                clp_model_->setField(configId, CommunityModel::CommentsRole,
                                     QVariant::fromValue(t_opt->comments));
                break;
              }
            }
          });
}

// ── 资料区刷新 ──

void UserUploadsDialog::updateProfileInfo() {
  const QString t_display_name =
      cl_profile_.nickname.isEmpty() ? cl_profile_.username : cl_profile_.nickname;
  // 昵称过长省略号处理：资料区 239px；折叠态按钮 97 - 25(头像 icon) - 6×2(padding) = 60px
  const QString t_elided =
      DeSheng::elideTextWithDots(t_display_name, clp_name_label_->font(), 239);
  clp_name_label_->setText(t_elided);
  // 长昵称省略号贴右（左对齐）；短昵称保持居中与头像/徽章视觉一致
  clp_name_label_->setAlignment(t_elided != t_display_name ? Qt::AlignLeft | Qt::AlignVCenter
                                                           : Qt::AlignCenter);
  clp_profile_button_->setText(
      DeSheng::elideTextWithDots(t_display_name, clp_profile_button_->font(), 60));
  updateProfileAvatar(cl_profile_.avatar);
  updateBadges();
  clp_bio_label_->setText(cl_profile_.bio);
}

void UserUploadsDialog::updateBadges() {
  // 等级胶囊
  clp_level_label_->setVisible(cl_profile_.level > 0);
  if (cl_profile_.level > 0)
    clp_level_label_->setText(QStringLiteral("Lv.%1").arg(cl_profile_.level));

  // 清空旧动态徽章（先删旧再重建；元素为 QPointer，auto* 无法推导，显式裸指针隐式转换）
  for (QLabel *t_badge : clp_badge_labels_) delete t_badge;  // QPointer 自动置空
  clp_badge_labels_.clear();
  auto* t_row_layout = static_cast<QHBoxLayout*>(clp_badges_row_->layout());
  while (t_row_layout->count() > 1) {
    auto* t_item = t_row_layout->takeAt(1);
    delete t_item;
  }

  // 徽章顺序与主页面 delegate 一致：主播 → 官方 → 职业 → 大神
  // 图片存在 → 3px 圆角 clip；缺图 → 纯色胶囊 + 居中文字
  const auto t_add_badge = [this](bool on, const QString& imgPath, int w, int h,
                                  const QString& bg, const QString& fg,
                                  const QString& text) {
    if (!on) return;
    auto* t_label = new QLabel(clp_badges_row_);
    t_label->setFixedSize(w, h);
    t_label->setAlignment(Qt::AlignCenter);
    const QPixmap t_img(imgPath);
    if (!t_img.isNull()) {
      t_label->setPixmap(roundedBadgePixmap(t_img, w, h));
    } else {
      t_label->setText(text);
      t_label->setStyleSheet(QStringLiteral("background:%1;color:%2;border-radius:3px;"
                                            "font-size:10px;font-weight:600;")
                                 .arg(bg, fg));
    }
    static_cast<QHBoxLayout*>(clp_badges_row_->layout())->addWidget(t_label);
    clp_badge_labels_.append(QPointer<QLabel>(t_label));
  };
  t_add_badge(cl_profile_.isStreamer, QStringLiteral(":/Skin/Images/modules/community/host.png"),
              46, 19, QStringLiteral("#e6498f"), QStringLiteral("#ffffff"), tr("主播"));
  t_add_badge(cl_profile_.isOfficial,
              QStringLiteral(":/Skin/Images/modules/community/official.png"), 56, 18,
              QStringLiteral("#10b981"), QStringLiteral("#ffffff"), tr("官方"));
  t_add_badge(cl_profile_.isProfessional,
              QStringLiteral(":/Skin/Images/modules/community/professional.png"), 50, 18,
              QStringLiteral("#8b5cf6"), QStringLiteral("#ffffff"), tr("职业"));
  t_add_badge(cl_profile_.isExpert,
              QStringLiteral(":/Skin/Images/modules/community/god.png"), 53, 20,
              QStringLiteral("#f5c211"), QStringLiteral("#3d2e00"), tr("大神"));

  // 徽章行显示条件：level > 0 || 徽章非空
  const bool t_any_badge = cl_profile_.isStreamer || cl_profile_.isOfficial ||
                           cl_profile_.isProfessional || cl_profile_.isExpert;
  clp_badges_row_->setVisible(cl_profile_.level > 0 || t_any_badge);
}

void UserUploadsDialog::updateProfileAvatar(const QPixmap& pm) {
  if (!pm.isNull()) cl_profile_.avatar = pm;  // 缓存：公开信息刷新时不会回退占位图
  QPixmap t_avatar = cl_profile_.avatar;
  // 头像为空 → 系统默认头像（@2x 图按显示尺寸缩放；不写回缓存，公开信息返回后仍可补拉）
  if (t_avatar.isNull())
    t_avatar =
        QPixmap(QStringLiteral(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"));
  clp_avatar_label_->setPixmap(roundedAvatar(t_avatar, 81));
  clp_profile_button_->setIcon(QIcon(roundedAvatar(t_avatar, 25)));
}

void UserUploadsDialog::setStatusText(const QString& text) {
  // 状态文本仅加载中提示（产品要求）：显示后 0.5s 自动隐藏（singleShot 带 this 上下文，弹窗销毁自动取消）
  clp_status_label_->setText(text);
  clp_status_label_->setVisible(!text.isEmpty() && !cl_profile_collapsed_);  // 折叠态不显示
  if (!text.isEmpty()) {
    QTimer::singleShot(500, this, [this] { setStatusText(QString()); });
  }
}

// ── 数据加载 ──

void UserUploadsDialog::loadPage(int page) {
  if (cl_profile_.userId <= 0) {
    cl_loading_ = false;
    return;  // 用户信息不可用：不提示（状态文本仅加载中）
  }
  cl_loading_ = true;
  cl_page_ = page;
  setStatusText(page == 1 ? tr("加载中...") : tr("加载更多..."));
  QMap<QString, QString> t_filters;
  t_filters.insert(QStringLiteral("device_type"), QStringLiteral("headset"));  // 只拉耳机
  clp_repo_->getUserConfigs(QString::number(cl_profile_.userId), page, kPageSize, t_filters);
}

void UserUploadsDialog::applyConfigs(
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
    const PaginatedResult& pg) {
  const auto t_items = configToItems(list);
  if (pg.page <= 1) {
    clp_model_->replaceAll(t_items);
    setStatusText(QString());  // 空态不提示（状态文本仅加载中）
  } else {
    clp_model_->addItems(t_items);
    // 加载更多结束：清掉"加载更多..."
    setStatusText(QString());
  }
  cl_has_more_ = pg.hasMore();
  fetchAvatarsForList(list);
  cl_loading_ = false;
}

void UserUploadsDialog::fetchAvatarsForList(
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list) {
  for (const auto& t_config : list) {
    if (t_config.author.avatar.isEmpty()) continue;
    clp_repo_->fetchAvatar(t_config.id, t_config.author.avatar);
  }
}

// ── 折叠/展开（资料区直接隐藏；资料按钮顶部平移滑入/滑出） ──

void UserUploadsDialog::updateHeaderForScroll(int value) {
  const bool t_collapsed = value > kHideProfileScroll;
  if (t_collapsed == cl_profile_collapsed_) return;  // 状态守卫防重入
  cl_profile_collapsed_ = t_collapsed;
  if (!t_collapsed) {
    // 展开：守卫防重复插入（折叠动画进行中回滚时 tabRow/view 仍在布局）
    if (clp_shell_layout_->indexOf(clp_tab_row_) == -1) {
      clp_shell_layout_->insertWidget(2, clp_tab_row_);
      clp_shell_layout_->insertWidget(3, clp_view_, 1);
    }
    clp_profile_widget_->setVisible(true);
    if (!clp_status_label_->text().isEmpty()) clp_status_label_->show();
    // 资料按钮：上移滑出到顶部，完成后隐藏（见动画 finished 连接）
    clp_profile_button_anim_->stop();
    clp_profile_button_anim_->setStartValue(clp_profile_button_->pos());
    clp_profile_button_anim_->setEndValue(QPoint(kProfileBtnX, -kProfileBtnH));
    clp_profile_button_anim_->start();
  }
  animateHeader(t_collapsed);
}

void UserUploadsDialog::animateHeader(bool collapsed) {
  if (collapsed) {
    // 折叠（用户定稿：资料区直接隐藏，无动画）：tabRow/view 移出布局瞬间定位，按钮从顶部平移滑入
    clp_profile_widget_->hide();
    clp_status_label_->hide();
    clp_shell_layout_->removeWidget(clp_tab_row_);
    clp_tab_row_->setGeometry(26, kTabRowCollapsedY, kTabRowCollapsedW, 39);
    clp_tab_row_->raise();  // 防被 view 遮挡
    clp_shell_layout_->removeWidget(clp_view_);
    clp_view_->setGeometry(26, kViewCollapsedY, 356, kViewCollapsedH);
    clp_view_->show();
    clp_profile_button_->setFixedSize(kProfileBtnW, kProfileBtnH);
    clp_profile_button_->show();
    clp_profile_button_->move(kProfileBtnX, -kProfileBtnH);
    clp_profile_button_anim_->stop();
    clp_profile_button_anim_->setStartValue(QPoint(kProfileBtnX, -kProfileBtnH));
    clp_profile_button_anim_->setEndValue(QPoint(kProfileBtnX, kProfileBtnY));
    clp_profile_button_anim_->start();
  } else {
    // 展开：资料区恢复自然高度（布局位置由 updateHeaderForScroll 插入）；按钮滑出由 updateHeaderForScroll 触发
    clp_profile_widget_->setMinimumHeight(0);
    clp_profile_widget_->setMaximumHeight(QWIDGETSIZE_MAX);
  }
}

// ── 互动操作（乐观更新 + 服务端收敛） ──

void UserUploadsDialog::onLikeClicked(int userId, bool liked) {
  if (liked) {
    auto t_opt = clp_model_->findById(userId);
    if (t_opt.has_value() && t_opt->isDisliked) {
      // 与踩互斥：先取消踩
      clp_model_->setField(userId, CommunityModel::IsDislikedRole, false);
      clp_model_->setField(userId, CommunityModel::DislikeCountRole,
                           (std::max)(0, t_opt->dislikeCount - 1));
      clp_scheme_svc_->toggleDislike(userId, true);
    }
  }
  clp_model_->setField(userId, CommunityModel::IsLikedRole, liked);
  auto t_opt = clp_model_->findById(userId);
  if (t_opt.has_value()) {
    const int t_count =
        liked ? (std::max)(0, t_opt->likeCount + 1) : (std::max)(0, t_opt->likeCount - 1);
    clp_model_->setField(userId, CommunityModel::LikeCountRole, t_count);
  }
  clp_scheme_svc_->toggleLike(userId, !liked);  // service 参数 = 原状态
}

void UserUploadsDialog::onDislikeClicked(int userId, bool disliked) {
  if (disliked) {
    auto t_opt = clp_model_->findById(userId);
    if (t_opt.has_value() && t_opt->isLiked) {
      // 与点赞互斥：先取消赞
      clp_model_->setField(userId, CommunityModel::IsLikedRole, false);
      clp_model_->setField(userId, CommunityModel::LikeCountRole,
                           (std::max)(0, t_opt->likeCount - 1));
      clp_scheme_svc_->toggleLike(userId, true);
    }
  }
  clp_model_->setField(userId, CommunityModel::IsDislikedRole, disliked);
  auto t_opt = clp_model_->findById(userId);
  if (t_opt.has_value()) {
    const int t_count =
        disliked ? (std::max)(0, t_opt->dislikeCount + 1) : (std::max)(0, t_opt->dislikeCount - 1);
    clp_model_->setField(userId, CommunityModel::DislikeCountRole, t_count);
  }
  clp_scheme_svc_->toggleDislike(userId, !disliked);
}

void UserUploadsDialog::onDownloadClicked(int userId) {
  clp_delegate_->setDownloadProgress(userId, 0);
  clp_view_->viewport()->update();
  // 完成：进度 100 → 导入方案库 → 计数收敛 → 500ms 后进度恢复（校验 cid 防串扰）
  connectOnce(clp_scheme_svc_, &SchemeService::downloadFileSaved, this,
              [this, userId](int cid, const QString& t_file_path) {
                if (cid != userId) return;
                clp_delegate_->setDownloadProgress(userId, 100);
                clp_view_->viewport()->update();
                if (m) m->importDownloadedPlan(t_file_path);
                clp_scheme_svc_->refreshCounts(userId);
                QTimer::singleShot(500, this, [this, userId] {
                  clp_delegate_->setDownloadProgress(userId, -1);
                  clp_view_->viewport()->update();
                });
                setStatusText(QString());  // 仅加载中提示，下载完成不显示
              });
  connectOnce(clp_scheme_svc_, &SchemeService::errorOccurred, this,
              [this, userId](const QString&, const QString&) {
                clp_delegate_->setDownloadProgress(userId, -1);
                clp_view_->viewport()->update();
                setStatusText(QString());  // 仅加载中提示，下载失败不显示
              });
  clp_scheme_svc_->download(userId);
}

void UserUploadsDialog::onShareClicked(int userId) {
  // 先断开旧的 shareCodeReady（分享专用，安全）— 快速连点分享时防旧响应覆盖新剪贴板
  QObject::disconnect(clp_scheme_svc_, &SchemeService::shareCodeReady, this, nullptr);
  connectOnce(clp_scheme_svc_, &SchemeService::shareCodeReady, this,
              [this, userId](const QString& shareCode, int shareCount) {
                QString t_share_text = "sq" + shareCode;
                auto t_opt = clp_model_->findById(userId);
                if (t_opt.has_value()) {
                  t_share_text = t_opt->planName + "+" + t_opt->deviceName + "+" +
                                 t_opt->tags.join("+") + "+sq" + shareCode;
                  clp_model_->setField(userId, CommunityModel::ShareCountRole, shareCount);
                }
                clp_scheme_svc_->refreshCounts(userId);
                QApplication::clipboard()->setText(t_share_text);
                setStatusText(QString());  // 仅加载中提示，分享成功不显示
              });
  connectOnce(clp_scheme_svc_, &SchemeService::errorOccurred, this,
              [this](const QString&, const QString&) {
                // 状态文本仅加载中提示（产品要求），分享失败不占用状态区
                setStatusText(QString());
              });
  clp_scheme_svc_->share(userId);
}

// ── 绘制 ──

/// \brief 圆形头像 — 椭圆路径 clip + KeepAspectRatioByExpanding；空图绿底渐变圆占位
QPixmap UserUploadsDialog::roundedAvatar(const QPixmap& pixmap, int size) {
  QPixmap t_result(size, size);
  t_result.fill(Qt::transparent);
  QPainter t_painter(&t_result);
  t_painter.setRenderHint(QPainter::Antialiasing);
  t_painter.setRenderHint(QPainter::SmoothPixmapTransform);
  if (pixmap.isNull()) {
    t_painter.setBrush(QColor(QStringLiteral("#2f7d32")));
    t_painter.setPen(Qt::NoPen);
    t_painter.drawEllipse(0, 0, size, size);
    QRadialGradient t_gradient(size / 2.0, size / 2.0, size * 0.42);
    t_gradient.setColorAt(0.0, QColor(80, 180, 74, 180));
    t_gradient.setColorAt(1.0, QColor(80, 180, 74, 0));
    t_painter.setBrush(t_gradient);
    t_painter.drawEllipse(int(size * 0.08), int(size * 0.08), int(size * 0.84),
                          int(size * 0.84));
  } else {
    const QPixmap t_scaled = pixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation);
    QPainterPath t_path;
    t_path.addEllipse(0, 0, size, size);
    t_painter.setClipPath(t_path);
    t_painter.drawPixmap((size - t_scaled.width()) / 2, (size - t_scaled.height()) / 2,
                         t_scaled);
  }
  return t_result;
}

/// \brief 徽章整图 3px 圆角 clip
QPixmap UserUploadsDialog::roundedBadgePixmap(const QPixmap& pixmap, int w, int h) {
  const QPixmap t_scaled = pixmap.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  QPixmap t_result(w, h);
  t_result.fill(Qt::transparent);
  QPainter t_painter(&t_result);
  t_painter.setRenderHint(QPainter::Antialiasing);
  QPainterPath t_path;
  t_path.addRoundedRect(0, 0, w, h, 3, 3);
  t_painter.setClipPath(t_path);
  t_painter.drawPixmap(0, 0, t_scaled);
  return t_result;
}
