#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/personal_center_settings_main_page.h"
#include "ui_personal_center_settings_main_page.h"

#include "LoadLib.h"
#include "APOThread/ApoManager.h"
#include "data/api_global.h" ///< 统一 API 入口（数据类型 + ApiPaths）
#include "modules/Common/DeviceRegistry.h" ///< shortDisplayName 设备名映射
#include "network/http_client.h"
#include "network/request_options.h"
#include "repository/paginated_repository.h"

#include <QDir>
#include <QFont>
#include <QHBoxLayout>
#include <QHttpMultiPart>
#include <QLabel>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QScrollBar>
#include <QTimer>

#include "model/community_item_data.h" ///< configToItems
#include "modules/CommunityModule/ui/community/community_delegate.h" ///< setMoreVisible（delegateForTab 返回完整类型）
#include "modules/CommunityModule/ui/community/community_panel.h" ///< stateOverlay（空态/错误态覆盖层）
#include "modules/CommunityModule/ui/community/community_state_overlay.h" ///< retryClicked

namespace {
constexpr int kPlansLoadingWidth = 120;
constexpr int kPlansLoadingHeight = 20;
constexpr int kPlansLoadingIconSize = 14;
constexpr int kPlansLoadingIconTextGap = 6;
constexpr int kUploadedNoMoreMinWidth = 180;
constexpr int kPlansBottomStatusHeight = 20;
const QColor kPlansBottomStatusTextColor(QStringLiteral("#8a94a6"));
}

// ── Data Mapping（统一映射见 model/community_item_data.cpp 的 configToItems） ──

/// \brief 构造函数
PersonalCenterSettingsMainPage::PersonalCenterSettingsMainPage(QWidget *parent,
                                                               UserSettingMainPage *targetObject,
                                                               int theme)
    : QWidget(parent)
    , ui(new Ui::PersonalCenterSettingsMainPage)
    , clp_target_user_setting_main_page_(targetObject)
    , cl_theme_(theme)
{
    ui->setupUi(this);
    InitUIInformation(theme); // 初始化UI的默认信息
    InitMember();             // 初始化内部成员
    InitConnect();            // 连接默认的信号槽
    applyTheme(theme);
}

/// \brief 析构函数 — 先将池 widget 归还（detach parent），避免 QWidget 树销毁时 double-free
PersonalCenterSettingsMainPage::~PersonalCenterSettingsMainPage()
{
    resetData();
    delete ui;
}

/// \brief 更新个人中心页面 UI 信息（供父级调用）
void PersonalCenterSettingsMainPage::UpdatePersonalCenterSettingsUIInformation()
{
    // WBLIU: 更新个人中心 UI 信息预留

    {
        clp_user_info_->setId(g_user_information.network.id);             // 更新用户 ID
        clp_user_info_->setNickname(g_user_information.network.username); // 更新用户昵称
        clp_user_info_->setSignature(g_user_information.network.bio);     // 更新个性签名
        clp_user_info_->setRoles(g_user_information.network.roles);       // 更新用户角色
        clp_user_info_->setTitles(g_user_information.network.titles);     // 更新用户头衔
        // 加载本地头像
        {
            QString t_file = g_user_information.avatarFilePath();
            if (QFile::exists(t_file))
                clp_user_info_->setAvatar(QPixmap(t_file));
            else
                clp_user_info_->setAvatar(QPixmap());
        }
        // 获取用户等级信息
        fetchUserLevel();
        // 登录后重新拉取方案列表
        fetchMyUploadedConfigPage(1);
    }
    // 更新 个人信息 编辑页面
    {
        clp_user_info_change_page_->setCl_nickname(g_user_information.network.username); //更新昵称
        clp_user_info_change_page_->setCl_signature(g_user_information.network.bio); //更新个性签名
        QString t_file = g_user_information.avatarFilePath();
        if (QFile::exists(t_file))
            clp_user_info_change_page_->setCl_avatar(QPixmap(t_file));
        else
            clp_user_info_change_page_->setCl_avatar(QPixmap());
    }
}

/// \brief 拉取已上传第 page 页（走共享 UserConfigRepository，50/页）
void PersonalCenterSettingsMainPage::fetchUploadedPage(int page)
{
    if (!clp_config_repo_) return; ///< 未注入 Service（页面未接线），静默跳过
    cl_uploaded_page_ = page;
    cl_uploaded_fetching_ = true;  ///< 请求中标记（错误态覆盖层用）
    hideUploadedNoMoreStatus();
    showLoading();
    clp_config_repo_->getMyConfigs(page, 50, {{"device_type", "headset"}});
}

/// \brief 拉取已点赞第 page 页
void PersonalCenterSettingsMainPage::fetchLikedPage(int page)
{
    if (!clp_config_repo_) return;
    cl_liked_page_ = page;
    cl_liked_fetching_ = true;  ///< 请求中标记（错误态覆盖层用）
    hideUploadedNoMoreStatus();
    showLoading();
    clp_config_repo_->getMyLikes(page, 50, {{"device_type", "headset"}});
}

