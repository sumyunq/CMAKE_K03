#include "modules/GeneralCustomUI/custom_QWidget_single_plans.h"
#include "ui_custom_QWidget_single_plans.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include "Popup/Plans/DelReset.h"
#include <QFileInfo>
#include <QPointer>
#include <QTimer>
#include "LoadLib.h" ///< g_shareCodeCopyHint
#include "data/api_global.h" ///< ApiPaths + 响应类型
#include "network/http_client.h"
#include "network/request_options.h"
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge.h" ///< 文字徽章
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge_container.h" ///< 标签容器
#include "modules/Common/DeviceRegistry.h" ///< shortDisplayName
#include "modules/Common/elide_text.h" ///< DeSheng::elideTextWithDots
#include "modules/GeneralCustomUI/custom_QLabel_tag.h" ///< 子控件：标签流

namespace {

constexpr int kAvatarLeftMargin = 24;
constexpr int kAvatarTopMargin = 22;
constexpr int kAvatarSize = 48;
constexpr int kNameLeftMargin = 82;
constexpr int kNameRightMargin = 21;
constexpr int kNameTop = 26;
constexpr int kNameHeight = 17;
constexpr int kLevelY = 52;
constexpr int kLevelW = 25;
constexpr int kLevelBadgeH = 13;
constexpr int kNameLevelSpacing = 5;
constexpr int kMoreSize = 20;
constexpr int kMoreRightMargin = 21;
constexpr int kEyeBtnW = 18;
constexpr int kEyeBtnH = 16;
constexpr int kEyeMoreGap = 10;
constexpr int kTopInfoHeight = 48;
constexpr int kSpacerAfterTopInfo = 20;
constexpr int kSpacerAfterComments = 12;

} // namespace

CustomQWidgetSinglePlans::CustomQWidgetSinglePlans(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
    , ui(new Ui::CustomQWidgetSinglePlans)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽
    applyTheme(theme);
}

CustomQWidgetSinglePlans::~CustomQWidgetSinglePlans()
{
    delete ui;
}

