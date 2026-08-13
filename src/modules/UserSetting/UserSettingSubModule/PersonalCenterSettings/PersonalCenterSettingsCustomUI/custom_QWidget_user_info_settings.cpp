#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_user_info_settings.h"
#include "ui_custom_QWidget_user_info_settings.h"

#include <QPainter>

#include "data/userConfig/user_config_api.h" ///< kRole*（角色映射）

CustomQWidgetUserInfoSettings::CustomQWidgetUserInfoSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetUserInfoSettings)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQWidgetUserInfoSettings::~CustomQWidgetUserInfoSettings()
{
    delete ui;
}

void CustomQWidgetUserInfoSettings::LanguageSet()
{
    // 代码内一次性 setText 的文本（构造时按当时语言设置，语言切换后重设）
    if (clp_signature_label_) clp_signature_label_->setText(tr("个性签名"));
    // 签名值二态：空 → 显示"无"占位（需跟随语言）；非空 → 用户数据不动
    if (clp_signature_text_ && !cl_signature_has_value_)
        clp_signature_text_->setText(tr("无"));
    if (clp_edit_profile_button_) clp_edit_profile_button_->setText(tr("编辑资料"));
    if (clp_logout_button_) clp_logout_button_->setText(tr("退出登录"));
}

void CustomQWidgetUserInfoSettings::InitUIInformation()
{
    {
        // ID
        clp_id_label_ = new QLabel(this);
        clp_id_label_->setText("ID:12345678");
        clp_id_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        clp_id_label_->setMinimumSize(cl_id_label_min_size_);
        clp_id_label_->move(cl_id_label_default_point_);
        clp_id_label_->setStyleSheet(R"(
        QLabel{
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 10px;
            color: #616975;
        }
)");
    }
    {
        // 头像
        clp_icon_head_portrait_ = new QLabel(this);
        clp_icon_head_portrait_->setMinimumSize(cl_icon_head_portrait_min_size_);
        clp_icon_head_portrait_->move(cl_icon_head_portrait_default_point_);
        clp_icon_head_portrait_->setStyleSheet(R"()");
    }
    {
        // 角色徽章（主播/职业，默认隐藏，尺寸与社区卡片徽章一致）
        clp_role_label_ = new QLabel(this);
        clp_role_label_->setMinimumSize(cl_role_label_min_size_);
        clp_role_label_->move(cl_role_label_default_point_);
        clp_role_label_->setAlignment(Qt::AlignCenter);
        clp_role_label_->hide();
    }
    {
        // 头衔徽章（默认隐藏）
        clp_title_label_ = new QLabel(this);
        clp_title_label_->setMinimumSize(cl_title_label_min_size_);
        clp_title_label_->move(cl_title_label_default_point_);
        clp_title_label_->setAlignment(Qt::AlignCenter);
        clp_title_label_->hide();
    }
    {
        // 等级状态
        clp_grade_status_ = new CustomQWidgetGradeStatus(this);
        clp_grade_status_->setMinimumSize(cl_grade_status_min_size_);
        clp_grade_status_->move(cl_grade_status_default_point_);
        clp_grade_status_->setStyleSheet(R"()");
    }
    {
        // 昵称
        clp_nickname_label_ = new QLabel(this);
        clp_nickname_label_->setMinimumSize(cl_nickname_label_min_size_);
        clp_nickname_label_->move(cl_nickname_label_default_point_);
        clp_nickname_label_->setAlignment(Qt::AlignCenter);
        clp_nickname_label_->setStyleSheet(R"(
        QLabel{
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 16px;
            color: #A1A8B3;
        }
)");
    }
    {
        // 个性签名
        clp_signature_label_ = new QLabel(this);
        clp_signature_label_->setText(tr("个性签名"));
        clp_signature_label_->setMinimumSize(cl_signature_label_min_size_);
        clp_signature_label_->move(cl_signature_label_default_point_);
        clp_signature_label_->setStyleSheet(R"(
        QLabel{
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 14px;
            color: #A1A8B3;
        }
)");
    }
    {
        // 个性签名（用户输入，只读）
        clp_signature_text_ = new QTextEdit(this);
        clp_signature_text_->setReadOnly(true);
        clp_signature_text_->setTextInteractionFlags(Qt::NoTextInteraction); // 文本不可选
        clp_signature_text_->setCursor(Qt::ArrowCursor);                     // 标准箭头光标
        clp_signature_text_->setMinimumSize(cl_signature_text_min_size_);
        clp_signature_text_->setText(tr("无"));
        clp_signature_text_->move(cl_signature_text_default_point_);
        clp_signature_text_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        clp_signature_text_->setStyleSheet(R"(
            QTextEdit {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: rgba(161, 168, 179, 0.5);
                background: /*red;*/transparent;
                border: none;
            }
        )");
    }
    {
        // 编辑资料
        clp_edit_profile_button_ = new QPushButton(this);
        clp_edit_profile_button_->setText(tr("编辑资料"));
        clp_edit_profile_button_->setMinimumSize(cl_edit_profile_button_min_size_);
        clp_edit_profile_button_->move(cl_edit_profile_button_default_point_);
        clp_edit_profile_button_->setCursor(Qt::PointingHandCursor);
        clp_edit_profile_button_->setStyleSheet(R"(
        QPushButton{
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 12px;
            background: transparent;
            border: none;
            color: #616871;
        }
        QPushButton:hover{
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 12px;
            background: transparent;
            border: none;
            color: #009FEF;
        }
)");
    }
    {
        // 退出登录
        clp_logout_button_ = new QPushButton(this);
        clp_logout_button_->setText(tr("退出登录"));
        clp_logout_button_->setMinimumSize(cl_logout_button_min_size_);
        clp_logout_button_->setCursor(Qt::PointingHandCursor);
        clp_logout_button_->move(cl_logout_button_default_point_);
        clp_logout_button_->setStyleSheet(R"(
        QPushButton{
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 12px;
            background: transparent;
            border: none;
            color: #616871;
        }
        QPushButton:hover{
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 12px;
            background: transparent;
            border: none;
            color: #D44040;
        }
)");
    }
}

