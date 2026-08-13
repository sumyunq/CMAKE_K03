#ifndef COMMUNITY_H
#define COMMUNITY_H

#include <QHash>
#include <QWidget>
#include "modules/CommunityModule/ui/ranking/ranking_list.h"
#include "qlabel.h"
#include "modules/CommunityModule/ui/community/community_main_page.h"
#include "modules/CommunityModule/ui/community/user_uploads_dialog.h"  ///< UserProfile（前三名头像点击弹窗）

namespace Ui {
class Community;
}

class QEvent;
class QResizeEvent;

class Community : public QWidget
{
    Q_OBJECT

public:
    explicit Community(QWidget *parent = nullptr);
    ~Community();


    void setTheme_Community(int idx);

    void LanguageSet();

    /// \brief 设置社区模块登录 token（登录成功后调用）
    void setCommunityAuthToken(const QString &token);

    /// \brief 获取 CommunityMainPage（供 MainWindow 转发共享 Service 给个人中心）
    CommunityMainPage *mainPage() const { return m_communityMainPage; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::Community *ui;

    RankingList *m_rankingList = nullptr;
    QWidget *m_rankingMask = nullptr;

    // (badge, avatar) 对，用于布局完成后重定位
    QVector<QPair<QLabel *, QLabel *>> m_badges;

    CommunityMainPage *m_communityMainPage = nullptr; ///< 社区模块主页面（嵌入 widget_left）
    class AvatarCache *m_avatarCache = nullptr;       ///< 头像下载缓存（前三名头像）

    QHash<int, UserProfile> cl_top_three_users_;  ///< 前三名用户资料（key：点赞榜 0-2 / 下载榜 10-12，点击弹窗用）

    void setupAvatars();
    void hideTopThreeAvatars();
    void fetchTopThreeAvatars();  ///< 点赞/下载榜前三名头像（服务器数据）
    void updateTopThreeFromRankingData(
        bool isLikeRanking,
        const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &items);
    void updateTopThreeGroup(
        int keyBase,
        const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &items);
    void ensureTopThreeAvatarCache();
    class AvatarButton *avatarButtonForKey(int key);  ///< key → 头像位按钮（0-2 点赞榜 / 10-12 下载榜）
    void showRankingOverlay();
    void hideRankingOverlay();
    void positionRankingOverlay();

};

#endif // COMMUNITY_H
