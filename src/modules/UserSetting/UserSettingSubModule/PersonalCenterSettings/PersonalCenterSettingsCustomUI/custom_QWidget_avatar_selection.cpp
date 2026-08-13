#include "modules/UserSetting/UserSettingSubModule/PersonalCenterSettings/PersonalCenterSettingsCustomUI/custom_QWidget_avatar_selection.h"
#include "ui_custom_QWidget_avatar_selection.h"

#include <QGridLayout>
#include <QPainterPath>

/// \brief 构造函数
CustomQWidgetAvatarSelection::CustomQWidgetAvatarSelection(QWidget *parent, int theme)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetAvatarSelection)
    , cl_theme_(theme)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化 UI 默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认信号槽
}

CustomQWidgetAvatarSelection::~CustomQWidgetAvatarSelection()
{
    delete ui;
}

/// \brief 按主题更新样式
void CustomQWidgetAvatarSelection::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
        setStyleSheet(R"(
            QWidget {
                background-color: #10151D;
                border-radius: 8px;
            }
        )");
        clp_title_label_->setStyleSheet(R"(
            QLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #A1A8B3;
                border: none;
            }
        )");
        clp_tip_text_label_->setStyleSheet(R"(
            QLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 11px;
                color: rgba(212, 64, 64, 0.7);
                border: none;
            }
        )");
    } break;
    default: {
        setStyleSheet(R"(
            QWidget {
                background-color: rgb(42, 49, 61);
                border-radius: 8px;
            }
        )");
        clp_title_label_->setStyleSheet(R"()");
        clp_tip_text_label_->setStyleSheet(R"()");
    } break;
    }
}

/// \brief 初始化 UI 默认信息
void CustomQWidgetAvatarSelection::InitUIInformation(int theme)
{
    setFixedSize(272, 344);
    setObjectName("CustomQWidgetAvatarSelection");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);  // 应用级模态
    setAttribute(Qt::WA_TranslucentBackground, false);
    // 用 mask 裁切圆角 10px
    QPainterPath t_path;
    t_path.addRoundedRect(rect(), 10, 10);
    setMask(t_path.toFillPolygon().toPolygon());

    {
        // 图标
        clp_icon_label_ = new QLabel(this);
        clp_icon_label_->setObjectName("CustomQWidgetAvatarSelection_icon");
        clp_icon_label_->setFixedSize(cl_icon_label_size_);
        clp_icon_label_->move(cl_icon_label_point_);
        clp_icon_label_->setPixmap(QPixmap(":/Skin/Images/more/personal_center_settings/edit_icon_16_16_2x.png")
                                .scaled(cl_icon_label_size_,
                                        Qt::KeepAspectRatio, // 保持宽高比
                                        Qt::SmoothTransformation));
    }
    {
        // 标题
        clp_title_label_ = new QLabel(this);
        clp_title_label_->setObjectName("CustomQWidgetAvatarSelection_title");
        clp_title_label_->setText(tr("选择头像"));
        clp_title_label_->setFixedSize(cl_title_label_size_);
        clp_title_label_->move(cl_title_label_point_);
    }
    {
        // 关闭按键
        clp_close_button_ = new QPushButton(this);
        clp_close_button_->setObjectName("CustomQWidgetAvatarSelection_close");
        clp_close_button_->setFixedSize(cl_close_button_size_);
        clp_close_button_->move(cl_close_button_point_);
        clp_close_button_->setCursor(Qt::PointingHandCursor);
        clp_close_button_->setStyleSheet(R"(
            QPushButton#CustomQWidgetAvatarSelection_close
            {
                border-radius:0px;
                border-image: url(:/Skin/Images/Popup/close-no.png);
                background:transparent;
            }
            QPushButton:hover
            {
                border-image: url(:/Skin/Images/Popup/close-ho.png);
            }
        )");
    }
    {
        // 提示图标
        clp_tip_icon_label_ = new QLabel(this);
        clp_tip_icon_label_->setObjectName("CustomQWidgetAvatarSelection_tipIcon");
        clp_tip_icon_label_->setFixedSize(cl_tip_icon_label_size_);
        clp_tip_icon_label_->move(cl_tip_icon_label_point_);
        clp_tip_icon_label_->setPixmap(QPixmap(":/Skin/Images/more/personal_center_settings/warning_icon_14_14_2x.png")
                                       .scaled(cl_tip_icon_label_size_,
                                               Qt::KeepAspectRatio, // 保持宽高比
                                               Qt::SmoothTransformation));
    }
    {
        // 提示文字
        clp_tip_text_label_ = new QLabel(this);
        clp_tip_text_label_->setObjectName("CustomQWidgetAvatarSelection_tipText");
        clp_tip_text_label_->setText(tr("使用系统头像将无法恢复登录头像"));
        clp_tip_text_label_->setFixedSize(cl_tip_text_label_size_);
        clp_tip_text_label_->move(cl_tip_text_label_point_);
    }

    // 标题栏区域 — 安装拖拽事件过滤器
    clp_icon_label_->installEventFilter(this);
    clp_title_label_->installEventFilter(this);
    installEventFilter(this);

    applyTheme(theme);
}

