#include "modules/CommunityModule/ui/ranking/ranking_list.h"
#include "ui_ranking_list.h"

#include <QGraphicsDropShadowEffect>
#include <QListView>
#include <QPainterPath>
#include <QScrollBar>
#include <QTimer>

#include "LoadLib.h"  ///< extern MainWindow *m（importDownloadedPlan 导入方案库）
#include "repository/ranking_helper.h"

namespace {
constexpr int kRankingPanelWidth = 400;          ///< 排行榜弹窗和内部 frame 的固定宽度。
constexpr int kRankingListViewWidth = 390;       ///< 榜单列表视图宽度，右侧预留滚动条区域。
constexpr int kRankingListTop = 77;              ///< 榜单列表顶部 y，避开标题和筛选控件区域。
constexpr int kRankingListBottomMargin = 30;     ///< 榜单列表底部与弹窗底部的距离。
constexpr int kRankingLoadingBottomGap = 27;     ///< loading 区域位于 listView 底部后的预留高度。
constexpr qreal kPanelRadius = 20.0;             ///< 弹窗默认圆角半径。
constexpr qreal kPanelTopRightRadius = 4.0;      ///< 弹窗右上角圆角半径。

QPainterPath buildPanelPath(const QRect &rect)
{
    QRectF t_rect(rect);
    QPainterPath t_path;
    if (t_rect.isEmpty())
        return t_path;

    const qreal t_max_radius = qMin(t_rect.width(), t_rect.height()) / 2.0;
    const qreal t_tl_radius = qMin(kPanelRadius, t_max_radius);
    const qreal t_tr_radius = qMin(kPanelTopRightRadius, t_max_radius);
    const qreal t_br_radius = qMin(kPanelRadius, t_max_radius);
    const qreal t_bl_radius = qMin(kPanelRadius, t_max_radius);

    t_path.moveTo(t_rect.left() + t_tl_radius, t_rect.top());
    t_path.lineTo(t_rect.right() - t_tr_radius, t_rect.top());
    t_path.quadTo(t_rect.right(), t_rect.top(), t_rect.right(), t_rect.top() + t_tr_radius);
    t_path.lineTo(t_rect.right(), t_rect.bottom() - t_br_radius);
    t_path.quadTo(t_rect.right(), t_rect.bottom(), t_rect.right() - t_br_radius, t_rect.bottom());
    t_path.lineTo(t_rect.left() + t_bl_radius, t_rect.bottom());
    t_path.quadTo(t_rect.left(), t_rect.bottom(), t_rect.left(), t_rect.bottom() - t_bl_radius);
    t_path.lineTo(t_rect.left(), t_rect.top() + t_tl_radius);
    t_path.quadTo(t_rect.left(), t_rect.top(), t_rect.left() + t_tl_radius, t_rect.top());
    t_path.closeSubpath();
    return t_path;
}
}  // namespace

RankingList::RankingList(QWidget *parent, SchemeService *svc, UserConfigRepository *repo)
    : QWidget(parent)
      , ui(new Ui::RankingList)
{
    ui->setupUi(this);
    setFixedWidth(kRankingPanelWidth);
    setMinimumHeight(1);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    ui->frame->setAttribute(Qt::WA_StyledBackground, true);
    ui->frame->setFixedWidth(kRankingPanelWidth);
    ui->frame->setMinimumHeight(1);
    ui->frame->setMaximumHeight(QWIDGETSIZE_MAX);
    ui->frame->setFrameShape(QFrame::NoFrame);
    ui->frame->setFrameShadow(QFrame::Plain);
    ui->frame->setStyleSheet("border-radius: 20px;border-top-right-radius: 4px;background: #263040;");
    cl_update_round_clip_();

    // 数据层：优先注入社区共享 service/repo（排行榜点赞/下载与社区 5-model 双向同步）；
    // 未注入时自建（独立模式，仅排行榜内部生效）
    if (svc && repo) {
        clp_service_ = svc;
        clp_repo_ = repo;
    } else {
        clp_repo_ = new UserConfigRepository(this);
        clp_service_ = new SchemeService(this);
        clp_service_->init(clp_repo_);
    }

    InitMember();
    // 点击列表空白/其他条目 → 关闭方案卡片（viewport 事件过滤；后装先调，先于内部 scrollarea 过滤器执行）
    ui->listView->viewport()->installEventFilter(this);
    InitConnect();
}

