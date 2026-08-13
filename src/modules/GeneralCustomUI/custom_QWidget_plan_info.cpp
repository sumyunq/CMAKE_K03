#include "modules/GeneralCustomUI/custom_QWidget_plan_info.h"
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge_container.h"
#include "ui_custom_QWidget_plan_info.h"

#include <QLabel>
#include <QResizeEvent>
#include "data/api_global.h" ///< CategoryIcon
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots

CustomQWidgetPlanInfo::CustomQWidgetPlanInfo(QWidget *parent, PlanInfoMode mode)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetPlanInfo)
    , cl_mode_(mode)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
    resize(286, 95);
}

CustomQWidgetPlanInfo::~CustomQWidgetPlanInfo()
{
    delete ui;
}

void CustomQWidgetPlanInfo::InitUIInformation()
{
    // 设置自身背景 + 圆角
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("CustomQWidgetPlanInfo");
    setStyleSheet(R"(
        #CustomQWidgetPlanInfo {
            background: rgba(0, 0, 0, 0.2);
            border-radius: 8px;
        }
    )");

    {
        // 图标
        clp_icon_label_ = new QLabel(this);
        clp_icon_label_->setObjectName("CustomQWidgetPlanInfo_icon");
        clp_icon_label_->setScaledContents(true);
    }
    {
        // 方案名称
        clp_name_label_ = new QLabel(this);
        clp_name_label_->setObjectName("CustomQWidgetPlanInfo_name");
        clp_name_label_->setStyleSheet(R"(
            #CustomQWidgetPlanInfo_name {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #FFFFFF;
                background: transparent;
            }
        )");
    }
    {
        // 方案描述
        clp_desc_label_ = new QLabel(this);
        clp_desc_label_->setObjectName("CustomQWidgetPlanInfo_desc");
        clp_desc_label_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        clp_desc_label_->setWordWrap(true);
        clp_desc_label_->setStyleSheet(R"(
            #CustomQWidgetPlanInfo_desc {
                font-family: "Noto Sans S Chinese";
                font-weight: 400;
                font-size: 10px;
                color: rgba(161, 168, 179, 0.5);
                background: transparent;
            }
        )");
    }
    {
        // 标签容器
        clp_tag_container_ = new CustomQWidgetTextBadgeContainer(this);
        clp_tag_container_->setObjectName("CustomQWidgetPlanInfo_tagContainer");
    }
    {
        // 应用默认模式样式
        applyMode();
    }
}

void CustomQWidgetPlanInfo::InitMember() {}

void CustomQWidgetPlanInfo::InitConnect() {}

void CustomQWidgetPlanInfo::applyMode()
{
    switch (cl_mode_) {
    case PlanInfoMode::Compact_40_40:
        cl_icon_label_size_ = cl_icon_size_Compact_40_40_;
        cl_icon_label_point_ = cl_icon_point_Compact_40_40_;
        cl_name_label_point_ = cl_name_point_Compact_40_40_;
        cl_desc_label_point_ = cl_desc_point_Compact_40_40_;
        cl_tag_container_point_ = cl_tag_container_point_Compact_40_40_;
        break;
    case PlanInfoMode::Normal_50_50:
        cl_icon_label_size_ = cl_icon_size_Normal_50_50_;
        cl_icon_label_point_ = cl_icon_point_Normal_50_50_;
        cl_name_label_point_ = cl_name_point_Normal_50_50_;
        cl_desc_label_point_ = cl_desc_point_Normal_50_50_;
        cl_tag_container_point_ = cl_tag_container_point_Normal_50_50_;
        break;
    case PlanInfoMode::Large_71_71:
        cl_icon_label_size_ = cl_icon_size_Large_71_71_;
        cl_icon_label_point_ = cl_icon_point_Large_71_71_;
        cl_name_label_point_ = cl_name_point_Large_71_71_;
        cl_desc_label_point_ = cl_desc_point_Large_71_71_;
        cl_tag_container_point_ = cl_tag_container_point_Large_71_71_;
        break;
    case PlanInfoMode::Large_58_58:
    default:
        cl_icon_label_size_ = cl_icon_size_Large_58_58_;
        cl_icon_label_point_ = cl_icon_point_Large_58_58_;
        cl_name_label_point_ = cl_name_point_Large_58_58_;
        cl_desc_label_point_ = cl_desc_point_Large_58_58_;
        cl_tag_container_point_ = cl_tag_container_point_Large_58_58_;
        break;
    }

    clp_icon_label_->setFixedSize(cl_icon_label_size_);
    clp_icon_label_->setGeometry(QRect(cl_icon_label_point_, cl_icon_label_size_));
    clp_name_label_->setGeometry(QRect(cl_name_label_point_, cl_name_label_size_));
    clp_desc_label_->setGeometry(QRect(cl_desc_label_point_, cl_desc_label_size_));
    clp_tag_container_->setGeometry(
        QRect(cl_tag_container_point_, cl_tag_container_size_));
}

void CustomQWidgetPlanInfo::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    clp_icon_label_->setGeometry(QRect(cl_icon_label_point_, cl_icon_label_size_));
    clp_name_label_->setGeometry(QRect(cl_name_label_point_, cl_name_label_size_));
    clp_desc_label_->setGeometry(QRect(cl_desc_label_point_, cl_desc_label_size_));
    clp_tag_container_->setGeometry(
        QRect(cl_tag_container_point_, cl_tag_container_size_));
}

// ── 标签容器 ──

CustomQWidgetTextBadgeContainer *CustomQWidgetPlanInfo::clp_tag_container() const
{
    return clp_tag_container_;
}

PlanInfoMode CustomQWidgetPlanInfo::cl_mode() const
{
    return cl_mode_;
}

void CustomQWidgetPlanInfo::setCl_mode(PlanInfoMode mode)
{
    if (cl_mode_ == mode)
        return;
    cl_mode_ = mode;
    applyMode();
}

QString CustomQWidgetPlanInfo::cl_plan_name() const
{
    return clp_name_label_->text();
}

void CustomQWidgetPlanInfo::setCl_plan_name(const QString &name)
{
    QFontMetrics t_fm(clp_name_label_->font());
    QString t_elided = DeSheng::elideTextWithDots(name, clp_name_label_->font(),
                                                  cl_name_label_size_.width());
    clp_name_label_->setText(t_elided);
}

QString CustomQWidgetPlanInfo::cl_plan_desc() const
{
    return clp_desc_label_->text();
}

void CustomQWidgetPlanInfo::setCl_plan_desc(const QString &desc)
{
    clp_desc_label_->setText(DeSheng::elideTextWithDots(desc, clp_desc_label_->font(),
                                                        cl_desc_label_size_.width()));
}

void CustomQWidgetPlanInfo::setCl_category_icon(const QStringList &user_tags, bool t_selected)
{
    QString t_base = CategoryIcon::kDefaultBase;

    for (const auto &t_tag : user_tags) {
        if (CategoryIcon::kIconMap.contains(t_tag)) {
            t_base = CategoryIcon::kIconMap.value(t_tag);
            break; ///< 首个匹配即生效
        }
    }

    clp_icon_label_->setPixmap(QPixmap(
        CategoryIcon::buildPath(t_base, t_selected, false)));
}
