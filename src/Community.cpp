#include "Community.h"
#include "ui_Community.h"
#include "LoadLib.h"
#include "network/auth_store.h"  ///< token 变化监听
#include "network/http_client.h"
#include "network/request_options.h"
#include "network/avatar_cache.h"
#include "repository/ranking_helper.h"
#include "data/api_global.h"
#include "data/userConfig/user_config_api.h"  ///< kRoleOfficial/kRoleStreamer/kRoleProfessional（前三名徽章）
#include <QEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QtGlobal>

namespace {
constexpr int kRankingPanelWidth = 400;        ///< 排行榜弹窗宽度。
constexpr int kRankingPanelGap = 14;           ///< 排行榜弹窗与右侧榜单区域的水平间距。
constexpr int kRankingPanelBottomMargin = 28;  ///< 排行榜弹窗底部与 APP 底部的固定距离。
constexpr auto kDefaultRankingAvatarPath = ":/Skin/Images/system/system_avatar/system_avatar_2x_01.png";

/// \brief 返回排行榜弹窗在顶层窗口坐标系中的顶部 y，和右侧排行榜入口按钮顶部对齐。
int rankingPanelTopY(const QWidget *rankingButton, const QWidget *root)
{
    if (!rankingButton || !root)
        return 0;
    return rankingButton->mapTo(root, QPoint(0, 0)).y();
}

/// \brief 根据顶部 y 和底部间距计算排行榜弹窗高度，使底部距 APP 底部固定。
int rankingPanelHeight(const QWidget *root, int topY)
{
    if (!root)
        return 0;
    return qMax(1, root->height() - topY - kRankingPanelBottomMargin);
}
}

void Community::setCommunityAuthToken(const QString &token)
{
    if (m_communityMainPage) {
        m_communityMainPage->setAuthToken(token);
        m_communityMainPage->loadInitialData();
    }
}

Community::Community(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Community)
{
    ui->setupUi(this);

    // ── 社区模块：嵌入左侧主区域 ──
    m_communityMainPage = new CommunityMainPage(ui->widget_left);
    // 目标服务器（国内正式服）
    m_communityMainPage->setServer(QStringLiteral("domestic"),
                                   QStringLiteral("https://hubsys.xiberia.net/api/v1"));
    m_communityMainPage->setServer(QStringLiteral("domestic-t"),
                                   QStringLiteral("https://hubsystest.xiberia.net/api/v1"));
    {
        auto *layout = new QVBoxLayout(ui->widget_left);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_communityMainPage);
    }
    // 登录成功后（AuthStore token 变化）自动加载社区数据
    connect(&AuthStore::instance(), &AuthStore::tokenChanged, this, [this]() {
        if (m_communityMainPage && !AuthStore::instance().token().isEmpty())
            m_communityMainPage->loadInitialData();
        fetchTopThreeAvatars();
    });

    m_rankingMask = new QWidget(this);
    m_rankingMask->setObjectName(QStringLiteral("rankingMask"));
    m_rankingMask->setAttribute(Qt::WA_StyledBackground, true);
    m_rankingMask->setStyleSheet(QStringLiteral(
        "QWidget#rankingMask{background:rgba(0, 0, 0, 0.4);}"));
    m_rankingMask->setGeometry(rect());
    m_rankingMask->hide();
    m_rankingMask->installEventFilter(this);

    // 注入社区共享 service/repo：排行榜点赞/下载与社区 5-model 双向同步
    m_rankingList = new RankingList(this, m_communityMainPage->schemeService(),
                                     m_communityMainPage->configRepo());
    m_rankingList->hide();
    connect(m_rankingList, &RankingList::topThreeDataReady,
            this, &Community::updateTopThreeFromRankingData);
    //点击排行榜按钮切换排行榜列表显示/隐藏
    connect(ui->pBt_ranking,&QPushButton::clicked,this,[this]()
            {
                if (m_rankingList->isHidden()) {
                    showRankingOverlay();
                } else {
                    hideRankingOverlay();
                }
            });

    setupAvatars();
    fetchTopThreeAvatars();
}

Community::~Community()
{
    delete m_rankingList;
    m_rankingList = nullptr;
    delete m_rankingMask;
    m_rankingMask = nullptr;
    delete ui;
}

void Community::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    positionRankingOverlay();
}