/// \brief 获取已上传配置（公开入口，供父级调用；page=1 全量刷新）
void PersonalCenterSettingsMainPage::fetchMyUploadedConfigPage(int page)
{
    fetchUploadedPage(page);
}

/// \brief 获取已点赞配置（公开入口）
void PersonalCenterSettingsMainPage::fetchMyLikedConfigPage(int page)
{
    fetchLikedPage(page);
}

/// \brief 注入共享 Service（CommunityMainPage 创建，MainWindow 转发）
void PersonalCenterSettingsMainPage::injectServices(SchemeService *svc, UserConfigRepository *repo)
{
    clp_scheme_svc_ = svc;
    clp_config_repo_ = repo;
    initPlansPanels();
}

/// \brief 创建已上传/已点赞 CommunityPanel + Model（MV 渲染，替代 WidgetPool 网格）
void PersonalCenterSettingsMainPage::initPlansPanels()
{
    if (!ui || !clp_config_repo_)
        return;

    // 已上传（stackedWidget page 0）— 置顶条仅此面板展示（自己的方案可置顶，my 列表返回 is_pinned）
    clp_uploaded_model_ = new CommunityModel(this);
    clp_uploaded_panel_ = new CommunityPanel(ui->stackedWidget_plansPage_uploaded_01);
    clp_uploaded_panel_->initView(0, clp_uploaded_model_);
    if (auto *t_d = clp_uploaded_panel_->delegateForTab(0)) {
        t_d->setMoreVisible(true);  ///< 已上传可删除
        t_d->setPinnedBarEnabled(true);
    }
    ui->stackedWidget_plansPage_uploaded_01->layout()->addWidget(clp_uploaded_panel_);

    // 已点赞（stackedWidget page 1）— 非自己的方案，不显示置顶条
    clp_liked_model_ = new CommunityModel(this);
    clp_liked_panel_ = new CommunityPanel(ui->stackedWidget_plansPage_Liked_02);
    clp_liked_panel_->initView(0, clp_liked_model_);
    if (auto *t_d = clp_liked_panel_->delegateForTab(0)) {
        t_d->setMoreVisible(false);
        t_d->setPinnedBarEnabled(false);
    }
    ui->stackedWidget_plansPage_Liked_02->layout()->addWidget(clp_liked_panel_);

    // repository → model（page>1 追加，避免分页加载覆盖前一页）
    connect(clp_config_repo_, &UserConfigRepository::myConfigsReady, this,
            [this](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &list,
                   const PaginatedResult &pageResult) {
                cl_uploaded_fetching_ = false;
                if (!cl_liked_fetching_)
                    hideLoading();
                const auto items = configToItems(list);
                if (cl_uploaded_page_ <= 1) {
                    clp_uploaded_model_->replaceAll(items);
                    // 首屏空 → 空态覆盖层（PlanEmpty 图标 + 提示字）；有数据 → 清除
                    if (list.isEmpty())
                        clp_uploaded_panel_->stateOverlay()->showState(
                            tr("这里什么也没有，快去上传方案吧~"), false,
                            QStringLiteral(":/Skin/Images/GeneralIcon/Empty/PlanEmpty.png"));
                    else
                        clp_uploaded_panel_->stateOverlay()->hideState();
                } else {
                    clp_uploaded_model_->addItems(items);
                }
                cl_uploaded_has_more_ = pageResult.hasMore();
                for (const auto &c : list)
                    if (!c.author.avatar.isEmpty())
                        clp_config_repo_->fetchAvatar(c.id, c.author.avatar);
                QTimer::singleShot(0, this, &PersonalCenterSettingsMainPage::updateUploadedNoMoreStatus);
            });
    connect(clp_config_repo_, &UserConfigRepository::myLikesReady, this,
            [this](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &list,
                   const PaginatedResult &pageResult) {
                cl_liked_fetching_ = false;
                if (!cl_uploaded_fetching_)
                    hideLoading();
                const auto items = configToItems(list);
                if (cl_liked_page_ <= 1) {
                    clp_liked_model_->replaceAll(items);
                    // 首屏空 → 空态覆盖层（PlanEmpty 图标 + 提示字）；有数据 → 清除
                    if (list.isEmpty())
                        clp_liked_panel_->stateOverlay()->showState(
                            tr("这里什么也没有，快去给心仪的方案点赞吧~"), false,
                            QStringLiteral(":/Skin/Images/GeneralIcon/Empty/PlanEmpty.png"));
                    else
                        clp_liked_panel_->stateOverlay()->hideState();
                } else {
                    clp_liked_model_->addItems(items);
                }
                cl_liked_has_more_ = pageResult.hasMore();
                for (const auto &c : list)
                    if (!c.author.avatar.isEmpty())
                        clp_config_repo_->fetchAvatar(c.id, c.author.avatar);
            });
    // 拉取失败 → 错误态覆盖层（仅"请求中"标记的首屏请求；重试 = 重新拉第一页）
    connect(clp_config_repo_, &UserConfigRepository::errorOccurred, this,
            [this](const QString &) {
                if (cl_uploaded_fetching_) {
                    cl_uploaded_fetching_ = false;
                    if (clp_uploaded_model_->isEmpty())
                        clp_uploaded_panel_->stateOverlay()->showState(
                            tr("加载失败，点击重试"), true,
                            QStringLiteral(":/Skin/Images/GeneralIcon/Empty/NetError.png"));
                }
                if (cl_liked_fetching_) {
                    cl_liked_fetching_ = false;
                    if (clp_liked_model_->isEmpty())
                        clp_liked_panel_->stateOverlay()->showState(
                            tr("加载失败，点击重试"), true,
                            QStringLiteral(":/Skin/Images/GeneralIcon/Empty/NetError.png"));
                }
                hideLoading();
            });
    // 覆盖层点击重试 → 重新拉第一页
    connect(clp_uploaded_panel_->stateOverlay(), &CommunityStateOverlay::retryClicked, this,
            [this]() { fetchUploadedPage(1); });
    connect(clp_liked_panel_->stateOverlay(), &CommunityStateOverlay::retryClicked, this,
            [this]() { fetchLikedPage(1); });

    // 滚动到底 → 下一页（panel 内部 loadMoreRequested 信号，各面板独立 tab 标识）
    connect(clp_uploaded_panel_, &CommunityPanel::loadMoreRequested, this,
            [this](int) {
                if (cl_uploaded_has_more_ && !cl_uploaded_fetching_) {
                    fetchUploadedPage(cl_uploaded_page_ + 1);
                    return;
                }
                updateUploadedNoMoreStatus();
            });
    connect(clp_liked_panel_, &CommunityPanel::loadMoreRequested, this,
            [this](int) {
                if (cl_liked_has_more_ && !cl_liked_fetching_)
                    fetchLikedPage(cl_liked_page_ + 1);
            });

    if (auto *t_scroll_bar = clp_uploaded_panel_->scrollBarForTab(0)) {
        connect(t_scroll_bar, &QScrollBar::valueChanged, this,
                [this] { updateUploadedNoMoreStatus(); });
        connect(t_scroll_bar, &QScrollBar::rangeChanged, this,
                [this] { updateUploadedNoMoreStatus(); });
    }
}

