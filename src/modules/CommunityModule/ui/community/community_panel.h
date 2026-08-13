#ifndef COMMUNITY_PANEL_H
#define COMMUNITY_PANEL_H

#include <QWidget>

class QStackedWidget;
class QRect;
class QShowEvent;
class CommunityFlowView;
class CommunityModel;
class CommunityDelegate;
struct CommunityItemData;

/// \brief 社区面板 — 3 个 View + QStackedWidget / 3-view facade with stacked widget
class CommunityPanel : public QWidget {
  Q_OBJECT

public:
  explicit CommunityPanel(QWidget* parent = nullptr);
  ~CommunityPanel() override = default;

  // ── 初始化 ──
  /// \brief 为指定 tab 创建 View+Delegate 并绑定 model / Create view for tab, bind model
  void initView(int tab, CommunityModel* model);

  // ── Tab 切换 ──
  /// \brief 切换到指定 tab / Switch to tab (0=square, 1=expert, 2=official)
  void setCurrentTab(int tab);
  /// \brief 返回当前激活 tab / Return current tab index
  int currentTab() const;

  // ── View 操作 ──
  /// \brief 强制刷新布局（-1=所有, 0/1/2=指定 tab）
  void refreshView(int tab = -1);
  /// \brief 语言切换：重绘 3 个视图（delegate 绘制期 tr() 文本随新语言生效）
  void LanguageSet();
  /// \brief 设置下载进度（转发到 3 个 delegate）
  /// \brief 获取指定 tab 的 delegate / Get delegate for tab (0/1/2)
  CommunityDelegate* delegateForTab(int tab) const;
  /// \brief 获取指定 tab 的垂直滚动条（回顶/刷新显隐控制用）
  class QScrollBar* scrollBarForTab(int tab) const;
  void setDownloadProgress(int userId, int percent);
  /// \brief 空态/错误态覆盖层（暂无数据 / 加载失败点击重试）
  class CommunityStateOverlay* stateOverlay() const { return clp_state_overlay_; }

  /// \brief 置顶上限：一个用户最多置顶的方案数（达到后"置顶"菜单项灰掉）
  static constexpr int kPinnedLimit = 3;

signals:
  void avatarClicked(int userId);
  void iconClicked(int userId);
  void likeToggled(int userId, bool liked);
  void dislikeToggled(int userId, bool disliked);
  void downloadRequested(int userId);
  void shareRequested(int userId);
  void commentTagClicked(int configId, int commentId, bool nowClicked);
  void loadMoreRequested(int tab); ///< 带 tab 标识（防 tab 切换竞态加载错页）
  /// \brief 删除方案请求 / Config delete requested from more menu
  void deleteRequested(int userId);
  /// \brief 置顶/取消置顶请求（pin=true 置顶）/ Pin or unpin requested
  void pinRequested(int userId, bool pin);
  /// \brief 仅自己可见/公开请求（privateOnly=true 仅自己可见）/ Visibility toggle requested
  void visibilityRequested(int userId, bool privateOnly);

private:
  void initUi();
  void initConnectionsForTab(int tab, CommunityModel* model);
  void showEvent(QShowEvent* event) override;    ///< 页面重新显示时同步空态覆盖层几何
  void resizeEvent(QResizeEvent* event) override;  ///< 覆盖层跟随面板尺寸
  /// \brief 弹出更多操作菜单（锚定 more 按钮下方 6px、右缘对齐）/ Show more menu
  void showMoreMenu(int userId, const QRect& btnRect);

  QStackedWidget* clp_stack_ = nullptr;
  class CommunityStateOverlay* clp_state_overlay_ = nullptr;  ///< 空态/错误态覆盖层
  CommunityFlowView* clp_views_[3] = {nullptr, nullptr, nullptr};
  CommunityDelegate* clp_delegates_[3] = {nullptr, nullptr, nullptr};
  CommunityModel* clp_models_[3] = {nullptr, nullptr, nullptr};
  int cl_current_tab_ = 0;
};

#endif  // COMMUNITY_PANEL_H