bool Community::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_rankingMask && event->type() == QEvent::MouseButtonPress) {
        hideRankingOverlay();
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void Community::showRankingOverlay()
{
    positionRankingOverlay();
    m_rankingMask->show();
    m_rankingMask->raise();
    m_rankingList->show();
    m_rankingList->raise();
}

void Community::hideRankingOverlay()
{
    if (m_rankingList)
        m_rankingList->hide();
    if (m_rankingMask)
        m_rankingMask->hide();
}

void Community::positionRankingOverlay()
{
    QWidget *root = window();
    if (!root)
        root = this;

    if (m_rankingMask && m_rankingMask->parentWidget() != root) {
        m_rankingMask->hide();
        m_rankingMask->setParent(root);
        m_rankingMask->installEventFilter(this);
    }
    if (m_rankingList && m_rankingList->parentWidget() != root) {
        m_rankingList->hide();
        m_rankingList->setParent(root);
    }

    if (m_rankingMask)
        m_rankingMask->setGeometry(root->rect());
    if (!m_rankingList || !ui || !ui->widget)
        return;

    const QPoint sideTopLeft = ui->widget->mapTo(root, QPoint(0, 0));
    const int x = sideTopLeft.x() - kRankingPanelGap - kRankingPanelWidth;
    const int y = rankingPanelTopY(ui->pBt_ranking, root);
    m_rankingList->setFixedSize(kRankingPanelWidth, rankingPanelHeight(root, y));
    m_rankingList->move(qMax(0, x), qMax(0, y));
}

// ── 六个头像位：头像 + 边框合成 + 独立数字角标 ──────
// 头像数据未到前全部隐藏（"空图就隐藏"）；fetchTopThreeAvatars 数据返回后按需显示。
// 点击头像 → 用户上传弹窗（与社区 item 点击头像行为一致）
void Community::setupAvatars()
{
    auto bindClick = [this](AvatarButton *btn, int key) {
        connect(btn, &AvatarButton::clicked, this, [this, key]() {
            if (!cl_top_three_users_.contains(key))
                return;
            auto *t_dialog = new UserUploadsDialog(cl_top_three_users_.value(key), this);
            t_dialog->show();
        });
    };

    ui->btn_Like_No1->setSlotType(AvatarButton::LikeNo1);
    bindClick(ui->btn_Like_No1, 0);
    ui->btn_Like_No2->setSlotType(AvatarButton::LikeNo2);
    bindClick(ui->btn_Like_No2, 1);
    ui->btn_Like_No3->setSlotType(AvatarButton::LikeNo3);
    bindClick(ui->btn_Like_No3, 2);
    ui->btn_download_No1->setSlotType(AvatarButton::LikeNo1);
    bindClick(ui->btn_download_No1, 10);
    ui->btn_download_No2->setSlotType(AvatarButton::LikeNo2);
    bindClick(ui->btn_download_No2, 11);
    ui->btn_download_No3->setSlotType(AvatarButton::LikeNo3);
    bindClick(ui->btn_download_No3, 12);

    ui->btn_Like_No1->hide();
    ui->btn_Like_No2->hide();
    ui->btn_Like_No3->hide();
    ui->btn_download_No1->hide();
    ui->btn_download_No2->hide();
    ui->btn_download_No3->hide();
}

// ── 点赞/下载榜前三名头像（服务器数据，头像经 AvatarCache 下载） ──────
void Community::hideTopThreeAvatars()
{
    cl_top_three_users_.clear();
    for (int i = 0; i < 3; ++i) {
        if (AvatarButton *t_btn = avatarButtonForKey(i))
            t_btn->hide();
        if (AvatarButton *t_btn = avatarButtonForKey(10 + i))
            t_btn->hide();
    }
}

void Community::fetchTopThreeAvatars()
{
    hideTopThreeAvatars();
    if (!AuthStore::instance().hasToken())
        return;

    ensureTopThreeAvatarCache();

    auto fetchOne = [this](const QString &sort, int keyBase) {
        // 请求/解析统一走 RankingHelper（repository/ranking_helper）
        RankingHelper::fetchTop(sort, 3, false, this,
                                [this, sort, keyBase](const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &t_list, bool ok) {
            if (!ok) {
                qWarning() << "[ranking] fetch top three failed:" << sort;
                return;
            }
            updateTopThreeGroup(keyBase, t_list);
        });
    };

    fetchOne("like", 0);
    fetchOne("download", 10);
}

void Community::ensureTopThreeAvatarCache()
{
    if (m_avatarCache)
        return;
    m_avatarCache = new AvatarCache(this);
    connect(m_avatarCache, &AvatarCache::avatarReady, this, [this](int key, const QPixmap &pm) {
        AvatarButton *t_btn = avatarButtonForKey(key);
        if (t_btn && !pm.isNull())
            t_btn->setAvatarPixmap(pm);
        if (t_btn)
            t_btn->show();
    });
}

void Community::updateTopThreeFromRankingData(
    bool isLikeRanking,
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &items)
{
    updateTopThreeGroup(isLikeRanking ? 0 : 10, items);
}

void Community::updateTopThreeGroup(
    int keyBase,
    const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &items)
{
    ensureTopThreeAvatarCache();
    for (int i = 0; i < 3; ++i) {
        const int t_key = keyBase + i;
        cl_top_three_users_.remove(t_key);
        if (AvatarButton *t_btn = avatarButtonForKey(t_key))
            t_btn->hide();
    }

    for (int i = 0; i < items.size() && i < 3; ++i) {
        const int t_key = keyBase + i;
        const auto &t_author = items.at(i).author;
        UserProfile t_profile;
        t_profile.userId = t_author.user_id;
        t_profile.username = t_author.username;
        t_profile.nickname = t_author.nickname.isEmpty() ? t_author.username : t_author.nickname;
        t_profile.level = t_author.level;
        t_profile.isOfficial = t_author.roles.contains(DeSheng::kRoleOfficial);
        t_profile.isStreamer = t_author.roles.contains(DeSheng::kRoleStreamer);
        t_profile.isProfessional = t_author.roles.contains(DeSheng::kRoleProfessional);
        cl_top_three_users_.insert(t_key, t_profile);

        AvatarButton *t_btn = avatarButtonForKey(t_key);
        if (!t_btn)
            continue;
        t_btn->setAvatarPixmap(QPixmap(QString::fromLatin1(kDefaultRankingAvatarPath)));
        t_btn->show();
        if (!t_author.avatar.isEmpty())
            m_avatarCache->fetchAvatar(t_key, t_author.avatar);
    }
}

AvatarButton *Community::avatarButtonForKey(int key)
{
    if (key < 10) {
        return key == 0 ? ui->btn_Like_No1 : (key == 1 ? ui->btn_Like_No2 : ui->btn_Like_No3);
    }
    return key == 10 ? ui->btn_download_No1
                     : (key == 11 ? ui->btn_download_No2 : ui->btn_download_No3);
}



void Community::setTheme_Community(int idx)
{
    QString textColor,colorStr;

    //按钮图片
    QString suffix;
    switch (idx)
    {
    case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
    case 1: suffix = "_white";  break;//白色
    case 2: suffix = "_black";  break;//黑色
    default: suffix = "";      break;
    }
    //查看排行榜按钮
    ui->pBt_ranking->setStyleSheet(
        QString("QPushButton{"
                "border-radius: 29px;"
                "background:transparent;"
                "border-image: url(:/Skin/Images/Community/ranking-no%1.png);"
                "}"
                "QPushButton::hover{"
                "border-image: url(:/Skin/Images/Community/ranking-ho%1.png);"
                "}").arg(suffix)
        );
    //奖杯
    ui->lab_Trophy->setStyleSheet(
        QString("background:transparent;"
                "image: url(:/Skin/Images/Community/Trophy_logo%1.png);")
            .arg(suffix)
        );
    //文字
    switch (idx) {
    case 0: {colorStr = "#FFD42D"; break;}   // 深蓝色
    case 1: {colorStr = "#FFD42D"; break;}   // 白色
    case 2: {colorStr = "#FFD42D"; break;}   // 黑色
    default: {colorStr = "#FFD42D"; break;}
    }
    //点赞榜
    ui->lab_Like_ranking->setStyleSheet(
        QString("border:none;"
                "background: transparent;"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;")
            .arg(colorStr)
        );
    //下载榜
    ui->lab_download_ranking->setStyleSheet(
        QString("border:none;"
                "background: transparent;"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;")
            .arg(colorStr)
        );
    switch (idx) {
    case 0: {colorStr = "rgba(255, 255, 255, 0.1)"; break;}   // 深蓝色
    case 1: {colorStr = "rgba(255, 255, 255, 0.1)"; break;}   // 白色
    case 2: {colorStr = "rgba(255, 255, 255, 0.1)"; break;}   // 黑色
    default: {colorStr = "rgba(255, 255, 255, 0.1)"; break;}
    }
    //隔断
    ui->Separator_label->setStyleSheet(
        QString("border-radius: 1px;"
                "background: %1;")
        .arg(colorStr)
        );
}

//翻译
void Community::LanguageSet()
{
    //刷新文本
    ui->retranslateUi(this);

    // 下传新社区模块（排行榜 + 方案广场页）：语言切换时同步刷新
    if (m_rankingList) m_rankingList->LanguageSet();
    if (m_communityMainPage) m_communityMainPage->LanguageSet();

    int fontSize = 14;

    if(LanguageIdx == 0)//简体
    {
        fontSize = 14;
    }else if(LanguageIdx == 1)//繁體
    {
        fontSize =  14;
    }else if(LanguageIdx == 2)//英文
    {
        fontSize = 10;
    }

    // 固定样式模板，用占位符替换字号
    QString styleTemplate =
        "border: none;"
        "background: transparent;"
        "color: #FFD42D;"
        "font-family: \"Noto Sans S Chinese\";"
        "font-weight: 500;"
        "font-size: %1px;";

    ui->lab_Like_ranking->setStyleSheet(styleTemplate.arg(fontSize));
    ui->lab_download_ranking->setStyleSheet(styleTemplate.arg(fontSize));
}