void PersonalCenterSettingsMainPage::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
        setStyleSheet(R"()");
    } break;
    default: {
        setStyleSheet(R"()");
    } break;
    }
}

/// \brief 获取用户等级信息
void PersonalCenterSettingsMainPage::fetchUserLevel()
{
    QNetworkReply *t_reply = HttpClient::instance().get(
        DeSheng::ApiPaths::kUserLevel,
        RequestOptions{}.withTag("userLevel"));
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply]() {
        int t_status = t_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (t_reply->error() != QNetworkReply::NoError) {
            qWarning() << "GetUserLevel failed: status=" << t_status
                       << "error=" << t_reply->errorString();
        } else {
            QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
            DeSheng::GetUserLevelResponse t_resp;
            if (DeSheng::ProcessGetUserLevelResult(t_resp, t_doc)
                && t_resp.code == "success") {
                // 更新等级状态控件
                clp_user_info_->clp_grade_status_->setCl_grade_level(t_resp.data.level);
                clp_user_info_->clp_grade_status_->setCl_progress(t_resp.data.current_experience,
                                                                   t_resp.data.exp_cap);
                clp_user_info_->clp_grade_status_->setCl_empirical_value(
                    t_resp.data.current_experience, t_resp.data.exp_cap);
            } else {
                qWarning() << "GetUserLevel API error: code=" << t_resp.code
                           << "message=" << t_resp.message;
            }
        }
        t_reply->deleteLater();
    });
}

void PersonalCenterSettingsMainPage::showLoading()
{
    if (!clp_plans_loading_)
        return;
    hideUploadedNoMoreStatus();
    ui->widget_refresh_wait_icon->show();
    setPlansLoadingText(tr("加载中"));
    clp_plans_loading_->show();
    clp_plans_loading_->raise();
    clp_plans_loading_->start();
}

void PersonalCenterSettingsMainPage::hideLoading()
{
    if (!clp_plans_loading_)
        return;
    clp_plans_loading_->stop();
    clp_plans_loading_->hide();
    updateUploadedNoMoreStatus();
}

void PersonalCenterSettingsMainPage::setPlansLoadingText(const QString &text)
{
    if (!clp_plans_loading_)
        return;

    CustomQWidgetLoadingConfig t_config = clp_plans_loading_->cl_config();
    const int t_icon_y = (kPlansLoadingHeight - kPlansLoadingIconSize) / 2;
    const int t_text_x = kPlansLoadingIconSize + kPlansLoadingIconTextGap;
    t_config.text = text;
    t_config.text_visible = true;
    t_config.text_color = kPlansBottomStatusTextColor;
    t_config.arc_rect = QRect(0, t_icon_y, kPlansLoadingIconSize, kPlansLoadingIconSize);
    t_config.text_rect = QRect(t_text_x, 0,
                               kPlansLoadingWidth - t_text_x,
                               kPlansLoadingHeight);
    QFont t_font = clp_plans_loading_->font();
    t_font.setPointSize(12);
    t_font.setWeight(QFont::Normal);
    clp_plans_loading_->setFont(t_font);
    clp_plans_loading_->setCl_config(t_config);
}

