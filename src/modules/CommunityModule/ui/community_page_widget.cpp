#include "modules/CommunityModule/ui/community_page_widget.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include "modules/CommunityModule/ui/community/community_delegate.h"  ///< delegateForTab 返回完整类型（setPinnedBarEnabled）
#include "modules/CommunityModule/ui/community/community_model.h"
#include "modules/CommunityModule/ui/community/community_panel.h"
#include "modules/CommunityModule/ui/community/community_state_overlay.h"
#include "modules/CommunityModule/ui/community/scheme_filter_popup.h"
#include "modules/CommunityModule/ui/community/user_uploads_dialog.h"  ///< 头像点击弹窗（UserProfile）
#include "model/community_item_data.h"
#include "data/userConfig/user_config_api.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"
#include "modules/CommunityModule/infrastructure/compat/qt_compat.h"
#include "network/http_client.h"
#include "network/request_options.h"
#include "network/server_router.h"
#include "repository/paginated_repository.h"
#include "repository/user_config_repository.h"
#include "modules/CommunityModule/service/scheme_service.h"
#include "LoadLib.h" ///< extern MainWindow *m（importDownloadedPlan）
#include "APOThread/ApoManager.h" ///< requestlogWithTime
#include "Popup/Plans/UploadMyPlans.h" ///< 上传方案弹窗
#include "Popup/Plans/UploadPlanSuccess.h" ///< 上传成功弹窗
#include "data/api_global.h" ///< kConfigTodayCount + ApiServerSwitch
#include "modules/Common/DeviceRegistry.h" ///< shortDisplayName 设备名映射
#include "modules/GeneralCustomUI/custom_QWidget_notification.h" ///< 上限提示弹窗
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_loading.h"

#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QRadioButton>
#include <QScrollBar>
#include <QVBoxLayout>

namespace {
constexpr int kStatusLabelMinWidth = 320;
constexpr int kStatusLabelHeight = 20;
constexpr int kStatusIconWidth = 40;
constexpr int kStatusIconHeight = 1;
constexpr int kStatusIconTextSpacing = 10;
constexpr int kStatusLoadingWidth = 120;
constexpr int kStatusLoadingHeight = 20;
constexpr int kStatusLoadingIconSize = 14;
constexpr int kStatusLoadingIconTextGap = 6;
const QColor kStatusTextColor(QStringLiteral("#8a94a6"));
}

CommunityPageWidget::CommunityPageWidget(QWidget* parent) : QWidget(parent) {
  clp_status_timer_ = new QTimer(this);
  clp_status_timer_->setSingleShot(true);
  connect(clp_status_timer_, &QTimer::timeout, this, [this] { clearStatus(); });

  initUi();
  // 信号连接在 injectServices() 中建立（依赖注入的 Service 非空）
}

void CommunityPageWidget::LanguageSet() {
  // Tab 按钮（构造时用当前语言创建；语言切换后需重设）
  if (clp_btn_set1_) clp_btn_set1_->setText(tr("方案广场"));
  if (clp_btn_set2_) clp_btn_set2_->setText(tr("大神分享"));
  if (clp_btn_set3_) clp_btn_set3_->setText(tr("官方预设"));
  // 搜索框提示文字 / 上传方案 / 国内 / 海外（同样为构造时一次性设置）
  if (clp_keyword_edit_) clp_keyword_edit_->setPlaceholderText(tr("请输入您要搜索的方案名"));
  if (clp_btn_upload_) clp_btn_upload_->setText(tr("上传方案"));
  if (clp_btn_domestic_) clp_btn_domestic_->setText(tr("国内"));
  if (clp_btn_overseas_) clp_btn_overseas_->setText(tr("海外"));
  // 筛选弹窗：重建翻译后的分组标签，保持已选值
  if (clp_filter_popup_) clp_filter_popup_->LanguageSet();
  // 面板：视图重绘（delegate 徽章等绘制期 tr() 文本）
  if (clp_community_) clp_community_->LanguageSet();
}

// ── UI ──

void CommunityPageWidget::initUi() {
  auto* vbox = new QVBoxLayout(this);
  vbox->setContentsMargins(0, 0, 0, 0);
  vbox->setSpacing(0);

  // 顶部工具栏（Tab 切换 + 筛选 + 后续菜单入口）
  vbox->addWidget(createToolBar());

  clp_community_ = new CommunityPanel(this);
  vbox->addWidget(clp_community_, 1);

  // 底部状态栏 — 固定高度防抖动，仅内容显隐
  clp_status_bar_ = new QWidget(this);
  clp_status_bar_->setFixedHeight(kStatusLabelHeight);
  auto* statusLayout = new QHBoxLayout(clp_status_bar_);
  statusLayout->setContentsMargins(0, 0, 0, 0);
  statusLayout->setSpacing(kStatusIconTextSpacing);

  clp_status_left_icon_ = new QLabel(clp_status_bar_);
  clp_status_left_icon_->setFixedSize(kStatusIconWidth, kStatusIconHeight);
  clp_status_left_icon_->hide();

  clp_status_label_ = new QLabel(clp_status_bar_);
  clp_status_label_->setFixedHeight(kStatusLabelHeight);
  clp_status_label_->setMinimumWidth(kStatusLabelMinWidth);
  clp_status_label_->setAlignment(Qt::AlignCenter);
  clp_status_label_->setStyleSheet(
      "color:#8a94a6; font-size:12px; background:transparent;");

  clp_status_right_icon_ = new QLabel(clp_status_bar_);
  clp_status_right_icon_->setFixedSize(kStatusIconWidth, kStatusIconHeight);
  clp_status_right_icon_->hide();

  clp_status_loading_ = new CustomQWidgetLoading(clp_status_bar_);
  clp_status_loading_->setFixedSize(kStatusLoadingWidth, kStatusLoadingHeight);
  configureStatusLoading(tr("加载中..."));
  clp_status_loading_->hide();

  statusLayout->addStretch();
  statusLayout->addWidget(clp_status_loading_, 0, Qt::AlignVCenter);
  statusLayout->addWidget(clp_status_left_icon_, 0, Qt::AlignVCenter);
  statusLayout->addWidget(clp_status_label_, 0, Qt::AlignVCenter);
  statusLayout->addWidget(clp_status_right_icon_, 0, Qt::AlignVCenter);
  statusLayout->addStretch();
  clearStatus();
  vbox->addWidget(clp_status_bar_);
}

