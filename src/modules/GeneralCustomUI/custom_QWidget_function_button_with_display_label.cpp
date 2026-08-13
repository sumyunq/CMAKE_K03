#include "modules/GeneralCustomUI/custom_QWidget_function_button_with_display_label.h"
#include "modules/GeneralCustomUI/custom_QWidget_download_progress_ring.h"
#include "ui_custom_QWidget_function_button_with_display_label.h"

#include <QDebug>
#include <QStackedWidget>

namespace {

QString formatActionCount(int count)
{
    if (count < 10000)
        return QString::number(count);
    return QString::asprintf("%.1fw", count / 10000.0);
}

} // namespace

CustomQWidgetFunctionButtonWithDisplayLabel::CustomQWidgetFunctionButtonWithDisplayLabel(
    QWidget *parent, ButtonFuctionType type, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
    , ui(new Ui::CustomQWidgetFunctionButtonWithDisplayLabel)
    , cl_button_type_(type)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽
}

CustomQWidgetFunctionButtonWithDisplayLabel::~CustomQWidgetFunctionButtonWithDisplayLabel()
{
    delete ui;
}

void CustomQWidgetFunctionButtonWithDisplayLabel::InitUIInformation(int theme)
{
    {
        applyStyleForType();
        setMinimumSize(cl_min_rect_.size());
        ui->label->setText(formatActionCount(cl_count_));
        ui->label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->label->setStyleSheet(R"(
            QLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 400;
                font-size: 11px;
                color: #8A94A6;
                background: transparent;
            }
        )");

        ui->pushButton->setCheckable(true);
        ui->pushButton->setCursor(Qt::PointingHandCursor); //手型
    }
    {
        // 下载进度圆环 → 与 pushButton 一起放入 QStackedWidget
        clp_progress_ring_ = new CustomQWidgetDownloadProgressRing();
        clp_progress_ring_->setObjectName("CustomQWidgetFunctionButtonWithDisplayLabel_progressRing");

        clp_stack_ = new QStackedWidget(this);
        clp_stack_->setFixedSize(14, 14);
        ui->gridLayout->removeWidget(ui->pushButton);       ///< 从布局中取出
        clp_stack_->addWidget(ui->pushButton);              ///< index 0 — 正常图标
        clp_stack_->addWidget(clp_progress_ring_);          ///< index 1 — 下载进度圆环
        clp_stack_->setCurrentIndex(0);                     ///< 默认显示图标
        ui->gridLayout->addWidget(clp_stack_, 0, 0, Qt::AlignCenter); ///< 放回布局原位置
    }

    applyTheme(theme);
}

void CustomQWidgetFunctionButtonWithDisplayLabel::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    }
}

void CustomQWidgetFunctionButtonWithDisplayLabel::InitMember() {}

void CustomQWidgetFunctionButtonWithDisplayLabel::InitConnect()
{
    connect(ui->pushButton,
            &QPushButton::clicked,
            this,
            &CustomQWidgetFunctionButtonWithDisplayLabel::onInternalButtonClicked);
}

void CustomQWidgetFunctionButtonWithDisplayLabel::onInternalButtonClicked()
{
    switch (cl_button_type_) {
    case ButtonFuctionType::Like: {
        if (ui->pushButton->isChecked())
            emit liked();   ///< 选中 → 点赞
        else
            emit unliked(); ///< 取消选中 → 取消点赞
    } break;
    case ButtonFuctionType::Dislike: {
        if (ui->pushButton->isChecked())
            emit disliked();   ///< 选中 → 踩
        else
            emit undisliked(); ///< 取消选中 → 取消踩
    } break;
    case ButtonFuctionType::Download:
        // 状态由 doDownload() 全权管理，这里只发信号
        emit download();
        break;
    case ButtonFuctionType::Share:
        emit share();
        break;
    default:
        break; ///< 预留类型暂不发射
    }
}