void PersonalCenterSettingsMainPage::showUploadedNoMoreStatus()
{
    if (!clp_uploaded_no_more_label_)
        return;
    ui->widget_refresh_wait_icon->show();
    clp_uploaded_no_more_label_->setText(tr("没有更多方案了，快去上传方案吧"));
    clp_uploaded_no_more_label_->show();
    clp_uploaded_no_more_label_->raise();
}

void PersonalCenterSettingsMainPage::hideUploadedNoMoreStatus()
{
    if (clp_uploaded_no_more_label_)
        clp_uploaded_no_more_label_->hide();
}

void PersonalCenterSettingsMainPage::updateUploadedNoMoreStatus()
{
    if (!clp_uploaded_panel_ || !clp_uploaded_model_) {
        hideUploadedNoMoreStatus();
        return;
    }
    if (ui->stackedWidget_plans->currentIndex() != 0
        || cl_uploaded_fetching_
        || cl_uploaded_has_more_
        || clp_uploaded_model_->rowCount() <= 0) {
        hideUploadedNoMoreStatus();
        return;
    }

    auto *t_scroll_bar = clp_uploaded_panel_->scrollBarForTab(0);
    const bool t_at_bottom = t_scroll_bar
                             && t_scroll_bar->maximum() > 0
                             && t_scroll_bar->value() == t_scroll_bar->maximum();
    if (t_at_bottom)
        showUploadedNoMoreStatus();
    else
        hideUploadedNoMoreStatus();
}

/// \brief 刷新翻译文本
void PersonalCenterSettingsMainPage::LanguageSet()
{
    ui->retranslateUi(this);
    // 代码创建/一次性 setText 的子控件（.ui 之外，语言切换后需重设）
    if (clp_user_info_) clp_user_info_->LanguageSet();
    if (clp_user_info_change_page_) clp_user_info_change_page_->LanguageSet();
    if (clp_uploaded_no_more_label_) clp_uploaded_no_more_label_->setText(tr("没有更多方案了，快去上传方案吧"));
    setPlansLoadingText(tr("加载中"));
}

void PersonalCenterSettingsMainPage::resetData()
{
    // 清空 MV 数据（退出登录时调用）
    if (clp_uploaded_model_) clp_uploaded_model_->clear();
    if (clp_liked_model_) clp_liked_model_->clear();
    cl_uploaded_page_ = 0;
    cl_liked_page_ = 0;
    cl_uploaded_has_more_ = true;
    cl_liked_has_more_ = true;
    hideUploadedNoMoreStatus();

    // 切回默认 tab
    ui->stackedWidget_plans->setCurrentIndex(0);
    ui->pushButton_2->setChecked(true);
}