static QPushButton* makeStyledButton(const QString& text, QHBoxLayout* hbox) {
  // Tab 按钮 — 选中：#009FEF 圆角背景 + 白字；非选中：无背景 + #747880 字
  auto* btn = new QPushButton(text);
  btn->setCheckable(true);
  btn->setMinimumHeight(30);
  btn->setStyleSheet(
      "QPushButton{background:transparent;color:#747880;border:none;border-radius:6px;"
      "padding:0 16px;font-size:13px;}"
      "QPushButton:hover{background:rgba(223, 243, 255, 0.2);color:#ffffff;}"
      "QPushButton:checked{background:#009FEF;color:#FFFFFF;}");
  hbox->addWidget(btn);
  return btn;
}

void CommunityPageWidget::addFilterWidgets(QHBoxLayout* hbox) {
  // 右侧元素组 — 内部间距 12
  hbox->setSpacing(12);

  // 搜索框 254×32，内部左侧 15×15 图标
  clp_keyword_edit_ = new QLineEdit();
  clp_keyword_edit_->setFixedSize(254, 32);
  clp_keyword_edit_->setPlaceholderText(tr("请输入您要搜索的方案名"));
  clp_keyword_edit_->setClearButtonEnabled(true);
  clp_keyword_edit_->setStyleSheet(
      "QLineEdit{background:rgba(0, 0, 0, 0.4);color:#ffffff;border:1px solid #333d4d;"
      "border-radius:6px;padding-left:0px;font-size:12px;}");
  auto* searchAction = new QAction(clp_keyword_edit_);
  searchAction->setIcon(QIcon(QStringLiteral(":/Skin/Images/search/icon.png")));
  clp_keyword_edit_->addAction(searchAction, QLineEdit::LeadingPosition);
  hbox->addWidget(clp_keyword_edit_);

  // 排序按钮（点击弹排序 Dialog）— 用筛选图标
  clp_btn_sort_ = new QPushButton(this);
  clp_btn_sort_->setFixedSize(30, 32);
  clp_btn_sort_->setCursor(Qt::PointingHandCursor);
  clp_btn_sort_->setStyleSheet(
      "QPushButton{border:none;background:transparent;border-image:url(:/Skin/Images/Community/Filter-no.png);}"
      "QPushButton:hover{border-image:url(:/Skin/Images/Community/Filter-ho.png);}");
  connect(clp_btn_sort_, &QPushButton::clicked, this, &CommunityPageWidget::onSortClicked);
  hbox->addWidget(clp_btn_sort_);

  // 上传方案 — 样式/尺寸与个人中心 pushButton_upload_plan 一致（104×30 + confirm 图）
  clp_btn_upload_ = new QPushButton(tr("上传方案"));
  clp_btn_upload_->setFixedSize(104, 30);
  clp_btn_upload_->setCursor(Qt::PointingHandCursor);
  clp_btn_upload_->setStyleSheet(
      "QPushButton{font-family:\"Noto Sans S Chinese\";font-weight:500;font-size:12px;"
      "color:#FFFFFF;border-image:url(:/Skin/Images/Popup/confirm-no.png);}"
      "QPushButton:hover{border-image:url(:/Skin/Images/Popup/confirm-ho.png);}");
  connect(clp_btn_upload_, &QPushButton::clicked, this, &CommunityPageWidget::onUploadPlanClicked);
  hbox->addWidget(clp_btn_upload_);
}

