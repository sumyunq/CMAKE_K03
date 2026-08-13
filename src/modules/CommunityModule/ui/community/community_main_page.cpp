#include "modules/CommunityModule/ui/community/community_main_page.h"

#include <QShowEvent>
#include <QVBoxLayout>

#include "modules/Common/AppImageCache.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"
#include "network/auth_store.h"
#include "network/server_router.h"
#include "repository/user_config_repository.h"
#include "modules/CommunityModule/service/scheme_service.h"
#include "modules/CommunityModule/ui/community_page_widget.h"
#include "LoadLib.h"  ///< g_user_information（面板模糊/透明度设置）

CommunityMainPage::CommunityMainPage(QWidget* parent) : QWidget(parent) { initUi(); }

void CommunityMainPage::initUi() {
  // 共享 Service（唯一实例 — 社区页与个人中心双向同步的基础）
  clp_config_repo_ = new UserConfigRepository(this);
  clp_scheme_svc_ = new SchemeService(this);
  clp_scheme_svc_->init(clp_config_repo_);

  // 三 Tab 社区页（广场/大神/官方）— 全宽铺满
  // 已上传/已点赞列表由 K03 personal_center_settings_main_page 右侧承担，此处不建右侧面板
  clp_page_ = new CommunityPageWidget(this);
  clp_page_->injectServices(clp_scheme_svc_, clp_config_repo_);
  connect(clp_page_, &CommunityPageWidget::statusMessage, this,
          &CommunityMainPage::statusMessage);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(clp_page_);
}

// ── 使用前必调 ──

void CommunityMainPage::setServer(const QString& key, const QString& baseUrl) {
  auto& router = ServerRouter::instance();
  router.registerServer(key, baseUrl);
  router.setDefaultServer(key);
}

void CommunityMainPage::useServer(const QString& key) {
  ServerRouter::instance().setDefaultServer(key);
}

void CommunityMainPage::setAuthToken(const QString& token) {
  AuthStore::instance().setToken(token);
}

void CommunityMainPage::clearAuthToken() { AuthStore::instance().clear(); }

// ── 数据加载 ──

void CommunityMainPage::loadInitialData() {
  if (clp_page_) clp_page_->loadInitialData();
}

void CommunityMainPage::refreshCurrentTab() {
  if (clp_page_) clp_page_->refreshCurrentTab();
}

// ── 内部组件访问 ──

CommunityPageWidget* CommunityMainPage::pageWidget() const { return clp_page_; }

void CommunityMainPage::LanguageSet() {
  if (clp_page_) clp_page_->LanguageSet();
}

// ── 毛玻璃快照保障 ──

void CommunityMainPage::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  ensureBlurBackdrop();  // 每次显示都尝试（内部已就绪则跳过）
}

void CommunityMainPage::ensureBlurBackdrop() {
  // K03 启动时模糊快照可能未生成（只在设置变化时重算）— 显示时主动触发
  auto& cache = AppImageCache::instance();
  if (!cache.cl_background_blurred_cache_.isNull()) {
    LOG_DEBUG("[Blur] backdrop already ready: {}x{}", cache.cl_background_blurred_cache_.width(),
              cache.cl_background_blurred_cache_.height());
    cl_blur_ensured_ = true;
    return;
  }
  QWidget* win = window();
  if (!win || win->size().isEmpty()) {
    LOG_DEBUG("[Blur] skip: win null or size empty");
    return;  // 尺寸未就绪 → 下次 showEvent 重试
  }
  // 模糊半径：跟随 K03 设置，但下限兜底 — 默认 0.02×25≈1px 几乎无模糊，用 16px 保证可见效果
  // 最低模糊半径 1（模糊值不允许为 0）；存储值 0..1 与滑块往返一致
  int radius = qMax(1, qRound(g_user_information.local.panel_blur_radius_ * 25.0));
  if (radius < 12) radius = 16;
  LOG_DEBUG("[Blur] generating: size={}x{} radius={}", win->width(), win->height(), radius);
  cache.updateBlurredBackdrop(win->size(), radius,
                              g_user_information.local.panel_opacity_);
  cl_blur_ensured_ = true;
}