void CustomQWidgetUserInfoSettings::InitMember() {}

void CustomQWidgetUserInfoSettings::InitConnect() {}

/// \brief 设置用户 ID
void CustomQWidgetUserInfoSettings::setId(const QString &id)
{
    clp_id_label_->setText("ID:" + id);
}

/// \brief 设置用户昵称（过长省略显示 ...）
void CustomQWidgetUserInfoSettings::setNickname(const QString &nickname)
{
    cl_nickname_text_ = nickname.isEmpty() ? tr("无") : nickname;
    // 过长省略显示（...），宽度变化时在 resizeEvent 重新截断
    clp_nickname_label_->setText(
        clp_nickname_label_->fontMetrics().elidedText(cl_nickname_text_, Qt::ElideRight,
                                                      clp_nickname_label_->width()));
}

/// \brief 设置个性签名
void CustomQWidgetUserInfoSettings::setSignature(const QString &signature)
{
    cl_signature_has_value_ = !signature.isEmpty();
    clp_signature_text_->setText(signature.isEmpty() ? tr("无") : signature);
}

/// \brief 设置头像（QPainter drawEllipse 裁剪为圆形）
void CustomQWidgetUserInfoSettings::setAvatar(const QPixmap &pixmap)
{
    QPixmap t_avatar;
    if (pixmap.isNull()) {
        t_avatar = QPixmap(":/Skin/Images/system/system_avatar/system_avatar_2x_01.png");
    } else {
        t_avatar = pixmap;
    }
    // 裁剪为圆形（QSS border-radius 不能裁剪 QLabel pixmap 内容）
    int t_d = qMin(cl_icon_head_portrait_min_size_.width(), cl_icon_head_portrait_min_size_.height());
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

    clp_icon_head_portrait_->setStyleSheet("");
    clp_icon_head_portrait_->setFixedSize(t_d, t_d);
    clp_icon_head_portrait_->setAlignment(Qt::AlignCenter);
    clp_icon_head_portrait_->setPixmap(t_circular);
}

/// \brief 设置用户角色 — 与社区卡徽章一致：streamer→主播 / professional→职业
void CustomQWidgetUserInfoSettings::setRoles(const QStringList &roles)
{
    // 表驱动：角色 → {尺寸, 背景图}（背景图自带文案，与社区卡徽章一致）
    struct RoleBadge {
        const char *role;   ///< 角色值（kRole*）
        QSize size;         ///< 徽章尺寸
        const char *image;  ///< 背景图资源
    };
    //角色：主播，大神，官方
    static const RoleBadge kRoleBadges[] = {
        {DeSheng::kRoleStreamer, QSize(46, 19), ":/Skin/Images/modules/community/host.png"},
        {DeSheng::kRoleOfficial, QSize(56, 18), ":/Skin/Images/modules/community/official.png"},
        {DeSheng::kRoleProfessional, QSize(50, 18), ":/Skin/Images/modules/community/professional.png"},
    };

    for (const auto &t_badge : kRoleBadges) {
        if (!roles.contains(QLatin1String(t_badge.role))) continue;
        clp_role_label_->setFixedSize(t_badge.size.width(), t_badge.size.height());
        clp_role_label_->setStyleSheet(QStringLiteral(
            "QLabel{font-family:\"Noto Sans S Chinese\";font-weight:500;font-size:10px;"
            "color:#ffffff;border-image:url(%1);}").arg(t_badge.image));
        clp_role_label_->show();
        return;
    }
    clp_role_label_->hide();
}

