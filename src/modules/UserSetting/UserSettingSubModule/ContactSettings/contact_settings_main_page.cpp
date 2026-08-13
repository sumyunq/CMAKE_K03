#include "modules/UserSetting/UserSettingSubModule/ContactSettings/contact_settings_main_page.h"
#include "ui_contact_settings_main_page.h"

#include <QDesktopServices>
#include <QMouseEvent>
#include <QUrl>

#include "LoadLib.h" ///< access_token

/// \brief 构造函数
ContactSettingsMainPage::ContactSettingsMainPage(QWidget *parent,
                                                 UserSettingMainPage *targetObject,
                                                 int theme)
    : QWidget(parent)
    , cl_theme_(theme)
    , ui(new Ui::ContactSettingsMainPage)
    , clp_target_user_setting_main_page_(targetObject)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽

    applyTheme(theme);
}

ContactSettingsMainPage::~ContactSettingsMainPage()
{
    delete ui;
}

/// \brief 按主题更新样式
void ContactSettingsMainPage::applyTheme(int theme)
{
    cl_theme_ = theme;
}

/// \brief 刷新翻译文本
void ContactSettingsMainPage::LanguageSet()
{
    ui->retranslateUi(this);
    // 左侧内嵌的反馈页面（构造时一次性文本，随语言切换刷新）
    if (clp_feedBackPage_) {
        clp_feedBackPage_->LanguageSet();
    }
}

void ContactSettingsMainPage::UpdateContactSettingsUIInformation()
{
    {
        if (clp_target_user_setting_main_page_ != nullptr) {
            clp_target_user_setting_main_page_->DevGetVersion();  // 主动获取耳机版本信息
            clp_target_user_setting_main_page_->SoftGetVersion(); // 主动获取驱动版本信息
        }

        clp_feedBackPage_->showOutInfo(); // 准备一下相关信息
        clp_feedBackPage_->setCl_access_token(g_user_information.network.access_token);
    }
}

