#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_user_info_change.h"
#include "ui_custom_QWidget_user_info_change.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QTextCursor>

/// \brief 构造函数
CustomQWidgetUserInfoChange::CustomQWidgetUserInfoChange(QWidget *parent, int theme)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetUserInfoChange)
    , cl_theme_(theme)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽
}

void CustomQWidgetUserInfoChange::LanguageSet()
{
    // 代码内一次性 setText/placeholder（构造时按当时语言设置，语言切换后重设）
    if (clp_nickname_label_) clp_nickname_label_->setText(tr("昵称"));
    if (clp_signature_label_) clp_signature_label_->setText(tr("个性签名"));
    if (clp_nickname_edit_) clp_nickname_edit_->setPlaceholderText(tr("请输入昵称"));
    if (clp_signature_edit_) clp_signature_edit_->setPlaceholderText(tr("请输入个性签名"));
    if (clp_cancel_btn_) clp_cancel_btn_->setText(tr("取消"));
    if (clp_confirm_btn_) clp_confirm_btn_->setText(tr("确定"));
}

CustomQWidgetUserInfoChange::~CustomQWidgetUserInfoChange()
{
    delete ui;
}

/// \brief 初始化UI的默认信息
void CustomQWidgetUserInfoChange::InitUIInformation(int theme)
{
    {
        // 头像
        clp_avatar_label_ = new QLabel(this);
        clp_avatar_label_->setObjectName("UserInfoChange_avatar");
        clp_avatar_label_->setScaledContents(true);
        clp_avatar_label_->setCursor(Qt::PointingHandCursor);
        clp_avatar_label_->installEventFilter(this);
        clp_avatar_label_->setStyleSheet(R"(
            QLabel#UserInfoChange_avatar {
                border-radius: 40px;
                background-color: rgba(81, 96, 122, 200);
            }
            QLabel#UserInfoChange_avatar:hover {
                background-color: rgba(100, 115, 140, 230);
            }
        )");
    }
    {
        // 头像悬停遮罩+图标
        clp_avatar_hover_ = new QLabel(clp_avatar_label_);
        clp_avatar_hover_->setObjectName("UserInfoChange_avatar_hover");
        clp_avatar_hover_->setFixedSize(cl_avatar_size_);
        clp_avatar_hover_->setAlignment(Qt::AlignCenter);
        clp_avatar_hover_->setStyleSheet(R"(
            QLabel#UserInfoChange_avatar_hover {
                border-radius: 40px;
                border-image: url(:/Skin/Images/more/personal_center_settings/edit_icon_80_80_2x.png);
            }
        )");
        clp_avatar_hover_->hide();
    }
    {
        // 昵称 标签
        clp_nickname_label_ = new QLabel(this);
        clp_nickname_label_->setText(tr("昵称"));
        clp_nickname_label_->setObjectName("UserInfoChange_nickname_label");
    }
    {
        // 签名 标签
        clp_signature_label_ = new QLabel(this);
        clp_signature_label_->setText(tr("个性签名"));
        clp_signature_label_->setObjectName("UserInfoChange_signature_label");
    }
    {
        // 昵称 编辑器
        clp_nickname_edit_ = new QLineEdit(this);
        clp_nickname_edit_->setObjectName("UserInfoChange_nickname_edit");
        clp_nickname_edit_->setPlaceholderText(tr("请输入昵称"));
        clp_nickname_edit_->installEventFilter(this);
    }
    {
        // 昵称 字数
        clp_nickname_counter_ = new QLabel("0/" + QString::number(cl_nickname_max_), this);
        clp_nickname_counter_->setObjectName("UserInfoChange_nickname_counter");
        clp_nickname_counter_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    {
        // 签名 编辑器
        clp_signature_edit_ = new QPlainTextEdit(this);
        clp_signature_edit_->setObjectName("UserInfoChange_signature_edit");
        clp_signature_edit_->setPlaceholderText(tr("请输入个性签名"));
        clp_signature_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        clp_signature_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        // clp_signature_edit_->setCenterOnScroll(false); // 点击时，确保不会移动（未达到预期效果）
        clp_signature_edit_->installEventFilter(this);
    }
    {
        // 签名 字数
        clp_signature_counter_ = new QLabel("0/" + QString::number(cl_signature_max_), this);
        clp_signature_counter_->setObjectName("UserInfoChange_signature_counter");
        clp_signature_counter_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    {
        // 取消 按钮
        clp_cancel_btn_ = new QPushButton(this);
        clp_cancel_btn_->setObjectName("UserInfoChange_cancel_btn");
        clp_cancel_btn_->setCursor(Qt::PointingHandCursor);
        clp_cancel_btn_->setText(tr("取消"));
    }
    {
        // 确认 按钮
        clp_confirm_btn_ = new QPushButton(this);
        clp_confirm_btn_->setObjectName("UserInfoChange_confirm_btn");
        clp_confirm_btn_->setCursor(Qt::PointingHandCursor);
        clp_confirm_btn_->setText(tr("确定"));
    }

    applyTheme(theme);
}

/// \brief 按主题更新样式
void CustomQWidgetUserInfoChange::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
        {
            this->setStyleSheet(R"(



            )");
        }
        {
            clp_avatar_label_->setStyleSheet(R"(
                QLabel#UserInfoChange_avatar {
                    border-radius: 40px;
                    background-color: rgba(81, 96, 122, 200);
                }
                QLabel#UserInfoChange_avatar:hover {
                    background-color: rgba(100, 115, 140, 230);
                }
            )");
            clp_avatar_label_->setCursor(Qt::PointingHandCursor);
        }
        {
            clp_nickname_label_->setStyleSheet(R"(
                QLabel {
                    font-family: "Noto Sans S Chinese";
                    font-weight : 500;
                    font-size: 14px;
                    color:#A1A8B3;
                }
            )");
        }
        {
            clp_signature_label_->setStyleSheet(R"(
                QLabel {
                    font-family: "Noto Sans S Chinese";
                    font-weight : 500;
                    font-size: 14px;
                    color:#A1A8B3;
                }
            )");
        }
        {
            clp_nickname_edit_->setStyleSheet(R"(
                QLineEdit {
                    font-family: "Noto Sans S Chinese";
                    font-weight : 500;
                    font-size: 12px;
                    color:#616871;
                    background-color: rgba(0, 0, 0, 0.2);
                    border: none;
                    border-radius: 4px;
                    padding: 8px 37px 8px 10px;
                }
            )");

            QPalette palette = clp_nickname_edit_->palette();
            palette.setColor(QPalette::PlaceholderText, QColor("#616871")); // 内部提示字体
            clp_nickname_edit_->setPalette(palette);
        }
        {
            clp_nickname_counter_->setStyleSheet(R"(
                QLabel {
                    font-family: "Noto Sans S Chinese";
                    font-weight : 500;
                    font-size: 10px;
                    color:#454D57;
                }
            )");
        }
        {
            clp_signature_edit_->setStyleSheet(R"(
                QPlainTextEdit {
                    font-family: "Noto Sans S Chinese";
                    font-weight : 500;
                    font-size: 12px;
                    color:#616871;
                    background-color: rgba(0, 0, 0, 0.2);
                    border: none;
                    border-radius: 4px;
                    padding: 8px 10px 8px 10px;
                }
            )");
            clp_signature_edit_->viewport()->setAutoFillBackground(false);
            clp_signature_edit_->setAutoFillBackground(false);

            QPalette palette = clp_signature_edit_->palette();
            palette.setColor(QPalette::PlaceholderText, QColor("#616871")); // 内部提示字体
            clp_signature_edit_->setPalette(palette);
        }
        {
            clp_signature_counter_->setStyleSheet(R"(
                QLabel {
                    font-family: "Noto Sans S Chinese";
                    font-weight : 500;
                    font-size: 10px;
                    color:#454D57;
                }
            )");
        }
        {
            clp_cancel_btn_->setStyleSheet(R"(
            QPushButton {
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#009FEF;
                border-image: url(:/Skin/Images/Popup/cancel-no.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/Popup/cancel-ho.png);
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#009FEF;
            }

            )");
        }
        {
            clp_confirm_btn_->setStyleSheet(R"(
            QPushButton {
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#FFFFFF;
                border-image: url(:/Skin/Images/Popup/confirm-no.png);
            }

            QPushButton:hover {
                border-image: url(:/Skin/Images/Popup/confirm-ho.png);
                font-family: "Noto Sans S Chinese";
                font-weight : 500;
                font-size: 12px;
                color:#FFFFFF;
            }
            )");
        }
    } break;
    default: {
        clp_avatar_label_->setStyleSheet(R"(
            QLabel#UserInfoChange_avatar {
                border-radius: 40px;
                background-color: rgba(81, 96, 122, 200);
            }
        )");
        clp_nickname_label_->setStyleSheet(R"()");
        clp_signature_label_->setStyleSheet(R"()");
        clp_nickname_edit_->setStyleSheet(R"()");
        clp_nickname_counter_->setStyleSheet(R"()");
        clp_signature_edit_->setStyleSheet(R"()");
        clp_signature_counter_->setStyleSheet(R"()");
        clp_cancel_btn_->setStyleSheet(R"()");
        clp_confirm_btn_->setStyleSheet(R"()");
    } break;
    }
}