QWidget* CommunityPageWidget::createRegionSelector() {
  // 国内/海外开关 — 互斥，默认国内；海外暂隐藏（服务器未上线）
  auto* regionBox = new QWidget();
  regionBox->setObjectName(QStringLiteral("regionBox"));
  regionBox->setAttribute(Qt::WA_StyledBackground, true);  ///< QWidget 需此属性才绘制 QSS 背景（否则圆角背景不生效）
  regionBox->setFixedHeight(32);  ///< 容器高 32（圆角 19 胶囊背景）
  regionBox->setStyleSheet(QStringLiteral(
      "QWidget#regionBox{background:#0D0F14;border-radius:16px;}"));  ///< ID 选择器：只作用于容器自身，不级联子按钮；半径 ≤ 半高 16（Qt QSS 超半高绘制异常）
  auto* regionLayout = new QHBoxLayout(regionBox);
  regionLayout->setContentsMargins(0, 0, 0, 0);
  regionLayout->setSpacing(0);
  clp_btn_domestic_ = new QPushButton(tr("国内"));
  clp_btn_overseas_ = new QPushButton(tr("海外"));
  for (auto* btn : {clp_btn_domestic_, clp_btn_overseas_}) {
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton{"
        "  background:#1e2735;"
        "  color:#8a94a6;"
        "  border:1px solid #333d4d;"
        "  border-radius:4px;"
        "  padding:0 12px;"
        "  font-size:12px;"
        "}"
        "QPushButton:checked{"
        "  background:#009FEF;"
        "  color:#FFFFFF;"
        "  border-color:#009FEF;"
        "}"
        "QPushButton:disabled{"
        "  background:#2a3342;"
        "  color:#5a6476;"
        "  border-color:#2a3342;"
        "}"
        );
    regionLayout->addWidget(btn);
  }
  clp_btn_domestic_->setFixedSize(58, 32);  ///< 国内按键 58×32，圆角 16（≤ 半高）
  clp_btn_domestic_->setStyleSheet(
      "QPushButton{background:transparent;color:#747880;border:none;"
      "border-radius:16px;padding:0 12px;font-size:12px;}"
      "QPushButton:checked{background:#009FEF;color:#FFFFFF;border-radius:16px;}");
  clp_btn_overseas_->setFixedSize(58, 32);   ///< 国外按键 58×32，圆角 19
  clp_btn_overseas_->setStyleSheet(
      "QPushButton{background:transparent;color:#747880;border:none;"
      "border-radius:16px;padding:0 12px;font-size:12px;}"
      "QPushButton:checked{background:#009FEF;color:#FFFFFF;border-radius:16px;}");
  clp_btn_domestic_->setChecked(true);
  clp_btn_domestic_->setEnabled(false);  ///< 先禁用（暂不支持交互），默认选中"国内"
  clp_btn_overseas_->setEnabled(false);
  clp_btn_domestic_->hide();
  clp_btn_overseas_->hide();  ///< 海外服务器未上线，暂隐藏
  regionBox->hide();
  connect(clp_btn_domestic_, &QPushButton::toggled, this, [this](bool on) {
    if (on) clp_btn_overseas_->setChecked(false);
  });
  connect(clp_btn_overseas_, &QPushButton::toggled, this, [this](bool on) {
    if (on) clp_btn_domestic_->setChecked(false);
  });
  return regionBox;
}

QPushButton* CommunityPageWidget::makeSmallButton(const QString& iconNo, const QString& iconHo,
                                                 int size, QWidget* parent) {
  auto* btn = new QPushButton(parent);
  btn->setFixedSize(size, size);
  btn->setCursor(Qt::PointingHandCursor);
  btn->setStyleSheet(
      "QPushButton{border:none;background:transparent;border-image:url(" + iconNo + ");}"
      "QPushButton:hover{border-image:url(" + iconHo + ");}");
  return btn;
}

void CommunityPageWidget::positionFloatButtons() {
  if (!clp_btn_top_float_ || !clp_btn_refresh_float_) return;
  const int rightMargin = 14;  ///< 2026-08-05 调整：距视图右侧 4px → 14px
  clp_btn_refresh_float_->move(width() - rightMargin - clp_btn_refresh_float_->width(),
                               height() - 20 - clp_btn_refresh_float_->height());
  clp_btn_top_float_->move(width() - rightMargin - clp_btn_top_float_->width(),
                           height() - 70 - clp_btn_top_float_->height());
}

void CommunityPageWidget::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  positionFloatButtons();
}

QWidget* CommunityPageWidget::createToolBar() {
  // 工具栏 — 全透明背景
  auto* bar = new QWidget();
  auto* hbox = new QHBoxLayout(bar);
  hbox->setContentsMargins(8, 6, 8, 6);
  hbox->setSpacing(12);

  // 左侧 Tab 按键区域：289×44 背景块
  auto* tabArea = new QWidget();
  tabArea->setFixedSize(289, 44);
  tabArea->setAttribute(Qt::WA_StyledBackground, true);
  tabArea->setStyleSheet("background:rgba(0, 0, 0, 0.3);border-radius:6px;");
  auto* tabLayout = new QHBoxLayout(tabArea);
  tabLayout->setContentsMargins(4, 7, 4, 7);
  tabLayout->setSpacing(4);
  clp_btn_set1_ = makeStyledButton(tr("方案广场"), tabLayout);
  clp_btn_set2_ = makeStyledButton(tr("大神分享"), tabLayout);
  clp_btn_set3_ = makeStyledButton(tr("官方预设"), tabLayout);
  hbox->addWidget(tabArea);

  hbox->addSpacing(50);
  hbox->addWidget(createRegionSelector());
  hbox->addStretch();

  addFilterWidgets(hbox);

  auto connectTab = [this](QPushButton* btn, int tab, const QString& label) {
    connect(btn, &QPushButton::clicked, this, [this, tab, label] {
      cl_active_tab_ = tab;
      // 互斥选中：当前按钮 checked，其余取消
      clp_btn_set1_->setChecked(tab == 0);
      clp_btn_set2_->setChecked(tab == 1);
      clp_btn_set3_->setChecked(tab == 2);
      // 切 Tab 总是刷新当前 Tab（含筛选条件）：避免缓存数据与当前筛选不一致（旧数据/无法滚动到新内容）
      cl_tabs_[tab].page = 1;
      cl_tabs_[tab].hasMore = true;
      cl_tabs_[tab].fetching = false;
      refreshCurrentTab();  // 内部已先清除覆盖层
      clp_community_->stateOverlay()->hideState();  // 双保险：清旧 Tab 空态/错误态（覆盖层面板级共享）
      clp_community_->setCurrentTab(tab);
      updateScrollButtons();  ///< 切 tab 立即反映该 tab 的滚动状态（各 tab 独立显隐）
      emit statusMessage(label, 2000);
    });
  };
  connectTab(clp_btn_set1_, 0, tr("方案广场"));
  connectTab(clp_btn_set2_, 1, tr("大神分享"));
  connectTab(clp_btn_set3_, 2, tr("官方预设"));

  auto resetPageAndRefresh = [this] {
    cl_tabs_[cl_active_tab_].page = 1;
    cl_tabs_[cl_active_tab_].hasMore = true;
    cl_tabs_[cl_active_tab_].fetching = false;
    refreshCurrentTab();
  };
  connect(clp_keyword_edit_, &QLineEdit::returnPressed, this, resetPageAndRefresh);
  connect(clp_keyword_edit_, &QLineEdit::textChanged, this, resetPageAndRefresh);

  return bar;
}

