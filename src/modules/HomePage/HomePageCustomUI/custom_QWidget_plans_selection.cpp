#include "modules/HomePage/HomePageCustomUI/custom_QWidget_plans_selection.h"
#include "modules/HomePage/home_page_main_page.h"
#include "ui_custom_QWidget_plans_selection.h"

CustomQWidgetPlansSelection::CustomQWidgetPlansSelection(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetPlansSelection)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQWidgetPlansSelection::~CustomQWidgetPlansSelection()
{
    delete ui;
}

void CustomQWidgetPlansSelection::updatePlansUIInfo(DeviceInfo sel_device_info)
{
    auto safePlanName = [this](const QString& name, const QString& dev) -> QString {
        NewRadioBtn* btn = findHomePlan(qMakePair(name, dev));
        return btn ? btn->lab2->text() : QString();
    };

    auto setupPlanSlot = [this](int idx) {
        disconnect(cl_plans_.at(idx).get(), &QPushButton::clicked, this, nullptr);
        QObject::connect(cl_plans_.at(idx).get(), &QPushButton::clicked, this, [this, idx]() {
            if (findHomePlan(cl_plans_.at(idx)->cl_plan_key_)) {
                emit requestHomePlanSelected(cl_plans_.at(idx)->cl_plan_key_);
            }
        });
    };

    // T10
    if (sel_device_info.SelDev_device_name_.contains("T10", Qt::CaseInsensitive)) {
        // 有线
        if (!sel_device_info.SelDev_device_name_.contains("Wireless", Qt::CaseInsensitive)) {
            cl_plans_.at(0)->cl_plans_name_ = safePlanName("CSGO优化版", "T10有线");
            cl_plans_.at(0)->setImages(icon_path_CSGO_normal_, icon_path_CSGO_hover_, icon_path_CSGO_checked_);
            cl_plans_.at(0)->setPlan_key(QPair("CSGO优化版", "T10有线"));
            setupPlanSlot(0);

            cl_plans_.at(1)->cl_plans_name_ = safePlanName("绝地求生优化版", "T10有线");
            cl_plans_.at(1)->setImages(icon_path_PUBG_normal_, icon_path_PUBG_hover_, icon_path_PUBG_checked_);
            cl_plans_.at(1)->setPlan_key(QPair("绝地求生优化版", "T10有线"));
            setupPlanSlot(1);

            cl_plans_.at(2)->cl_plans_name_ = safePlanName("三角洲-清风优化版", "T10有线");
            cl_plans_.at(2)->setImages(icon_path_delta_normal_, icon_path_delta_hover_, icon_path_delta_checked_);
            cl_plans_.at(2)->setPlan_key(QPair("三角洲-清风优化版", "T10有线"));
            setupPlanSlot(2);

            cl_plans_.at(3)->cl_plans_name_ = safePlanName("无畏契约-焚决V1.1", "T10有线");
            cl_plans_.at(3)->setImages(icon_path_valorant_normal_, icon_path_valorant_hover_, icon_path_valorant_checked_);
            cl_plans_.at(3)->setPlan_key(QPair("无畏契约-焚决V1.1", "T10有线"));
            setupPlanSlot(3);
        } else {
            //无线
            cl_plans_.at(0)->cl_plans_name_ = safePlanName("洲-清风优化版", "T10无线");
            cl_plans_.at(0)->setImages(icon_path_delta_normal_, icon_path_delta_hover_, icon_path_delta_checked_);
            cl_plans_.at(0)->setPlan_key(QPair("洲-清风优化版", "T10无线"));
            setupPlanSlot(0);

            cl_plans_.at(1)->cl_plans_name_ = safePlanName("瓦-焚决V1.0", "T10无线");
            cl_plans_.at(1)->setImages(icon_path_valorant_normal_, icon_path_valorant_hover_, icon_path_valorant_checked_);
            cl_plans_.at(1)->setPlan_key(QPair("瓦-焚决V1.0", "T10无线"));
            setupPlanSlot(1);

            cl_plans_.at(2)->cl_plans_name_ = safePlanName("CS学长优化V1", "T10无线");
            cl_plans_.at(2)->setImages(icon_path_CSGO_normal_, icon_path_CSGO_hover_, icon_path_CSGO_checked_);
            cl_plans_.at(2)->setPlan_key(QPair("CS学长优化V1", "T10无线"));
            setupPlanSlot(2);

            cl_plans_.at(3)->cl_plans_name_ = safePlanName("FPS模式-今晚吃鸡", "T10无线");
            cl_plans_.at(3)->setImages(icon_path_PUBG_normal_, icon_path_PUBG_hover_, icon_path_PUBG_checked_);
            cl_plans_.at(3)->setPlan_key(QPair("FPS模式-今晚吃鸡", "T10无线"));
            setupPlanSlot(3);
        }
        return;
    }

    // K06S
    if (sel_device_info.SelDev_device_name_.contains("K06S", Qt::CaseInsensitive)) {
        cl_plans_.at(0)->cl_plans_name_ = safePlanName("FPS模式-瓦-优化", "K06S");
        cl_plans_.at(0)->setImages(icon_path_valorant_normal_, icon_path_valorant_hover_, icon_path_valorant_checked_);
        cl_plans_.at(0)->setPlan_key(QPair("FPS模式-瓦-优化", "K06S"));
        setupPlanSlot(0);

        cl_plans_.at(1)->cl_plans_name_ = safePlanName("洲-焚决V1.0", "K06S");
        cl_plans_.at(1)->setImages(icon_path_delta_normal_, icon_path_delta_hover_, icon_path_delta_checked_);
        cl_plans_.at(1)->setPlan_key(QPair("洲-焚决V1.0", "K06S"));
        setupPlanSlot(1);

        cl_plans_.at(2)->cl_plans_name_ = safePlanName("CS学长特调V1", "K06S");
        cl_plans_.at(2)->setImages(icon_path_CSGO_normal_, icon_path_CSGO_hover_, icon_path_CSGO_checked_);
        cl_plans_.at(2)->setPlan_key(QPair("CS学长特调V1", "K06S"));
        setupPlanSlot(2);

        cl_plans_.at(3)->cl_plans_name_ = safePlanName("吃鸡V1", "K06S");
        cl_plans_.at(3)->setImages(icon_path_PUBG_normal_, icon_path_PUBG_hover_, icon_path_PUBG_checked_);
        cl_plans_.at(3)->setPlan_key(QPair("吃鸡V1", "K06S"));
        setupPlanSlot(3);
        return;
    }

    // T7 GT
    if (sel_device_info.SelDev_device_name_.contains("T7 GT", Qt::CaseInsensitive)) {
        cl_plans_.at(0)->cl_plans_name_ = safePlanName("T7-焚决V1.0", "T7");
        cl_plans_.at(0)->setImages(icon_path_delta_normal_, icon_path_delta_hover_, icon_path_delta_checked_);
        cl_plans_.at(0)->setPlan_key(QPair("T7-焚决V1.0", "T7"));
        setupPlanSlot(0);

        cl_plans_.at(1)->cl_plans_name_ = safePlanName("无畏契约-焚决V1", "T7");
        cl_plans_.at(1)->setImages(icon_path_valorant_normal_, icon_path_valorant_hover_, icon_path_valorant_checked_);
        cl_plans_.at(1)->setPlan_key(QPair("无畏契约-焚决V1", "T7"));
        setupPlanSlot(1);

        cl_plans_.at(2)->cl_plans_name_ = safePlanName("T7 PUBG优化", "T7");
        cl_plans_.at(2)->setImages(icon_path_PUBG_normal_, icon_path_PUBG_hover_, icon_path_PUBG_checked_);
        cl_plans_.at(2)->setPlan_key(QPair("T7 PUBG优化", "T7"));
        setupPlanSlot(2);

        cl_plans_.at(3)->cl_plans_name_ = safePlanName("T7 CS优化", "T7");
        cl_plans_.at(3)->setImages(icon_path_CSGO_normal_, icon_path_CSGO_hover_, icon_path_CSGO_checked_);
        cl_plans_.at(3)->setPlan_key(QPair("T7 CS优化", "T7"));
        setupPlanSlot(3);
        return;
    }

    // T7
    if (sel_device_info.SelDev_device_name_.contains("T7", Qt::CaseInsensitive)) {
        cl_plans_.at(0)->cl_plans_name_ = safePlanName("T7-焚决V1.0", "T7");
        cl_plans_.at(0)->setImages(icon_path_delta_normal_, icon_path_delta_hover_, icon_path_delta_checked_);
        cl_plans_.at(0)->setPlan_key(QPair("T7-焚决V1.0", "T7"));
        setupPlanSlot(0);

        cl_plans_.at(1)->cl_plans_name_ = safePlanName("无畏契约-焚决V1", "T7");
        cl_plans_.at(1)->setImages(icon_path_valorant_normal_, icon_path_valorant_hover_, icon_path_valorant_checked_);
        cl_plans_.at(1)->setPlan_key(QPair("无畏契约-焚决V1", "T7"));
        setupPlanSlot(1);

        cl_plans_.at(2)->cl_plans_name_ = safePlanName("T7 PUBG优化", "T7");
        cl_plans_.at(2)->setImages(icon_path_PUBG_normal_, icon_path_PUBG_hover_, icon_path_PUBG_checked_);
        cl_plans_.at(2)->setPlan_key(QPair("T7 PUBG优化", "T7"));
        setupPlanSlot(2);

        cl_plans_.at(3)->cl_plans_name_ = safePlanName("T7 CS优化", "T7");
        cl_plans_.at(3)->setImages(icon_path_CSGO_normal_, icon_path_CSGO_hover_, icon_path_CSGO_checked_);
        cl_plans_.at(3)->setPlan_key(QPair("T7 CS优化", "T7"));
        setupPlanSlot(3);
        return;
    }

    // K03S超竞版
    if (sel_device_info.SelDev_device_name_.contains("K03S", Qt::CaseInsensitive)
        && ( (sel_device_info.SelDev_pid_ == 0xF016) || (sel_device_info.SelDev_pid_ == 0xF017) )) {
        cl_plans_.at(0)->cl_plans_name_ = safePlanName("三角洲-焚决V1.0", "K03S超竞版");
        cl_plans_.at(0)->setImages(icon_path_delta_normal_, icon_path_delta_hover_, icon_path_delta_checked_);
        cl_plans_.at(0)->setPlan_key(QPair("三角洲-焚决V1.0", "K03S超竞版"));
        setupPlanSlot(0);

        cl_plans_.at(1)->cl_plans_name_ = safePlanName("无畏契约-焚决V1.0", "K03S超竞版");
        cl_plans_.at(1)->setImages(icon_path_valorant_normal_, icon_path_valorant_hover_, icon_path_valorant_checked_);
        cl_plans_.at(1)->setPlan_key(QPair("无畏契约-焚决V1.0", "K03S超竞版"));
        setupPlanSlot(1);

        cl_plans_.at(2)->cl_plans_name_ = safePlanName("GO学长专用", "K03S超竞版");
        cl_plans_.at(2)->setImages(icon_path_CSGO_normal_, icon_path_CSGO_hover_, icon_path_CSGO_checked_);
        cl_plans_.at(2)->setPlan_key(QPair("GO学长专用", "K03S超竞版"));
        setupPlanSlot(2);

        cl_plans_.at(3)->cl_plans_name_ = safePlanName("大吉大利今晚吃鸡", "K03S超竞版");
        cl_plans_.at(3)->setImages(icon_path_PUBG_normal_, icon_path_PUBG_hover_, icon_path_PUBG_checked_);
        cl_plans_.at(3)->setPlan_key(QPair("大吉大利今晚吃鸡", "K03S超竞版"));
        setupPlanSlot(3);
        return;
    }

    // S21无线智充版（预设方案占位预留，目前无官方方案）
    if (sel_device_info.SelDev_device_name_.contains("S21", Qt::CaseInsensitive)) {
        // TODO: S21 官方预设方案待补充（暂无，槽位保持空）
        return;
    }
}