/// \brief 初始化内部成员
void CustomQWidgetUserInfoChange::InitMember()
{
    // WBLIU: 预留
}

/// \brief 连接默认的信号槽
void CustomQWidgetUserInfoChange::InitConnect()
{
    // 昵称 字数统计
    connect(clp_nickname_edit_,
            &QLineEdit::textChanged,
            this,
            &CustomQWidgetUserInfoChange::updateNicknameCounter);

    // 签名 字数统计
    connect(clp_signature_edit_,
            &QPlainTextEdit::textChanged,
            this,
            &CustomQWidgetUserInfoChange::updateSignatureCounter);

    // 取消 按钮
    connect(clp_cancel_btn_, &QPushButton::clicked, this, &CustomQWidgetUserInfoChange::cancelled);

    // 确认 按钮
    connect(clp_confirm_btn_, &QPushButton::clicked, this, &CustomQWidgetUserInfoChange::confirmed);
}

//更新昵称字数，且昵称超过最大值时，截断
void CustomQWidgetUserInfoChange::updateNicknameCounter()
{
    QString t_text = clp_nickname_edit_->text();
    // 逐字符统计非空格字符数，找到正确的截断位置
    int t_nonSpace = 0;
    int t_cutPos = t_text.length(); // 默认不截断
    for (int i = 0; i < t_text.length(); ++i) {
        if (!t_text[i].isSpace()) {
            if (t_nonSpace >= cl_nickname_max_) {
                t_cutPos = i; // 在第 i 个字符处截断（不含当前字符）
                break;
            }
            ++t_nonSpace;
        }
    }
    if (t_cutPos < t_text.length()) {
        clp_nickname_edit_->blockSignals(true);
        clp_nickname_edit_->setText(t_text.left(t_cutPos));
        clp_nickname_edit_->blockSignals(false);
        t_nonSpace = cl_nickname_max_;
    }
    clp_nickname_counter_->setText(QString("%1/%2").arg(t_nonSpace).arg(cl_nickname_max_));
}