/// \brief 初始化 UI 默认信息
void PersonalCenterSettingsMainPage::InitUIInformation(int theme)
{
    {
        // 用户信息容器
        ui->widget_user_info->setObjectName("PersonalCenterSettingsMainPage_widget_user_info");
        ui->widget_user_info->setCornerRadius(12);
        ui->widget_user_info->setStyleSheet(R"(
            QWidget#PersonalCenterSettingsMainPage_widget_user_info {
                border-radius: 12px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
    }
    {
        // 方案设置容器
        ui->widget_plans_settings->setObjectName(
            "PersonalCenterSettingsMainPage_widget_plans_settings");
        ui->widget_plans_settings->setStyleSheet(R"(
            QWidget#PersonalCenterSettingsMainPage_widget_plans_settings {
                border: none;
                background-color: transparent;
            }
        )");
    }
    {
        // 上传 按钮
        ui->pushButton_2->setObjectName("PersonalCenterSettingsMainPage_btn_upload");
        ui->pushButton_2->setCheckable(true);
        ui->pushButton_2->setCursor(Qt::PointingHandCursor); //手型
        ui->pushButton_2->setStyleSheet(R"(
            #PersonalCenterSettingsMainPage_btn_upload {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #FFFFFF;
                border: none;
            }
            #PersonalCenterSettingsMainPage_btn_upload:hover {
                color: #009FEF;
                border: none;
            }
            #PersonalCenterSettingsMainPage_btn_upload:checked {
                color: #009FEF;
                border-bottom: 2px solid #4A90D9;
            }
        )");
    }
    {
        // 点赞 按钮
        ui->pushButton_3->setObjectName("PersonalCenterSettingsMainPage_btn_like");
        ui->pushButton_3->setCheckable(true);
        ui->pushButton_3->setCursor(Qt::PointingHandCursor); //手型
        ui->pushButton_3->setStyleSheet(R"(
            #PersonalCenterSettingsMainPage_btn_like {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #FFFFFF;
                border: none;
            }
            #PersonalCenterSettingsMainPage_btn_like:hover {
                color: #009FEF;
                border: none;
            }
            #PersonalCenterSettingsMainPage_btn_like:checked {
                color: #009FEF;
                border-bottom: 2px solid #4A90D9;
            }
        )");
    }
    {
        // 方案操作 按钮（预留）
        ui->pushButton_upload_plan->setCursor(Qt::PointingHandCursor);
        ui->pushButton_upload_plan->setObjectName("PersonalCenterSettingsMainPage_btn_plan_action");
        ui->pushButton_upload_plan->setStyleSheet(R"(
            #PersonalCenterSettingsMainPage_btn_plan_action {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #FFFFFF;
                border-image: url(:/Skin/Images/Popup/confirm-no.png);
            }
            #PersonalCenterSettingsMainPage_btn_plan_action:hover {
               border-image: url(:/Skin/Images/Popup/confirm-ho.png);
            }
        )");

    }
    {
        // 左侧：用户信息面板
        clp_user_info_ = new CustomQWidgetUserInfoSettings(ui->widget_user_info);
        clp_user_info_->setObjectName("PersonalCenterSettingsMainPage_user_info");
        ui->widget_user_info->layout()->addWidget(clp_user_info_);
    }
    {
        // 左侧：用户信息变更面板（与用户信息面板共享同一容器，默认隐藏）
        clp_user_info_change_page_ = new CustomQWidgetUserInfoChange(ui->widget_user_info, theme);
        clp_user_info_change_page_->setObjectName(
            "PersonalCenterSettingsMainPage_user_info_change");
        clp_user_info_change_page_->hide();
    }
    {
        // 加载中图标，默认隐藏
        ui->widget_refresh_wait_icon->show();
        auto *t_loading_layout = new QHBoxLayout(ui->widget_refresh_wait_icon);
        t_loading_layout->setContentsMargins(0, 0, 0, 0);
        t_loading_layout->setSpacing(0);

        clp_plans_loading_ = new CustomQWidgetLoading(ui->widget_refresh_wait_icon);
        clp_plans_loading_->setFixedSize(kPlansLoadingWidth, kPlansLoadingHeight);
        setPlansLoadingText(tr("加载中"));
        clp_plans_loading_->hide();
        t_loading_layout->addWidget(clp_plans_loading_, 0, Qt::AlignCenter);

        clp_uploaded_no_more_label_ = new QLabel(ui->widget_refresh_wait_icon);
        clp_uploaded_no_more_label_->setFixedHeight(kPlansBottomStatusHeight);
        clp_uploaded_no_more_label_->setMinimumWidth(kUploadedNoMoreMinWidth);
        clp_uploaded_no_more_label_->setAlignment(Qt::AlignCenter);
        clp_uploaded_no_more_label_->setStyleSheet(
            QStringLiteral("color:#8a94a6;font-size:12px;background:transparent;"));
        clp_uploaded_no_more_label_->hide();
        t_loading_layout->addWidget(clp_uploaded_no_more_label_, 0, Qt::AlignCenter);
    }
}

/// \brief 初始化内部成员
void PersonalCenterSettingsMainPage::InitMember()
{
    // 顶部标签按钮互斥组：上传 / 点赞 二选一
    {
        clp_tab_button_group_ = new QButtonGroup(this);
        clp_tab_button_group_->setExclusive(true);
        clp_tab_button_group_->addButton(ui->pushButton_2);
        clp_tab_button_group_->addButton(ui->pushButton_3);
        ui->pushButton_2->setChecked(true); // 默认选中「上传」
    }
    {
        UploadMyplans = new UploadMyPlans(m);
        UploadMyplans->setModal(true);
        UploadMyplans->adjustSize();
        QRect t_geom = m->geometry();
        // 上传成功 → 刷新"已上传"列表（DataSync 联动）
        connect(UploadMyplans, &UploadMyPlans::planUploaded, this,
                [this](int) { fetchUploadedPage(1); });
        UploadMyplans->move(t_geom.center() - UploadMyplans->rect().center());
    }
    {
        UploadPlanOk = new UploadPlanSuccess(m);
        UploadPlanOk->setModal(true);
        UploadPlanOk->adjustSize();
        QRect t_geom = m->geometry();
        UploadPlanOk->move(t_geom.center() - UploadPlanOk->rect().center());
    }

    // 默认显示已上传页面
    ui->stackedWidget_plans->setCurrentIndex(0);
    fetchMyUploadedConfigPage(1);
}