void CustomQWidgetFunctionButtonWithDisplayLabel::applyStyleForType()
{
    switch (cl_button_type_) {
    case ButtonFuctionType::Like:
        ui->pushButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                border-radius: 0px;
                border-image: url(:/Skin/Images/GeneralIcon/liked_1x_normal_darkBlue.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/liked_1x_hover_darkBlue.png);
            }

            QPushButton:checked {
                border-image: url(:/Skin/Images/GeneralIcon/liked_1x_checked_darkBlue.png);
            }
        )");
        break;
    case ButtonFuctionType::Dislike:
        ui->pushButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                border-radius: 0px;
                border-image: url(:/Skin/Images/GeneralIcon/disliked_1x_normal_darkBlue.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/disliked_1x_hover_darkBlue.png);
            }

            QPushButton:checked {
                border-image: url(:/Skin/Images/GeneralIcon/disliked_1x_checked_darkBlue.png);
            }
        )");
        break;
    case ButtonFuctionType::Download:
        ui->pushButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                border-radius: 0px;
                border-image: url(:/Skin/Images/GeneralIcon/download_1x_normal_darkBlue.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/download_1x_hover_darkBlue.png);
            }
        )");
        break;
    case ButtonFuctionType::Share:
        ui->pushButton->setStyleSheet(R"(
            QPushButton {
                border: none;
                border-radius: 0px;
                border-image: url(:/Skin/Images/GeneralIcon/share_1x_normal_darkBlue.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/share_1x_hover_darkBlue.png);
            }
        )");
        break;
    default:
        break; ///< 预留类型保持当前样式
    }
}

ButtonFuctionType CustomQWidgetFunctionButtonWithDisplayLabel::cl_button_type() const
{
    return cl_button_type_;
}

void CustomQWidgetFunctionButtonWithDisplayLabel::setCl_button_type(ButtonFuctionType type)
{
    if (cl_button_type_ == type)
        return;
    cl_button_type_ = type;
    applyStyleForType();
}

int CustomQWidgetFunctionButtonWithDisplayLabel::cl_count() const
{
    return cl_count_;
}

void CustomQWidgetFunctionButtonWithDisplayLabel::setCl_count(int count)
{
    cl_count_ = count;
    ui->label->setText(formatActionCount(cl_count_));
}

void CustomQWidgetFunctionButtonWithDisplayLabel::setChecked(bool checked)
{
    if (ui->pushButton->isChecked() == checked)
        return; ///< 同值不重复设置，避免无效操作
    ui->pushButton->setChecked(checked);
}

void CustomQWidgetFunctionButtonWithDisplayLabel::setDownloadProgress(int percent)
{
    if (clp_progress_ring_) {
        clp_progress_ring_->setCl_progress(percent);
    }
}

void CustomQWidgetFunctionButtonWithDisplayLabel::setDownloadState(DownloadState state)
{
    if (cl_download_state_ == state)
        return;

    // 防御：确保 QStackedWidget 至少有 2 个 widget（pushButton + 进度环）
    if (!clp_stack_ || clp_stack_->count() < 2) {
        qWarning() << "CustomQWidgetFunctionButtonWithDisplayLabel::setDownloadState:"
                    << "clp_stack_ is null or has < 2 widgets, count ="
                    << (clp_stack_ ? clp_stack_->count() : -1);
        return;
    }

    cl_download_state_ = state;

    switch (state) {
    case DownloadState::Normal: {
        clp_stack_->setCurrentIndex(0); ///< 显示 pushButton
    } break;
    case DownloadState::Downloading: {
        clp_progress_ring_->reset();
        clp_stack_->setCurrentIndex(1); ///< 显示进度圆环
    } break;
    case DownloadState::Done: {
        clp_stack_->setCurrentIndex(0); ///< 恢复显示 pushButton
    } break;
    }
}

DownloadState CustomQWidgetFunctionButtonWithDisplayLabel::downloadState() const
{
    return cl_download_state_;
}