// ── Service（注入，不自建 — 共享实例保证社区与个人中心双向同步） ──

void CommunityPageWidget::injectServices(SchemeService* svc, UserConfigRepository* repo) {
  clp_config_repo_ = repo;
  clp_scheme_svc_ = svc;

  cl_tabs_.resize(3);
  for (int i = 0; i < 3; ++i)
    cl_tabs_[i].model = new CommunityModel(this);

  // 头像下载 → 更新三个 model
  connect(clp_config_repo_, &UserConfigRepository::avatarReady, this,
          [this](int userId, const QPixmap& pm) {
            for (auto& t : cl_tabs_)
              t.model->setField(userId, CommunityModel::AvatarRole, QVariant::fromValue(pm));
          });

  // 三个 Tab 绑定独立 View
  for (int i = 0; i < 3; ++i) {
    clp_community_->initView(i, cl_tabs_[i].model);
    // 社区广场/大神/官方不显示置顶条（公开列表；置顶条仅个人中心"已上传"展示）
    clp_community_->delegateForTab(i)->setPinnedBarEnabled(false);
  }
  clp_community_->setCurrentTab(0);
  // 默认选中第一个 Tab 按钮
  if (clp_btn_set1_) clp_btn_set1_->setChecked(true);

  // Service 已就绪，建立全部信号连接（构造时 svc/repo 为 null，connect 会静默失败）
  initConnections();

  // 浮动回顶/刷新（右下角 40×40）
  clp_btn_top_float_ = makeSmallButton(":/Skin/Images/Community/BackTop-no.png",
                                       ":/Skin/Images/Community/BackTop-ho.png", 40, this);
  clp_btn_refresh_float_ = makeSmallButton(":/Skin/Images/Community/Refresh-no.png",
                                           ":/Skin/Images/Community/Refresh-ho.png", 40, this);
  connect(clp_btn_top_float_, &QPushButton::clicked, this, [this] {
    if (auto* sb = clp_community_->scrollBarForTab(cl_active_tab_)) sb->setValue(0);
  });
  connect(clp_btn_refresh_float_, &QPushButton::clicked, this, [this] { refreshCurrentTab(); });
  positionFloatButtons();

  // 显隐控制：监听 3 个 view 的滚动条（仅当前活动 tab 的滚动驱动按钮显隐，各 tab 独立）
  for (int i = 0; i < 3; ++i) {
    if (auto* sb = clp_community_->scrollBarForTab(i)) {
      connect(sb, &QScrollBar::valueChanged, this, [this, i] {
        if (i == cl_active_tab_) {
          updateScrollButtons();
          updateNoMoreStatus();
        }
      });
      connect(sb, &QScrollBar::rangeChanged, this, [this, i] {
        if (i == cl_active_tab_)
          updateNoMoreStatus();
      });
    }
  }
  updateScrollButtons();
}

void CommunityPageWidget::updateScrollButtons() {
  auto* sb = clp_community_->scrollBarForTab(cl_active_tab_);
  const bool atTop = (!sb || sb->value() == 0);
  if (clp_btn_top_float_) clp_btn_top_float_->setVisible(!atTop);
  if (clp_btn_refresh_float_) clp_btn_refresh_float_->setVisible(true);
}

/// \brief 上传方案 — 与个人中心逻辑一致：今日计数预检 + UploadMyPlans + 成功弹窗
void CommunityPageWidget::onUploadPlanClicked() {
  auto todayCount = []() -> int {
    int cnt = 0;
    QUrlQuery t_query;
    t_query.addQueryItem("device_name", SelDev_DeviceName);
    QNetworkReply* reply = HttpClient::instance().get(
        DeSheng::ApiPaths::kConfigTodayCount,
        RequestOptions{}.withQuery(t_query).withTag("userConfig"));
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, [&] { loop.quit(); });
    connect(reply, &QNetworkReply::finished, &loop, [&, reply] {
      if (reply->error() == QNetworkReply::NoError) {
        const QJsonDocument t_doc = QJsonDocument::fromJson(reply->readAll());
        cnt = t_doc.object().value("data").toObject().value("today_count").toInt(0);
      }
    });
    loop.exec();
    reply->deleteLater();
    return cnt;
  };

  if (todayCount() >= 10) {
    auto* t_notif = new CustomQWidgetNotification(
        tr("今日上传方案已达上限，请明天再来"), QString(), m);
    QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                     t_notif, &QWidget::deleteLater);
    t_notif->show();
    return;
  }

  auto* t_upload = new UploadMyPlans(m);
  t_upload->setModal(true);
  t_upload->adjustSize();
  QRect t_geom = m->geometry();
  t_upload->move(t_geom.center() - t_upload->rect().center());
  t_upload->showMyPlans();
  int t_res = t_upload->exec();
  if (t_res == QDialog::Accepted) {
    auto* t_ok = new UploadPlanSuccess(m);
    t_ok->setModal(true);
    t_ok->adjustSize();
    t_ok->move(t_geom.center() - t_ok->rect().center());
    t_ok->ShowUploadPlanCnt(todayCount());  ///< 与个人中心一致：成功框显示最新今日上传数（否则显示旧值）
    t_ok->exec();
    // 上传成功 → 重拉已上传列表（共享模型，个人中心已上传同步更新）；本项目仅耳机设备，固定 device_type=headset
    if (clp_config_repo_) clp_config_repo_->getMyConfigs(1, 50, {{"device_type", "headset"}});
  }
  t_upload->deleteLater();
}