void CustomQWidgetUserInfoChange::updateSignatureCounter()
{
    const QString t_text = clp_signature_edit_->toPlainText();
    // 逐字符统计非空格字符数，找到正确的截断位置
    int t_nonSpace = 0;
    int t_cutPos = t_text.length(); // 默认不截断
    for (int i = 0; i < t_text.length(); ++i) {
        if (!t_text[i].isSpace()) {
            if (t_nonSpace >= cl_signature_max_) {
                t_cutPos = i; // 在第 i 个字符处截断（不含当前字符）
                break;
            }
            ++t_nonSpace;
        }
    }
    if (t_cutPos < t_text.length()) {
        clp_signature_edit_->blockSignals(true);
        clp_signature_edit_->setPlainText(t_text.left(t_cutPos));
        QTextCursor t_cursor = clp_signature_edit_->textCursor();
        t_cursor.movePosition(QTextCursor::End);
        clp_signature_edit_->setTextCursor(t_cursor);
        clp_signature_edit_->blockSignals(false);
        t_nonSpace = cl_signature_max_;
    }
    clp_signature_counter_->setText(QString("%1/%2").arg(t_nonSpace).arg(cl_signature_max_));
}

void CustomQWidgetUserInfoChange::setCl_nickname(const QString &text)
{
    clp_nickname_edit_->setText(text.isEmpty() ? tr("无") : text);
}