void CustomQWidgetPlansSelection::updatePlansUIInfo(QPair<QString, QString> target_plan_Key)
{
    // 纯显示同步（不产生业务副作用）：
    // 方案开关状态真源为持久化的 HomePagePlansOpen（MainWindow 维护），此处仅按真源刷新显示；
    // 按键组高亮反映当前方案是否命中 4 个首页预设。
    // 保存互斥状态
    bool wasExclusive = cl_plans_buttonGroup_->exclusive();

    // 临时关闭互斥
    cl_plans_buttonGroup_->setExclusive(false);

    NewRadioBtn *targetPlan = findHomePlan(target_plan_Key);

    // 遍历按钮组，根据 cl_plan_key_ 判断
    for (QAbstractButton *btn : cl_plans_buttonGroup_->buttons()) {
        CustomQPushButtonSinglePlan *planBtn = qobject_cast<CustomQPushButtonSinglePlan *>(btn);
        if (planBtn && targetPlan && findHomePlan(planBtn->cl_plan_key_) == targetPlan) {
            planBtn->setChecked(true);
        } else {
            // 取消选中
            planBtn->setChecked(false);
        }
    }
    // 恢复互斥状态
    cl_plans_buttonGroup_->setExclusive(wasExclusive);

    // 开关显示与真源一致（不重算、不改真源）
    QSignalBlocker blocker(cl_customPushButton_);
    cl_customPushButton_->setChecked(HomePagePlansOpen);
}