// ── Data Mapping（统一映射见 model/community_item_data.cpp 的 configToItems） ──

void CommunityPageWidget::fetchAvatarsForList(
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list, CommunityModel* model) {
  (void)model;
  for (const auto& c : list) {
    if (c.author.avatar.isEmpty()) continue;
    clp_config_repo_->fetchAvatar(c.id, c.author.avatar);
  }
}

// ── Signal Wiring ──

void CommunityPageWidget::initConnections() {
  // ── Service → Model ──
  connect(clp_scheme_svc_, &SchemeService::squareDataReady, this,
          [this](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                 const PaginatedResult& pg) {
            cl_tabs_[0].fetching = false;
            const auto items = configToItems(list);
            if (cl_tabs_[0].page == 1) {
              cl_tabs_[0].model->replaceAll(items);
              // 首屏空 → 空态覆盖层；有数据 → 清除。
              // 仅当回包属于当前 Tab 时碰覆盖层（过期回包：切 Tab 后旧请求才返回）
              if (cl_active_tab_ == 0) {
                if (list.isEmpty()) showListEmptyState();
                else clp_community_->stateOverlay()->hideState();
              }
            } else {
              cl_tabs_[0].model->addItems(items);
            }
            cl_tabs_[0].hasMore = pg.hasMore();
            if (cl_active_tab_ == 0) updateNoMoreStatus();
            fetchAvatarsForList(list, cl_tabs_[0].model);
          });
  connect(clp_scheme_svc_, &SchemeService::expertDataReady, this,
          [this](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                 const PaginatedResult& pg) {
            cl_tabs_[1].fetching = false;
            LOG_DEBUG("[PageWidget] expertDataReady: {} items page:{}", list.size(), cl_tabs_[1].page);
            const auto items = configToItems(list);
            if (cl_tabs_[1].page == 1) {
              cl_tabs_[1].model->replaceAll(items);
              // 仅当回包属于当前 Tab 时碰覆盖层（过期回包竞态防护）
              if (cl_active_tab_ == 1) {
                if (list.isEmpty()) showListEmptyState();
                else clp_community_->stateOverlay()->hideState();
              }
            } else {
              cl_tabs_[1].model->addItems(items);
            }
            cl_tabs_[1].hasMore = pg.hasMore();
            if (cl_active_tab_ == 1) updateNoMoreStatus();
            fetchAvatarsForList(list, cl_tabs_[1].model);
          });
  connect(clp_scheme_svc_, &SchemeService::officialDataReady, this,
          [this](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem>& list,
                 const PaginatedResult& pg) {
            cl_tabs_[2].fetching = false;
            const auto items = configToItems(list);
            if (cl_tabs_[2].page == 1) {
              cl_tabs_[2].model->replaceAll(items);
              // 仅当回包属于当前 Tab 时碰覆盖层（过期回包竞态防护）
              if (cl_active_tab_ == 2) {
                if (list.isEmpty()) showListEmptyState();
                else clp_community_->stateOverlay()->hideState();
              }
            } else {
              cl_tabs_[2].model->addItems(items);
            }
            cl_tabs_[2].hasMore = pg.hasMore();
            if (cl_active_tab_ == 2) updateNoMoreStatus();
            fetchAvatarsForList(list, cl_tabs_[2].model);
          });
  // 空态/错误态覆盖层点击重试 → 刷新当前 Tab
  connect(clp_community_->stateOverlay(), &CommunityStateOverlay::retryClicked, this,
          [this]() { refreshCurrentTab(); });
  connect(clp_scheme_svc_, &SchemeService::errorOccurred, this,
          [this](const QString& action, const QString& reason) {
            if (action == QLatin1String("download")) return;  // 下载失败由 downloadFailed 专门提示
            if (action.contains("fetch")) {
              // 拉取失败：先清空当前 Tab 旧数据，错误态覆盖层独占可视区
              //（与排行榜 clp_model_->clear() 行为一致；不清数据会叠在旧卡片上看不到）
              cl_tabs_[cl_active_tab_].model->clear();
              clp_community_->stateOverlay()->showState(
                  tr("预设加载失败，请检查您的网络"), true,
                  QStringLiteral(":/Skin/Images/GeneralIcon/Empty/NetError.png"));
              cl_tabs_[cl_active_tab_].fetching = false;
              cl_tabs_[cl_active_tab_].hasMore = true;  // 恢复 hasMore — 防滚动加载永久卡死
              clearStatus();
              return;
            }
            LOG_DEBUG("[PageWidget] Service error: {} {}", action.toStdString(), reason.toStdString());
          });

  // 下载失败 — 具体原因提示（服务端已删除配置时提示"配置不存在"；模型移除由 DataSync 处理）
  connect(clp_scheme_svc_, &SchemeService::downloadFailed, this,
          [this](int, const QString& reason) {
            if (!g_shareCodeCopyHint) return;
            g_shareCodeCopyHint->setText(
                reason.contains(QStringLiteral("不存在")) ? reason : tr("下载失败，请检查网络"));
            g_shareCodeCopyHint->show();
            QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
          });

  // 删除：社区页 ZoneMore 默认隐藏（无删除入口）— 删除仅个人中心已上传面板（DataSync 处理）
  // 若未来社区开放删除：connect deleteRequested → deleteConfig + getMyConfigs(1,50) 联动已上传

  // ── Panel → SchemeService (乐观更新) ──
  connect(clp_community_, &CommunityPanel::likeToggled, this,
          [this](int userId, bool liked) { clp_scheme_svc_->toggleLike(userId, !liked); });
  connect(clp_community_, &CommunityPanel::dislikeToggled, this,
          [this](int userId, bool disliked) { clp_scheme_svc_->toggleDislike(userId, !disliked); });

  // 头像点击 → 用户上传弹窗（三个 Tab 均可触发：按 userId 查模型构造 UserProfile）
  connect(clp_community_, &CommunityPanel::avatarClicked, this, [this](int userId) {
    std::optional<CommunityItemData> t_item;
    for (auto& t_tab : cl_tabs_) {
      t_item = t_tab.model->findById(userId);
      if (t_item.has_value()) break;
    }
    if (!t_item.has_value()) return;
    if (t_item->authorUserId <= 0) return;  // 作者用户 ID 缺失（老数据）→ 不开弹窗
    UserProfile t_profile;
    t_profile.userId = t_item->authorUserId;  ///< 注意：用作者用户 ID，勿用 item.userId（语义为配置 ID）
    t_profile.username = t_item->name;
    t_profile.nickname = t_item->nickname;
    t_profile.avatar = t_item->avatar;
    t_profile.level = t_item->authorLevel;
    t_profile.isOfficial = t_item->isOfficial;
    t_profile.isStreamer = t_item->isStreamer;
    t_profile.isProfessional = t_item->isProfessional;
    // isExpert 初始 false：弹窗内由 GET /user/:id 的 titles 决定
    auto* t_dialog = new UserUploadsDialog(t_profile, this);
    t_dialog->show();
  });

  // 下载进度 — 常驻连接 + 按 configId 查表分发（避免 disconnect-all 竞态：快速多下载各显各的）
  connect(clp_scheme_svc_, &SchemeService::downloadProgress, this,
          [this](int configId, int percent) {
            if (auto* p = cl_download_targets_.value(configId, nullptr))
              p->setDownloadProgress(configId, percent);
          });

  // 下载
  connect(clp_community_, &CommunityPanel::downloadRequested, this, [this](int userId) {
    cl_download_targets_[userId] = clp_community_;
    connectOnce(clp_scheme_svc_, &SchemeService::downloadFileSaved, this,
                [this, userId](int, const QString& t_filePath) {
                  cl_download_targets_.remove(userId);
                  clp_community_->setDownloadProgress(userId, 100);
                  emit statusMessage(tr("下载完成"), 2000);
                  // 导入方案库 + 清理临时文件（与个人中心旧卡片 doDownload 行为一致）
                  if (m) m->importDownloadedPlan(t_filePath);
                  clp_scheme_svc_->refreshCounts(userId);
                  QTimer::singleShot(500, this, [this, userId] { clp_community_->setDownloadProgress(userId, -1); });
                });
    connectOnce(clp_scheme_svc_, &SchemeService::errorOccurred, this,
                [this, userId](const QString&, const QString&) {
                  cl_download_targets_.remove(userId);
                  clp_community_->setDownloadProgress(userId, -1);
                });
    clp_community_->setDownloadProgress(userId, 0);
    clp_scheme_svc_->download(userId);
  });

  // 分享
  connect(clp_community_, &CommunityPanel::shareRequested, this, [this](int userId) {
    // 先断开旧的 shareCodeReady 连接（分享专用，安全）— 快速连点分享时防旧响应覆盖新剪贴板
    QObject::disconnect(clp_scheme_svc_, &SchemeService::shareCodeReady, this, nullptr);
    connectOnce(clp_scheme_svc_, &SchemeService::shareCodeReady, this,
                [this, userId](const QString& shareCode, int shareCount) {
                  QString shareText = "sq" + shareCode;
                  for (auto* m : {cl_tabs_[0].model, cl_tabs_[1].model, cl_tabs_[2].model}) {
                    auto opt = m->findById(userId);
                    if (opt.has_value()) {
                      shareText = opt->planName + "+" + opt->deviceName + "+" +
                                  opt->tags.join("+") + "+sq" + shareCode;
                      m->setField(userId, CommunityModel::ShareCountRole, shareCount);
                      break;
                    }
                  }
                  clp_scheme_svc_->refreshCounts(userId);
                  QApplication::clipboard()->setText(shareText);
                  // 全局提示条（与旧卡片/个人中心一致）
                  if (g_shareCodeCopyHint) {
                    g_shareCodeCopyHint->setText(tr("分享码已复制"));
                    g_shareCodeCopyHint->show();
                    QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                  }
                });
    connectOnce(clp_scheme_svc_, &SchemeService::errorOccurred, this,
                [this](const QString&, const QString& reason) {
                  emit statusMessage(tr("分享失败: %1").arg(reason), 3000);
                });
    clp_scheme_svc_->share(userId);
  });

  // 评论标签
  connect(clp_community_, &CommunityPanel::commentTagClicked, this,
          [this](int configId, int commentId, bool nowClicked) {
            clp_scheme_svc_->toggleCommentClick(configId, commentId, nowClicked);
          });

  // 无限滚动（带触发 tab — 防止 tab 切换后旧 view 的滚动事件串加载到新 tab）
  connect(clp_community_, &CommunityPanel::loadMoreRequested, this, [this](int tab) {
    if (tab < 0 || tab >= cl_tabs_.size()) return;
    auto& t = cl_tabs_[tab];
    if (t.fetching) return;
    if (!t.hasMore) {
      if (tab == cl_active_tab_) updateNoMoreStatus();
      return;
    }
    t.fetching = true;
    if (tab == cl_active_tab_)
      updateNoMoreStatus();
    ++t.page;
    if (tab == cl_active_tab_) refreshCurrentTab();
  });

  // API 后纠正计数
  connect(clp_scheme_svc_, &SchemeService::countsSynced, this,
          [this](const DeSheng::GetPublicConfigurationListResponse::ListItem& info) {
            const int id = info.id;
            auto upd = [&](CommunityModel* m) {
              m->setField(id, CommunityModel::LikeCountRole, info.like_count);
              m->setField(id, CommunityModel::DislikeCountRole, info.dislike_count);
              m->setField(id, CommunityModel::DownloadCountRole, info.download_count);
              m->setField(id, CommunityModel::ShareCountRole, info.share_count);
              m->setField(id, CommunityModel::IsLikedRole, info.is_liked);
              m->setField(id, CommunityModel::IsDislikedRole, info.is_disliked);
            };
            upd(cl_tabs_[0].model); upd(cl_tabs_[1].model); upd(cl_tabs_[2].model);
          });

  // 评论标签失败回滚 → 3 model 翻转恢复（与 delegate 乐观更新对称）
  connect(clp_scheme_svc_, &SchemeService::commentClickReverted, this,
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
            upd(cl_tabs_[0].model); upd(cl_tabs_[1].model); upd(cl_tabs_[2].model);
          });
}

