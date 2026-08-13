#include "modules/HomePage/home_page_main_page.h"
#include "APOThread/ApoManager.h"
#include "ui_home_page_main_page.h"

QList<int> HomePageExtraEQValue = {0, 0, 0, 0, 0, 0, 0}; /// 首页 额外eq 默认 0
bool HomePageExtraEQOpen = false;                        ///< 首页算法开关
bool HomePagePlansOpen = false;                          ///< 首页预设方案开关

HomePageMainPage::HomePageMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HomePageMainPage)
{
    ui->setupUi(this);
    InitUIInformation(); // 初始化UI的默认信息
    InitMember();        // 初始化内部成员
    InitConnect();       // 连接默认的信号槽
}

HomePageMainPage::~HomePageMainPage()
{
    delete ui;
}

void HomePageMainPage::updateUIInfo()
{
    // 以下恢复不依赖 currentPlanRadio（设备未连接/方案未就绪时也按 INI 值恢复显示）
    {
        // 算法页面
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_->updateValue(
            HomePageExtraEQValue.at(0));
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_->updateValue(
            HomePageExtraEQValue.at(1));
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_->updateValue(
            HomePageExtraEQValue.at(2));
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_->updateValue(
            HomePageExtraEQValue.at(3));

        // 避免重复
        cl_algorithm_adjustment_setting->cl_customPushButton_->blockSignals(true);
        cl_algorithm_adjustment_setting->cl_customPushButton_->setChecked(HomePageExtraEQOpen);
        cl_algorithm_adjustment_setting->cl_customPushButton_->blockSignals(false);

        // 手动发射信号（恢复即应用：仅在方案/设备就绪时重放，避免设备未连接时
        // 触发算法开关 on 分支副作用——强制关闭方案开关并落盘）
        if (currentPlanRadio != nullptr) {
            emit cl_algorithm_adjustment_setting->cl_customPushButton_->toggled(HomePageExtraEQOpen);
        }
    }
    {
        // 预设方案开关：与 HomePagePlansOpen（真源）同步（原仅 EQ 页 UpdateHomePageUIInfo 信号恢复，
        // 重启后该信号可能不触发 → 按钮状态与持久化值不匹配）
        QSignalBlocker blocker(cl_plans_selection_->cl_customPushButton_);
        cl_plans_selection_->cl_customPushButton_->setChecked(HomePagePlansOpen);
    }

    if (currentPlanRadio == nullptr) {
        return; // 方案库按键组高亮依赖当前方案，未就绪时跳过
    }
    {
        // 主页预设方案页面
        QPair<QString, QString> t_target_plan_Key = qMakePair(currentPlanRadio->lab_name->text(),
                                                              currentPlanRadio->lab1->text());
        cl_plans_selection_->updatePlansUIInfo(t_target_plan_Key);
    }

}

void HomePageMainPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 每次首页可见刷新算法滑块与预设选中显示：
    // 设备连接前/后、切页返回均覆盖（原仅 SelDevSuccess 触发，未连接时滑块恒显示 0）
    // updateValue 不 blockSignals：恢复值经 valueChanged 应用到 APO（恢复即应用语义），冗余保存无害
    updateUIInfo();
}