QPair<QString, QString> CustomQWidgetPlansSelection::normalizePlanKey(
    const QPair<QString, QString> &planKey) const
{
    if (MovieVal.AllPlanRadioHash.contains(planKey)) {
        return planKey;
    }

    const QPair<QString, QString> fallbackKey = qMakePair(planKey.first, QString());
    if (MovieVal.AllPlanRadioHash.contains(fallbackKey)) {
        return fallbackKey;
    }

    return planKey;
}

NewRadioBtn *CustomQWidgetPlansSelection::findHomePlan(
    const QPair<QString, QString> &planKey) const
{
    return MovieVal.AllPlanRadioHash.value(normalizePlanKey(planKey), nullptr);
}

NewRadioBtn *CustomQWidgetPlansSelection::firstAvailableHomePlan() const
{
    for (const auto &planButton : cl_plans_) {
        if (!planButton || planButton->cl_plan_key_.first.isEmpty()) {
            continue;
        }

        if (NewRadioBtn *plan = findHomePlan(planButton->cl_plan_key_)) {
            return plan;
        }
    }

    return nullptr;
}

bool CustomQWidgetPlansSelection::isCurrentPlanHomePlan() const
{
    for (const auto &planButton : cl_plans_) {
        if (!planButton || planButton->cl_plan_key_.first.isEmpty()) {
            continue;
        }

        if (findHomePlan(planButton->cl_plan_key_) == currentPlanRadio) {
            return true;
        }
    }

    return false;
}