// ── Filter helpers ──

QString CommunityPageWidget::currentDeviceType() const {
  return QStringLiteral("headset");  ///< 设备下拉已移除，固定"耳机"
}
QString CommunityPageWidget::currentSort() const {
  // 筛选值唯一来源在弹窗内；未创建（未点过筛选）时默认 "new"（与弹窗默认一致）
  return clp_filter_popup_ ? clp_filter_popup_->value(QStringLiteral("sort"))
                           : QStringLiteral("new");
}

/// \brief 筛选弹窗 — 排序/场景/机型分组过滤（迁移自 WidgetCMake SchemeFilterPopup）
void CommunityPageWidget::onSortClicked() {
  ensureFilterPopup();
  // 弹窗居中显示在应用窗口中间（与上传弹窗同定位方式；点击外部仍可关闭）
  const QRect t_geom = m->geometry();
  clp_filter_popup_->move(t_geom.center() - clp_filter_popup_->rect().center());
  clp_filter_popup_->show();
  clp_filter_popup_->raise();
}

void CommunityPageWidget::ensureFilterPopup() {
  if (clp_filter_popup_) return;
  clp_filter_popup_ = new SchemeFilterPopup(this);
  // 任一筛选变化（含重置）→ 回第一页重新拉取
  connect(clp_filter_popup_, &SchemeFilterPopup::filterChanged, this, [this]() {
    cl_tabs_[cl_active_tab_].page = 1;
    cl_tabs_[cl_active_tab_].hasMore = true;
    cl_tabs_[cl_active_tab_].fetching = false;
    refreshCurrentTab();
  });
  connect(clp_filter_popup_, &SchemeFilterPopup::filtersReset, this, [this]() {
    cl_tabs_[cl_active_tab_].page = 1;
    cl_tabs_[cl_active_tab_].hasMore = true;
    cl_tabs_[cl_active_tab_].fetching = false;
    refreshCurrentTab();
  });
}
QString CommunityPageWidget::currentKeyword() const {
  return clp_keyword_edit_->text().trimmed();
}

