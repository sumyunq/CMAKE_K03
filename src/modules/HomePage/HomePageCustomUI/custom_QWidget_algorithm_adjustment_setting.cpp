#include "modules/HomePage/HomePageCustomUI/custom_QWidget_algorithm_adjustment_setting.h"
#include "ui_custom_QWidget_algorithm_adjustment_setting.h"

#include <QEvent>

CustomQWidgetAlgorithmAdjustmentSetting::CustomQWidgetAlgorithmAdjustmentSetting(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetAlgorithmAdjustmentSetting)
{
    ui->setupUi(this);
    InitUIInformation();
    InitMember();
    InitConnect();
    InitEventFilter();
}

CustomQWidgetAlgorithmAdjustmentSetting::~CustomQWidgetAlgorithmAdjustmentSetting()
{
    delete ui;
}

void CustomQWidgetAlgorithmAdjustmentSetting::setEditStatus(bool status)
{
    cl_single_algorithm_setting_1_->setEditStatus(status);
    cl_single_algorithm_setting_2_->setEditStatus(status);
    cl_single_algorithm_setting_3_->setEditStatus(status);
    cl_single_algorithm_setting_4_->setEditStatus(status);
}

void CustomQWidgetAlgorithmAdjustmentSetting::InitUIInformation()
{
    {
        cl_content_widget_ = new QWidget(this);
        // 测试用
        //         cl_content_widget_->setStyleSheet(R"(
        //             background-color: #FF0000;
        // )");
    }
    {
        cl_text_label_ = new QLabel(this); ///< 算法文字
        cl_text_label_->setMinimumSize(28, 20);
        cl_text_label_->setText(tr("算法"));
        cl_text_label_->setObjectName("CustomQWidgetAlgorithmAdjustmentSetting_cl_text_label_");
        cl_text_label_->setStyleSheet(R"(
            QLabel#CustomQWidgetAlgorithmAdjustmentSetting_cl_text_label_ {
                        font-family: "Noto Sans S Chinese";
                        font-weight: 700;
                        font-size: 14px;
                        color: #A1A8B3;
            }
)");


    }
    {
        // 说明按键
        cl_pBt_explain_ = new QPushButton(this);
        cl_pBt_explain_->setFixedSize(cl_pBt_explain_default_size_);
        cl_pBt_explain_->setObjectName("AlgorithmAdjustment_cl_pBt_explain");
        cl_pBt_explain_->setCursor(Qt::PointingHandCursor);
        cl_pBt_explain_->setStyleSheet(R"(
        QPushButton{
            border-image: url(:/Skin/Images/homePage/annotation_13_13_2x.png);
        }
)");
        {
            clp_tip_explain_ = new NewCustomToolTip(cl_pBt_explain_);
            clp_tip_explain_->setLabelStyle(0);
            clp_tip_explain_->AddToolTip(cl_pBt_explain_, tr("提供脚步增强、枪声弱化、声场控制等游戏音效算法。拖动滑块调节各算法强度，自由组合各项参数，适配不同游戏场景。"), Qt::AlignHCenter);
        }
    }
    {
        cl_customPushButton_ = new CustomPushButton(this); ///< 启用按键
        cl_customPushButton_->setMinimumSize(46, 22);
    }
    {
        cl_hBoxLayout_ = new QHBoxLayout(cl_content_widget_); ///< 水平布局
        cl_hBoxLayout_->setSpacing(4);
        cl_hBoxLayout_->setContentsMargins(0, 0, 0, 0);

        cl_hBoxLayout_->addWidget(cl_text_label_);
        cl_hBoxLayout_->addWidget(cl_pBt_explain_);
        cl_hBoxLayout_->addStretch();
        cl_hBoxLayout_->addWidget(cl_customPushButton_);

        // 设置布局
        cl_content_widget_->setLayout(cl_hBoxLayout_);
    }

    cl_single_algorithm_setting_1_ = new CustomQWidgetSingleAlgorithmSetting(
        this); /// 单个算法页面(脚步增强)
    cl_single_algorithm_setting_1_->updateAlgorithmEffect(tr("脚步增强"));
    cl_single_algorithm_setting_2_ = new CustomQWidgetSingleAlgorithmSetting(
        this); /// 单个算法页面(枪声弱化)
    cl_single_algorithm_setting_2_->updateAlgorithmEffect(tr("枪声弱化"));

    cl_single_algorithm_setting_3_ = new CustomQWidgetSingleAlgorithmSetting(
        this); /// 单个算法页面(声场控制)
    cl_single_algorithm_setting_3_->updateAlgorithmEffect(tr("声场控制"));
    cl_single_algorithm_setting_3_->updateValueRange(-5, 5);

    cl_single_algorithm_setting_4_ = new CustomQWidgetSingleAlgorithmSetting(
        this); /// 单个算法页面(清晰度)
    cl_single_algorithm_setting_4_->updateAlgorithmEffect(tr("清晰度"));

    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(this->layout());
    if (vlayout) {
        vlayout->addWidget(cl_content_widget_);
        QSpacerItem *spacer = new QSpacerItem(0, 21, QSizePolicy::Minimum, QSizePolicy::Fixed);
        vlayout->addSpacerItem(spacer);
        vlayout->addWidget(cl_single_algorithm_setting_1_);
        QSpacerItem *spacer_2 = new QSpacerItem(0, 23, QSizePolicy::Minimum, QSizePolicy::Fixed);
        vlayout->addSpacerItem(spacer_2);
        vlayout->addWidget(cl_single_algorithm_setting_2_);
        QSpacerItem *spacer_3 = new QSpacerItem(0, 23, QSizePolicy::Minimum, QSizePolicy::Fixed);
        vlayout->addSpacerItem(spacer_3);
        vlayout->addWidget(cl_single_algorithm_setting_3_);
        QSpacerItem *spacer_4 = new QSpacerItem(0, 23, QSizePolicy::Minimum, QSizePolicy::Fixed);
        vlayout->addSpacerItem(spacer_4);
        vlayout->addWidget(cl_single_algorithm_setting_4_);
        vlayout->addStretch();
    }

}