/// \brief 初始化内部成员
void CustomQWidgetAvatarSelection::InitMember()
{
    // 内容容器 — 4 列网格
    auto *t_container = new QWidget(this);
    t_container->setObjectName("CustomQWidgetAvatarSelection_container");
    t_container->setGeometry(0, 41, 272, 257);
    t_container->setStyleSheet(R"(
        QWidget#CustomQWidgetAvatarSelection_container {
            background-color: transparent;
        }
    )");

    auto *t_grid = new QGridLayout(t_container);
    t_grid->setContentsMargins(25, 20, 25, 15);
    t_grid->setSpacing(10);

    // 创建 15 个头像
    for (int t_i = 1; t_i <= 15; ++t_i) {  // 从 1 到 15
        auto *t_avatar = new QLabel(t_container);
        t_avatar->setObjectName(QString("CustomQWidgetAvatarSelection_avatar_%1").arg(t_i));
        t_avatar->setFixedSize(48, 48);
        t_avatar->setCursor(Qt::PointingHandCursor);

        QString imageIndex = QString("%1").arg(t_i, 2, 10, QChar('0'));  // 格式化为 01, 02, ... 15

        t_avatar->setStyleSheet(QString(R"(
        QLabel#%1 {
            border-radius: 24px;
            image: url(:/Skin/Images/system/system_avatar/system_avatar_2x_%2.png);
        }
        QLabel#%1:hover {
            border:2px solid #009FEF
        }
    )").arg(t_avatar->objectName()).arg(imageIndex));

        // border: 3px solid rgba(33, 150, 243, 150);//

        t_grid->addWidget(t_avatar, (t_i - 1) / 4, (t_i - 1) % 4);
        cl_avatar_list_.append(t_avatar);

        // 全局哈希表：序号 → 资源路径
        QString t_path = QString(":/Skin/Images/system/system_avatar/"
                                  "system_avatar_2x_%1.png").arg(imageIndex);
        cl_avatar_res_map_.insert(t_i, t_path);

        // 安装事件过滤器捕获双击
        t_avatar->installEventFilter(this);
    }
}

/// \brief 连接默认信号槽
void CustomQWidgetAvatarSelection::InitConnect()
{
    // 关闭按钮
    connect(clp_close_button_, &QPushButton::clicked, this, [this]() { close(); });
}

void CustomQWidgetAvatarSelection::resizeEvent(QResizeEvent *event)
{
    {
        clp_icon_label_->move(cl_icon_label_point_);
    }
    {
        clp_title_label_->move(cl_title_label_point_);
    }
    {
        clp_close_button_->move(cl_close_button_point_);
    }
    {
        clp_tip_icon_label_->move(cl_tip_icon_label_point_);
    }
    {
        clp_tip_text_label_->move(cl_tip_text_label_point_);
    }
    QWidget::resizeEvent(event);
}

bool CustomQWidgetAvatarSelection::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        auto *t_mouse = static_cast<QMouseEvent *>(event);
        if (t_mouse->button() == Qt::LeftButton) {
            cl_is_dragging_ = true;
            cl_drag_offset_ = t_mouse->globalPos() - frameGeometry().topLeft();
        }
    } else if (event->type() == QEvent::MouseMove) {
        auto *t_mouse = static_cast<QMouseEvent *>(event);
        if (cl_is_dragging_ && (t_mouse->buttons() & Qt::LeftButton)) {
            move(t_mouse->globalPos() - cl_drag_offset_);
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        cl_is_dragging_ = false;
    }

    // 头像双击 — 发射选中信号
    if (event->type() == QEvent::MouseButtonDblClick) {
        auto *t_mouse = static_cast<QMouseEvent *>(event);
        if (t_mouse->button() == Qt::LeftButton) {
            QLabel *t_label = qobject_cast<QLabel *>(watched);
            if (t_label) {
                for (int t_i = 0; t_i < cl_avatar_list_.size(); ++t_i) {
                    if (cl_avatar_list_[t_i] == t_label) {
                        emit avatarSelected(t_i + 1);
                        break;
                    }
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void CustomQWidgetAvatarSelection::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void CustomQWidgetAvatarSelection::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void CustomQWidgetAvatarSelection::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}