void CommunityPageWidget::showListEmptyState() {
  const bool t_searching = !currentKeyword().isEmpty();
  clp_community_->stateOverlay()->showState(
      t_searching ? tr("未搜索到该方案") : tr("暂无数据"), /*retryEnabled=*/false,
      t_searching ? QStringLiteral(":/Skin/Images/GeneralIcon/Empty/searchPlanEmpty.png")
                  : QStringLiteral(":/Skin/Images/GeneralIcon/Empty/PlanEmpty.png"));
}

void CommunityPageWidget::refreshCurrentTab() {
  if (!clp_scheme_svc_ || cl_active_tab_ < 0 || cl_active_tab_ >= cl_tabs_.size()) return;
  clp_community_->stateOverlay()->hideState();  // 新请求开始：清除旧空态/错误态
  const QString dt = currentDeviceType();
  const QString s = currentSort();
  const QString kw = currentKeyword();
  const int page = cl_tabs_[cl_active_tab_].page;
  if (page == 1 && cl_tabs_[cl_active_tab_].model) {
    cl_tabs_[cl_active_tab_].model->clear();
  }
  // 场景/机型筛选（空 = 全部，请求层跳过）
  const QString scene =
      clp_filter_popup_ ? clp_filter_popup_->value(QStringLiteral("scene")) : QString();
  // 机型短名 → API device_name 英文全名（如 "T10有线" → "XIBERIA T10G"）
  const QString modelRaw =
      clp_filter_popup_ ? clp_filter_popup_->value(QStringLiteral("model")) : QString();
  const QString model =
      modelRaw.isEmpty() ? modelRaw : DeSheng::DeviceRegistry::deviceNameParam(modelRaw);
  if (page == 1) {
    clearStatus();
  }
  cl_tabs_[cl_active_tab_].fetching = true;
  updateNoMoreStatus();
  // 排序/场景/机型/关键词对三个 Tab 均生效
  switch (cl_active_tab_) {
    case 0: clp_scheme_svc_->fetchSquare(page, s, dt, model, kw, scene); break;
    case 1: clp_scheme_svc_->fetchExpert(page, s, dt, model, scene, kw); break;
    case 2: clp_scheme_svc_->fetchOfficial(page, s, dt, model, scene, kw); break;
  }
}