void CustomQWidgetSinglePlans::InitUIInformation(int theme)
{
    Q_UNUSED(theme)

    {
        // 面板边框与背景
        setAttribute(Qt::WA_StyledBackground, true);
        setObjectName("singlePlans");
        setStyleSheet(R"(
            #singlePlans {
                border-radius: 10px;
                background-color: #10151D;
            }
        )");
        auto *t_shadow = new QGraphicsDropShadowEffect(this);
        t_shadow->setBlurRadius(20);
        t_shadow->setOffset(0, 0);
        t_shadow->setColor(QColor(0, 0, 0, 128));
        setGraphicsEffect(t_shadow);
        // // 用 mask 裁切圆角 10px
        // QPainterPath t_path;
        // t_path.addRoundedRect(rect(), 10, 10);
        // setMask(t_path.toFillPolygon().toPolygon());
    }
    {
        ui->widget_plans_info->setObjectName("widget_plans_info");
        ui->widget_plans_info->setStyleSheet(R"(
            #widget_plans_info {
                border-radius: 10px;
                background-color: #10151D;
            }
        )");
    }
    {
        // 用户头像（widget_01 内部，48×48 圆形）
        ui->label_avatar->setObjectName("singlePlans_avatar");
        ui->label_avatar->setStyleSheet(R"(
            #singlePlans_avatar {
                border-radius: 24px;
                background-color: rgba(255, 255, 255, 0.05);
            }
        )");
        ui->label_avatar->setFixedSize(kAvatarSize, kAvatarSize);
        ui->label_level->setFixedSize(kLevelW, kLevelBadgeH);
    }
    {
        // 用户等级
        ui->label_level->setObjectName("singlePlans_level");
        ui->label_level->setStyleSheet(R"(
            #singlePlans_level {
                font-family: "Noto Sans S Chinese";
                font-weight: 400;
                font-size: 10px;
                color: #FFFFFF;
                background-color: #2D9DF0;
                border-radius: 3px;
            }
        )");
    }
    {
        // 用户昵称
        ui->label_nickname->setObjectName("singlePlans_nickname");
        ui->label_nickname->setStyleSheet(R"(
            #singlePlans_nickname {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #FFFFFF;
                background: transparent;
            }
        )");
        ui->label_nickname->setFixedHeight(17);
    }
    {
        ui->pushButton->setObjectName("singlePlans_actionBtn");
        ui->pushButton->setStyleSheet(R"(
            QPushButton#singlePlans_actionBtn {
                border: none;
                border-image: url(:/Skin/Images/GeneralIcon/action.png);
            }
            QPushButton#singlePlans_actionBtn:hover {
                border-image: url(:/Skin/Images/GeneralIcon/action.png);
            }
        )");
        ui->label_limit->setPixmap(QPixmap(QStringLiteral(":/Skin/Images/Community/private.png"))
                                       .scaled(18, 16, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation));
        ui->label_limit->hide();
    }
    {
        while (QLayoutItem *t_item = ui->horizontalLayout->takeAt(0)) {
            delete t_item;
        }
        ui->label_avatar->setGeometry(kAvatarLeftMargin, 0, kAvatarSize, kAvatarSize);
        ui->label_nickname->setGeometry(kNameLeftMargin, kNameTop - kAvatarTopMargin,
                                        cl_size_.width() - kNameLeftMargin - kNameRightMargin,
                                        kNameHeight);
        ui->label_level->setGeometry(kNameLeftMargin, kLevelY - kAvatarTopMargin,
                                     kLevelW, kLevelBadgeH);
        ui->pushButton->setGeometry(cl_size_.width() - kMoreRightMargin - kMoreSize,
                                    (kAvatarSize - kMoreSize) / 2,
                                    kMoreSize, kMoreSize);
        ui->label_limit->setGeometry(ui->pushButton->x() - kEyeMoreGap - kEyeBtnW,
                                     ui->pushButton->y() + kMoreSize - kEyeBtnH,
                                     kEyeBtnW, kEyeBtnH);
        cl_author_next_badge_x_ = kNameLeftMargin + kLevelW + kNameLevelSpacing;
    }
    {
        // 评论区标签流（widget_02 内部）
        auto *t_w2_layout = new QVBoxLayout(ui->widget_02);
        t_w2_layout->setContentsMargins(24, 0, 24, 0);
        clp_comments_ = new CustomQWidgetComments(ui->widget_02);
        t_w2_layout->addWidget(clp_comments_);
        // // 示例标签
        // auto addTag = [this](int key, const QString &text, int num, TagLabelStyle style = TagLabelStyle::not_selected) {
        //     auto *t_tag = new CustomQWidgetTagLabel;
        //     t_tag->updateTag(text, num);
        //     t_tag->setCl_tag_style(style);
        //     clp_comments_->addTag(key, t_tag);
        // };
        // addTag(0, "测试", 0);
        // addTag(1, "中音温润", 0, TagLabelStyle::selected);
        // addTag(2, "下载次数", 99999, TagLabelStyle::selected);
        // addTag(3, "方案数", 12);
        // for (int i = 4; i <= 20; ++i)
        //     addTag(i, "方案数", 12 + i);
        // addTag(21, "方案数", 100999); // 10w+ 未选中
    }
    {
        // 其他区域固定高度
        this->setFixedHeight(cl_size_.height());
        ui->widget_01->setFixedHeight(cl_widget_01_size_.height());
        ui->widget_03->setFixedHeight(cl_widget_03_size_.height());
        ui->widget_04->setFixedHeight(cl_widget_04_size_.height());
        ui->verticalSpacer_4->changeSize(17, kAvatarTopMargin, QSizePolicy::Minimum,
                                         QSizePolicy::Fixed);
        ui->verticalSpacer_3->changeSize(17, kSpacerAfterTopInfo, QSizePolicy::Minimum,
                                         QSizePolicy::Fixed);
        ui->verticalSpacer_2->changeSize(17, kSpacerAfterComments, QSizePolicy::Minimum,
                                         QSizePolicy::Fixed);
    }
    {
        // 方案信息
        clp_plan_info_ = new CustomQWidgetPlanInfo(ui->widget_03);
        ui->widget_03->layout()->addWidget(clp_plan_info_);
        clp_plan_info_->setCl_plan_name("测试");
        ui->widget_03->layout()->activate();
        ui->widget_03->setFixedHeight(cl_widget_03_size_.height());
    }
    {
        // 按键水平布局
        clp_button_layout_ = new QHBoxLayout(ui->widget_04);
        clp_button_layout_->setContentsMargins(37, 0, 37, 0);
        clp_button_layout_->setSpacing(31);
        ui->widget_04->setLayout(clp_button_layout_);
    }
    {
        /// 点赞按键
        clp_like_button_
            = new CustomQWidgetFunctionButtonWithDisplayLabel(ui->widget_04, ButtonFuctionType::Like);
        clp_like_button_->setVisible(actionVisible(ActionLike));
        clp_button_layout_->addWidget(clp_like_button_);
    }
    {
        /// 踩按键
        clp_dislike_button_
            = new CustomQWidgetFunctionButtonWithDisplayLabel(ui->widget_04, ButtonFuctionType::Dislike);
        clp_dislike_button_->setVisible(actionVisible(ActionDislike));
        clp_button_layout_->addWidget(clp_dislike_button_);
    }
    {
        /// 下载按键
        clp_download_button_
            = new CustomQWidgetFunctionButtonWithDisplayLabel(ui->widget_04, ButtonFuctionType::Download);
        clp_download_button_->setVisible(actionVisible(ActionDownload));
        clp_button_layout_->addWidget(clp_download_button_);
    }
    {
        /// 分享按键
        clp_share_button_
            = new CustomQWidgetFunctionButtonWithDisplayLabel(ui->widget_04, ButtonFuctionType::Share);
        clp_share_button_->setVisible(actionVisible(ActionShare));
        clp_button_layout_->addWidget(clp_share_button_);
    }
}

void CustomQWidgetSinglePlans::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    }
}

