#ifndef COMMUNITY_PAGE_WIDGET_H
#define COMMUNITY_PAGE_WIDGET_H

#include <QHash>
#include <QVector>
#include <QWidget>

#include "data/userConfig/user_config_api.h"  ///< ListItem（原 entity/user_config.h 的 UserConfigInfo）

class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class CustomQWidgetLoading;
class CommunityPanel;
class CommunityModel;
class SchemeFilterPopup;
class SchemeService;
class UserConfigRepository;

namespace CommunityBottomStatusAssets {
inline constexpr const char* kLeftIconPath = ":/Skin/Images/Community/null_label.png";//底部左边图标
inline constexpr const char* kRightIconPath = ":/Skin/Images/Community/null_label.png";//底部右边图标
}  // namespace CommunityBottomStatusAssets

/// \brief Tab 状态封装 — 消除 4 处 switch(cl_active_tab_)
struct TabState {
  CommunityModel* model = nullptr;
  int page = 1;
  bool hasMore = true;
  bool fetching = false;
};

/// \brief 社区页面 — 封装 Panel + 筛选栏 + Service + 分页
/// 自包含 Widget，可嵌入 QMainWindow / QDialog / QStackedWidget
class CommunityPageWidget : public QWidget {
  Q_OBJECT

public:
  explicit CommunityPageWidget(QWidget* parent = nullptr);
  ~CommunityPageWidget() override = default;

  /// \brief 语言切换：刷新 Tab 按钮/筛选弹窗/面板视图文本
  void LanguageSet();

  /// \brief 注入共享 Service（由 CommunityMainPage 创建后传入，替代自建）
  void injectServices(class SchemeService* svc, class UserConfigRepository* repo);
  /// \brief 注入 Auth token 后首次加载数据 / Load initial data after login
  void loadInitialData();
  /// \brief 刷新当前 Tab / Refresh current tab
  void refreshCurrentTab();
  /// \brief 获取配置仓库（供右侧面板复用）/ Get config repository
  class UserConfigRepository* configRepo() const;
  /// \brief 获取方案 Service（供右侧面板复用）/ Get scheme service
  class SchemeService* schemeService() const;
  /// \brief 获取社区面板（供下载进度等调用）/ Get community panel
  class CommunityPanel* communityPanel() const;
  /// \brief 获取左侧三个 Model / Get left-side models (0=hot, 1=new, 2=follow)
  class CommunityModel* leftModel(int tab) const;

signals:
  /// \brief 状态消息 → MainWindow statusBar
  void statusMessage(const QString& message, int timeoutMs);

private:
  void initUi();
  void initConnections();

  QWidget* createToolBar();
  QWidget* createRegionSelector();
  void addFilterWidgets(QHBoxLayout* hbox);
  void updateScrollButtons();          ///< 按当前视图滚动条更新回顶/刷新按钮显隐
  QPushButton* makeSmallButton(const QString& iconNo, const QString& iconHo, int size,
                               QWidget* parent);  ///< 方形图标按钮（回顶/刷新，no/ho 两态）
  void onUploadPlanClicked();          ///< 上传方案（与个人中心逻辑一致）
  void onSortClicked();                ///< 筛选弹窗（排序/场景/机型分组过滤）
  void ensureFilterPopup();            ///< 懒创建筛选弹窗并接线（filterChanged/filtersReset → 刷新）
  void positionFloatButtons();         ///< 浮动按钮右下角定位

  void fetchAvatarsForList(const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                           CommunityModel* model);
  void showListEmptyState();  ///< 列表空态：搜索中 → searchPlanEmpty 图 + "未搜索到该方案"；否则文字"暂无数据"
  void showStatus(const QString& text, int autoHideMs = 0);
  void showLoadingStatus();  ///< 底部加载中：左侧动画 + 右侧文字
  void hideLoadingStatus();
  void configureStatusLoading(const QString& text);
  void clearStatus();
  void updateNoMoreStatus();
  void updateStatusDecoration(bool visible);

  QString currentDeviceType() const;
  QString currentSort() const;
  QString currentKeyword() const;

  // ── UI ──
  CommunityPanel* clp_community_ = nullptr;
  QWidget* clp_status_bar_ = nullptr;
  QLabel* clp_status_left_icon_ = nullptr;
  QLabel* clp_status_label_ = nullptr;               ///< 底部状态文本
  QLabel* clp_status_right_icon_ = nullptr;
  CustomQWidgetLoading* clp_status_loading_ = nullptr; ///< 底部加载中动画
  QTimer* clp_status_timer_ = nullptr;               ///< 状态栏自动隐藏定时器
  QPushButton *clp_btn_set1_ = nullptr, *clp_btn_set2_ = nullptr, *clp_btn_set3_ = nullptr;
  QPushButton *clp_btn_sort_ = nullptr;              ///< 筛选按钮（点击弹筛选浮层）
  QLineEdit* clp_keyword_edit_ = nullptr;
  SchemeFilterPopup* clp_filter_popup_ = nullptr;    ///< 筛选弹窗（懒创建；筛选值唯一来源，页面不镜像）
  QPushButton *clp_btn_top_float_ = nullptr;         ///< 浮动回顶 40×40（右下角，滚动条≠0 显示）
  QPushButton *clp_btn_refresh_float_ = nullptr;     ///< 浮动刷新 40×40（右下角，常显）
  QPushButton *clp_btn_upload_ = nullptr;            ///< 上传方案
  QPushButton *clp_btn_domestic_ = nullptr;          ///< 国内（默认选中）
  QPushButton *clp_btn_overseas_ = nullptr;          ///< 海外（暂隐藏）

  // ── Service ──
  SchemeService* clp_scheme_svc_ = nullptr;
  UserConfigRepository* clp_config_repo_ = nullptr;

  // ── Tabs ──
  int cl_active_tab_ = 0;
  QVector<TabState> cl_tabs_;  ///< [0]=方案广场, [1]=大神分享, [2]=官方预设
  QHash<int, CommunityPanel*> cl_download_targets_;  ///< configId → 下载进度面板（按 id 分发，避免 disconnect-all 竞态）

  // QWidget interface
protected:
  void resizeEvent(QResizeEvent* event) override;
};

#endif  // COMMUNITY_PAGE_WIDGET_H