void CommunityPageWidget::loadInitialData() { refreshCurrentTab(); }

UserConfigRepository* CommunityPageWidget::configRepo() const { return clp_config_repo_; }

SchemeService* CommunityPageWidget::schemeService() const { return clp_scheme_svc_; }

CommunityPanel* CommunityPageWidget::communityPanel() const { return clp_community_; }

CommunityModel* CommunityPageWidget::leftModel(int tab) const {
  if (tab < 0 || tab >= cl_tabs_.size()) return nullptr;
  return cl_tabs_[tab].model;
}

void CommunityPageWidget::showStatus(const QString& text, int autoHideMs) {
  clp_status_timer_->stop();
  hideLoadingStatus();
  clp_status_label_->show();
  clp_status_label_->setText(text);
  updateStatusDecoration(!text.isEmpty());
  if (autoHideMs > 0)
    clp_status_timer_->start(autoHideMs);
  else if (autoHideMs == 0)
    clp_status_timer_->start(2000);  // default 2s（200ms 瞬态等于不可见）
}

void CommunityPageWidget::showLoadingStatus() {
  if (!clp_status_loading_)
    return;
  clp_status_timer_->stop();
  clp_status_label_->clear();
  clp_status_label_->hide();
  updateStatusDecoration(false);
  configureStatusLoading(tr("加载中..."));
  clp_status_loading_->show();
  clp_status_loading_->raise();
  clp_status_loading_->start();
}

void CommunityPageWidget::hideLoadingStatus() {
  if (!clp_status_loading_)
    return;
  clp_status_loading_->stop();
  clp_status_loading_->hide();
}

void CommunityPageWidget::configureStatusLoading(const QString& text) {
  if (!clp_status_loading_)
    return;

  CustomQWidgetLoadingConfig t_config = clp_status_loading_->cl_config();
  const int t_icon_y = (kStatusLoadingHeight - kStatusLoadingIconSize) / 2;
  const int t_text_x = kStatusLoadingIconSize + kStatusLoadingIconTextGap;
  t_config.text = text;
  t_config.text_visible = true;
  t_config.text_color = kStatusTextColor;
  t_config.arc_rect = QRect(0, t_icon_y, kStatusLoadingIconSize, kStatusLoadingIconSize);
  t_config.text_rect = QRect(t_text_x, 0,
                             kStatusLoadingWidth - t_text_x,
                             kStatusLoadingHeight);

  QFont t_font = clp_status_loading_->font();
  t_font.setPointSize(12);
  t_font.setWeight(QFont::Normal);
  clp_status_loading_->setFont(t_font);
  clp_status_loading_->setCl_config(t_config);
}

void CommunityPageWidget::clearStatus() {
  if (clp_status_timer_)
    clp_status_timer_->stop();
  hideLoadingStatus();
  if (clp_status_label_)
    clp_status_label_->clear();
  updateStatusDecoration(false);
}

void CommunityPageWidget::updateNoMoreStatus() {
  if (cl_active_tab_ < 0 || cl_active_tab_ >= cl_tabs_.size()) {
    clearStatus();
    return;
  }

  const auto& t = cl_tabs_[cl_active_tab_];
  if (t.fetching) {
    showLoadingStatus();
    return;
  }

  const bool shouldCheck = t.model && t.model->rowCount() > 0 && !t.fetching && !t.hasMore;
  if (!shouldCheck) {
    clearStatus();
    return;
  }

  auto* sb = clp_community_->scrollBarForTab(cl_active_tab_);
  const bool atBottom = sb && sb->maximum() > 0 && sb->value() == sb->maximum();
  if (atBottom)
    showStatus(tr("没有更多了，快去上传方案吧"), -1);
  else
    clearStatus();
}

void CommunityPageWidget::updateStatusDecoration(bool visible) {
  auto updateIcon = [visible](QLabel* label, const char* path) {
    if (!label)
      return;
    const bool hasPath = path && path[0] != '\0';
    label->setVisible(visible && hasPath);
    if (!hasPath)
      return;
    label->setStyleSheet(QStringLiteral("border-image:url(%1);").arg(QString::fromLatin1(path)));
  };

  updateIcon(clp_status_left_icon_, CommunityBottomStatusAssets::kLeftIconPath);
  updateIcon(clp_status_right_icon_, CommunityBottomStatusAssets::kRightIconPath);
}