/// \brief 设置用户头衔 — 与角色徽章一致，目前仅支持大神（expert，徽章与社区卡一致）
void CustomQWidgetUserInfoSettings::setTitles(const QStringList &titles)
{
    if (!titles.contains(DeSheng::kTitleExpert)) {
        clp_title_label_->hide();
        return;
    }
    clp_title_label_->setFixedSize(53, 20); // 大神徽章，尺寸与社区卡一致
    clp_title_label_->setStyleSheet(QStringLiteral(
        "QLabel{font-family:\"Noto Sans S Chinese\";font-weight:500;font-size:10px;"
        "color:#ffffff;border-image:url(:/Skin/Images/modules/community/god.png);}"));
    clp_title_label_->show();
}

/// \brief 设置等级与经验进度
void CustomQWidgetUserInfoSettings::setGrade(int level, int currentXp, int requiredXp)
{
    clp_grade_status_->setCl_grade_level(level);
    clp_grade_status_->setCl_progress(currentXp, requiredXp);
    clp_grade_status_->setCl_empirical_value(currentXp, requiredXp);
}

void CustomQWidgetUserInfoSettings::resizeEvent(QResizeEvent *event)
{
    {
        clp_id_label_->setGeometry(rect().width() - 24 - clp_id_label_->width(),
                                   23,
                                   clp_id_label_->width(),
                                   cl_id_label_current_size_.height());
    }
    {
        clp_icon_head_portrait_
            ->setGeometry(rect().width() / 2 - clp_icon_head_portrait_->width() / 2, 81, 80, 80);
    }
    {
        // 昵称：左右边框各保持 46px 间距，过长省略号显示
        clp_nickname_label_->setGeometry(46,
                                         cl_nickname_label_current_point_.y(),
                                         rect().width() - 46 * 2,
                                         cl_nickname_label_current_size_.height());
        clp_nickname_label_->setText(
            clp_nickname_label_->fontMetrics().elidedText(cl_nickname_text_, Qt::ElideRight,
                                                          clp_nickname_label_->width()));
    }
    {
        // 与头像一致：x 随面板宽度等比变化（设计宽度 332 时 x=116，保持 116/332 比例）
        const int t_role_x = qRound(rect().width() * cl_role_label_default_point_.x() / 332.0);
        clp_role_label_->setGeometry(t_role_x,
                                     cl_role_label_current_point_.y(),
                                     clp_role_label_->width(),
                                     clp_role_label_->height());
    }
    {
        // 与头像一致：x 随面板宽度等比变化（设计宽度 332 时 x=168，保持 168/332 比例）
        const int t_title_x = qRound(rect().width() * cl_title_label_default_point_.x() / 332.0);
        clp_title_label_->setGeometry(t_title_x,
                                      cl_title_label_current_point_.y(),
                                      clp_title_label_->width(),
                                      clp_title_label_->height());
    }
    {
        clp_grade_status_->setGeometry(cl_grade_status_current_point_.x(),
                                       cl_grade_status_current_point_.y(),
                                       rect().width(),
                                       cl_grade_status_current_size_.height());
    }
    {
        clp_signature_label_->setGeometry(cl_signature_label_current_point_.x(),
                                          cl_signature_label_current_point_.y(),
                                          cl_signature_label_current_size_.width(),
                                          cl_signature_label_current_size_.height());
    }
    {
        clp_signature_text_->setGeometry(cl_signature_text_current_point_.x(),
                                         cl_signature_text_current_point_.y(),
                                         rect().width() - cl_signature_text_current_point_.x() - 39,
                                         clp_signature_text_->height());
    }
    {
        clp_edit_profile_button_->setGeometry(cl_edit_profile_button_current_point_.x(),
                                              rect().height() - 26
                                                  - cl_edit_profile_button_current_size_.height(),
                                              cl_edit_profile_button_current_size_.width(),
                                              cl_edit_profile_button_current_size_.height());
    }
    {
        clp_logout_button_->setGeometry(rect().width() - 76 - cl_logout_button_current_size_.width(),
                                        rect().height() - 26
                                            - cl_logout_button_current_size_.height(),
                                        cl_logout_button_current_size_.width(),
                                        cl_logout_button_current_size_.height());
    }
}