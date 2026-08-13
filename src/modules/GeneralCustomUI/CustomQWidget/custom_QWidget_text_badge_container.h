#ifndef CUSTOM_QWIDGET_TEXT_BADGE_CONTAINER_H
#define CUSTOM_QWIDGET_TEXT_BADGE_CONTAINER_H

#include <QWidget>

class QHBoxLayout;
class CustomQWidgetTextBadge;

///
/// \brief 文字徽章容器（水平排列多个 CustomQWidgetTextBadge，右侧弹簧左挤）
///
/// 使用场景：状态标签行、分类标签栏、搜索结果关键词高亮等。
///
/// \code
/// auto *t_container = new CustomQWidgetTextBadgeContainer(parent);
/// t_container->setCl_spacing(8);
/// t_container->addBadge(tr("CSGO"));
/// t_container->addBadge(tr("FPS"))->setCl_fire_enabled(true);  // 热门标签加火焰
/// t_container->addBadge(tr("官方"));
/// \endcode
class CustomQWidgetTextBadgeContainer : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetTextBadgeContainer(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetTextBadgeContainer();

    void applyTheme(int theme);                         ///< 应用主题样式

    CustomQWidgetTextBadge *addBadge(const QString &text); ///< 添加徽章，返回指针供链式配置
    void removeBadge(CustomQWidgetTextBadge *badge);       ///< 移除指定徽章
    void removeBadgeAt(int index);                         ///< 按索引移除
    void clearBadges();                                    ///< 清空全部徽章

    int  badgeCount() const;                               ///< 徽章数量
    CustomQWidgetTextBadge *badgeAt(int index) const;      ///< 获取第 N 个徽章（0-based）

    void setCl_spacing(int spacing);                       ///< 徽章间距，默认 6
    void setCl_margin(int left, int top, int right, int bottom); ///< 容器外边距，默认 0/0/0/0

    int  cl_spacing() const;                               ///< 获取间距

private:
    void InitUIInformation(int theme);
    void InitMember();
    void InitConnect();

private:
    QHBoxLayout *clp_layout_ = nullptr;                   ///< 水平布局
    int cl_theme_ = 0;                                    ///< 主题
};

#endif // CUSTOM_QWIDGET_TEXT_BADGE_CONTAINER_H