QSize CustomQWidgetSinglePlans::sizeHint() const
{
    if (ui && layout())
        return layout()->sizeHint();
    return QWidget::sizeHint();
}

QSize CustomQWidgetSinglePlans::cl_widget_01_size() const { return cl_widget_01_size_; }
void CustomQWidgetSinglePlans::setCl_widget_01_size(const QSize &size) { cl_widget_01_size_ = size; }

QSize CustomQWidgetSinglePlans::cl_widget_02_size() const { return cl_widget_02_size_; }
void CustomQWidgetSinglePlans::setCl_widget_02_size(const QSize &size) { cl_widget_02_size_ = size; }

QSize CustomQWidgetSinglePlans::cl_widget_03_size() const { return cl_widget_03_size_; }
void CustomQWidgetSinglePlans::setCl_widget_03_size(const QSize &size) { cl_widget_03_size_ = size; }

QSize CustomQWidgetSinglePlans::cl_widget_04_size() const { return cl_widget_04_size_; }
void CustomQWidgetSinglePlans::setCl_widget_04_size(const QSize &size) { cl_widget_04_size_ = size; }

QSize CustomQWidgetSinglePlans::cl_size() const { return cl_size_; }
void CustomQWidgetSinglePlans::setCl_size(const QSize &size) { cl_size_ = size; }

void CustomQWidgetSinglePlans::InitMember() {}

void CustomQWidgetSinglePlans::setActionVisible(ActionButton btn, bool visible) {
    cl_action_visibility_[btn] = visible;
    switch (btn) {
    case ActionMore:
        if (clp_action_btn_) clp_action_btn_->setVisible(visible);
        break;
    case ActionLike:
        if (clp_like_button_) clp_like_button_->setVisible(visible);
        break;
    case ActionDislike:
        if (clp_dislike_button_) clp_dislike_button_->setVisible(visible);
        break;
    case ActionDownload:
        if (clp_download_button_) clp_download_button_->setVisible(visible);
        break;
    case ActionShare:
        if (clp_share_button_) clp_share_button_->setVisible(visible);
        break;
    }
}

bool CustomQWidgetSinglePlans::actionVisible(ActionButton btn) const {
    // 与社区 item 一致：更多按钮默认隐藏，其余默认显示
    return cl_action_visibility_.value(btn, btn != ActionMore);
}

void CustomQWidgetSinglePlans::InitConnect()
{
    // 点赞（选中）→ 踩置否 + POST /like
    connect(clp_like_button_, &CustomQWidgetFunctionButtonWithDisplayLabel::liked, this,
            [this]() {
                if (clp_dislike_button_) clp_dislike_button_->setChecked(false);
                doLike();
            }, Qt::UniqueConnection);

    // 取消点赞（取消选中）→ DELETE /like
    connect(clp_like_button_, &CustomQWidgetFunctionButtonWithDisplayLabel::unliked, this,
            &CustomQWidgetSinglePlans::doUnlike, Qt::UniqueConnection);

    // 踩（选中）→ 点赞置否 + POST /dislike
    connect(clp_dislike_button_, &CustomQWidgetFunctionButtonWithDisplayLabel::disliked, this,
            [this]() {
                if (clp_like_button_) clp_like_button_->setChecked(false);
                doDislike();
            }, Qt::UniqueConnection);

    // 取消踩（取消选中）→ DELETE /dislike
    connect(clp_dislike_button_, &CustomQWidgetFunctionButtonWithDisplayLabel::undisliked, this,
            &CustomQWidgetSinglePlans::doUndislike, Qt::UniqueConnection);

    // 下载 → 执行下载
    connect(clp_download_button_, &CustomQWidgetFunctionButtonWithDisplayLabel::download, this,
            &CustomQWidgetSinglePlans::doDownload, Qt::UniqueConnection);

    // 分享 → 执行分享
    connect(clp_share_button_, &CustomQWidgetFunctionButtonWithDisplayLabel::share, this,
            &CustomQWidgetSinglePlans::doShare, Qt::UniqueConnection);

    // 标签被点击 → 乐观更新已切样式，发 API 确认
    QPointer<CustomQWidgetSinglePlans> t_self_tag(this);
    connect(clp_comments_, &CustomQWidgetComments::tagClicked, this,
            [this, t_self_tag](int key) {
        if (!t_self_tag) return;
        auto *t_tag = clp_comments_->tag(key);
        if (!t_tag || cl_config_id_ <= 0) return;

        bool t_now_selected = (t_tag->cl_tag_style() == TagLabelStyle::selected);

        // 内联 API 回调，失败时 revert 乐观更新
        if (t_now_selected) {
            doClickComment(key, t_tag);
        } else {
            doCancelClickComment(key, t_tag);
        }
    }, Qt::UniqueConnection);

    // 评论区高度变化 → 面板高度跟随
    connect(clp_comments_, &CustomQWidgetComments::heightChanged, this, [this](int newHeight) {
        const int t_comment_area_h = qMax(cl_widget_02_size_.height(), newHeight);
        int t_h = kAvatarTopMargin
                + ui->widget_01->height()
                + kSpacerAfterTopInfo
                + t_comment_area_h
                + kSpacerAfterComments
                + ui->widget_03->height()
                + 27    // verticalSpacer
                + ui->widget_04->height()
                + 31;   // verticalSpacer_5
        this->setFixedHeight(qMax(cl_size_.height(), t_h));
    });

    // 更多操作按钮 → 弹出菜单
    {
        clp_action_btn_ = ui->pushButton;
        clp_action_btn_->setFixedSize(20,20);
        clp_action_btn_->setStyleSheet("border-image: url(:/Skin/Images/Community/clp_action_menu_-bk.png);");
        clp_action_btn_->setCursor(Qt::PointingHandCursor);
        clp_action_btn_->setVisible(actionVisible(ActionMore));  ///< 与社区 item 一致：更多按钮默认隐藏
        clp_action_menu_ = new QMenu(this);
        clp_action_menu_->setFixedSize(127, 95);
        clp_action_menu_->setStyleSheet(R"(
            QMenu {
                background-color: #0D0F14;
                border: 1px solid #0D0F14;
                border-radius: 6px;
                padding: 6px 4px;
                margin: 2px;
            }
            QMenu::item {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #FFFFFF;
                padding: 6px 4px;
                border-radius: 4px;
            }
            QMenu::item:selected {
                background-color: rgba(255, 255, 255, 0.1);
            }
        )");
        clp_action_menu_->addAction(tr("删除"), this, [this]() { doDeletePlan(); });
        clp_action_menu_->addAction(tr("置顶"), this, [this]() { emit pinRequested(); });
        clp_action_menu_->addAction(tr("仅自己可见"), this, [this]() { emit visibilityRequested(); });
        connect(clp_action_btn_, &QPushButton::clicked, this, [this]() {
            QPoint t_pos = clp_action_btn_->mapToGlobal(
                QPoint(clp_action_btn_->width() - clp_action_menu_->width(),
                       clp_action_btn_->height() + 6));
            clp_action_menu_->exec(t_pos);
        });
    }
}

