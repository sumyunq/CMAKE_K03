#include "modules/HomePage/HomePageCustomUI/custom_QPushbutton_hover_leftward.h"

CustomQPushButtonHoverLeftward::CustomQPushButtonHoverLeftward(QWidget *parent)
    : QPushButton(parent)
{
    InitUIInformation(); // 初始化UI的默认信息
    InitMember();        // 初始化内部成员
    InitConnect();       // 连接默认的信号槽
}

CustomQPushButtonHoverLeftward::~CustomQPushButtonHoverLeftward() {}

void CustomQPushButtonHoverLeftward::InitUIInformation()
{
    setMouseTracking(true); // 启用鼠标追踪
    this->setMinimumSize(cl_default_size_);



    setCursor(Qt::PointingHandCursor); /// 手型光标
}

void CustomQPushButtonHoverLeftward::InitMember()
{
    {
        cl_size_anim_ = new QPropertyAnimation(this, "size");
        cl_size_anim_->setDuration(200); // 动画时间
    }
    {
        cl_pos_anim_ = new QPropertyAnimation(this, "pos");
        cl_pos_anim_->setDuration(200);
    }

    {
        cl_anim_group_ = new QParallelAnimationGroup(this);
        cl_anim_group_->addAnimation(cl_size_anim_);
        cl_anim_group_->addAnimation(cl_pos_anim_);
    }
}

void CustomQPushButtonHoverLeftward::InitConnect() {}

void CustomQPushButtonHoverLeftward::setCl_reference_point(QPoint newCl_reference_point)
{
    cl_reference_point_ = QPoint(newCl_reference_point.x() + cl_default_size_.width(),
                                 newCl_reference_point.y()); // 记录右上角 坐标
}

void CustomQPushButtonHoverLeftward::setCl_text(const QString &newCl_text)
{
    cl_text_ = newCl_text;
}

void CustomQPushButtonHoverLeftward::setCl_default_size(const QSize &newCl_default_size)
{
    cl_default_size_ = newCl_default_size;
}

QSize CustomQPushButtonHoverLeftward::cl_default_size() const
{
    return cl_default_size_;
}

void CustomQPushButtonHoverLeftward::setCl_expand_size(const QSize &newCl_expand_size)
{
    cl_expand_size_ = newCl_expand_size;
}

void CustomQPushButtonHoverLeftward::setCl_text_rect(const QRect &newCl_text_rect)
{
    cl_text_rect_ = newCl_text_rect;
}

void CustomQPushButtonHoverLeftward::setCl_pixmap(const QPixmap &newCl_pixmap)
{
    QPixmap scaled = newCl_pixmap.scaled(17, 16,
                                     Qt::KeepAspectRatio,      // 保持宽高比
                                     Qt::SmoothTransformation); // 平滑缩放

    cl_pixmap_ = scaled;
}

void CustomQPushButtonHoverLeftward::setCl_default_pixmap(const QPixmap &newCl_default_pixmap)
{
    QPixmap scaled = newCl_default_pixmap.scaled(17, 16,
                                         Qt::KeepAspectRatio,      // 保持宽高比
                                         Qt::SmoothTransformation); // 平滑缩放
    cl_default_pixmap_ = scaled;
}

void CustomQPushButtonHoverLeftward::enterEvent(QEvent *event)
{
    if (cl_pbt_status_ == Custom_QPushButton_Status::ExpandStatus)
        return;
    // 先停止动画
    cl_anim_group_->stop();
    // 断开之前的连接（避免重复连接）
    disconnect(cl_anim_group_, &QParallelAnimationGroup::finished,
               this, nullptr);

    cl_pbt_status_ = Custom_QPushButton_Status::ExpandStatus;
    cl_size_anim_->setEndValue(cl_expand_size_);

    cl_pos_anim_->setEndValue(QPoint(
        cl_reference_point_.x() - cl_default_size_.width()
            - (cl_expand_size_.width() - cl_default_size_.width()),
        cl_reference_point_
            .y())); // 向前展开  x:参考点位置 - (cl_expand_size_.width () - cl_default_size_.width ())  y不变
    // cl_pos_anim_->setEasingCurve(QEasingCurve::OutQuad); // 缓动曲线

    cl_anim_group_->start();

}

void CustomQPushButtonHoverLeftward::leaveEvent(QEvent *event)
{
    if (cl_pbt_status_ == Custom_QPushButton_Status::ReboundStatus || cl_pbt_status_ == Custom_QPushButton_Status::DefaultStatus)
        return;
    // 先停止动画
    cl_anim_group_->stop();

    // 断开之前的连接（避免重复连接）
    disconnect(cl_anim_group_, &QParallelAnimationGroup::finished,
               this, nullptr);

    cl_pbt_status_ = Custom_QPushButton_Status::ReboundStatus;
    cl_size_anim_->setEndValue(cl_default_size_);

    cl_pos_anim_->setEndValue(QPoint(cl_reference_point_.x() - cl_default_size_.width(),
                                     cl_reference_point_.y())); // 结束位置 ==> 参考点位置
    // cl_pos_anim_->setEasingCurve(QEasingCurve::OutQuad); // 缓动曲线

    // 单次连接：动画结束后执行，执行后自动断开
    QMetaObject::Connection *conn = new QMetaObject::Connection;
    *conn = connect(cl_anim_group_, &QParallelAnimationGroup::finished, this, [this, conn]() {
        cl_pbt_status_ = Custom_QPushButton_Status::DefaultStatus;
        update();
                        // 断开连接
                        disconnect(*conn);
                        delete conn;
                    });

    cl_anim_group_->start();
}

void CustomQPushButtonHoverLeftward::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event); // 先画按钮背景和边框

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    switch (cl_pbt_status_) {
    case Custom_QPushButton_Status::DefaultStatus: {
        setStyleSheet(R"(
    QPushButton{
        border-radius: 16px;
}
)");
        // 绘制阴影
        painter.setPen (Qt::NoPen);
        painter.setBrush(QColor(230, 240, 255, 25));
        painter.drawEllipse (rect ());

        // 绘制默认图标
        if (!cl_default_pixmap_.isNull()) {
            // 绘制在指定位置
            painter.drawPixmap(rect().adjusted (8,8,-8,-8), cl_default_pixmap_);
        }

        break;
    }
    case Custom_QPushButton_Status::ExpandStatus: {

        setStyleSheet(R"(
    QPushButton{
        border-radius: 16px;
        background-color: #0091DA;
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 12px;
        color: #FFFFFF;
}
)");
        // 绘制图标
        if (!cl_pixmap_.isNull()) {
            // 绘制在指定位置
            painter.drawPixmap(cl_pixmap_rect_, cl_pixmap_);
        }

        // 绘制文字（避开图标区域）
        painter.drawText(cl_text_rect_, Qt::AlignLeft | Qt::AlignVCenter, cl_text_);
        break;
    }
    case Custom_QPushButton_Status::ReboundStatus: {

        setStyleSheet(R"(
    QPushButton{
        border-radius: 16px;
        background-color: #0091DA;
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 12px;
        color: #FFFFFF;
}
)");
        // 绘制图标
        if (!cl_pixmap_.isNull()) {
            // 绘制在指定位置
            painter.drawPixmap(cl_pixmap_rect_, cl_pixmap_);
        }

        // 绘制文字（避开图标区域）
        painter.drawText(cl_text_rect_, Qt::AlignLeft | Qt::AlignVCenter, cl_text_);
        break;
    }
    default:
        break;
    }
}