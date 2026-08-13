#include "modules/CommunityModule/ui/community/community_panel.h"

#include <QMenu>
#include <QScrollBar>
#include <QShowEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "modules/CommunityModule/ui/community/community_delegate.h"
#include "modules/CommunityModule/ui/community/community_flow_view.h"
#include "modules/CommunityModule/ui/community/community_model.h"
#include "modules/CommunityModule/ui/community/community_state_overlay.h"
#include "model/community_item_data.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"

CommunityPanel::CommunityPanel(QWidget* parent) : QWidget(parent) { initUi(); }

void CommunityPanel::initUi() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  clp_stack_ = new QStackedWidget(this);
  layout->addWidget(clp_stack_);

  // 空态/错误态覆盖层（置顶，随面板尺寸）
  clp_state_overlay_ = new CommunityStateOverlay(this);
  clp_state_overlay_->setGeometry(rect());
}

void CommunityPanel::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  if (clp_state_overlay_) {
    clp_state_overlay_->setGeometry(rect());
    clp_state_overlay_->refreshLayout();
  }
}

void CommunityPanel::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  if (clp_state_overlay_) {
    clp_state_overlay_->setGeometry(rect());
    clp_state_overlay_->refreshLayout();
  }
}

void CommunityPanel::initView(int tab, CommunityModel* model) {
  if (tab < 0 || tab > 2 || !model) return;

  clp_models_[tab] = model;

  auto* view = new CommunityFlowView(clp_stack_);
  auto* delegate = new CommunityDelegate(view);

  view->setModel(model);
  view->setItemDelegate(delegate);

  clp_views_[tab] = view;
  clp_delegates_[tab] = delegate;

  clp_stack_->addWidget(view);

  initConnectionsForTab(tab, model);
}

void CommunityPanel::initConnectionsForTab(int tab, CommunityModel* model) {
  auto* delegate = clp_delegates_[tab];
  auto* view = clp_views_[tab];

  // 展开/收起 → 操作绑定的 model
  connect(delegate, &CommunityDelegate::iconClicked, this, [this, model](int userId) {
    LOG_DEBUG("[Panel] iconClicked userId: {} tab: {}", userId, cl_current_tab_);
    model->toggleExpanded(userId);
    emit iconClicked(userId);
  });

  // 点赞（K03: 与踩互斥）
  connect(delegate, &CommunityDelegate::likeClicked, this, [this, model](int userId, bool liked) {
    if (liked) {
      auto opt = model->findById(userId);
      if (opt.has_value() && opt->isDisliked) {
        model->setField(userId, CommunityModel::IsDislikedRole, false);
        model->setField(userId, CommunityModel::DislikeCountRole, (std::max)(0, opt->dislikeCount - 1));
        emit dislikeToggled(userId, false);
      }
    }
    model->setField(userId, CommunityModel::IsLikedRole, liked);
    auto opt = model->findById(userId);
    if (opt.has_value()) {
      const int c = liked ? (std::max)(0, opt->likeCount + 1) : (std::max)(0, opt->likeCount - 1);
      model->setField(userId, CommunityModel::LikeCountRole, c);
    }
    emit likeToggled(userId, liked);
  });

  // 踩（K03: 与点赞互斥）
  connect(delegate, &CommunityDelegate::dislikeClicked, this, [this, model](int userId, bool disliked) {
    if (disliked) {
      auto opt = model->findById(userId);
      if (opt.has_value() && opt->isLiked) {
        model->setField(userId, CommunityModel::IsLikedRole, false);
        model->setField(userId, CommunityModel::LikeCountRole, (std::max)(0, opt->likeCount - 1));
        emit likeToggled(userId, false);
      }
    }
    model->setField(userId, CommunityModel::IsDislikedRole, disliked);
    auto opt = model->findById(userId);
    if (opt.has_value()) {
      const int c = disliked ? (std::max)(0, opt->dislikeCount + 1) : (std::max)(0, opt->dislikeCount - 1);
      model->setField(userId, CommunityModel::DislikeCountRole, c);
    }
    emit dislikeToggled(userId, disliked);
  });

  // 透传信号
  connect(delegate, &CommunityDelegate::avatarClicked, this, &CommunityPanel::avatarClicked);
  connect(delegate, &CommunityDelegate::downloadClicked, this, &CommunityPanel::downloadRequested);
  connect(delegate, &CommunityDelegate::shareClicked, this, &CommunityPanel::shareRequested);
  connect(delegate, &CommunityDelegate::commentTagClicked, this, &CommunityPanel::commentTagClicked);

  // 无限滚动（带 tab 标识，防止 tab 切换后旧 view 的滚动事件串加载到新 tab）
  connect(view, &CommunityFlowView::loadMoreRequested, this,
          [this, tab] { emit loadMoreRequested(tab); });

  // 更多操作菜单（ZoneMore）— 锚定 more 按钮，逻辑见 showMoreMenu
  connect(delegate, &CommunityDelegate::moreClicked, this,
          [this](int userId, const QRect& btnRect) { showMoreMenu(userId, btnRect); });
}