int CustomQWidgetSinglePlans::cl_config_id() const { return cl_config_id_; }

void CustomQWidgetSinglePlans::setCl_config_id(int id) { cl_config_id_ = id; }


void CustomQWidgetSinglePlans::setAvatarPixmap(const QPixmap &pixmap)
{
    // 头像为空（含用户数据 avatar 字段为空）→ 系统默认头像；同时清除池复用的旧头像
    QPixmap t_source = pixmap;
    if (t_source.isNull())
        t_source = QPixmap(QStringLiteral(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png"));
    // 裁剪为圆形（Qt QSS border-radius 不能裁剪 QLabel pixmap 内容）
    int t_size = qMin(t_source.width(), t_source.height());
    QPixmap t_square = t_source.copy((t_source.width() - t_size) / 2,
                                     (t_source.height() - t_size) / 2, t_size, t_size);
    t_square = t_square.scaled(kAvatarSize, kAvatarSize, Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);

    QPixmap t_circular(kAvatarSize, kAvatarSize);
    t_circular.fill(Qt::transparent);
    QPainter t_painter(&t_circular);
    t_painter.setRenderHint(QPainter::Antialiasing);
    t_painter.setBrush(t_square);
    t_painter.setPen(Qt::NoPen);
    t_painter.drawEllipse(0, 0, kAvatarSize, kAvatarSize);
    t_painter.end();

    ui->label_avatar->setFixedSize(kAvatarSize, kAvatarSize);
    ui->label_avatar->setAlignment(Qt::AlignCenter);
    ui->label_avatar->setPixmap(t_circular);
}

void CustomQWidgetSinglePlans::setAuthorInfo(const QString &nickname, int level)
{
    ui->label_nickname->setText(DeSheng::elideTextWithDots(nickname,
                                                           ui->label_nickname->font(),
                                                           ui->label_nickname->width()));
    ui->label_level->setText(QString("Lv.%1").arg(level));
}

void CustomQWidgetSinglePlans::clearAuthorBadges()
{
    for (auto *t_badge : clp_role_badges_) {
        if (t_badge) {
            t_badge->deleteLater();
        }
    }
    clp_role_badges_.clear();
}

void CustomQWidgetSinglePlans::addAuthorBadge(const QString &pixmapPath, const QSize &size)
{
    auto *t_badge = new QLabel(ui->widget_01);
    t_badge->setFixedSize(size);
    t_badge->setPixmap(QPixmap(pixmapPath).scaled(size, Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation));
    t_badge->setAlignment(Qt::AlignCenter);
    t_badge->setStyleSheet(QStringLiteral("background: transparent;"));
    const int t_level_bottom = kLevelY - kAvatarTopMargin + kLevelBadgeH;
    t_badge->setGeometry(cl_author_next_badge_x_, t_level_bottom - size.height(),
                         size.width(), size.height());
    cl_author_next_badge_x_ += size.width() + kNameLevelSpacing;
    clp_role_badges_.append(t_badge);
}

void CustomQWidgetSinglePlans::setAuthorBadges(const QStringList &roles, bool expertTag)
{
    clearAuthorBadges();
    cl_author_next_badge_x_ = kNameLeftMargin + kLevelW + kNameLevelSpacing;
    if (roles.contains(DeSheng::kRoleStreamer))
        addAuthorBadge(QStringLiteral(":/Skin/Images/modules/community/host.png"), QSize(46, 19));
    if (roles.contains(DeSheng::kRoleOfficial))
        addAuthorBadge(QStringLiteral(":/Skin/Images/modules/community/official.png"), QSize(56, 18));
    if (roles.contains(DeSheng::kRoleProfessional))
        addAuthorBadge(QStringLiteral(":/Skin/Images/modules/community/professional.png"), QSize(50, 18));
    if (expertTag)
        addAuthorBadge(QStringLiteral(":/Skin/Images/modules/community/god.png"), QSize(53, 20));
}

void CustomQWidgetSinglePlans::setVisibility(const QString &visibility)
{
    ui->label_limit->setVisible(visibility == DeSheng::kVisibilityPrivate && actionVisible(ActionMore));
}

void CustomQWidgetSinglePlans::setPlanInfo(const QString &title, const QString &desc,
                                            const QStringList &user_tags)
{
    cl_plan_name_full_ = title; ///< 保存全名，分享拼接用
    clp_plan_info_->setCl_plan_name(title);
    clp_plan_info_->setCl_plan_desc(desc);
    clp_plan_info_->setCl_category_icon(user_tags);
}

void CustomQWidgetSinglePlans::setComments(
    const QList<DeSheng::GetPublicConfigurationListResponse::Comment> &comments)
{
    clearComments();
    for (const auto &t_comment : comments) {
        auto *t_tag = new CustomQLabelTag;
        t_tag->updateTag(t_comment.comment_text, t_comment.count);
        t_tag->setCl_tag_style(t_comment.is_clicked ? TagLabelStyle::selected
                                                    : TagLabelStyle::not_selected);
        clp_comments_->addTag(t_comment.id, t_tag);
    }
}

void CustomQWidgetSinglePlans::setActionState(int likeCount, int dislikeCount,
                                              int downloadCount, int shareCount,
                                              bool liked, bool disliked)
{
    clp_like_button_->setCl_count(likeCount);
    clp_like_button_->setChecked(liked);
    clp_dislike_button_->setCl_count(dislikeCount);
    clp_dislike_button_->setChecked(disliked);
    clp_download_button_->setCl_count(downloadCount);
    clp_share_button_->setCl_count(shareCount);
}

void CustomQWidgetSinglePlans::setCl_device_name(const QString &name)
{
    if (cl_device_name_ == name) return;
    cl_device_name_ = name;
    clp_plan_info_->clp_tag_container()->clearBadges();
    if (!name.isEmpty()) {
        clp_plan_info_->clp_tag_container()->addBadge(
            DeSheng::DeviceRegistry::shortDisplayName(name))->setCl_radius(8.5);
    }
    if (!cl_scene_.isEmpty()) {
        auto *t_scene = clp_plan_info_->clp_tag_container()->addBadge(cl_scene_);
        t_scene->setCl_radius(8.5);
        // t_scene->setCl_fire_enabled(true); ///< 场景标签 — 火焰动画
    }
}

void CustomQWidgetSinglePlans::setCl_scene(const QString &scene)
{
    if (cl_scene_ == scene) return;
    cl_scene_ = scene;
    clp_plan_info_->clp_tag_container()->clearBadges();
    if (!cl_device_name_.isEmpty()) {
        clp_plan_info_->clp_tag_container()->addBadge(
            DeSheng::DeviceRegistry::shortDisplayName(cl_device_name_))->setCl_radius(8.5);
    }
    if (!scene.isEmpty()) {
        auto *t_scene = clp_plan_info_->clp_tag_container()->addBadge(scene);
        t_scene->setCl_radius(8.5);
        // t_scene->setCl_fire_enabled(true); ///< 场景标签 — 火焰动画
    }
}

void CustomQWidgetSinglePlans::clearComments()
{
    if (clp_comments_) {
        clp_comments_->clearTags(); ///< 清空全部标签
    }
}

void CustomQWidgetSinglePlans::abortDownload()
{
    if (clp_active_download_reply_) {
        clp_active_download_reply_->abort();
        clp_active_download_reply_ = nullptr;
    }
    clp_download_button_->setDownloadState(DownloadState::Normal);
}

void CustomQWidgetSinglePlans::unbindData(WidgetStateCache &cache)
{
    // 中断进行中的下载
    abortDownload();

    // 断开池复用时的外部信号连接，防止 UniqueConnection 无法去重不同 lambda 导致累积
    QObject::disconnect(this, &CustomQWidgetSinglePlans::liked, nullptr, nullptr);
    QObject::disconnect(this, &CustomQWidgetSinglePlans::unliked, nullptr, nullptr);
    QObject::disconnect(this, &CustomQWidgetSinglePlans::disliked, nullptr, nullptr);
    QObject::disconnect(this, &CustomQWidgetSinglePlans::undisliked, nullptr, nullptr);

    // 保存展开状态
    if (clp_comments_ && cl_config_id_ > 0) {
        WidgetDisplayState t_state;
        t_state.is_expanded = clp_comments_->cl_expanded();
        cache.insert(cl_config_id_, t_state);
    }

    // 重置按钮状态 + 计数
    clp_like_button_->setChecked(false);
    clp_like_button_->setCl_count(0);
    clp_dislike_button_->setChecked(false);
    clp_dislike_button_->setCl_count(0);
    clp_download_button_->setDownloadProgress(0);
    clp_download_button_->setCl_count(0);
    clp_share_button_->setCl_count(0);

    // 清空评论
    clearComments();
    clearAuthorBadges();
    ui->label_limit->hide();

    // 清空头像 + 作者信息（防止池复用时闪现旧数据）
    ui->label_avatar->clear();
    ui->label_nickname->clear();
    ui->label_level->clear();

    // 清空方案信息
    clp_plan_info_->setCl_plan_name(QString());
    clp_plan_info_->setCl_plan_desc(QString());
    clp_plan_info_->clp_tag_container()->clearBadges();

    // 重置 config_id + 分享信息
    cl_config_id_ = 0;
    cl_device_name_.clear();
    cl_scene_.clear();
    cl_plan_name_full_.clear();
}

void CustomQWidgetSinglePlans::restoreState(const WidgetStateCache &cache)
{
    if (cl_config_id_ <= 0)
        return;

    auto t_it = cache.find(cl_config_id_);
    if (t_it != cache.end() && clp_comments_) {
        clp_comments_->setCl_expanded(t_it->is_expanded);
    }
}

void CustomQWidgetSinglePlans::doLike()
{
    if (cl_config_id_ <= 0)
        return;
    QNetworkReply *t_reply = HttpClient::instance().post(
        QString(DeSheng::ApiPaths::kConfigLike).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));
    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        if (!t_self) { t_reply->deleteLater(); return; }
        if (t_reply->error() == QNetworkReply::NoError) {
            refreshCounts(); ///< 拉服务端真实计数纠正
            emit liked();
        }
        t_reply->deleteLater();
    });
}

