#ifndef CUSTOM_QWIDGET_SINGLE_PLANS_H
#define CUSTOM_QWIDGET_SINGLE_PLANS_H

#include <QHBoxLayout>
#include <QHash>
#include <QMenu>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "data/userConfig/user_config_api.h"
#include "modules/GeneralCustomUI/custom_QWidget_comments.h" ///< 子控件：评论区标签流
#include "modules/GeneralCustomUI/custom_QWidget_function_button_with_display_label.h" ///< 子控件：功能按键（点赞/下载/分享等）
#include "modules/GeneralCustomUI/custom_QWidget_plan_info.h" ///< 子控件：方案信息
#include "modules/GeneralCustomUI/WidgetStateCache.h" ///< 状态缓存（展开/收起等）

class QNetworkReply;
class QLabel;

namespace Ui {
class CustomQWidgetSinglePlans;
}

///
/// \brief The CustomQWidgetSinglePlans class
/// 单个方案展示
/// 子控件：
///     QWidget (widget_01)                    顶部区
///     QWidget (widget_02)                    中部区（含 pushButton 展开按钮）
///     QWidget (widget_03)                    方案信息区
///     QWidget (widget_04)                    底部按键区
class CustomQWidgetSinglePlans : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetSinglePlans(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetSinglePlans();

    QSize sizeHint() const override; ///< 返回内部布局内容高度

    QSize cl_widget_01_size() const;                ///< 获取 widget_01 标准尺寸
    void setCl_widget_01_size(const QSize &size);   ///< 设置 widget_01 标准尺寸
    QSize cl_widget_02_size() const;
    void setCl_widget_02_size(const QSize &size);
    QSize cl_widget_03_size() const;
    void setCl_widget_03_size(const QSize &size);
    QSize cl_widget_04_size() const;
    void setCl_widget_04_size(const QSize &size);
    QSize cl_size() const;
    void setCl_size(const QSize &size);

    void applyTheme(int theme); ///< 应用主题样式

    /// \brief 可显隐元素（对齐社区 CommunityDelegate::ActionButton）
    enum ActionButton { ActionMore = 0, ActionLike, ActionDislike, ActionDownload, ActionShare };

    /// \brief 设置元素显隐（默认：更多按钮隐藏，点赞/踩/下载/分享显示——与社区 item 一致）
    void setActionVisible(ActionButton btn, bool visible);
    bool actionVisible(ActionButton btn) const;

    /// \brief 设置用户头像（自动缩放到 40×40 圆形）
    void setAvatarPixmap(const QPixmap &pixmap);

    /// \brief 设置作者信息（昵称 + 等级）
    void setAuthorInfo(const QString &nickname, int level);

    /// \brief 设置方案基本信息（名称、描述、分类图标）
    void setPlanInfo(const QString &title, const QString &desc,
                     const QStringList &user_tags);

    int cl_config_id() const;                              ///< 获取配置 ID
    void setCl_config_id(int id);                          ///< 设置配置 ID
    void setCl_device_name(const QString &name);           ///< 设置机型（分享拼接用）
    void setCl_scene(const QString &scene);                ///< 设置场景（分享拼接用）
    void setAuthorBadges(const QStringList &roles, bool expertTag); ///< 设置作者身份徽标
    void setVisibility(const QString &visibility);         ///< 设置私密状态图标
    void setComments(const QList<DeSheng::GetPublicConfigurationListResponse::Comment> &comments);
    void setActionState(int likeCount, int dislikeCount, int downloadCount, int shareCount,
                        bool liked, bool disliked);

    /// \brief 解绑数据：保存展开状态到 cache、清空评论标签
    void unbindData(WidgetStateCache &cache);

    /// \brief 从状态缓存恢复展开/收起状态
    void restoreState(const WidgetStateCache &cache);

signals:
    void liked();      ///< 点赞完成
    void unliked();    ///< 取消点赞完成
    void disliked();   ///< 踩完成
    void undisliked(); ///< 取消踩完成
    void download();   ///< 下载完成
    void share();      ///< 分享完成
    void deleteRequested();    ///< 请求删除本方案
    void pinRequested();       ///< 请求置顶本方案
    void visibilityRequested();///< 请求切换可见性

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void doLike();            ///< 执行点赞
    void doUnlike();          ///< 执行取消点赞
    void doDislike();         ///< 执行踩
    void doUndislike();       ///< 执行取消踩
    void doDownload();        ///< 执行下载
    void doShare();           ///< 执行分享
    void doClickComment(int comment_id, class CustomQLabelTag *tag);       ///< 点击评论
    void doCancelClickComment(int comment_id, class CustomQLabelTag *tag); ///< 取消点击评论
    void doDeletePlan();      ///< 删除本方案（确认弹窗 + DELETE API + 成功 emit deleteRequested）
    void refreshCounts();     ///< 拉取配置详情，用服务端数据纠正乐观更新
    void clearAuthorBadges();
    void addAuthorBadge(const QString &pixmapPath, const QSize &size);

public:
    void clearComments();     ///< 清空评论标签，复用前调用
    void abortDownload();     ///< 中断进行中的下载，unbind 时调用

public:
    CustomQWidgetPlanInfo *clp_plan_info_ = nullptr; ///< 方案信息
    QPushButton *clp_action_btn_ = nullptr;                ///< 更多操作按钮
    QMenu *clp_action_menu_ = nullptr;                     ///< 操作菜单
    QPointer<CustomQWidgetComments> clp_comments_;          ///< 评论区标签流

public:
    CustomQWidgetFunctionButtonWithDisplayLabel *clp_like_button_ = nullptr;     ///< 点赞按键
    CustomQWidgetFunctionButtonWithDisplayLabel *clp_dislike_button_ = nullptr;  ///< 踩按键
    CustomQWidgetFunctionButtonWithDisplayLabel *clp_download_button_ = nullptr; ///< 下载按键
    CustomQWidgetFunctionButtonWithDisplayLabel *clp_share_button_ = nullptr;    ///< 分享按键

public:
    QSize cl_widget_01_size_ = QSize(330, 48);  ///< widget_01 标准尺寸
    QSize cl_widget_02_size_ = QSize(330, 46);  ///< widget_02 标准尺寸
    QSize cl_widget_03_size_ = QSize(330, 95);  ///< widget_03 标准尺寸
    QSize cl_widget_04_size_ = QSize(330, 14);  ///< widget_04 标准尺寸
    QSize cl_size_ = QSize(330, 315);           ///< 面板整体标准尺寸

private:
    Ui::CustomQWidgetSinglePlans *ui;

private:
    QHBoxLayout *clp_button_layout_ = nullptr; ///< 按键水平布局
    QHash<int, bool> cl_action_visibility_;    ///< 元素显隐（未显式设置时 ActionMore 默认隐藏，其余显示）

private:
    int cl_config_id_ = 0;                                 ///< 配置 ID
    int cl_theme_ = 0;                                     ///< 当前主题
    QString cl_device_name_;                                ///< 设备名称（机型，分享拼接用）
    QString cl_scene_;                                      ///< 场景（标签，分享拼接用）
    QString cl_plan_name_full_;                             ///< 方案全名（截断前，分享拼接用）
    QList<QLabel *> clp_role_badges_;                       ///< 作者身份徽标
    int cl_author_next_badge_x_ = 0;                        ///< 作者徽标下一绘制 X

    QNetworkReply *clp_active_download_reply_ = nullptr;   ///< 进行中的下载请求（unbind 时 abort）
};

#endif // CUSTOM_QWIDGET_SINGLE_PLANS_H
