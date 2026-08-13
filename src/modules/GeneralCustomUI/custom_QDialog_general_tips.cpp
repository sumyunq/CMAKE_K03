#include "modules/GeneralCustomUI/custom_QDialog_general_tips.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QScreen>

/// \brief 构造函数
CustomQDialogGeneralTips::CustomQDialogGeneralTips(QWidget *parent, int theme)
    : QDialog(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽
}

/// \brief 初始化UI的默认信息
void CustomQDialogGeneralTips::InitUIInformation(int theme)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setFixedSize(cl_dialog_size_);

    // 主容器
    clp_container_ = new QWidget(this);
    clp_container_->setObjectName("CustomQDialogGeneralTips_container");

    // 关闭按钮
    clp_close_btn_ = new QPushButton(clp_container_);
    clp_close_btn_->setObjectName("CustomQDialogGeneralTips_close");
    clp_close_btn_->setCursor(Qt::PointingHandCursor);

    // 标题
    clp_title_label_ = new QLabel(cl_title_text_, clp_container_);
    clp_title_label_->setObjectName("CustomQDialogGeneralTips_title");
    clp_title_label_->setAlignment(Qt::AlignCenter);

    // 内容
    clp_message_label_ = new QLabel(cl_message_text_, clp_container_);
    clp_message_label_->setObjectName("CustomQDialogGeneralTips_message");
    clp_message_label_->setAlignment(Qt::AlignCenter);
    clp_message_label_->setWordWrap(true);
    clp_message_label_->hide();

    // 取消按钮
    clp_cancel_btn_ = new QPushButton(cl_cancel_text_, clp_container_);
    clp_cancel_btn_->setObjectName("CustomQDialogGeneralTips_cancel");
    clp_cancel_btn_->setCursor(Qt::PointingHandCursor);

    // 确认按钮
    clp_confirm_btn_ = new QPushButton(cl_confirm_text_, clp_container_);
    clp_confirm_btn_->setObjectName("CustomQDialogGeneralTips_confirm");
    clp_confirm_btn_->setCursor(Qt::PointingHandCursor);

    // 阴影效果 box-shadow: 0 0 10px 0
    auto *t_shadow = new QGraphicsDropShadowEffect(clp_container_);
    t_shadow->setBlurRadius(10);
    t_shadow->setColor(QColor(0, 0, 0, 80));
    t_shadow->setOffset(0, 0);
    clp_container_->setGraphicsEffect(t_shadow);

    applyTheme(theme);
    updateChildGeometry();
}