void CustomQWidgetSinglePlans::doUnlike()
{
    if (cl_config_id_ <= 0)
        return;
    QNetworkReply *t_reply = HttpClient::instance().del(
        QString(DeSheng::ApiPaths::kConfigLike).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));
    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        if (!t_self) { t_reply->deleteLater(); return; }
        if (t_reply->error() == QNetworkReply::NoError) {
            refreshCounts();
            emit unliked();
        }
        t_reply->deleteLater();
    });
}

void CustomQWidgetSinglePlans::doDislike()
{
    if (cl_config_id_ <= 0)
        return;
    QNetworkReply *t_reply = HttpClient::instance().post(
        QString(DeSheng::ApiPaths::kConfigDislike).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));
    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        if (!t_self) { t_reply->deleteLater(); return; }
        if (t_reply->error() == QNetworkReply::NoError) {
            refreshCounts();
            emit disliked();
        }
        t_reply->deleteLater();
    });
}

void CustomQWidgetSinglePlans::doUndislike()
{
    if (cl_config_id_ <= 0)
        return;
    QNetworkReply *t_reply = HttpClient::instance().del(
        QString(DeSheng::ApiPaths::kConfigDislike).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));
    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        if (!t_self) { t_reply->deleteLater(); return; }
        if (t_reply->error() == QNetworkReply::NoError) {
            refreshCounts();
            emit undisliked();
        }
        t_reply->deleteLater();
    });
}

