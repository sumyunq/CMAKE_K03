#include "modules/GeneralCustomUI/custom_QWidget_notification.h"
#include "ui_custom_QWidget_notification.h"

CustomQWidgetNotification::CustomQWidgetNotification(const QString &text,
                                                   const QString &btnText,
                                                   QWidget *parent,
                                                   int theme)
    : QWidget(parent)
    , ui(new Ui::CustomQWidgetNotification)
    , cl_theme_(theme)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽

    if (!text.isEmpty()) {
        ui->label_notification->setText(text);
    }
    if (!btnText.isEmpty()) {
        ui->pushButton_accept->setText(btnText);
    }
}

CustomQWidgetNotification::~CustomQWidgetNotification()
{
    delete ui;
}

void CustomQWidgetNotification::InitUIInformation(int theme)
{
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        setWindowModality(Qt::ApplicationModal);
        // 用 mask 裁切圆角 10px
        QPainterPath t_path;
        t_path.addRoundedRect(rect(), 16, 16);
        setMask(t_path.toFillPolygon().toPolygon());

        this->setStyleSheet(R"(
        QWidget{
            border-radius: 16px;
            background-color: #10151D;
        }
        )");

        clp_shadow_ = new QGraphicsDropShadowEffect(this);
        clp_shadow_->setBlurRadius(10);
        clp_shadow_->setXOffset(0);
        clp_shadow_->setYOffset(0);
        clp_shadow_->setColor(QColor(0, 0, 0, 128));
        setGraphicsEffect(clp_shadow_);
    }
    {
        // 提示文字
        ui->label_notification->setObjectName("CustomQWidgetNotification_label");
        ui->label_notification->setStyleSheet(R"(
            QLabel#CustomQWidgetNotification_label {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 16px;
                color: #A1A8B3;
                background: transparent;
            }
        )");
    }
    {
        // 确认按钮
        ui->pushButton_accept->setObjectName("CustomQWidgetNotification_acceptBtn");
        ui->pushButton_accept->setCursor(Qt::PointingHandCursor);
        ui->pushButton_accept->setStyleSheet(R"(
            QPushButton#CustomQWidgetNotification_acceptBtn {
                color: #FFFFFF;
                border-radius: 15px;
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_normal_darkBlue.png);
            }
            QPushButton#CustomQWidgetNotification_acceptBtn:hover {
                border-image: url(:/Skin/Images/GeneralIcon/QPushButton/blue_QPushButton_104_30_2x_hover_darkBlue.png);
            }
        )");
    }

    applyTheme(theme);
}

void CustomQWidgetNotification::InitMember() {}

void CustomQWidgetNotification::InitConnect()
{
    // 点击确认按钮 → 发射 accepted 信号
    QObject::connect(ui->pushButton_accept, &QPushButton::clicked,
                     this, &CustomQWidgetNotification::accepted);
}

void CustomQWidgetNotification::applyTheme(int theme)
{
    cl_theme_ = theme;
    // 主题样式后续按需补充
}

void CustomQWidgetNotification::setNotificationText(const QString &text)
{
    ui->label_notification->setText(text);
}

void CustomQWidgetNotification::setAcceptButtonText(const QString &text)
{
    ui->pushButton_accept->setText(text);
}

QString CustomQWidgetNotification::notificationText() const
{
    return ui->label_notification->text();
}