RankingList::~RankingList()
{
    delete clp_plan_card_;
    clp_plan_card_ = nullptr;
    delete ui;
}

void RankingList::LanguageSet()
{
    ui->retranslateUi(this);
    if (clp_loading_) {
        clp_loading_->setCl_text(tr("加载中"));
    }
    if (clp_retry_btn_) {
        clp_retry_btn_->setText(tr("刷新"));
    }
}

void RankingList::InitMember()
{
    // 数据层（clp_repo_/clp_service_ 已在构造函数注入或自建）
    clp_model_ = new RankingModel(this);
    clp_delegate_ = new RankingDelegate(this);

    // 视图
    ui->listView->setModel(clp_model_);
    ui->listView->setItemDelegate(clp_delegate_);
    ui->listView->setSpacing(5);
    ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 双击不进入编辑态（否则产生 QLineEdit 且二次触发 editorEvent）
    ui->listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->listView->setUniformItemSizes(true);
    ui->listView->setFrameShape(QFrame::NoFrame);
    ui->listView->setViewportMargins(20, 0, 10, 0);  // 行距左边 20（对齐旧布局），右侧留滚动条位
    ui->listView->setStyleSheet(
        "QListView{background:transparent;border:none;}"
        "QListView::item{border:none;background:transparent;outline:0;}");
    ui->listView->verticalScrollBar()->setStyleSheet(R"(
    QScrollBar:vertical {
        background-color: transparent;
        width: 10px;
        margin: 0px;
        padding: 0px;
        border-radius: 5px;
    }
    QScrollBar::handle:vertical {
        background: rgba(0, 0, 0,51);
        border-radius: 5px;
        min-height: 97px;
    }
    QScrollBar::sub-line:vertical,
    QScrollBar::add-line:vertical {
        height: 0px;
        background: none;
    }
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical {
        background: none;
    }
)");

    // 语言切换下拉框（样式与旧版一致）
    const QString t_combo_style = R"(
    QComboBox {
        font-family: "Noto Sans S Chinese";
        font-size: 12px;
        font-weight: 500;
        color: #A1A8B3;
        background-color: rgba(0, 0, 0, 0.2);
        border-radius: 2px;
        padding-left: 10px;
    }
    QComboBox::drop-down {
        image: url(:/Skin/Images/more/interface_settings/comBox_drop_down_darkBlue_.png);
        subcontrol-origin: padding;
        subcontrol-position: center right;
        margin-right: 10px;
        width: 9px;
        height: 12px;
    }
    QComboBox::drop-down:checked {
        width: 12px;
        height: 12px;
        image: url(:/Skin/Images/more/interface_settings/comBox_drop_down_checked_darkBlue.png);
    }
)";
    ui->cBox_list->setStyleSheet(t_combo_style);
    cl_set_combo_shadow_();
    ui->cBox_list->setPopupOffsetXY(-8, 2);
    ui->cBox_list->setCurrentIndex(0);
    // 下拉列表样式（旧版一致：无滚动条、禁自动滚动、高度随项数增长）
    const QString t_list_view_style = R"(
    QListView {
        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 12px;
        background: #0D0F14;
        border-radius: 6px;
        padding-left: 6px;
        padding-right: 6px;
        padding-top: 6px;
        padding-bottom: 6px;
        outline: 0;
    }
    QListView::item {
        width: 243px;
        height: 25px;
        margin-top: 4px;
        margin-bottom: 4px;
        margin-left: 6px;
        margin-right: 6px;
        padding-left: 6px;
        color: #A1A8B3;
        background-color: transparent;
        outline: 0;
    }
    QListView::item:hover {
        background-color: rgba(223, 243, 255, 0.2);
        border-radius: 4px;
    }
    QListView::item:selected {
        background-color: #0091DA;
        border-radius: 4px;
        color: #FFFFFF;
    }
)";
    auto *t_list_view = new QListView();
    t_list_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    t_list_view->setAutoScroll(false);
    t_list_view->setStyleSheet(t_list_view_style);
    ui->cBox_list->setView(t_list_view);
    ui->cBox_list->setMaxVisibleItems(INT_MAX);

    // Tab 按钮（点赞/下载）
    ui->Like_leaderboard_pbt->setCheckable(true);
    ui->Download_rankings_pbt->setCheckable(true);
    clp_tab_group_ = new QButtonGroup(this);
    clp_tab_group_->addButton(ui->Like_leaderboard_pbt, 0);
    clp_tab_group_->addButton(ui->Download_rankings_pbt, 1);
    const QString t_tab_style =
        "QPushButton {"
        "  border: none;"
        "  font-size: 12px;"
        "  color: #747880;"
        "  font-family: \"Noto Sans S Chinese\";"
        "  font-weight: 500;"
        "}"
        "QPushButton:checked {"
        "  font-size: 16px;"
        "  color: #FFD42D;"
        "}";
    ui->Like_leaderboard_pbt->setStyleSheet(t_tab_style);
    ui->Download_rankings_pbt->setStyleSheet(t_tab_style);
    ui->Like_leaderboard_pbt->setChecked(true);

    // loading / 空态
    clp_loading_ = new CustomQWidgetLoading(this);
    clp_loading_->setCl_text(tr("加载中"));
    clp_loading_->start();
    // 空态/错误图标（视口内坐标：画布 y − 77 视口起点；x 水平居中）— 04 错误态图标 96×96 @y87
    clp_empty_icon_label_ = new QLabel(ui->listView->viewport());
    clp_empty_icon_label_->setAlignment(Qt::AlignCenter);
    clp_empty_icon_label_->setCursor(Qt::PointingHandCursor);
    clp_empty_icon_label_->hide();
    clp_empty_icon_label_->installEventFilter(this);

    // 空态/失败提示文字（视口内坐标；16px；点击重试）
    clp_empty_label_ = new QLabel(ui->listView->viewport());
    clp_empty_label_->setAlignment(Qt::AlignCenter);
    clp_empty_label_->setStyleSheet(
        "font-family:\"Noto Sans S Chinese\";font-size:16px;color:#747880;background:transparent;");
    clp_empty_label_->setCursor(Qt::PointingHandCursor);
    clp_empty_label_->hide();
    clp_empty_label_->installEventFilter(this);

    // 刷新按钮（加载失败时显示，rect 148,395,104,30；样式与"上传方案"一致 confirm 图）
    clp_retry_btn_ = new QPushButton(tr("刷新"), this);
    clp_retry_btn_->setGeometry(148, 395, 104, 30);
    clp_retry_btn_->setCursor(Qt::PointingHandCursor);
    clp_retry_btn_->setStyleSheet(
        "QPushButton{font-family:\"Noto Sans S Chinese\";font-weight:500;font-size:12px;"
        "color:#FFFFFF;border-image:url(:/Skin/Images/Popup/confirm-no.png);}"
        "QPushButton:hover{border-image:url(:/Skin/Images/Popup/confirm-ho.png);}");
    clp_retry_btn_->hide();
    connect(clp_retry_btn_, &QPushButton::clicked, this, [this] { cl_refresh_data_(); });

    // 轻提示（下载失败等，2 秒自动隐藏）
    clp_toast_label_ = new QLabel(this);
    clp_toast_label_->setAlignment(Qt::AlignCenter);
    clp_toast_label_->setStyleSheet(
        "font-family:\"Noto Sans S Chinese\";font-size:12px;color:#FFFFFF;"
        "background:rgba(0,0,0,180);border-radius:4px;padding:4px 12px;");
    clp_toast_label_->hide();

    // 方案卡片（复用共享控件，数据在 cl_populate_card_ 填充）。
    // 打开时会重挂到应用根窗口，避免被 RankingList 的圆角 mask 裁切。
    clp_plan_card_ = new CustomQWidgetSinglePlans(this, 0);
    clp_plan_card_->hide();

    cl_update_loading_position_();
}