void CustomQWidgetSinglePlans::doShare()
{
    if (cl_config_id_ <= 0)
        return;
    QNetworkReply *t_reply = HttpClient::instance().post(
        QString(DeSheng::ApiPaths::kConfigShare).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));
    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        t_reply->deleteLater();
        if (!t_self) return;

        // 网络传输层错误
        if (t_reply->error() != QNetworkReply::NoError) {
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("分享失败，请检查网络"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
            return;
        }

        // 解析分享码 + 分享次数
        QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
        DeSheng::ShareConfigurationResponse t_resp;
        if (!DeSheng::ProcessShareConfigurationResult(t_resp, t_doc)
            || t_resp.code != "success") {
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("分享失败：%1").arg(t_resp.message));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
            return;
        }

        // 拼接分享文本: 名称+机型+场景+sq分享码
        if (!t_resp.data.share_code.isEmpty()) {
            QString t_text = cl_plan_name_full_
                             + "+" + cl_device_name_
                             + "+" + cl_scene_
                             + "+sq" + t_resp.data.share_code;
            QApplication::clipboard()->setText(t_text);
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("方案分享码已复制"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
        }

        // 用服务端返回的真实分享次数更新
        clp_share_button_->setCl_count(t_resp.data.share_count);

        emit share();
    });
}