/// \brief 按主题更新样式
void CustomQDialogGeneralTips::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: { // 深色主题
        clp_container_->setStyleSheet(R"(
            QWidget#CustomQDialogGeneralTips_container {
                background-color: #10151D;
                border-radius: 12px;
            }
        )");
        clp_close_btn_->setStyleSheet(R"(
            QPushButton#CustomQDialogGeneralTips_close {
                border: none;
                border-image: url(:/Skin/Images/Popup/close-no.png);
            }
            QPushButton#CustomQDialogGeneralTips_close:hover {
                border-image: url(:/Skin/Images/Popup/close-ho.png);
            }
            QPushButton#CustomQDialogGeneralTips_close:pressed {
                border-image: url(:/Skin/Images/Popup/close-ho.png);
            }
        )");

        clp_title_label_->setStyleSheet(R"(
            QLabel#CustomQDialogGeneralTips_title {
                font-family: "Noto Sans S Chinese"; font-weight: 500;
                font-size: 16px; color: #A1A8B3; background: transparent;
            }
        )");
        clp_message_label_->setStyleSheet(R"(
            QLabel#CustomQDialogGeneralTips_message {
                font-family: "Noto Sans S Chinese"; font-weight: 500;
                font-size: 14px; color: rgba(161, 168, 179, 0.5);
                background: transparent;
            }
        )");
        clp_cancel_btn_->setStyleSheet(R"(
            QPushButton {
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#009FEF;
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/cancel_QPushBUtton_104_30_2x_normal_darkBlue.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/cancel_QPushBUtton_104_30_2x_hover_darkBlue.png);
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#009FEF;
            }
        )");
        clp_confirm_btn_->setStyleSheet(R"(
            QPushButton {
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#FFFFFF;
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_normal_darkBlue.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_hover_darkBlue.png);
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#FFFFFF;
            }
        )");
    } break;
    default: { // 预留
        clp_container_->setStyleSheet(R"(
            QWidget#CustomQDialogGeneralTips_container {
                background-color: rgba(49, 58, 72, 255);
                border-radius: 12px;
            }
        )");
        clp_close_btn_->setStyleSheet(R"(
            QPushButton#CustomQDialogGeneralTips_close {
                border: none;
                border-image: url(:/Skin/Images/Popup/close-no.png);
            }
            QPushButton#CustomQDialogGeneralTips_close:hover {
                border-image: url(:/Skin/Images/Popup/close-ho.png);
            }
            QPushButton#CustomQDialogGeneralTips_close:pressed {
                border-image: url(:/Skin/Images/Popup/close-ho.png);
            }
        )");
        clp_title_label_->setStyleSheet(R"(
            QLabel#CustomQDialogGeneralTips_title {
                font-family: "Noto Sans S Chinese"; font-weight: 500;
                font-size: 14px; color: #A1A8B3; background: transparent;
            }
        )");
        clp_message_label_->setStyleSheet(R"(
            QLabel#CustomQDialogGeneralTips_message {
                font-family: "Noto Sans S Chinese"; font-weight: 500;
                font-size: 14px; color: rgba(161, 168, 179, 0.5);
                background: transparent;
            }
        )");
        clp_cancel_btn_->setStyleSheet(R"(
            QPushButton#CustomQDialogGeneralTips_cancel {
                font-family: "Noto Sans S Chinese"; font-weight: 500;
                font-size: 12px; color: #A1A8B3;
                background-color: rgba(81, 96, 122, 200); border-radius: 4px;
            }
            QPushButton#CustomQDialogGeneralTips_cancel:hover {
                background-color: rgba(100, 115, 140, 230);
            }
        )");
        clp_confirm_btn_->setStyleSheet(R"(
            QPushButton#CustomQDialogGeneralTips_confirm {
                font-family: "Noto Sans S Chinese"; font-weight: 500;
                font-size: 12px; color: #FFFFFF;
                background-color: rgba(0, 145, 218, 200); border-radius: 4px;
            }
            QPushButton#CustomQDialogGeneralTips_confirm:hover {
                background-color: rgba(0, 160, 235, 230);
            }
        )");
    } break;
    }
}

/// \brief 初始化内部成员
void CustomQDialogGeneralTips::InitMember()
{
    // WBLIU: 预留
}

/// \brief 连接默认的信号槽
void CustomQDialogGeneralTips::InitConnect()
{
    // 关闭按钮
    connect(clp_close_btn_, &QPushButton::clicked, this, [this]() {
        emit cancelled();
        reject();
    });

    // 取消按钮
    connect(clp_cancel_btn_, &QPushButton::clicked, this, [this]() {
        emit cancelled();
        reject();
    });

    // 确认按钮
    connect(clp_confirm_btn_, &QPushButton::clicked, this, [this]() {
        emit confirmed();
        accept();
    });
}

void CustomQDialogGeneralTips::resizeEvent(QResizeEvent *event)
{
    updateChildGeometry();
    QDialog::resizeEvent(event);
}

void CustomQDialogGeneralTips::showEvent(QShowEvent *event)
{
    centerOnOwnerWindow();
    QDialog::showEvent(event);
}

void CustomQDialogGeneralTips::updateChildGeometry()
{
    clp_container_->setGeometry(QRect(QPoint(0, 0), cl_container_size_));
    clp_close_btn_->setGeometry(QRect(cl_close_btn_point_, cl_close_btn_size_));
    clp_title_label_->setGeometry(QRect(cl_title_label_point_, cl_title_label_size_));
    clp_message_label_->setGeometry(QRect(cl_message_label_point_, cl_message_label_size_));
    clp_message_label_->setVisible(cl_message_visible_ && !cl_message_text_.isEmpty());
    clp_cancel_btn_->setGeometry(QRect(cl_cancel_btn_point_, cl_cancel_btn_size_));
    clp_cancel_btn_->setVisible(cl_cancel_visible_);
    clp_confirm_btn_->setGeometry(QRect(cl_confirm_btn_point_, cl_confirm_btn_size_));
}

