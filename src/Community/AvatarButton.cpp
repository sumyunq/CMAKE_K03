#include "Community/AvatarButton.h"
#include "qpainterpath.h"
#include <QBitmap>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

AvatarButton::AvatarButton(QWidget *parent)
    : QPushButton(parent)
{
    setStyleSheet("QPushButton { border: none; background: transparent; }");
    setCursor(Qt::PointingHandCursor);
    applySlotResources();   // 使用默认的 m_slot (LikeNo1) 初始化
    setFixedSize(42, 42);
}

AvatarButton::AvatarButton(SlotType slot, QWidget *parent)
    : QPushButton(parent), m_slot(slot)
{
    setStyleSheet("QPushButton { border: none; background: transparent; }");
    setCursor(Qt::PointingHandCursor);
    applySlotResources();
}

void AvatarButton::setSlotType(SlotType slot)
{
    if (m_slot == slot)
        return;
    m_slot = slot;
    applySlotResources();
    updateIcon();       // 边框可能变化，需要重新合成
    repositionBadge();  // 确保角标位置立即更新
}

void AvatarButton::applySlotResources()
{
    // 根据当前 m_slot 确定边框和角标图片路径
    switch (m_slot) {
    case LikeNo1:
        m_borderPath = ":/Skin/Images/Community/No1_Border.png";
        m_badgePath  = ":/Skin/Images/Community/No1_number.png";
        break;
    case LikeNo2:
        m_borderPath = ":/Skin/Images/Community/No2_Border.png";
        m_badgePath  = ":/Skin/Images/Community/No2_number.png";
        break;
    case LikeNo3:
        m_borderPath = ":/Skin/Images/Community/No3_Border.png";
        m_badgePath  = ":/Skin/Images/Community/No3_number.png";
        break;
    case DownloadNo1:
        m_borderPath = ":/Skin/Images/Community/No1_Border.png";
        m_badgePath  = ":/Skin/Images/Community/No1_number.png";
        break;
    case DownloadNo2:
        m_borderPath = ":/Skin/Images/Community/No2_Border.png";
        m_badgePath  = ":/Skin/Images/Community/No2_number.png";
        break;
    case DownloadNo3:
        m_borderPath = ":/Skin/Images/Community/No3_Border.png";
        m_badgePath  = ":/Skin/Images/Community/No3_number.png";
        break;
    }

    // 如果角标标签尚不存在则创建，否则更新图片
    if (!m_badgeLabel) {
        m_badgeLabel = new QLabel(this);
        m_badgeLabel->setFixedSize(BADGE_SIZE, BADGE_SIZE);
        m_badgeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_badgeLabel->setStyleSheet("background: transparent;");
    }

    QPixmap badgePix(m_badgePath);
    if (!badgePix.isNull()) {
        m_badgeLabel->setPixmap(badgePix.scaled(BADGE_SIZE, BADGE_SIZE,
                                                Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_badgeLabel->show();
}

void AvatarButton::setAvatar(const QString &imagePath)
{
    m_avatarPath = imagePath;
    updateIcon();
}

void AvatarButton::setAvatarPixmap(const QPixmap &pm)
{
    m_avatarPixmap = pm;
    updateIcon();
}

void AvatarButton::updateIcon()
{
    if (width() <= 0 || height() <= 0)
        return;

    QPixmap result(BUTTON_SIZE, BUTTON_SIZE);
    result.fill(Qt::transparent);

    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 1. 绘制头像圆 -> 坐标 6*6（网络头像优先，本地路径兜底）
    QPixmap avatar = m_avatarPixmap.isNull() ? QPixmap(m_avatarPath) : m_avatarPixmap;
    if (!avatar.isNull()) {
        int sq = qMin(avatar.width(), avatar.height());
        QPixmap cropped = avatar.copy((avatar.width()-sq)/2, (avatar.height()-sq)/2, sq, sq);
        QPixmap scaledAvatar = cropped.scaled(AVATAR_SIZE, AVATAR_SIZE,
                                              Qt::KeepAspectRatio, Qt::SmoothTransformation);

        int ax = 6;
        int ay = 6;

        QPainterPath clipPath;
        clipPath.addEllipse(ax, ay, AVATAR_SIZE, AVATAR_SIZE);
        p.save();
        p.setClipPath(clipPath);
        p.drawPixmap(ax, ay, scaledAvatar);
        p.restore();
    }

    // 2. 绘制边框圆环 -> 坐标 3*3
    QPixmap border(m_borderPath);
    if (!border.isNull()) {
        QPixmap scaledBorder = border.scaled(BORDER_SIZE, BORDER_SIZE,
                                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int bx = 3;
        int by = 3;
        p.drawPixmap(bx, by, scaledBorder);
    }

    p.end();
    setIcon(QIcon(result));
    setIconSize(QSize(BORDER_SIZE, BORDER_SIZE));
}

void AvatarButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    repositionBadge(); // 确保大小变化时角标重定位
}

void AvatarButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && rect().contains(event->pos()))
        emit clicked();
    QPushButton::mouseReleaseEvent(event);
}

// 数字角标位置 -> 坐标 0*0
void AvatarButton::repositionBadge()
{
    if (!m_badgeLabel)
        return;

    QWidget *parent = this->parentWidget();
    if (!parent)
        return;

    // 确保角标的父控件是按钮的父控件（允许超出按钮边界）
    if (m_badgeLabel->parentWidget() != parent) {
        m_badgeLabel->setParent(parent);
        m_badgeLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_badgeLabel->setStyleSheet("background: transparent;");
        m_badgeLabel->show();
    }

    // 数字角标的左上角坐标为 (0, 0)
    qreal badgeLocalX = 0;
    qreal badgeLocalY = 0;

    // 转换为父控件坐标
    QPoint badgePos = this->pos() + QPoint(qRound(badgeLocalX), qRound(badgeLocalY));
    m_badgeLabel->move(badgePos);
    m_badgeLabel->raise();
    m_badgeLabel->setVisible(isVisible());
}

void AvatarButton::moveEvent(QMoveEvent *event)
{
    QPushButton::moveEvent(event);
    repositionBadge();
}

void AvatarButton::showEvent(QShowEvent *event)
{
    QPushButton::showEvent(event);
    repositionBadge();
}

void AvatarButton::hideEvent(QHideEvent *event)
{
    QPushButton::hideEvent(event);
    if (m_badgeLabel)
        m_badgeLabel->hide();
}