void CustomQWidgetSinglePlans::doDownload()
{
    if (cl_config_id_ <= 0)
        return;

    if (clp_active_download_reply_) return; ///< 下载进行中，防重复点击

    XIBERIA_X_HUB_Utils::ensureDownloadTempDir(); ///< 确保下载临时目录存在

    clp_download_button_->setDownloadState(DownloadState::Downloading); ///< 进下载态，显示圆环

    // 第一步：请求下载 URL
    QNetworkReply *t_reply = HttpClient::instance().get(
        QString(DeSheng::ApiPaths::kConfigDownload).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));
    clp_active_download_reply_ = t_reply; ///< 追踪，unbind 时可 abort

    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        t_reply->deleteLater();
        if (!t_self) return; ///< widget 已被池回收，放弃回调

        // 无论成功/失败都必须清除追踪指针，否则后续下载永久阻塞
        if (clp_active_download_reply_ == t_reply)
            clp_active_download_reply_ = nullptr;

        if (t_reply->error() != QNetworkReply::NoError) {
            clp_download_button_->setDownloadState(DownloadState::Normal);
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("下载失败，请检查网络"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
            return;
        }

        // 解析 config_url
        QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
        DeSheng::DownloadTargetConfigurationResponse t_resp;
        if (!DeSheng::ProcessDownloadTargetConfigurationResult(t_resp, t_doc)
            || t_resp.data.config_url.isEmpty()) {
            clp_download_button_->setDownloadState(DownloadState::Normal);
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("下载失败，方案文件不存在"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
            return;
        }

        QUrl t_config_url(t_resp.data.config_url);
        if (!t_config_url.isValid()) {
            clp_download_button_->setDownloadState(DownloadState::Normal);
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("下载失败，链接无效"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
            return;
        }

        // 第二步：从 config_url 下载实际文件（带进度）
        QNetworkRequest t_file_req(t_config_url);
        t_file_req.setTransferTimeout(60000); ///< 与新栈默认超时一致（ApiConfig 已退役）
        QNetworkReply *t_file_reply = HttpClient::instance().manager()->get(t_file_req);
        clp_active_download_reply_ = t_file_reply; ///< 切换追踪到文件下载

        // 真实文件下载进度 → 圆环更新
        connect(t_file_reply, &QNetworkReply::downloadProgress, this,
                [t_self](qint64 received, qint64 total) {
            if (!t_self) return;
            if (total > 0) {
                t_self->clp_download_button_->setDownloadProgress(
                    static_cast<int>(received * 100 / total));
            }
        });

        // 文件下载完成 → 保存到临时目录，延迟恢复图标
        connect(t_file_reply, &QNetworkReply::finished, this,
                [this, t_file_reply, t_config_url, t_self]() {
            t_file_reply->deleteLater();
            if (!t_self) return; ///< widget 已被池回收，放弃回调
            if (clp_active_download_reply_ == t_file_reply)
                clp_active_download_reply_ = nullptr;

            if (t_file_reply->error() == QNetworkReply::NoError) {
                // 保存到 ProgramData/downloads/tempfiles/
                QString t_dir = XIBERIA_X_HUB_Utils::downloadTempDir();

                // 文件名: {config_id}_{原始文件名}
                QString t_origin_name = QFileInfo(t_config_url.path()).fileName();
                if (t_origin_name.isEmpty())
                    t_origin_name = QString::number(cl_config_id_) + ".json";
                else
                    t_origin_name = QString::number(cl_config_id_) + "_" + t_origin_name;

                QString t_file_path = t_dir + "/" + t_origin_name;
                QFile t_file(t_file_path);
                bool t_written = false;
                if (t_file.open(QIODevice::WriteOnly)) {
                    t_file.write(t_file_reply->readAll());
                    t_file.close();
                    t_written = true;
                }

                // 导入方案库（含清理临时文件）
                if (t_written && m) {
                    m->importDownloadedPlan(t_file_path);
                } else {
                    // 写盘失败 / m 不可用 → 手动清理临时文件 + toast
                    if (t_written) QFile::remove(t_file_path);
                    if (g_shareCodeCopyHint) {
                        g_shareCodeCopyHint->setText(tr("导入失败"));
                        g_shareCodeCopyHint->show();
                        QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                    }
                }

                // refreshCounts 在回调中可能 widget 已被 unbind（cl_config_id_=0），
                // 回调内部以 t_self 守卫 + cl_config_id_ > 0 双重校验
                refreshCounts();
                emit download();

                // 延迟 500ms 恢复图标，用户能看到 100% 完成态
                QTimer::singleShot(500, this, [t_self]() {
                    if (!t_self) return;
                    t_self->clp_download_button_->setDownloadState(DownloadState::Normal);
                });
            } else {
                clp_download_button_->setDownloadState(DownloadState::Normal);
                if (g_shareCodeCopyHint) {
                    g_shareCodeCopyHint->setText(tr("下载失败，请检查网络"));
                    g_shareCodeCopyHint->show();
                    QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                }
            }
        });
    });
}