void RankingList::InitConnect()
{
    // Tab 点赞/下载切换 → 图标系列 + 热度字段 + 刷新
    connect(clp_tab_group_, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, [this](int t_id) {
                cl_is_like_tab_ = (t_id == 0);
                clp_delegate_->setRankingType(cl_is_like_tab_ ? 0 : 1);
                clp_model_->setHeatField(cl_is_like_tab_);
                cl_refresh_data_();
            });
    // 月度/总榜切换 → 刷新
    connect(ui->cBox_list, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { cl_refresh_data_(); });
    // 行交互
    connect(clp_delegate_, &RankingDelegate::buttonClicked,
            this, &RankingList::cl_on_button_clicked_);
    // 2026-08-07 恢复：行内点方案名弹出方案卡片（点击处定位 + 屏幕自适应；8-05 曾屏蔽）
    connect(clp_delegate_, &RankingDelegate::planNameClicked,
            this, [this](const QModelIndex &t_index, const QPoint &t_pos) {
                cl_populate_card_(t_index.row(), t_pos);
            });
    // 头像下载完成 → 写回模型
    connect(clp_repo_, &UserConfigRepository::avatarReady, this,
            [this](int t_config_id, const QPixmap &t_pm) {
                const int t_row = clp_model_->rowOfConfigId(t_config_id);
                if (t_row >= 0)
                    clp_model_->setAvatar(t_row, t_pm);  // 过期回执（id 已不在榜单）自动丢弃
            });
    // likeToggled：新状态 = 乐观确认（本地不应用，等 countsSynced 服务器计数）；原状态 = 失败回执（解锁防连点）
    connect(clp_service_, &SchemeService::likeToggled, this,
            [this](int t_config_id, bool t_state) {
                if (!cl_pending_likes_.contains(t_config_id))
                    return;
                const auto t_it = cl_pending_like_originals_.constFind(t_config_id);
                if (t_it == cl_pending_like_originals_.constEnd())
                    return;
                if (t_state != t_it.value())
                    return;  // 乐观确认（新状态），本地不应用
                // 失败回执（原状态）：无本地改动可回滚，仅解锁防连点
                cl_pending_likes_.remove(t_config_id);
                cl_pending_like_originals_.remove(t_config_id);
            });
    // 服务器真实计数覆盖（点赞 pending 或下载进行中才应用）
    connect(clp_service_, &SchemeService::countsSynced, this,
            [this](const DeSheng::GetPublicConfigurationListResponse::ListItem &t_item) {
                const bool t_like_pending = cl_pending_likes_.contains(t_item.id);
                const bool t_downloading = cl_downloading_ids_.contains(t_item.id);
                if (!t_like_pending && !t_downloading)
                    return;
                const int t_row = clp_model_->rowOfConfigId(t_item.id);
                if (t_row >= 0)
                    clp_model_->applyServerItem(t_row, t_item);
                cl_pending_likes_.remove(t_item.id);
                cl_pending_like_originals_.remove(t_item.id);
                if (t_downloading) {
                    cl_downloading_ids_.remove(t_item.id);
                    clp_model_->setDownloading(t_item.id, false);
                }
            });
    // 下载完成 → 导入方案库 + 拉取真实计数（countsSynced 负责收尾状态）
    connect(clp_service_, &SchemeService::downloadFileSaved, this,
            [this](int t_config_id, const QString &t_path) {
                if (!cl_downloading_ids_.contains(t_config_id))
                    return;
                if (m)
                    m->importDownloadedPlan(t_path);
                clp_service_->refreshCounts(t_config_id);
            });
    // 下载进度 → 模型（行内圆环绘制；-1 = 服务端失败标记，忽略）
    connect(clp_service_, &SchemeService::downloadProgress, this,
            [this](int t_config_id, int t_percent) {
                if (!cl_downloading_ids_.contains(t_config_id))
                    return;
                if (t_percent < 0)
                    return;
                clp_model_->setDownloadProgress(t_config_id, t_percent);
            });
    // 下载失败 → 按 configId 精准解除状态 + 原因提示（服务端已删除时提示"配置不存在"并移除该行）
    connect(clp_service_, &SchemeService::downloadFailed, this,
            [this](int t_config_id, const QString &t_reason) {
                cl_downloading_ids_.remove(t_config_id);
                clp_model_->setDownloading(t_config_id, false);
                if (t_reason.contains(QStringLiteral("不存在")))
                    clp_model_->removeByConfigId(t_config_id);  // 与社区 5-model 移除一致
                cl_show_toast_(t_reason.contains(QStringLiteral("不存在"))
                                   ? t_reason
                                   : tr("下载失败，请检查网络"));
            });
}