/// \brief 连接默认信号槽
void PersonalCenterSettingsMainPage::InitConnect()
{
    // 点击 编辑资料 → 切换到用户信息变更面板
    connect(
        clp_user_info_->clp_edit_profile_button_,
        &QPushButton::clicked,
        this,
        [this]() {
            auto *t_layout = ui->widget_user_info->layout();
            clp_user_info_->hide();
            t_layout->removeWidget(clp_user_info_);

            // 更新 个人信息 编辑页面
            {
                clp_user_info_change_page_->setCl_avatar(
                    g_user_information.avatarFilePath()); //更新为本地头像
                clp_user_info_change_page_->setCl_nickname(g_user_information.network.username); //更新昵称
                clp_user_info_change_page_->setCl_signature(g_user_information.network.bio); //更新个性签名
            }

            t_layout->addWidget(clp_user_info_change_page_);
            clp_user_info_change_page_->show();
        },
        Qt::UniqueConnection);

    // 点击 取消 → 返回用户信息面板
    connect(
        clp_user_info_change_page_->clp_cancel_btn_,
        &QPushButton::clicked,
        this,
        [this]() {
            auto *t_layout = ui->widget_user_info->layout();
            clp_user_info_change_page_->hide();
            t_layout->removeWidget(clp_user_info_change_page_);
            t_layout->addWidget(clp_user_info_);
            clp_user_info_->show();
        },
        Qt::UniqueConnection);

    // 点击 确认 → 提交用户信息更新
    connect(
        clp_user_info_change_page_->clp_confirm_btn_,
        &QPushButton::clicked,
        this,
        [this]() {
            // 获取编辑后的值
            QString t_new_nickname = clp_user_info_change_page_->cl_nickname();
            QString t_new_signature = clp_user_info_change_page_->cl_signature();
            QString t_new_avatar_path = cl_selected_avatar_path_;

            // 构建 UpdateUser 请求
            auto doUpdateUser = [this](const QString &t_nickname, const QString &t_signature,
                                       const QString &t_avatar_url) {
                DeSheng::UpdateUserRequest t_req;
                t_req.username = t_nickname;    //程序内 username 就是昵称，nickname 不使用
                t_req.bio = t_signature;
                if (!t_avatar_url.isEmpty())
                    t_req.avatar = t_avatar_url;

                QJsonObject t_body = DeSheng::UpdateUserRequestToJson(t_req);
                QByteArray t_data = QJsonDocument(t_body).toJson();
                QNetworkReply *t_reply = HttpClient::instance().put(
                    "/user",
                    RequestOptions{}.withBody(t_data).withTag("user"));
                connect(t_reply, &QNetworkReply::finished, this, [this, t_reply]() {
                    int t_status
                        = t_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    if (t_reply->error() != QNetworkReply::NoError) {
                        qWarning() << "UpdateUser failed: status=" << t_status
                                   << "error=" << t_reply->errorString();
                        auto *t_notif = new CustomQWidgetNotification(
                            tr("资料修改失败，请检查网络后重试"), QString(), this);
                        QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                                         t_notif, &QWidget::deleteLater);
                        t_notif->show();
                    } else {
                        QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
                        DeSheng::UpdateUserResponse t_resp;
                        if (DeSheng::ProcessUpdateUserResult(t_resp, t_doc)
                            && t_resp.code == "success") {
                            g_user_information.updateFromServer(t_resp.data);

                            // 更新 UI 并切回用户信息面板
                            auto t_finishUpdate = [this]() {
                                UpdatePersonalCenterSettingsUIInformation();
                                emit userInfoUpdated();
                                auto *t_l = ui->widget_user_info->layout();
                                clp_user_info_change_page_->hide();
                                t_l->removeWidget(clp_user_info_change_page_);
                                t_l->addWidget(clp_user_info_);
                                clp_user_info_->show();
                            };

                            // 下载新头像到本地用户目录
                            if (!g_user_information.network.avatar.isEmpty()
                                && g_user_information.network.avatar.startsWith("http")) {
                                QString t_user_dir = g_user_information.userDirName();
                                QDir().mkpath(t_user_dir);
                                QString t_local = g_user_information.avatarFilePath();
                                QNetworkReply *t_avatar_reply
                                    = HttpClient::instance().manager()->get(
                                        QNetworkRequest(QUrl(g_user_information.network.avatar)));
                                connect(
                                    t_avatar_reply, &QNetworkReply::finished, this,
                                    [this, t_avatar_reply, t_local, t_finishUpdate]() {
                                        if (t_avatar_reply->error() != QNetworkReply::NoError) {
                                            qWarning() << "Avatar download failed:"
                                                       << t_avatar_reply->errorString();
                                        } else {
                                            QPixmap t_pm;
                                            t_pm.loadFromData(t_avatar_reply->readAll());
                                            t_pm.save(t_local, "PNG");
                                        }
                                        t_avatar_reply->deleteLater();
                                        t_finishUpdate();
                                    });
                            } else {
                                t_finishUpdate();
                            }
                        } else {
                            qWarning() << "UpdateUser API error: code=" << t_resp.code
                                       << "message=" << t_resp.message;
                            // 区分错误类型
                            QString t_err_msg;
                            if (t_resp.message.contains("昵称")
                                || t_resp.message.contains("username", Qt::CaseInsensitive)) {
                                t_err_msg = tr("昵称包含敏感词，请修改后重试");
                            } else if (t_resp.message.contains("签名")
                                       || t_resp.message.contains("bio", Qt::CaseInsensitive)
                                       || t_resp.message.contains("signature",
                                                                  Qt::CaseInsensitive)) {
                                t_err_msg = tr("个性签名包含敏感词，请修改后重试");
                            } else {
                                t_err_msg = tr("资料修改失败：%1").arg(t_resp.message);
                            }
                            auto *t_notif = new CustomQWidgetNotification(t_err_msg, QString(), this);
                            QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                                             t_notif, &QWidget::deleteLater);
                            t_notif->show();
                        }
                    }
                    t_reply->deleteLater();
                });
            };

            // 如果选择了系统头像（本地资源路径）→ 先上传再更新
            if (!t_new_avatar_path.isEmpty()
                && t_new_avatar_path.startsWith(":/")) {
                QString t_user_dir = g_user_information.userDirName();
                QDir().mkpath(t_user_dir);
                QString t_tmp_file = t_user_dir + "/avatar_upload_tmp.png";
                QPixmap(t_new_avatar_path).save(t_tmp_file, "PNG");

                QFile *t_file = new QFile(t_tmp_file);
                if (!t_file->open(QIODevice::ReadOnly)) {
                    delete t_file;
                    doUpdateUser(t_new_nickname, t_new_signature, QString());
                    return;
                }
                QHttpMultiPart *t_multi = new QHttpMultiPart(QHttpMultiPart::FormDataType);
                QHttpPart t_file_part;
                t_file_part.setHeader(QNetworkRequest::ContentDispositionHeader,
                                      QVariant("form-data; name=\"file\"; filename=\"avatar.png\""));
                t_file_part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
                t_file_part.setBodyDevice(t_file);
                t_file->setParent(t_multi);
                t_multi->append(t_file_part);

                QNetworkReply *t_upload_reply = HttpClient::instance().upload(
                    "/user/uploads",
                    t_multi,
                    RequestOptions{}.withTag("user"));
                t_multi->setParent(t_upload_reply);
                connect(t_upload_reply, &QNetworkReply::finished, this,
                        [this, t_upload_reply, t_file, t_tmp_file, t_new_nickname,
                         t_new_signature, doUpdateUser]() {
                            int t_status = t_upload_reply
                                               ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                                               .toInt();
                            QString t_avatar_url;
                            if (t_upload_reply->error() == QNetworkReply::NoError) {
                                QJsonDocument t_doc = QJsonDocument::fromJson(
                                    t_upload_reply->readAll());
                                DeSheng::FileUploadsResponse t_resp;
                                if (DeSheng::ProcessFileUploadsResult(t_resp, t_doc)
                                    && t_resp.code == "success") {
                                    t_avatar_url = t_resp.data.url;
                                } else {
                                    qWarning() << "Avatar upload API error: code=" << t_resp.code
                                               << "message=" << t_resp.message;
                                }
                            } else {
                                qWarning() << "Avatar upload failed: status=" << t_status
                                           << "error=" << t_upload_reply->errorString();
                            }
                            doUpdateUser(t_new_nickname, t_new_signature, t_avatar_url);
                            t_file->close();
                            QFile::remove(t_tmp_file);
                            t_upload_reply->deleteLater();
                        });
                return;
            }

            // 没有头像变更 → 直接更新
            doUpdateUser(t_new_nickname, t_new_signature, t_new_avatar_path);
        },
        Qt::UniqueConnection);

    // 点击 编辑页面中的 头像 进行编辑
    connect(
        clp_user_info_change_page_,
        &CustomQWidgetUserInfoChange::editAvatarRequested,
        this,
        [this]() {
            if (!clp_avatar_selection_) {
                // 头像编辑页面
                clp_avatar_selection_ = new CustomQWidgetAvatarSelection(this);
                // 头像选中 → 更新编辑页头像
                connect(clp_avatar_selection_,
                        &CustomQWidgetAvatarSelection::avatarSelected,
                        this,
                        [this](int t_index) {
                            QString t_path = CustomQWidgetAvatarSelection::cl_avatar_res_map_.value(
                                t_index);
                            if (!t_path.isEmpty()) {
                                cl_selected_avatar_path_ = t_path;
                                clp_user_info_change_page_->setCl_avatar(QPixmap(t_path));
                            }
                            clp_avatar_selection_->close();
                        },
                        Qt::UniqueConnection);
            }
            // 延迟到事件循环后定位，确保头像标签 geometry 已结算
            // 注意：clp_avatar_selection_ 有 Qt::Dialog 标志，是顶层窗口，move() 用屏幕坐标
            QTimer::singleShot(0, this, [this]() {
                QPoint t_global = clp_user_info_change_page_->clp_avatar_label_->mapToGlobal(
                    clp_user_info_change_page_->clp_avatar_label_->rect().center());
                clp_avatar_selection_->move(t_global);
                clp_avatar_selection_->show();
            });

        },
        Qt::UniqueConnection);

    // Tab 切换：上传 — 无缓存才拉，已有数据直接展示
    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget_plans->setCurrentIndex(0);
        if (!clp_uploaded_model_ || clp_uploaded_model_->isEmpty())
            fetchMyUploadedConfigPage(1);
        else
            updateUploadedNoMoreStatus();
    }, Qt::UniqueConnection);

    // Tab 切换：点赞 — 总是重拉（排行榜点赞不经 feed 本地同步，且覆盖服务端外部变更；
    // 注：刚点赞立刻切 Tab 时重拉结果可能先于点赞提交到达（理论竞态，概率低，下次刷新自愈）
    connect(ui->pushButton_3, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget_plans->setCurrentIndex(1);
        hideUploadedNoMoreStatus();
        fetchMyLikedConfigPage(1);
    }, Qt::UniqueConnection);

    // 上传方案 → 强制刷新
    connect(ui->pushButton_upload_plan, &QPushButton::clicked,
            this, [this]() { fetchMyUploadedConfigPage(1); },
            Qt::UniqueConnection);

}