void CustomQWidgetSinglePlans::doDeletePlan()
{
    if (cl_config_id_ <= 0)
        return;

    DelReset t_dialog(this);
    t_dialog.editText(6); ///< 删除已上传方案
    if (t_dialog.exec() != QDialog::Accepted)
        return;

    QNetworkReply *t_reply = HttpClient::instance().del(
        QString(DeSheng::ApiPaths::kConfigDetail).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));

    QPointer<CustomQWidgetSinglePlans> t_self(this);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self]() {
        t_reply->deleteLater();
        if (!t_self) return;

        if (t_reply->error() == QNetworkReply::NoError) {
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("删除成功"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
            emit deleteRequested(); ///< 通知父级刷新列表
        } else {
            if (g_shareCodeCopyHint) {
                g_shareCodeCopyHint->setText(tr("删除失败，请重试"));
                g_shareCodeCopyHint->show();
                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            }
        }
    });
}

void CustomQWidgetSinglePlans::doClickComment(int comment_id, CustomQLabelTag *tag)
{
    if (cl_config_id_ <= 0 || comment_id <= 0 || !tag)
        return;

    DeSheng::ClickCommentRequest t_req;
    t_req.id = cl_config_id_;
    t_req.comment_id = comment_id;

    QByteArray t_body = QJsonDocument(DeSheng::ClickCommentRequestToJson(t_req)).toJson();
    QNetworkReply *t_reply = HttpClient::instance().post(
        QString(DeSheng::ApiPaths::kConfigComments).arg(cl_config_id_),
        RequestOptions{}.withBody(t_body).withTag("userConfig"));

    QPointer<CustomQWidgetSinglePlans> t_self(this);
    QPointer<CustomQLabelTag> t_tag(tag);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_tag, comment_id, t_self]() {
        t_reply->deleteLater();
        if (!t_self || !t_tag) return; ///< widget 或 tag 已被回收，放弃回调
        if (t_reply->error() != QNetworkReply::NoError) {
            // 失败回滚：取消选中 + 数字 -1
            t_tag->setCl_tag_style(TagLabelStyle::not_selected);
            int t_n = t_tag->cl_tag_number();
            if (t_n > 0) t_tag->setCl_tag_number(t_n - 1);
        } // 成功：乐观更新已生效，无需额外操作
    });
}

void CustomQWidgetSinglePlans::doCancelClickComment(int comment_id, CustomQLabelTag *tag)
{
    if (cl_config_id_ <= 0 || comment_id <= 0 || !tag)
        return;

    DeSheng::CancelClickCommentRequest t_req;
    t_req.id = cl_config_id_;
    t_req.comment_id = comment_id;

    QByteArray t_body = QJsonDocument(DeSheng::CancelClickCommentRequestToJson(t_req)).toJson();

    // DELETE 带 body
    QNetworkReply *t_reply = HttpClient::instance().del(
        QString(DeSheng::ApiPaths::kConfigComments).arg(cl_config_id_),
        RequestOptions{}.withBody(t_body).withTag("userConfig"));

    QPointer<CustomQWidgetSinglePlans> t_self(this);
    QPointer<CustomQLabelTag> t_tag(tag);
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_tag, comment_id, t_self]() {
        t_reply->deleteLater();
        if (!t_self || !t_tag) return; ///< widget 或 tag 已被回收，放弃回调
        if (t_reply->error() != QNetworkReply::NoError) {
            // 失败回滚：恢复选中 + 数字 +1
            t_tag->setCl_tag_style(TagLabelStyle::selected);
            t_tag->setCl_tag_number(t_tag->cl_tag_number() + 1);
        } // 成功：乐观更新已生效
    });
}

void CustomQWidgetSinglePlans::refreshCounts()
{
    if (cl_config_id_ <= 0)
        return;

    QNetworkReply *t_reply = HttpClient::instance().get(
        QString(DeSheng::ApiPaths::kConfigDetail).arg(cl_config_id_),
        RequestOptions{}.withTag("userConfig"));

    QPointer<CustomQWidgetSinglePlans> t_self(this);
    const int t_config_id = cl_config_id_; ///< 捕获 config_id，回调中校验 widget 未被 unbind 更换方案
    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_self, t_config_id]() {
        t_reply->deleteLater();
        if (!t_self || t_config_id <= 0 || t_self->cl_config_id_ != t_config_id) return;
        if (t_reply->error() != QNetworkReply::NoError)
            return;

        QJsonDocument t_doc = QJsonDocument::fromJson(t_reply->readAll());
        DeSheng::GetConfigurationDetailsResponse t_resp;
        if (!DeSheng::ProcessGetConfigurationDetailsResult(t_resp, t_doc)
            || t_resp.code != "success")
            return;

        // 用服务端真实数据覆盖所有计数和选中态（批量更新防抖）
        setUpdatesEnabled(false);
        clp_like_button_->setCl_count(t_resp.data.like_count);
        clp_dislike_button_->setCl_count(t_resp.data.dislike_count);
        clp_download_button_->setCl_count(t_resp.data.download_count);
        clp_share_button_->setCl_count(t_resp.data.share_count);
        clp_like_button_->setChecked(t_resp.data.is_liked);
        clp_dislike_button_->setChecked(t_resp.data.is_disliked);
        setUpdatesEnabled(true);
    });
}
