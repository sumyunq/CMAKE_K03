#ifndef CUSTOM_QWIDGET_PLAN_INFO_H
#define CUSTOM_QWIDGET_PLAN_INFO_H

#include <QHash>
#include <QStringList>
#include <QWidget>

class CustomQWidgetTextBadgeContainer;

///
/// \brief 方案信息布局模式枚举
enum class PlanInfoMode {
    Compact_40_40 = 0, ///< 紧凑模式（图标 40×40）
    Normal_50_50,      ///< 普通模式（图标 50×50）
    Large_58_58,       ///< 宽松模式（图标 58×58）
    Large_71_71,       ///< 大图标模式（图标 71×71）
};

namespace Ui {
class CustomQWidgetPlanInfo;
}

class QLabel;

///
/// \brief The CustomQWidgetPlanInfo class
/// 方案信息展示
/// 子控件：
///     QLabel 图标
///     QLabel 方案名称
///     QLabel 方案描述
class CustomQWidgetPlanInfo : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetPlanInfo(QWidget *parent = nullptr,
                                    PlanInfoMode mode = PlanInfoMode::Large_71_71);
    ~CustomQWidgetPlanInfo();

    PlanInfoMode cl_mode() const;
    void setCl_mode(PlanInfoMode mode);

    QString cl_plan_name() const;
    void setCl_plan_name(const QString &name);

    QString cl_plan_desc() const;
    void setCl_plan_desc(const QString &desc);

    /// \brief 根据 user_tags 设置分类图标（首个匹配的标签 → icon，无匹配 → 默认游戏图标）
    void setCl_category_icon(const QStringList &user_tags, bool t_selected = false);

    /// \brief 获取标签容器（调用方通过 addBadge() 添加标签）
    CustomQWidgetTextBadgeContainer *clp_tag_container() const;

protected:
    void resizeEvent(QResizeEvent *event) override; ///< 更新所有子控件位置

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void applyMode();         ///< 根据模式更新图标

private:
    Ui::CustomQWidgetPlanInfo *ui;

    QLabel *clp_icon_label_ = nullptr;  ///< 图标
    QLabel *clp_name_label_ = nullptr;  ///< 方案名称
    QLabel *clp_desc_label_ = nullptr;  ///< 方案描述
    QPoint cl_icon_label_point_ = QPoint(12, 12);  ///< 图标位置
    QSize cl_icon_label_size_ = QSize(71, 71);    ///< 图标尺寸
    QPoint cl_name_label_point_ = QPoint(95, 18); ///< 名称位置
    QSize cl_name_label_size_ = QSize(160, 20);   ///< 名称尺寸
    QPoint cl_desc_label_point_ = QPoint(95, 47); ///< 描述位置
    QSize cl_desc_label_size_ = QSize(160, 14);   ///< 描述尺寸
    QPoint cl_tag_container_point_ = QPoint(95, 65); ///< 标签容器位置
    QSize cl_tag_container_size_ = QSize(160, 16);   ///< 标签容器尺寸

    CustomQWidgetTextBadgeContainer *clp_tag_container_ = nullptr; ///< 标签容器

    PlanInfoMode cl_mode_ = PlanInfoMode::Large_71_71; ///< 当前布局模式

    /// 各模式下的布局参数   {iconSize, iconPoint, namePoint, descPoint}
    QSize cl_icon_size_Compact_40_40_ = QSize(40, 40);
    QPoint cl_icon_point_Compact_40_40_ = QPoint(12, 12);
    QPoint cl_name_point_Compact_40_40_ = QPoint(60, 8);
    QPoint cl_desc_point_Compact_40_40_ = QPoint(60, 32);
    QPoint cl_tag_container_point_Compact_40_40_ = QPoint(60, 54);

    QSize cl_icon_size_Normal_50_50_ = QSize(50, 50);
    QPoint cl_icon_point_Normal_50_50_ = QPoint(12, 12);
    QPoint cl_name_point_Normal_50_50_ = QPoint(75, 14);
    QPoint cl_desc_point_Normal_50_50_ = QPoint(75, 38);
    QPoint cl_tag_container_point_Normal_50_50_ = QPoint(75, 58);

    QSize cl_icon_size_Large_58_58_ = QSize(58, 58);
    QPoint cl_icon_point_Large_58_58_ = QPoint(12, 12);
    QPoint cl_name_point_Large_58_58_ = QPoint(85, 18);
    QPoint cl_desc_point_Large_58_58_ = QPoint(85, 43);
    QPoint cl_tag_container_point_Large_58_58_ = QPoint(85, 63);

    // 已确定的
    QSize cl_icon_size_Large_71_71_ = QSize(71, 71);
    QPoint cl_icon_point_Large_71_71_ = QPoint(12, 12);
    QPoint cl_name_point_Large_71_71_ = QPoint(95, 14);
    QPoint cl_desc_point_Large_71_71_ = QPoint(95, 38);
    QPoint cl_tag_container_point_Large_71_71_ = QPoint(95, 65);
};

#endif // CUSTOM_QWIDGET_PLAN_INFO_H