/// \brief 弹出更多操作菜单（样式与 CustomQWidgetSinglePlans 的操作菜单一致）
void CommunityPanel::showMoreMenu(int userId, const QRect& btnRect) {
  auto* model = clp_models_[cl_current_tab_];
  if (!model) return;

  auto* menu = new QMenu(this);
  // 去掉投影 + 窗口边框（关键）
  menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
  // 允许透明背景
  menu->setAttribute(Qt::WA_TranslucentBackground);

  menu->setFixedSize(127, 95);
  menu->setStyleSheet(R"(
    QMenu {
        background-color: #0D0F14;
        border-radius: 6px;
        border: none;              /* ← 清除可能残留的边框绘制 */
        padding: 6px;
        margin: 4px;
    }
    QMenu::item {
        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 12px;
        color: #A1A8B3;
        min-width: 100px;
        min-height: 25px;
        padding-left: 6px;
        border-radius: 4px;
    }
    QMenu::item:enabled:selected {
        background-color: rgba(255, 255, 255, 0.1);
    }
    QMenu::item:disabled {
        color: #687079;              /* 置顶已达上限时灰显（悬浮无背景色） */
    }
QMenu::icon, QMenu::indicator {
    max-width: 0px;
    margin: 0px;
    padding: 0px;
    border: none;
}
)");

  menu->addAction(tr("删除"), this, [this, model, userId] {
    model->removeItem(userId);
    emit deleteRequested(userId);
  });
  // 置顶/取消置顶 — 文本按当前置顶态动态显示（K03: POST/DELETE /user-configs/:id/pin）
  const auto opt = model->findById(userId);
  const bool pinned = opt.has_value() && opt->isPinned;
  auto* pinAction = menu->addAction(pinned ? tr("取消置顶") : tr("置顶"), this,
                  [this, userId, pinned] { emit pinRequested(userId, !pinned); });
  // 置顶上限：一个用户最多置顶 kPinnedLimit 个自己的方案 — 已置顶满时未置顶方案的"置顶"灰掉
  // （本地实时统计 model 的 IsPinnedRole，置顶/取消置顶经 DataSync 同步后立即可见，无需异步查询）
  if (!pinned) {
    int t_pinned_cnt = 0;
    for (int t_i = 0; t_i < model->rowCount(); ++t_i)
      if (model->data(model->index(t_i), CommunityModel::IsPinnedRole).toBool())
        ++t_pinned_cnt;
    if (t_pinned_cnt >= kPinnedLimit)
      pinAction->setEnabled(false);
  }
  // 仅自己可见/设为公开 — 文本按当前可见性动态显示（PUT /user-configs/:id 更新 visibility）
  const bool privateOnly = opt.has_value() && opt->visibility == DeSheng::kVisibilityPrivate;
  menu->addAction(privateOnly ? tr("设为公开") : tr("仅自己可见"), this,
                  [this, userId, privateOnly] { emit visibilityRequested(userId, !privateOnly); });
  // 菜单在更多按钮下方 6px，右缘与按钮右缘对齐
  const QPoint t_pos(btnRect.right() - menu->width() + 1, btnRect.bottom() + 6);
  menu->popup(t_pos);
}

void CommunityPanel::setCurrentTab(int tab) {
  if (tab < 0 || tab > 2) return;
  cl_current_tab_ = tab;
  clp_stack_->setCurrentIndex(tab);
}

int CommunityPanel::currentTab() const { return cl_current_tab_; }

void CommunityPanel::LanguageSet() {
  // 重绘视图：delegate 在 paint() 中调用 tr()（徽章/展开收起等），语言切换后重绘即刷新
  for (int i = 0; i < 3; ++i) {
    if (clp_views_[i]) clp_views_[i]->viewport()->update();
  }
  // 覆盖层"刷新"按钮文本
  if (clp_state_overlay_) clp_state_overlay_->LanguageSet();
}

void CommunityPanel::refreshView(int tab) {
  if (tab == -1) {
    for (int i = 0; i < 3; ++i) {
      if (clp_views_[i]) {
        clp_views_[i]->calculateLayout();
        clp_views_[i]->viewport()->update();
      }
    }
  } else if (tab >= 0 && tab <= 2 && clp_views_[tab]) {
    clp_views_[tab]->calculateLayout();
    clp_views_[tab]->viewport()->update();
  }
}

CommunityDelegate* CommunityPanel::delegateForTab(int tab) const {
  return (tab >= 0 && tab < 3) ? clp_delegates_[tab] : nullptr;
}

QScrollBar* CommunityPanel::scrollBarForTab(int tab) const {
  return (tab >= 0 && tab < 3 && clp_views_[tab]) ? clp_views_[tab]->verticalScrollBar() : nullptr;
}

void CommunityPanel::setDownloadProgress(int userId, int percent) {
  for (int i = 0; i < 3; ++i) {
    if (clp_delegates_[i]) clp_delegates_[i]->setDownloadProgress(userId, percent);
  }
  // 只刷新可见 view
  if (clp_views_[cl_current_tab_]) clp_views_[cl_current_tab_]->viewport()->update();
}