void ContactSettingsMainPage::setQrCodePixmap(const QPixmap &t_pixmap)
{
    if (t_pixmap.isNull()) {
        // 无二维码（机型未提供）：隐藏二维码与"社群"标题
        ui->label_QR_code->hide();
        ui->label_4->hide();
        return;
    }

    // 有二维码：显示标题与二维码
    ui->label_QR_code->show();
    ui->label_4->show();
    ui->label_QR_code->clear();
    QPixmap t_scaled = t_pixmap.scaled(ui->label_QR_code->size(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
    ui->label_QR_code->setPixmap(t_scaled);
}

/// \brief 初始化UI的默认信息
void ContactSettingsMainPage::InitUIInformation(int theme)
{
    {
        // 左侧面板 — 用户反馈界面
        ui->widget_userFeedBack->setObjectName("ContactSettings_widget_userFeedBack");
        ui->widget_userFeedBack->setCornerRadius(12);
        ui->widget_userFeedBack->setStyleSheet(R"(
        QWidget#ContactSettings_widget_userFeedBack {
            border-radius: 12px;
            background-color: rgba(81, 96, 122, 0.2);
        }
)");
    }
    {
        // 右侧面板
        ui->widget_2->setObjectName("ContactSettings_widget_2");
        ui->widget_2->setCornerRadius(12);
        ui->widget_2->setStyleSheet(R"(
        QWidget#ContactSettings_widget_2 {
            border-radius: 12px;
            background-color: rgba(81, 96, 122, 0.2);
        }
)");
    }
    {
        // 标题 "联系我们"
        ui->label->setObjectName("ContactSettings_label");
        ui->label->setStyleSheet(R"(
            QLabel#ContactSettings_label {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 服务电话 标签
        ui->label_2->setObjectName("ContactSettings_label_2");
        ui->label_2->setStyleSheet(R"(
            QLabel#ContactSettings_label_2 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 服务电话 号码
        ui->label_6->setObjectName("ContactSettings_label_6");
        ui->label_6->setStyleSheet(R"(
            QLabel#ContactSettings_label_6 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 官网链接 标签
        ui->label_3->setObjectName("ContactSettings_label_3");
        ui->label_3->setStyleSheet(R"(
            QLabel#ContactSettings_label_3 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 官网链接 地址
        ui->label_7->setObjectName("ContactSettings_label_7");
        ui->label_7->setAttribute(Qt::WA_Hover, true); // 启用 hover 样式
        ui->label_7->setStyleSheet(R"(
            QLabel#ContactSettings_label_7 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #4E8FB5;
                background: transparent;
            }
            QLabel#ContactSettings_label_7:hover {
                color: #3F6A88;
                text-decoration: underline;
            }
        )");
        ui->label_7->setText("www.xiberia.net");
        ui->label_7->setCursor(Qt::PointingHandCursor);
    }
    {
        // 社群 标签
        ui->label_4->setObjectName("ContactSettings_label_4");
        ui->label_4->setStyleSheet(R"(
            QLabel#ContactSettings_label_4 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 二维码
        ui->label_QR_code->setObjectName("ContactSettings_label_5");
    }
}

/// \brief 初始化内部成员
void ContactSettingsMainPage::InitMember()
{
    {
        // 将用户反馈界面嵌入左侧面板
        clp_feedBackPage_ = new FeedbackMainPage(ui->widget_userFeedBack);
        ui->widget_userFeedBack->layout()->addWidget(clp_feedBackPage_);
    }
    {
        // 用户反馈回显信息 默认隐藏
        clp_feed_back_return_page = new CustomQWidgetFeedBackReturnPage(ui->widget_userFeedBack);
        clp_feed_back_return_page->hide();
    }
}

/// \brief 连接默认的信号槽
void ContactSettingsMainPage::InitConnect()
{
    // 官网链接 → 点击打开浏览器（eventFilter 捕获 MouseButtonPress）
    ui->label_7->installEventFilter(this);

    // 提交中 → 显示提交中状态
    QObject::connect(clp_feedBackPage_,
                     &FeedbackMainPage::FeedBackSubmitting,
                     clp_feed_back_return_page,
                     [this]() {
                         clp_feedBackPage_->hide();
                         ui->widget_userFeedBack->layout()->removeWidget(clp_feedBackPage_);

                         ui->widget_userFeedBack->layout()->addWidget(clp_feed_back_return_page);
                         clp_feed_back_return_page->showSubmitting();
                         clp_feed_back_return_page->show();
                     });

    // 提交成功 → 显示成功回显页
    QObject::connect(clp_feedBackPage_,
                     &FeedbackMainPage::FeedBackSubmitSucceed,
                     clp_feed_back_return_page,
                     [this]() {
                         clp_feedBackPage_->hide();
                         ui->widget_userFeedBack->layout()->removeWidget(clp_feedBackPage_);

                         ui->widget_userFeedBack->layout()->addWidget(clp_feed_back_return_page);
                         clp_feed_back_return_page->showSuccess();
                         clp_feed_back_return_page->show();
                     });

    // 提交失败 → 显示失败回显页
    QObject::connect(clp_feedBackPage_,
                     &FeedbackMainPage::FeedBackSubmitFail,
                     clp_feed_back_return_page,
                     [this]() {
                         clp_feedBackPage_->hide();
                         ui->widget_userFeedBack->layout()->removeWidget(clp_feedBackPage_);

                         ui->widget_userFeedBack->layout()->addWidget(clp_feed_back_return_page);
                         clp_feed_back_return_page->showFailure();
                         clp_feed_back_return_page->show();
                     });

    // 回显页按钮点击 → 返回反馈表单
    QObject::connect(clp_feed_back_return_page,
                     &CustomQWidgetFeedBackReturnPage::actionButtonClicked,
                     clp_feedBackPage_,
                     [this]() {
                         clp_feed_back_return_page->hide();
                         ui->widget_userFeedBack->layout()->removeWidget(clp_feed_back_return_page);

                         if (clp_feed_back_return_page->cl_result() == FeedBackResult::Success)
                             clp_feedBackPage_->resetForNewFeedback();
                         ui->widget_userFeedBack->layout()->addWidget(clp_feedBackPage_);
                         clp_feedBackPage_->show();
                     });
}

bool ContactSettingsMainPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->label_7 && event->type() == QEvent::MouseButtonPress) {
        QDesktopServices::openUrl(QUrl("https://www.xiberia.net"));
        return true;
    }
    return QWidget::eventFilter(watched, event);
}