void RankingList::cl_refresh_data_()
{
    const bool t_is_monthly = (ui->cBox_list->currentIndex() == 0);  // 0=月度 1=总榜

    clp_loading_->show();
    clp_loading_->start();
    clp_plan_card_->hide();  // 刷新时收起方案卡片
    cl_hide_empty_label_();  // 刷新开始即清旧空态/错误态（切榜时避免残留到新请求返回）
    // 新数据取代旧请求语境：进行中的点赞回执不再有意义（回滚锚点清空）
    cl_pending_likes_.clear();
    cl_pending_like_originals_.clear();

    RankingHelper::fetchTop(cl_is_like_tab_ ? "like" : "download", 100, t_is_monthly, this,
                            [this, t_is_monthly](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &t_list,
                                                 bool t_ok) {
        clp_loading_->stop();
        clp_loading_->hide();  // stop() 仅停定时器，须显式隐藏
        if (!t_ok) {
            clp_model_->clear();
            cl_show_empty_label_(tr("加载榜单失败，请检查网络"),
                                 QStringLiteral(":/Skin/Images/GeneralIcon/Empty/RankNetError.png"));
            clp_retry_btn_->show();
            clp_retry_btn_->raise();
            return;
        }
        if (t_list.isEmpty()) {
            clp_model_->clear();
            if (!t_is_monthly)
                emit topThreeDataReady(cl_is_like_tab_, t_list);
            cl_show_empty_label_(tr("暂无数据"));
            return;
        }
        cl_hide_empty_label_();

        clp_model_->replaceAll(t_list);
        if (!t_is_monthly)
            emit topThreeDataReady(cl_is_like_tab_, t_list);
        // 头像异步下载（配置 ID 作 key，避免刷新后行号错位）
        for (int t_i = 0; t_i < t_list.size(); ++t_i) {
            const QString t_url = t_list.at(t_i).author.avatar;
            if (!t_url.isEmpty())
                clp_repo_->fetchAvatar(t_list.at(t_i).id, t_url);  // 以配置 ID 为 key，刷新后行号会错位
        }
    });
}