void CustomQWidgetAlgorithmAdjustmentSetting::retranslateTexts()
{
    cl_text_label_->setText(tr("算法"));
    cl_single_algorithm_setting_1_->updateAlgorithmEffect(tr("脚步增强"));
    cl_single_algorithm_setting_2_->updateAlgorithmEffect(tr("枪声弱化"));
    cl_single_algorithm_setting_3_->updateAlgorithmEffect(tr("声场控制"));
    cl_single_algorithm_setting_4_->updateAlgorithmEffect(tr("清晰度"));
}

void CustomQWidgetAlgorithmAdjustmentSetting::InitMember() {}

void CustomQWidgetAlgorithmAdjustmentSetting::InitConnect() {}

void CustomQWidgetAlgorithmAdjustmentSetting::InitEventFilter()
{
    cl_need_checked_ = {cl_single_algorithm_setting_1_->cl_add_algorithm_value_,
                        cl_single_algorithm_setting_1_->cl_del_algorithm_value_,
                        cl_single_algorithm_setting_1_->cl_algorithm_value_hSlider_,
                        cl_single_algorithm_setting_2_->cl_add_algorithm_value_,
                        cl_single_algorithm_setting_2_->cl_del_algorithm_value_,
                        cl_single_algorithm_setting_2_->cl_algorithm_value_hSlider_,
                        cl_single_algorithm_setting_3_->cl_add_algorithm_value_,
                        cl_single_algorithm_setting_3_->cl_del_algorithm_value_,
                        cl_single_algorithm_setting_3_->cl_algorithm_value_hSlider_,
                        cl_single_algorithm_setting_4_->cl_add_algorithm_value_,
                        cl_single_algorithm_setting_4_->cl_del_algorithm_value_,
                        cl_single_algorithm_setting_4_->cl_algorithm_value_hSlider_};
    for (QWidget *target_widget : cl_need_checked_) {
        target_widget->installEventFilter(this);    // 过滤禁用状态下的点击事件
    }
}

bool CustomQWidgetAlgorithmAdjustmentSetting::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget *>(watched);

    if (widget && cl_need_checked_.contains(widget)) {
        // 禁用状态下点击算法调节控件 → 自动打开算法总开关（不拦截事件，控件正常处理）
        if (event->type() == QEvent::MouseButtonPress && !widget->isEnabled()
            && !cl_customPushButton_->isChecked()) {
            cl_customPushButton_->setChecked(true);
        }
    }
    // 不拦截事件，让控件正常处理
    return false;
}
