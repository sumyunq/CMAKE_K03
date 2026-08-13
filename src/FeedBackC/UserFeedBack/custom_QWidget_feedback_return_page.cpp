#include "FeedBackC/UserFeedBack/custom_QWidget_feedback_return_page.h"
#include "ui_custom_QWidget_feedback_return_page.h"

/// \brief 构造函数
CustomQWidgetFeedBackReturnPage::CustomQWidgetFeedBackReturnPage(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
    , ui(new Ui::CustomQWidgetFeedBackReturnPage)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽

    applyTheme(theme);
}

CustomQWidgetFeedBackReturnPage::~CustomQWidgetFeedBackReturnPage()
{
    delete ui;
}

/// \brief 按主题更新样式
void CustomQWidgetFeedBackReturnPage::applyTheme(int theme)
{
    cl_theme_ = theme;

    // 按主题切换按钮样式
    if (theme == 0) {
        ui->pushButton->setStyleSheet(R"(
            QPushButton#FeedBackReturn_pushButton {
                color: #FFFFFF;
                border-radius: 15px;
                text-align: center;
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_normal_darkBlue.png);
            }
            QPushButton#FeedBackReturn_pushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_hover_darkBlue.png);
            }
        )");
    } else {
        ui->pushButton->setStyleSheet(R"(
            QPushButton#FeedBackReturn_pushButton {
                color: #FFFFFF;
                border-radius: 15px;
                text-align: center;
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_normal.png);
            }
            QPushButton#FeedBackReturn_pushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_hover.png);
            }
        )");
    }
}

/// \brief 显示提交中状态
void CustomQWidgetFeedBackReturnPage::showSubmitting()
{
    cl_result_ = FeedBackResult::Submitting;
    ui->label->hide();
    ui->label_3->hide();
    ui->pushButton->hide();

    if (clp_gif_label_) {
        clp_gif_label_->movie()->start();
        clp_gif_label_->show();
    }
    ui->label_2->setStyleSheet(R"(
        QLabel#FeedBackReturn_label_2 {
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 16px;
            color: #009FEF;
            background: transparent;
        }
    )");
    ui->label_2->setText(tr("提交中"));
}

/// \brief 显示反馈成功状态
void CustomQWidgetFeedBackReturnPage::showSuccess()
{
    cl_result_ = FeedBackResult::Success;
    if (clp_gif_label_) {
        clp_gif_label_->movie()->stop();
        clp_gif_label_->hide();
    }
    ui->label->show();
    ui->label_3->show();
    ui->label->setPixmap(QPixmap(":/Skin/Images/more/contact_settings/userFeedback/feedback_submission _successful_104_104_2x_darkBlue.png"));
    ui->label_2->setStyleSheet(R"(
        QLabel#FeedBackReturn_label_2 {
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 16px;
            color: #009FEF;
            background: transparent;
        }
    )");
    ui->label_2->setText(tr("提交成功"));
    ui->label_3->setText(tr("感谢您的反馈"));
    ui->pushButton->setText(tr("再次提交"));
    ui->pushButton->show();
}

/// \brief 显示反馈失败状态
void CustomQWidgetFeedBackReturnPage::showFailure()
{
    cl_result_ = FeedBackResult::Failure;
    if (clp_gif_label_) {
        clp_gif_label_->movie()->stop();
        clp_gif_label_->hide();
    }
    ui->label->show();
    ui->label_3->show();
    ui->label->setPixmap(QPixmap(":/Skin/Images/more/contact_settings/userFeedback/feedBack_failed.png"));
    ui->label_2->setStyleSheet(R"(
        QLabel#FeedBackReturn_label_2 {
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 16px;
            color: #D44040;
            background: transparent;
        }
    )");
    ui->label_2->setText(tr("提交失败"));
    ui->label_3->setText(tr("请检查网络后重试"));
    ui->pushButton->setText(tr("重试"));
    ui->pushButton->show();
}

/// \brief 获取当前结果状态
FeedBackResult CustomQWidgetFeedBackReturnPage::cl_result() const
{
    return cl_result_;
}

/// \brief 初始化UI的默认信息
void CustomQWidgetFeedBackReturnPage::InitUIInformation(int theme)
{
    Q_UNUSED(theme)
    {
        // 图标
        ui->label->setObjectName("FeedBackReturn_label");
        ui->label->setScaledContents(true);
    }
    {
        // 标题文字
        ui->label_2->setObjectName("FeedBackReturn_label_2");
        ui->label_2->setStyleSheet(R"(
            QLabel#FeedBackReturn_label_2 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 16px;
                color: #009FEF;
                background: transparent;
            }
        )");
    }
    {
        // 副标题文字
        ui->label_3->setObjectName("FeedBackReturn_label_3");
        ui->label_3->setStyleSheet(R"(
            QLabel#FeedBackReturn_label_3 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 操作按钮
        ui->pushButton->setObjectName("FeedBackReturn_pushButton");
    }
}

/// \brief 初始化内部成员
void CustomQWidgetFeedBackReturnPage::InitMember()
{
    // GIF 动画标签
    clp_gif_label_ = new QLabel(this);
    clp_gif_label_->setFixedSize(60, 24);
    clp_gif_label_->setAlignment(Qt::AlignCenter);
    clp_gif_label_->setScaledContents(true);
    auto *t_movie = new QMovie(":/Skin/Images/more/contact_settings/userFeedback/waiting_return.gif", QByteArray(), clp_gif_label_);
    clp_gif_label_->setMovie(t_movie);
    clp_gif_label_->hide();
}

/// \brief 连接默认的信号槽
void CustomQWidgetFeedBackReturnPage::InitConnect()
{
    // 按钮点击 → 发射 actionButtonClicked 信号
    connect(ui->pushButton, &QPushButton::clicked, this, &CustomQWidgetFeedBackReturnPage::actionButtonClicked,
            Qt::UniqueConnection);
}

/// \brief 获取标题文字
QString CustomQWidgetFeedBackReturnPage::cl_title_text() const
{
    return ui->label_2->text();
}

/// \brief 设置标题文字
void CustomQWidgetFeedBackReturnPage::setCl_title_text(const QString &text)
{
    ui->label_2->setText(text);
}

/// \brief 获取副标题文字
QString CustomQWidgetFeedBackReturnPage::cl_subtitle_text() const
{
    return ui->label_3->text();
}

/// \brief 设置副标题文字
void CustomQWidgetFeedBackReturnPage::setCl_subtitle_text(const QString &text)
{
    ui->label_3->setText(text);
}

/// \brief 获取按钮文字
QString CustomQWidgetFeedBackReturnPage::cl_button_text() const
{
    return ui->pushButton->text();
}

/// \brief 设置按钮文字
void CustomQWidgetFeedBackReturnPage::setCl_button_text(const QString &text)
{
    ui->pushButton->setText(text);
}

/// \brief 获取图标
QPixmap CustomQWidgetFeedBackReturnPage::cl_icon_pixmap() const
{
    return cl_icon_pixmap_;
}

/// \brief 设置图标
void CustomQWidgetFeedBackReturnPage::setCl_icon_pixmap(const QPixmap &pixmap)
{
    cl_icon_pixmap_ = pixmap;
    ui->label->setPixmap(cl_icon_pixmap_.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

/// \brief 窗口大小变化时更新图标
void CustomQWidgetFeedBackReturnPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    clp_gif_label_->setGeometry((rect().width() - 60) / 2, 110, 60, 24);
    if (!cl_icon_pixmap_.isNull()) {
        ui->label->setPixmap(cl_icon_pixmap_.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}