//上传方案
void PersonalCenterSettingsMainPage::on_pushButton_upload_plan_clicked()
{
    ui->pushButton_upload_plan->setEnabled(false);
    //先判断用户当天上传的方案是否超过十个，超过则不可再上传
    int cnt = 10;
    cnt = GetUplodPlanCnt();
    if(cnt == -1)
    {
        ui->pushButton_upload_plan->setEnabled(true);
        return;
    }else if(cnt >= 10)
    {
        auto *t_notif = new CustomQWidgetNotification(
            tr("今日上传方案已达上限，请明天再来"), QString(), this);
        QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                         t_notif, &QWidget::deleteLater);
        t_notif->show();
        ui->pushButton_upload_plan->setEnabled(true);
        return;
    }

    QRect t_pRect = m->geometry();
    UploadMyplans->adjustSize();
    int x = t_pRect.x() + (t_pRect.width() - UploadMyplans->width() + 80) / 2;
    int y = t_pRect.y() + (t_pRect.height() - UploadMyplans->height() + 30) / 2;
    UploadMyplans->move(x, y);
    UploadMyplans->showMyPlans();
    int res = UploadMyplans->exec();
    if(res == QDialog::Accepted)
    {
        UploadPlanOk->ShowUploadPlanCnt(GetUplodPlanCnt());
        UploadPlanOk->adjustSize();
        UploadPlanOk->move(t_pRect.x() + (t_pRect.width() - UploadPlanOk->width() + 80) / 2,
                           t_pRect.y() + (t_pRect.height() - UploadPlanOk->height() + 30) / 2);
        UploadPlanOk->exec();
        // 上传成功 → 已上传列表即时刷新（新方案入列）
        fetchMyUploadedConfigPage(1);

    }

    ui->pushButton_upload_plan->setEnabled(true);
}

