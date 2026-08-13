#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge_container.h"
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_text_badge.h"

#include <QHBoxLayout>

CustomQWidgetTextBadgeContainer::CustomQWidgetTextBadgeContainer(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQWidgetTextBadgeContainer::~CustomQWidgetTextBadgeContainer() {}

void CustomQWidgetTextBadgeContainer::InitUIInformation(int theme)
{
    {
        setObjectName("CustomQWidgetTextBadgeContainer");

        clp_layout_ = new QHBoxLayout(this);
        clp_layout_->setContentsMargins(0, 0, 0, 0); // 默认外边距 0
        clp_layout_->setSpacing(6);                   // 默认间距 6
        clp_layout_->addStretch();                     // 右侧弹簧 ← 把徽章往左挤
    }
    applyTheme(theme);
}

void CustomQWidgetTextBadgeContainer::applyTheme(int theme)
{
    cl_theme_ = theme;
    switch (theme) {
    case 0: {
    } break;
    case 1: {
    } break;
    case 2: {
    } break;
    }
}

void CustomQWidgetTextBadgeContainer::InitMember() {}

void CustomQWidgetTextBadgeContainer::InitConnect() {}

// ── 徽章管理 ──

CustomQWidgetTextBadge *CustomQWidgetTextBadgeContainer::addBadge(const QString &text)
{
    auto *t_badge = new CustomQWidgetTextBadge(this, cl_theme_);
    t_badge->setCl_text(text);

    // 弹簧始终在最右侧 → 插入到弹簧之前（倒数第 2 个位置）
    const int t_count = clp_layout_->count();
    clp_layout_->insertWidget(t_count - 1, t_badge); // -1 = 弹簧前

    return t_badge;
}

void CustomQWidgetTextBadgeContainer::removeBadge(CustomQWidgetTextBadge *badge)
{
    if (!badge) return;
    clp_layout_->removeWidget(badge);
    badge->deleteLater();
}

void CustomQWidgetTextBadgeContainer::removeBadgeAt(int index)
{
    removeBadge(badgeAt(index));
}

void CustomQWidgetTextBadgeContainer::clearBadges()
{
    // 从后往前删除（跳过最后一个 = 弹簧）
    while (clp_layout_->count() > 1) {
        QLayoutItem *t_item = clp_layout_->takeAt(0);
        if (t_item->widget())
            t_item->widget()->deleteLater();
        delete t_item;
    }
}

int CustomQWidgetTextBadgeContainer::badgeCount() const
{
    // 弹簧占 1 个位置
    return qMax(0, clp_layout_->count() - 1);
}

CustomQWidgetTextBadge *CustomQWidgetTextBadgeContainer::badgeAt(int index) const
{
    if (index < 0 || index >= badgeCount()) return nullptr;
    return qobject_cast<CustomQWidgetTextBadge *>(
        clp_layout_->itemAt(index)->widget());
}

// ── 布局参数 ──

void CustomQWidgetTextBadgeContainer::setCl_spacing(int spacing)
{
    clp_layout_->setSpacing(spacing);
}

void CustomQWidgetTextBadgeContainer::setCl_margin(int left, int top, int right, int bottom)
{
    clp_layout_->setContentsMargins(left, top, right, bottom);
}

int CustomQWidgetTextBadgeContainer::cl_spacing() const
{
    return clp_layout_->spacing();
}