void RankingList::cl_on_button_clicked_(const QModelIndex &t_index)
{
    if (!t_index.isValid())
        return;
    if (cl_is_like_tab_)
        cl_do_like_(t_index);
    else
        cl_do_download_(t_index);
}

void RankingList::cl_do_like_(const QModelIndex &t_index)
{
    const int t_config_id =
        t_index.data(static_cast<int>(RankingModel::RankingRole::PlanIdRole)).toInt();
    const bool t_liked =
        t_index.data(static_cast<int>(RankingModel::RankingRole::IsLikedRole)).toBool();
    if (t_config_id <= 0)
        return;
    if (cl_pending_likes_.contains(t_config_id))
        return;  // 防连点：请求进行中忽略
    cl_pending_likes_.insert(t_config_id);
    cl_pending_like_originals_.insert(t_config_id, t_liked);
    // 与社区一致：不本地翻转，等服务器 refreshCounts 真实计数（countsSynced 应用）
    clp_service_->toggleLike(t_config_id, t_liked);
}

void RankingList::cl_do_download_(const QModelIndex &t_index)
{
    const int t_config_id =
        t_index.data(static_cast<int>(RankingModel::RankingRole::PlanIdRole)).toInt();
    if (t_config_id <= 0)
        return;
    if (cl_downloading_ids_.contains(t_config_id))
        return;  // 防重复点击
    cl_downloading_ids_.insert(t_config_id);
    clp_model_->setDownloading(t_config_id, true);
    clp_service_->download(t_config_id);
}