void HomePageMainPage::InitUIInformation()
{
    {
        // 产品展示页面
        cl_product_display_ = new CustomQWidgetProductDisplay(ui->widget_product_display);
        cl_product_display_->setMinimumSize(526, 616);
        // cl_product_display_->move(25, 24);
        // 在 HomePageMainPage::resizeEvent 中添加

        cl_product_display_->backgroundWidget = ui->widget_product_display;

        ui->widget_product_display->layout()->addWidget(cl_product_display_);

        // 产品展示页面背景
        ui->widget_product_display->setObjectName("widget_product_display");
        ui->widget_product_display->setCornerRadius(10);
        ui->widget_product_display->setStyleSheet(R"(
    QWidget#widget_product_display{
    background-image: url(:/test/hjk.png);
}
)");
    }
    {
        // 声音设置页面
        cl_speaker_setting_ = new CustomQWidetSpeakerSetting(ui->widget_speaker_setting);
        cl_speaker_setting_->setMinimumSize(528, 118);
        ui->widget_speaker_setting->layout()->addWidget(cl_speaker_setting_);
    }
    {
        // 麦克风设置页面
        cl_microphone_setting_ = new CustomQWidgetMicrophoneSetting(ui->widget_microphone_setting);
        cl_microphone_setting_->setMinimumSize(528, 118);
        ui->widget_microphone_setting->layout()->addWidget(cl_microphone_setting_);
    }
    {
        // 算法调节页面
        cl_algorithm_adjustment_setting = new CustomQWidgetAlgorithmAdjustmentSetting(
            ui->widget_algorithm_adjustment_setting);
        cl_algorithm_adjustment_setting->setMinimumSize(344, 353);
        ui->widget_algorithm_adjustment_setting->layout()->addWidget(
            cl_algorithm_adjustment_setting);
    }
    {
        // 方案选择页面
        cl_plans_selection_ = new CustomQWidgetPlansSelection(
            ui->widget_plans_selection); ///< 声音设置页面
        cl_plans_selection_->setMinimumSize(170, 170);
        ui->widget_plans_selection->layout()->addWidget(cl_plans_selection_);
    }

    {
        // 人声调节

        cl_microphone_adjustment_ = new CustomQWidgetMicrophoneAdjustment(
            ui->widget_microphone_adjustment); ///< 声音设置页面
        cl_microphone_adjustment_->setMinimumSize(170, 170);
        ui->widget_microphone_adjustment->layout()->addWidget(cl_microphone_adjustment_);
    }
    {
        // 产品展示页 样式
        ui->widget_product_display->setObjectName("widget_product_display");
        ui->widget_product_display->setCornerRadius(10);
        ui->widget_product_display->setStyleSheet(R"(
    QWidget#widget_product_display{
    border-radius: 10px;
    background: rgba(81, 96, 122, 0.2);
}
    )");
    }

    {
        // 声音设置页 样式
        ui->widget_speaker_setting->setObjectName("widget_speaker_setting");
        ui->widget_speaker_setting->setCornerRadius(10);
        ui->widget_speaker_setting->setStyleSheet(R"(
    QWidget#widget_speaker_setting{
    border-radius: 10px;
    background: rgba(81, 96, 122, 0.2);
}
    )");
    }

    {
        // 麦克风设置页 样式
        ui->widget_microphone_setting->setObjectName("widget_microphone_setting");
        ui->widget_microphone_setting->setCornerRadius(10);
        ui->widget_microphone_setting->setStyleSheet(R"(
    QWidget#widget_microphone_setting{
    border-radius: 10px;
    background: rgba(81, 96, 122, 0.2);
}
    )");
    }

    {
        // 算法设置页 样式
        ui->widget_algorithm_adjustment_setting->setObjectName(
            "widget_algorithm_adjustment_setting");
        ui->widget_algorithm_adjustment_setting->setCornerRadius(10);
        ui->widget_algorithm_adjustment_setting->setStyleSheet(R"(
    QWidget#widget_algorithm_adjustment_setting{
    border-radius: 10px;
    background: rgba(81, 96, 122, 0.2);
}
    )");
    }

    {
        // 预设方案选择页 样式
        ui->widget_plans_selection->setObjectName("widget_plans_selection");
        ui->widget_plans_selection->setCornerRadius(10);
        ui->widget_plans_selection->setStyleSheet(R"(
    QWidget#widget_plans_selection{
    border-radius: 10px;
    background: rgba(81, 96, 122, 0.2);
}
    )");
    }

    {
        // 人声算法调节页 样式
        ui->widget_microphone_adjustment->setObjectName("widget_microphone_adjustment");
        ui->widget_microphone_adjustment->setCornerRadius(10);
        ui->widget_microphone_adjustment->setStyleSheet(R"(
    QWidget#widget_microphone_adjustment{
    border-radius: 10px;
    background: rgba(81, 96, 122, 0.2);
}
    )");
    }
}

void HomePageMainPage::InitMember()
{
    // {
    //     // 首页互斥 按键组
    //     cl_button_group_ = new QButtonGroup(this);
    //     cl_button_group_->addButton(cl_plans_selection_->cl_customPushButton_);
    //     cl_button_group_->addButton(cl_algorithm_adjustment_setting->cl_customPushButton_);
    //     cl_button_group_->setExclusive(true);
    // }
}