void CustomQWidgetPlansSelection::InitUIInformation()
{
    {
        cl_top_content_widget_ = new QWidget(this); ///< 顶部内容，内部为  cl_hBoxLayout_
        cl_top_content_widget_->setFixedHeight(22);
    }
    {
        cl_text_label_ = new QLabel(tr("方案预设"), cl_top_content_widget_); ///< 方案预设 文字
        cl_text_label_->setMinimumSize(56, 20);
        cl_text_label_->setStyleSheet(R"(
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 14px;
            color: #A1A8B3;
)");
    }
    {
        // 说明按键
        cl_pBt_explain_ = new QPushButton(this);
        cl_pBt_explain_->setFixedSize (cl_pBt_explain_default_size_);
        cl_pBt_explain_->setObjectName("PlansSelection_cl_pBt_explain");
        cl_pBt_explain_->setCursor(Qt::PointingHandCursor);
        cl_pBt_explain_->setStyleSheet(R"(
        QPushButton{
            border-image: url(:/Skin/Images/homePage/annotation_13_13_2x.png);
        }
)");
        {
            clp_tip_explain_ = new NewCustomToolTip(cl_pBt_explain_);
            clp_tip_explain_->setLabelStyle(0);
            clp_tip_explain_->AddToolTip(cl_pBt_explain_, tr("保存多组音效设置，快速切换不同游戏/使用场景。"), Qt::AlignHCenter);
        }
    }
    {
        cl_customPushButton_ = new CustomPushButton(cl_top_content_widget_); ///< 启用按键
        cl_customPushButton_->setMinimumSize(46, 22);
        cl_customPushButton_->setFixedHeight(22);
    }
    {
        cl_hBoxLayout_ = new QHBoxLayout(cl_top_content_widget_); ///< 水平布局
        cl_hBoxLayout_->setSpacing(4);
        cl_hBoxLayout_->setContentsMargins(0, 0, 0, 0);
        cl_hBoxLayout_->addWidget(cl_text_label_);
        cl_hBoxLayout_->addWidget(cl_pBt_explain_);
        cl_hBoxLayout_->addStretch();
        cl_hBoxLayout_->addWidget(cl_customPushButton_);

        cl_top_content_widget_->setLayout(cl_hBoxLayout_);
    }

    {
        // 按键组
        cl_plans_buttonGroup_ = new QButtonGroup(this);
    }
    {
        // 四个方案按键
        cl_plans_.clear();
        for (int i = 0; i < cl_size_; ++i) {
            std::shared_ptr<CustomQPushButtonSinglePlan> t_pb_single
                = std::make_shared<CustomQPushButtonSinglePlan>(this);
            t_pb_single->setCheckable(true);

            cl_plans_.append(t_pb_single);
            cl_plans_buttonGroup_->addButton(t_pb_single.get(), i);
        }
    }

    {
        // 网格布局
        cl_gridLayout_ = new QGridLayout(this);
        cl_gridLayout_->setSpacing(14);
        cl_gridLayout_->setContentsMargins(20, 20, 20, 20);

        // 添加到布局中
        cl_gridLayout_->addWidget(cl_top_content_widget_, 0, 0, 1, 2); // 占两列
        cl_gridLayout_->addWidget(cl_plans_.at(0).get(), 1, 0);
        cl_gridLayout_->addWidget(cl_plans_.at(1).get(), 1, 1);
        cl_gridLayout_->addWidget(cl_plans_.at(2).get(), 2, 0);
        cl_gridLayout_->addWidget(cl_plans_.at(3).get(), 2, 1);
    }
    this->setLayout(cl_gridLayout_);

}

void CustomQWidgetPlansSelection::retranslateTexts()
{
    cl_text_label_->setText(tr("方案预设"));
}

void CustomQWidgetPlansSelection::InitMember() {}

void CustomQWidgetPlansSelection::InitConnect() {}