void CustomQDialogGeneralTips::centerOnOwnerWindow()
{
    if (QWidget *t_parent = parentWidget()) {
        const QWidget *t_owner = t_parent->window() ? t_parent->window() : t_parent;
        move(t_owner->frameGeometry().center() - rect().center());
        return;
    }

    if (QScreen *t_screen = QApplication::primaryScreen()) {
        move(t_screen->availableGeometry().center() - rect().center());
    }
}

void CustomQDialogGeneralTips::setCl_texts(const QString &title,
                                           const QString &cancel,
                                           const QString &confirm)
{
    setCl_title_text(title);
    setCl_cancel_text(cancel);
    setCl_confirm_text(confirm);
}

void CustomQDialogGeneralTips::setCl_texts(const QString &title,
                                           const QString &message,
                                           const QString &cancel,
                                           const QString &confirm)
{
    setCl_title_text(title);
    setCl_message_text(message);
    setCl_cancel_text(cancel);
    setCl_confirm_text(confirm);
}

void CustomQDialogGeneralTips::setCl_title_text(const QString &text)
{
    cl_title_text_ = text;
    clp_title_label_->setText(text);
}

void CustomQDialogGeneralTips::setCl_message_text(const QString &text)
{
    cl_message_text_ = text;
    clp_message_label_->setText(text);
    cl_message_visible_ = !text.isEmpty();
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_cancel_text(const QString &text)
{
    cl_cancel_text_ = text;
    clp_cancel_btn_->setText(text);
}

void CustomQDialogGeneralTips::setCl_confirm_text(const QString &text)
{
    cl_confirm_text_ = text;
    clp_confirm_btn_->setText(text);
}

void CustomQDialogGeneralTips::setCl_dialog_size(const QSize &size)
{
    if (!size.isValid()) {
        return;
    }

    cl_dialog_size_ = size;
    cl_container_size_ = size;
    cl_close_btn_point_ = QPoint(size.width() - cl_close_btn_size_.width() - 10, 10);
    cl_title_label_size_.setWidth(size.width());
    cl_message_label_size_.setWidth(qMax(0, size.width() - 90));
    setFixedSize(cl_dialog_size_);
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_title_geometry(const QRect &geometry)
{
    cl_title_label_point_ = geometry.topLeft();
    cl_title_label_size_ = geometry.size();
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_message_geometry(const QRect &geometry)
{
    cl_message_label_point_ = geometry.topLeft();
    cl_message_label_size_ = geometry.size();
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_close_button_geometry(const QRect &geometry)
{
    cl_close_btn_point_ = geometry.topLeft();
    cl_close_btn_size_ = geometry.size();
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_cancel_button_geometry(const QRect &geometry)
{
    cl_cancel_btn_point_ = geometry.topLeft();
    cl_cancel_btn_size_ = geometry.size();
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_confirm_button_geometry(const QRect &geometry)
{
    cl_confirm_btn_point_ = geometry.topLeft();
    cl_confirm_btn_size_ = geometry.size();
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_cancel_visible(bool visible)
{
    cl_cancel_visible_ = visible;
    updateChildGeometry();
}

void CustomQDialogGeneralTips::setCl_message_visible(bool visible)
{
    cl_message_visible_ = visible;
    updateChildGeometry();
}

QString CustomQDialogGeneralTips::cl_title_text() const
{
    return cl_title_text_;
}

QString CustomQDialogGeneralTips::cl_message_text() const
{
    return cl_message_text_;
}

QString CustomQDialogGeneralTips::cl_cancel_text() const
{
    return cl_cancel_text_;
}

QString CustomQDialogGeneralTips::cl_confirm_text() const
{
    return cl_confirm_text_;
}
