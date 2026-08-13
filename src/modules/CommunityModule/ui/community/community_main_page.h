#ifndef COMMUNITY_MAIN_PAGE_H
#define COMMUNITY_MAIN_PAGE_H

#include <QWidget>

class CommunityPageWidget;
class QShowEvent;

/// \brief 社区模块主页面 — 自包含入口（K03 user_setting_main_page 模式）
///
/// 使用方式：
/// \code
/// auto* page = new CommunityMainPage(parent);
/// page->setServer("domestic", "https://hubsys.xiberia.net/api/v1");  // ① 目标服务器
/// page->setAuthToken(token);                                          // ② 登录 token
/// page->loadInitialData();                                            // ③ 加载数据
/// \endcode
///
/// 内部自建：CommunityPageWidget（方案广场/大神分享/官方预设 + 筛选栏）。
/// 已上传/已点赞列表由 K03 personal_center_settings_main_page 右侧承担，本类不建右侧面板。
/// 不依赖 MainWindow / AppBootstrap，可直接嵌入任何 QWidget 容器。
class CommunityMainPage : public QWidget {
  Q_OBJECT

public:
  explicit CommunityMainPage(QWidget* parent = nullptr);
  ~CommunityMainPage() override = default;

  // ── 使用前必调 ──
  /// \brief 设置目标服务器（key + baseUrl），并设为默认
  void setServer(const QString& key, const QString& baseUrl);
  /// \brief 切换到已注册的服务器
  void useServer(const QString& key);
  /// \brief 设置登录 token（未设置则 API 请求返回 401）
  void setAuthToken(const QString& token);
  /// \brief 清除 token（退出登录时调用）
  void clearAuthToken();

  // ── 数据加载 ──
  /// \brief 登录后加载初始数据（三 Tab）
  void loadInitialData();
  /// \brief 刷新当前 Tab
  void refreshCurrentTab();

  // ── 语言切换 ──
  /// \brief 语言切换：下传至社区页面（Tab/筛选弹窗/面板视图）
  void LanguageSet();

  // ── 内部组件访问（可选，高级用途） ──
  CommunityPageWidget* pageWidget() const;
  /// \brief 共享 SchemeService（MainWindow 转发给个人中心用）
  class SchemeService* schemeService() const { return clp_scheme_svc_; }
  /// \brief 共享 UserConfigRepository（同上）
  class UserConfigRepository* configRepo() const { return clp_config_repo_; }

signals:
  /// \brief 状态消息（可转发到宿主 statusBar）
  void statusMessage(const QString& message, int timeoutMs);

protected:
  /// \brief 首次显示时确保毛玻璃模糊快照就绪 / Ensure blur backdrop on first show
  void showEvent(QShowEvent* event) override;

private:
  void initUi();
  void ensureBlurBackdrop();

  CommunityPageWidget* clp_page_ = nullptr;
  class SchemeService* clp_scheme_svc_ = nullptr;         ///< 共享方案服务（社区页 + 个人中心双向同步基础）
  class UserConfigRepository* clp_config_repo_ = nullptr; ///< 共享配置仓库
  bool cl_blur_ensured_ = false;
};

#endif  // COMMUNITY_MAIN_PAGE_H