void HomePageMainPage::InitConnect()
{
    // // 测试模糊度
    // QObject::connect(ui->horizontalSlider_2, &QSlider::valueChanged, this, [=](int value) {
    //     CumtomQWidgetGlobalBase::setS_g_BlurRadius(static_cast<qreal>(value));
    // });

    // // 支持单独关闭
    // connect(cl_button_group_,
    //         QOverload<QAbstractButton *,bool>::of(&QButtonGroup::buttonToggled),
    //         this,
    //         [this](QAbstractButton *button,bool checked) {
    //             // 如果点击的是已选中的按钮
    //             if (button->isChecked()) {
    //                 // 暂时关闭互斥，取消选中，再恢复互斥
    //                 cl_button_group_->setExclusive(false);
    //                 button->setChecked(false);
    //                 cl_button_group_->setExclusive(true);
    //             }
    //         });

    // 更新算法UI
    // QObject::connect(
    //     cl_plans_selection_,
    //     &CustomQWidgetPlansSelection::updatePlan,
    //     cl_algorithm_adjustment_setting,
    //     [=]() {
    //         cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_->updateValue(
    //             HomePageExtraEQValue.at(0));
    //         cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_->updateValue(
    //             HomePageExtraEQValue.at(1));
    //         cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_->updateValue(
    //             HomePageExtraEQValue.at(2));
    //         cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_->updateValue(
    //             HomePageExtraEQValue.at(3));
    //     });

    // 算法调节页面 各数值变化
    // 脚步增强
    QObject::connect(
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_->cl_algorithm_value_hSlider_,
        &QSlider::valueChanged,
        this,
        [=](int value) {
            if (value == 0) {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_
                    ->cl_del_algorithm_value_->setEnabled(false);
                //额外eq
                emit ApoManager::instance()->requestSetExtendEqState(2, false);

            } else {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_
                    ->cl_del_algorithm_value_->setEnabled(true);
                qDebug("requestSetExtendEqState2 true\n");
                emit ApoManager::instance()->requestSetExtendEqState(2, true);
                if (value
                    == cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_
                           ->cl_algorithm_value_hSlider_->maximum()) {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_
                        ->cl_add_algorithm_value_->setEnabled(false);
                } else {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_
                        ->cl_add_algorithm_value_->setEnabled(true);
                }
            }

            double freq[10] = {2500, 7000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000};
            // double freq[10] = {20,75,150,250,700,1500,2000,4000,8000,16000};//测试 {-12,-12,-12,-12,-12,-12,-12,-12,-12,-12}  {0,0,0,0,0,0,0,0,0,0} {-1.6,-2,0,0,0,0,0,0,0,0}
            QVector<double> freqVec(freq, freq + 10);
            emit ApoManager::instance()
                ->requestSetExtendEqualizerCenterFrequencyEx(2, freqVec); //设置频点
            double QVal[10] = {1.2, 1, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7};
            QVector<double> QValVec(QVal, QVal + 10);

            emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(2, QValVec); //设置Q

            double GVal[7][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-1.6, -2, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-3.2, -4, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-4.8, -6, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-6.4, -8, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-8, -10, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-9.6, -12, 0, 0, 0, 0, 0, 0, 0, 0}};

            QVector<double> GValVec(GVal[value], GVal[value] + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerGainEx(2, GValVec); //设置Gain

            cl_algorithm_adjustment_setting->cl_single_algorithm_setting_1_->cl_text_algorithm_value_
                ->setText(QString::number(value));
            HomePageExtraEQValue[0] = value;
            emit HomePageEQValueChange();
        });

    // 枪声弱化
    QObject::connect(
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_->cl_algorithm_value_hSlider_,
        &QSlider::valueChanged,
        this,
        [=](int value) {
            if (value == 0) {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_
                    ->cl_del_algorithm_value_->setEnabled(false);
                emit ApoManager::instance()->requestSetExtendEqState(3, false);
            } else {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_
                    ->cl_del_algorithm_value_->setEnabled(true);
                emit ApoManager::instance()->requestSetExtendEqState(3, true);
                if (value
                    == cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_
                           ->cl_algorithm_value_hSlider_->maximum()) {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_
                        ->cl_add_algorithm_value_->setEnabled(false);
                } else {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_
                        ->cl_add_algorithm_value_->setEnabled(true);
                }
            }
            double freq[10] = {90, 1500, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000};
            QVector<double> freqVec(freq, freq + 10);
            emit ApoManager::instance()
                ->requestSetExtendEqualizerCenterFrequencyEx(3, freqVec); //设置频点
            double QVal[10] = {2, 1, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7};
            QVector<double> QValVec(QVal, QVal + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(3, QValVec); //设置Q

            double GVal[7][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {2, 1.2, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {4, 2.4, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {6, 3.6, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {8, 4.8, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {10, 6, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {12, 7.2, 0, 0, 0, 0, 0, 0, 0, 0}};

            QVector<double> GValVec(GVal[value], GVal[value] + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerGainEx(3, GValVec); //设置Gain

            cl_algorithm_adjustment_setting->cl_single_algorithm_setting_2_->cl_text_algorithm_value_
                ->setText(QString::number(value));
            HomePageExtraEQValue[1] = value;
            emit HomePageEQValueChange();
        });

    // 声场控制
    QObject::connect(
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_->cl_algorithm_value_hSlider_,
        &QSlider::valueChanged,
        this,
        [=](int value) {
            if (value == 0) {
                emit ApoManager::instance()->requestSetExtendEqState(4, false);
            } else {
                emit ApoManager::instance()->requestSetExtendEqState(4, true);
            }
            if (value
                == cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_
                       ->cl_algorithm_value_hSlider_->minimum()) {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_
                    ->cl_del_algorithm_value_->setEnabled(false);

            } else {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_
                    ->cl_del_algorithm_value_->setEnabled(true);

                if (value
                    == cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_
                           ->cl_algorithm_value_hSlider_->maximum()) {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_
                        ->cl_add_algorithm_value_->setEnabled(false);
                } else {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_
                        ->cl_add_algorithm_value_->setEnabled(true);
                }
            }
            double freq[10] = {1000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000};
            QVector<double> freqVec(freq, freq + 10);
            emit ApoManager::instance()
                ->requestSetExtendEqualizerCenterFrequencyEx(4, freqVec); //设置频点
            double QVal[10] = {0.5, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7};
            QVector<double> QValVec(QVal, QVal + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(4, QValVec); //设置Q

            double GVal[11][10] = {{-6, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {-4.8, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {-3.6, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {-2.4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {-1.2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {1.2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {2.4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {3.6, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {4.8, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {6.0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

            QVector<double> GValVec(GVal[value + 5], GVal[value + 5] + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerGainEx(4, GValVec); //设置Gain

            cl_algorithm_adjustment_setting->cl_single_algorithm_setting_3_->cl_text_algorithm_value_
                ->setText(QString::number(value));
            HomePageExtraEQValue[2] = value;
            emit HomePageEQValueChange();
        });

    // 清晰度
    QObject::connect(
        cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_->cl_algorithm_value_hSlider_,
        &QSlider::valueChanged,
        this,
        [=](int value) {
            if (value == 0) {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_
                    ->cl_del_algorithm_value_->setEnabled(false);
                emit ApoManager::instance()->requestSetExtendEqState(5, false);
            } else {
                cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_
                    ->cl_del_algorithm_value_->setEnabled(true);
                emit ApoManager::instance()->requestSetExtendEqState(5, true);
                if (value
                    == cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_
                           ->cl_algorithm_value_hSlider_->maximum()) {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_
                        ->cl_add_algorithm_value_->setEnabled(false);
                } else {
                    cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_
                        ->cl_add_algorithm_value_->setEnabled(true);
                }
            }
            double freq[10] = {270, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000};
            QVector<double> freqVec(freq, freq + 10);
            emit ApoManager::instance()
                ->requestSetExtendEqualizerCenterFrequencyEx(5, freqVec); //设置频点
            double QVal[10] = {2, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7};
            QVector<double> QValVec(QVal, QVal + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(5, QValVec); //设置Q

            double GVal[7][10] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-4, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-6, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-8, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-10, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                  {-12, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

            QVector<double> GValVec(GVal[value], GVal[value] + 10);
            emit ApoManager::instance()->requestSetExtendEqualizerGainEx(5, GValVec); //设置Gain

            cl_algorithm_adjustment_setting->cl_single_algorithm_setting_4_->cl_text_algorithm_value_
                ->setText(QString::number(value));

            HomePageExtraEQValue[3] = value;
            emit HomePageEQValueChange();
        });


}

void HomePageMainPage::LanguageSet()
{
    ui->retranslateUi(this);
    // 刷新子部件的文字（这些是在构造函数中手动 setText/setCl_text 的）
    if (cl_product_display_) {
        cl_product_display_->retranslateTexts();
    }
    if (cl_algorithm_adjustment_setting) {
        cl_algorithm_adjustment_setting->retranslateTexts();
    }
    if (cl_microphone_adjustment_) {
        cl_microphone_adjustment_->retranslateTexts();
    }
    if (cl_plans_selection_) {
        cl_plans_selection_->retranslateTexts();
    }
}

void HomePageMainPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event); // 调用基类，保持基本行为
    //子部件
    {
        // 产品展示页面
        if (cl_product_display_ != nullptr) {
        }
    }
}