//获得今天已上传的方案数量
int PersonalCenterSettingsMainPage::GetUplodPlanCnt()
{
    int cnt = 0;
    // 今日创建数 → BaseClient
    QUrlQuery t_query;
    t_query.addQueryItem("device_name", SelDev_DeviceName);
    QNetworkReply *reply = HttpClient::instance().get(
        DeSheng::ApiPaths::kConfigTodayCount,
        RequestOptions{}.withQuery(t_query).withTag("userConfig"));

    // 使用 QEventLoop 同步等待
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, [&]() {
        // 保证结束时退出事件循环
        loop.quit();
    });

    // 连接 finished 信号
    connect(reply, &QNetworkReply::finished, this, [this, &cnt, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit ApoManager::instance()->requestlogWithTime(QString("%1 %2 netWorkReply error:%3")
                                                                .arg(__FUNCTION__)
                                                                .arg(__LINE__)
                                                                .arg(reply->errorString()));

            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            // 读取响应数据
            QByteArray responseData = reply->readAll();
            qDebug() << "获取今日创建方案数量请求 回显原始数据:" << QString::fromUtf8(responseData);



            // 解析 JSON
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = "JSON解析错误: " + parseError.errorString();
                emit ApoManager::instance()->requestlogWithTime(
                    QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg(errorMsg));

                return;
            }

            DeSheng::GetTodayCountListResponse t_today_resp;
            bool result = DeSheng::ProcessGetTodayCountListResult(t_today_resp, jsonDoc);
            if (result) {
                QString name = t_today_resp.data.device_name;
                cnt = t_today_resp.data.today_count;
                qDebug("获取今日创建方案数量成功，%d\n", cnt);
            } else {
                qDebug("获取今日创建方案数量失败\n");
                if (t_today_resp.code.contains("forbidden"))
                {
                    auto *t_notif = new CustomQWidgetNotification(tr("获取今日已上传方案数量失败，权限不足"), QString(), this);
                    QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                                     t_notif, &QWidget::deleteLater);
                    t_notif->show();
                }
                cnt = -1;

            }

        }
    });

    // 启动事件循环，直到请求结束
    loop.exec();
    reply->deleteLater();

    return cnt;
}