QString CustomQWidgetUserInfoChange::cl_nickname() const
{
    return clp_nickname_edit_->text();
}

void CustomQWidgetUserInfoChange::setCl_signature(const QString &text)
{
    clp_signature_edit_->setPlainText(text.isEmpty() ? tr("未设置") : text);
}

QString CustomQWidgetUserInfoChange::cl_signature() const
{
    return clp_signature_edit_->toPlainText();
}

/// \brief 设置头像（QPainter drawEllipse 裁剪为圆形）
void CustomQWidgetUserInfoChange::setCl_avatar(const QPixmap &pixmap)
{
    QPixmap t_avatar
        = pixmap.isNull()
              ? QPixmap(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png")
              : pixmap;
    // 裁剪为圆形（QSS border-radius 不能裁剪 QLabel pixmap 内容）
    int t_d = qMin(cl_avatar_size_.width(), cl_avatar_size_.height());
    int t_src = qMin(t_avatar.width(), t_avatar.height());
    QPixmap t_square = t_avatar.copy((t_avatar.width() - t_src) / 2,
                                      (t_avatar.height() - t_src) / 2, t_src, t_src);
    t_square = t_square.scaled(t_d, t_d, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QPixmap t_circular(t_d, t_d);
    t_circular.fill(Qt::transparent);
    QPainter t_painter(&t_circular);
    t_painter.setRenderHint(QPainter::Antialiasing);
    t_painter.setBrush(t_square);
    t_painter.setPen(Qt::NoPen);
    t_painter.drawEllipse(0, 0, t_d, t_d);
    t_painter.end();

    clp_avatar_label_->setStyleSheet("");
    clp_avatar_label_->setFixedSize(t_d, t_d);
    clp_avatar_label_->setAlignment(Qt::AlignCenter);
    clp_avatar_label_->setPixmap(t_circular);
}

void CustomQWidgetUserInfoChange::resizeEvent(QResizeEvent *event)
{
    {
        // 头像
        clp_avatar_label_->setGeometry(rect().width() / 2 - cl_avatar_size_.width() / 2,
                                       cl_avatar_point_.y(),
                                       cl_avatar_size_.width(),
                                       cl_avatar_size_.height());
        // 头像遮罩
        clp_avatar_hover_->setGeometry(clp_avatar_label_->rect ());
    }
    {
        // 昵称 标签
        clp_nickname_label_->setGeometry(cl_nickname_label_point_.x(),
                                         cl_nickname_label_point_.y(),
                                         rect().width() - cl_nickname_label_point_.x() * 2,
                                         cl_nickname_label_size_.height());
    }
    {
        // 签名 标签
        clp_signature_label_->setGeometry(cl_signature_label_point_.x(),
                                          cl_signature_label_point_.y(),
                                          rect().width() - cl_signature_label_point_.x() * 2,
                                          cl_signature_label_size_.height());
    }
    {
        // 昵称 编辑器
        clp_nickname_edit_->setGeometry(QRect(cl_nickname_edit_point_,
                                              QSize(rect().width() - cl_nickname_edit_point_.x() * 2,
                                                    cl_nickname_edit_size_.height())));
    }
    {
        // 昵称 字数
        clp_nickname_counter_->setGeometry(QRect(QPoint(clp_nickname_edit_->geometry().right()
                                                            - cl_nickname_counter_size_.width() - 10,
                                                        cl_nickname_counter_point_.y()),
                                                 cl_nickname_counter_size_));
    }
    {
        // 签名 编辑器
        clp_signature_edit_->setGeometry(
            QRect(cl_signature_edit_point_,
                  QSize(rect().width() - cl_signature_edit_point_.x() * 2,
                        cl_signature_edit_size_.height())));
    }
    {
        // 签名 字数
        clp_signature_counter_->setGeometry(
            QRect(QPoint(clp_signature_edit_->geometry().right()
                             - cl_signature_counter_size_.width() - 10,
                         cl_signature_counter_point_.y()),
                  cl_signature_counter_size_));
    }
    {
        // 取消 按钮
        clp_cancel_btn_->setGeometry(cl_cancel_btn_point_.x(),
                                     rect().height() - 19 - cl_cancel_btn_size_.height(),
                                     cl_cancel_btn_size_.width(),
                                     cl_cancel_btn_size_.height());
    }
    {
        // 确认 按钮
        clp_confirm_btn_->setGeometry(rect().width() - 48 - cl_confirm_btn_size_.width(),
                                      rect().height() - 19 - cl_confirm_btn_size_.height(),
                                      cl_confirm_btn_size_.width(),
                                      cl_confirm_btn_size_.height());
    }
    QWidget::resizeEvent(event);
}

bool CustomQWidgetUserInfoChange::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *t_key_event = static_cast<QKeyEvent *>(event);
        if (t_key_event->key() == Qt::Key_Space) {
            return true; // 阻止空格输入（昵称和签名均不支持空格）
        }
    }

    // 头像悬停 → 显示遮罩
    if (watched == clp_avatar_label_ && event->type() == QEvent::Enter) {
        clp_avatar_hover_->show();
    }
    if (watched == clp_avatar_label_ && event->type() == QEvent::Leave) {
        clp_avatar_hover_->hide();
    }

    // 头像点击 → 编辑头像
    if (watched == clp_avatar_label_ && event->type() == QEvent::MouseButtonPress) {
        auto *t_mouse_event = static_cast<QMouseEvent *>(event);
        if (t_mouse_event->button() == Qt::LeftButton) {
            emit editAvatarRequested();
            return true;
        }
    }

    if (watched == clp_signature_edit_ && event->type() == QEvent::FocusOut) {
        // 失去焦点时，清除文字选中
        QTextCursor cursor = clp_signature_edit_->textCursor();
        cursor.clearSelection();
        clp_signature_edit_->setTextCursor(cursor);
    }

    if (watched == clp_signature_edit_ && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Space) {
            clp_signature_edit_->verticalScrollBar()->setValue(0);
            return true; // 阻止空格输入（昵称和签名均不支持空格）
        }
        // 记录滚动位置
        QScrollBar *vBar = clp_signature_edit_->verticalScrollBar();
        int oldV = vBar->value();
        QScrollBar *hBar = clp_signature_edit_->horizontalScrollBar();
        int oldH = hBar->value();

        // 放行事件，让 QPlainTextEdit 正常处理输入
        bool result = QWidget::eventFilter(watched, event);

        // 恢复滚动位置
        vBar->setValue(oldV);
        hBar->setValue(oldH);

        clp_signature_edit_->horizontalScrollBar()->setRange(0, 0);
        clp_signature_edit_->verticalScrollBar()->setValue(0);
        return result;
    }

    return QWidget::eventFilter(watched, event);
}