void RankingList::cl_show_toast_(const QString &text)
{
    if (!clp_toast_label_)
        return;
    clp_toast_label_->setText(text);
    clp_toast_label_->adjustSize();
    clp_toast_label_->move((width() - clp_toast_label_->width()) / 2, height() - 45);
    clp_toast_label_->show();
    clp_toast_label_->raise();
    QTimer::singleShot(2000, clp_toast_label_, &QLabel::hide);
}

void RankingList::cl_populate_card_(int t_row, const QPoint &viewportPos)
{
    const auto t_item = clp_model_->itemAt(t_row);
    if (!t_item.has_value())
        return;
    if (!clp_plan_card_)
        return;

    const QModelIndex t_index = clp_model_->index(t_row);
    const QPixmap t_avatar =
        t_index.data(static_cast<int>(RankingModel::RankingRole::AvatarRole)).value<QPixmap>();

    clp_plan_card_->setCl_config_id(t_item->id);
    clp_plan_card_->setAvatarPixmap(t_avatar);  // 空头像 → setAvatarPixmap 内用默认图（且不残留上一卡片头像）
    clp_plan_card_->setAuthorInfo(t_item->author.nickname.isEmpty() ? t_item->author.username
                                                                    : t_item->author.nickname,
                                  t_item->author.level);
    clp_plan_card_->setAuthorBadges(t_item->author.roles, t_item->is_expert_tag);
    clp_plan_card_->setVisibility(t_item->visibility);
    clp_plan_card_->setPlanInfo(t_item->title, t_item->description, t_item->user_tags);
    clp_plan_card_->setCl_device_name(t_item->device_name);
    clp_plan_card_->setCl_scene(t_item->user_tags.join(QStringLiteral("+")));
    clp_plan_card_->setComments(t_item->comments);
    clp_plan_card_->setActionState(t_item->like_count, t_item->dislike_count,
                                   t_item->download_count, t_item->share_count,
                                   t_item->is_liked, t_item->is_disliked);

    QWidget *t_root = window();
    if (!t_root)
        t_root = this;
    if (clp_plan_card_->parentWidget() != t_root) {
        clp_plan_card_->hide();
        clp_plan_card_->setParent(t_root);
    }

    // 定位：点击处下方 + 应用窗口边界自适应。
    // 卡片显示在根窗口层，避免被排行榜 400x636 面板和圆角 mask 裁切。
    const QPoint t_click = t_root->mapFromGlobal(ui->listView->viewport()->mapToGlobal(viewportPos));
    const QSize t_card = clp_plan_card_->sizeHint();
    const int t_shadow_margin = 20;
    int t_x = t_click.x();
    int t_y = t_click.y() + 10;
    const QRect t_bounds = t_root->rect().adjusted(t_shadow_margin, t_shadow_margin,
                                                   -t_shadow_margin, -t_shadow_margin);
    // 垂直：下方空间不足 → 向上翻转（卡片底贴点击点 -10）
    if (t_y + t_card.height() > t_bounds.bottom())
        t_y = t_click.y() - 10 - t_card.height();
    // 水平：右侧空间不足 → 左移（卡片右缘贴窗口右缘 -8）
    if (t_x + t_card.width() > t_bounds.right() - 8)
        t_x = t_bounds.right() - 8 - t_card.width();
    t_x = qBound(t_bounds.left(), t_x, qMax(t_bounds.left(), t_bounds.right() - t_card.width()));
    t_y = qBound(t_bounds.top(), t_y, qMax(t_bounds.top(), t_bounds.bottom() - t_card.height()));
    clp_plan_card_->move(t_x, t_y);
    clp_plan_card_->show();
    clp_plan_card_->raise();
}

