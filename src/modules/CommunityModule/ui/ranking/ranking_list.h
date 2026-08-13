#ifndef UI_RANKING_RANKING_LIST_H
#define UI_RANKING_RANKING_LIST_H

#include <QButtonGroup>
#include <QHash>
#include <QLabel>
#include <QSet>
#include <QWidget>

#include "modules/CommunityModule/service/scheme_service.h"
#include "modules/CommunityModule/ui/ranking/ranking_delegate.h"
#include "modules/CommunityModule/ui/ranking/ranking_model.h"
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_loading.h"
#include "modules/GeneralCustomUI/custom_QWidget_single_plans.h"
#include "repository/user_config_repository.h"

namespace Ui {
class RankingList;
}

class QHideEvent;

/// \brief 排行榜弹窗 — Model/View 版（点赞/下载 × 月度/总榜）
class RankingList : public QWidget
{
    Q_OBJECT

 public:
    /// \brief 构造弹窗；svc/repo 注入社区共享实例（点赞/下载与社区 5-model 双向同步），
    ///        传 nullptr 则自建（独立模式）
    explicit RankingList(QWidget *parent = nullptr,
                         SchemeService *svc = nullptr, UserConfigRepository *repo = nullptr);
    ~RankingList() override;

    /// 语言切换：刷新 .ui 文本与代码内设置的"加载中"文字
    void LanguageSet();

 signals:
    void topThreeDataReady(
        bool isLikeRanking,
        const QList<DeSheng::GetPublicConfigurationListResponse::ListItem> &items);

 protected:
    void showEvent(QShowEvent *event) override;    ///< 每次打开自动刷新
    void hideEvent(QHideEvent *event) override;    ///< 关闭排行榜时同步关闭方案卡片
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;  ///< 空态标签点击重试；列表空白/其他条目点击 → 关闭方案卡片
    void mousePressEvent(QMouseEvent *event) override;       ///< 兜底：点击弹窗无子控件区域 → 关闭方案卡片

 private:
    void InitMember();
    void InitConnect();
    void cl_refresh_data_();                         ///< 拉取榜单（loading → 成功/空/失败）
    void cl_show_empty_label_(const QString &text, const QString &iconPath = QString());  ///< 空态/失败提示（iconPath 非空 → 图标 + 文字）
    void cl_hide_empty_label_();
    void cl_update_loading_position_();
    void cl_set_combo_shadow_();                     ///< NewComboBox 弹层阴影（旧逻辑迁移）
    void cl_update_round_clip_();                    ///< 同步弹窗视觉圆角与真实裁剪区域
    void cl_on_button_clicked_(const QModelIndex &index); ///< 行内按钮：点赞榜=点赞，下载榜=下载
    void cl_do_like_(const QModelIndex &index);           ///< 点赞流程（防连点 + 服务器同步）
    void cl_do_download_(const QModelIndex &index);       ///< 下载流程（防重复 + 导入方案库）
    void cl_show_toast_(const QString &text);             ///< 轻提示（2 秒自动隐藏）
    void cl_populate_card_(int row, const QPoint &viewportPos);  ///< 填充并定位方案卡片（点击处下方 + 屏幕自适应）

 private:
    Ui::RankingList *ui;

 private:
    RankingModel *clp_model_ = nullptr;       ///< 数据模型
    RankingDelegate *clp_delegate_ = nullptr; ///< 行委托
    UserConfigRepository *clp_repo_ = nullptr;///< 配置仓库（点赞/头像）
    SchemeService *clp_service_ = nullptr;    ///< 点赞同步 service
    QButtonGroup *clp_tab_group_ = nullptr;   ///< 点赞/下载 Tab

 private:
    CustomQWidgetLoading *clp_loading_ = nullptr;         ///< 加载动画
    QLabel *clp_empty_icon_label_ = nullptr;              ///< 空态/错误图标（视口内，96×96，点击重试）
    QLabel *clp_empty_label_ = nullptr;                   ///< 空态/失败提示文字（点击重试）
    class QPushButton *clp_retry_btn_ = nullptr;          ///< 刷新按钮（失败时显示，rect 148,395,104,30）
    QLabel *clp_toast_label_ = nullptr;                   ///< 轻提示标签（下载失败等）
    CustomQWidgetSinglePlans *clp_plan_card_ = nullptr;   ///< 方案卡片

 private:
    bool cl_is_like_tab_ = true;  ///< 当前榜类型：true=点赞榜 false=下载榜

 private:
    QSet<int> cl_pending_likes_;              ///< 点赞请求进行中的 config id 集合
    QHash<int, bool> cl_pending_like_originals_;  ///< 各请求点赞原状态（回滚锚点）
    QSet<int> cl_downloading_ids_;  ///< 下载进行中的配置 ID
};

#endif  // UI_RANKING_RANKING_LIST_H