void RankingList::cl_show_empty_label_(const QString &text, const QString &iconPath)
{
    if (!clp_empty_label_)
        return;
    // 视口内坐标（画布 y − 77 视口起点；x 水平居中）：值来自 tools/ui_mockups/04-05
    const int t_vw = ui->listView->viewport()->width();
    if (iconPath.isEmpty()) {
        // 空态：纯文字 @(0,173,宽,30)
        clp_empty_icon_label_->hide();
        clp_empty_label_->setGeometry(0, 173, t_vw, 30);
        clp_empty_label_->setText(text);
    } else {
        // 错误态：图标 96×96 @y87 + 下方文字 @y252
        const QPixmap t_pm(iconPath);
        clp_empty_icon_label_->setGeometry((t_vw - 96) / 2, 87, 96, 96);
        clp_empty_icon_label_->setPixmap(
            t_pm.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        clp_empty_icon_label_->show();
        clp_empty_label_->setGeometry(0, 252, t_vw, 20);
        clp_empty_label_->setText(text);
    }
    clp_empty_label_->show();
    clp_empty_label_->raise();
}

void RankingList::cl_hide_empty_label_()
{
    if (clp_empty_icon_label_)
        clp_empty_icon_label_->hide();
    if (clp_empty_label_)
        clp_empty_label_->hide();
    if (clp_retry_btn_)
        clp_retry_btn_->hide();
}

void RankingList::cl_update_loading_position_()
{
    if (!clp_loading_)
        return;
    const int t_container_bottom = ui->listView->geometry().bottom();
    const int t_loading_y = t_container_bottom
                            + (kRankingLoadingBottomGap - clp_loading_->height()) / 2;
    const int t_loading_x = (width() - clp_loading_->width()) / 2;
    clp_loading_->move(t_loading_x, t_loading_y);
}

void RankingList::cl_set_combo_shadow_()
{
    QWidget *t_container = ui->cBox_list->view()->parentWidget();
    if (!t_container)
        return;
    t_container->setWindowFlags(t_container->windowFlags() | Qt::FramelessWindowHint
                                | Qt::NoDropShadowWindowHint);
    t_container->setAttribute(Qt::WA_TranslucentBackground);
    t_container->setFixedWidth(146);
    if (t_container->layout())
        t_container->layout()->setContentsMargins(8, 8, 8, 8);
    QGraphicsDropShadowEffect *t_shadow = new QGraphicsDropShadowEffect(t_container);
    t_shadow->setBlurRadius(8);
    t_shadow->setColor(QColor(0, 0, 0, 128));
    t_shadow->setOffset(0, 4);
    t_container->setGraphicsEffect(t_shadow);
    ui->cBox_list->setPopupContainer(t_container);
}

void RankingList::cl_update_round_clip_()
{
    setMask(buildPanelPath(rect()).toFillPolygon().toPolygon());
    ui->frame->setMask(buildPanelPath(ui->frame->rect()).toFillPolygon().toPolygon());
}

// 每次打开弹窗自动刷新（保留当前榜类型）
void RankingList::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    cl_refresh_data_();
}

void RankingList::hideEvent(QHideEvent *event)
{
    if (clp_plan_card_)
        clp_plan_card_->hide();
    QWidget::hideEvent(event);
}

void RankingList::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    const int t_frame_height = ui->frame->height();
    int t_visible_height = t_frame_height - kRankingListTop - kRankingListBottomMargin;
    if (t_visible_height < 0)
        t_visible_height = 0;
    ui->listView->setGeometry(0, kRankingListTop, kRankingListViewWidth, t_visible_height);
    cl_update_round_clip_();
    cl_update_loading_position_();
}

// 空态标签点击 → 重试；列表空白/其他条目点击 → 关闭方案卡片
bool RankingList::eventFilter(QObject *obj, QEvent *event)
{
    // 空态/错误态图标与文字均可点击重试（原 HTML 组合拆为两个 label 后分别挂过滤）
    if ((obj == clp_empty_label_ || obj == clp_empty_icon_label_)
        && event->type() == QEvent::MouseButtonRelease) {
        cl_refresh_data_();
        return true;
    }
    if (obj == ui->listView->viewport() && event->type() == QEvent::MouseButtonPress
        && clp_plan_card_ && clp_plan_card_->isVisible()) {
        // 点列表其他条目/空白 → 关闭方案卡片（点击卡片自身落在卡片控件上而非 viewport，不经过此过滤）
        clp_plan_card_->hide();
    }
    return QWidget::eventFilter(obj, event);
}

void RankingList::mousePressEvent(QMouseEvent *event)
{
    // 兜底：点击弹窗无子控件区域（边框/空隙）时关闭卡片
    if (clp_plan_card_ && clp_plan_card_->isVisible())
        clp_plan_card_->hide();
    QWidget::mousePressEvent(event);
}
