#include "CustomControl/CustomSlider/GearSlider.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QPainterPath>
#include <QDebug>

#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>

GearSlider::GearSlider(QWidget *parent)
    : QSlider(parent)
    , wheelEnabled(false)
{
    setOrientation(Qt::Horizontal);
    setTickPosition(QSlider::NoTicks);
    setStyleSheet("QSlider::handle { width: 0px; height: 0px; }");

    m_blockHeight = 12;
    m_spacing = 5;
    setFixedHeight(m_blockHeight + 8);
}
/*//分两种模式，一种是0~正数。一种是负数~正数，首块左圆角，末块右圆角，中间全直角,选中无渐变
void GearSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int min = minimum();
    int max = maximum();
    bool isMixed = (min < 0 && max > 0);

    int totalBlocks;
    if (isMixed) {
        totalBlocks = -min + max;
    } else {
        totalBlocks = max - min;
    }
    if (totalBlocks <= 0) return;

    int totalSpacing;
    const int zeroGap = 16;
    if (isMixed && totalBlocks >= 2) {
        totalSpacing = (totalBlocks - 2) * m_spacing + zeroGap;
    } else {
        totalSpacing = (totalBlocks - 1) * m_spacing;
    }

    int blockWidth = (width() - totalSpacing) / totalBlocks;
    if (blockWidth <= 0) return;

    QColor litColor = isEnabled() ? QColor(0x00, 0x91, 0xDA) : QColor(0x0F, 0x67, 0x96);
    QColor darkColor(0, 0, 0, 51);
    const qreal radius = 5.0;

    int currentVal = value();
    int negCount = isMixed ? -min : 0;
    qreal negRightX = 0;
    qreal currentX = 0;

    for (int i = 0; i < totalBlocks; ++i) {
        int realVal = 0;
        bool lit = false;
        if (isMixed) {
            if (i < negCount)
                realVal = min + i;
            else
                realVal = i - negCount + 1;
        }

        if (isMixed) {
            if (currentVal > 0 && realVal > 0 && realVal <= currentVal)
                lit = true;
            else if (currentVal < 0 && realVal < 0 && realVal >= currentVal)
                lit = true;
        } else {
            if (i < currentVal - min)
                lit = true;
        }

        QRectF blockRect(currentX, (height() - m_blockHeight) / 2.0, blockWidth, m_blockHeight);

        painter.setBrush(lit ? litColor : darkColor);
        painter.setPen(Qt::NoPen);

        if (i == 0 && i == totalBlocks - 1) {
            QPainterPath path;
            path.addRoundedRect(blockRect, radius, radius);
            painter.drawPath(path);
        } else if (i == 0) {
            QPainterPath path;
            path.addRoundedRect(blockRect, radius, radius);
            QPainterPath coverRight;
            coverRight.addRect(blockRect.right() - radius, blockRect.y(),
                               radius, blockRect.height());
            path = path + coverRight;
            painter.drawPath(path);
        } else if (i == totalBlocks - 1) {
            QPainterPath path;
            path.addRoundedRect(blockRect, radius, radius);
            QPainterPath coverLeft;
            coverLeft.addRect(blockRect.x(), blockRect.y(),
                              radius, blockRect.height());
            path = path + coverLeft;
            painter.drawPath(path);
        } else {
            painter.drawRect(blockRect);
        }

        if (isMixed && i == negCount - 1)
            negRightX = blockRect.right();

        currentX += blockWidth;
        if (i < totalBlocks - 1) {
            if (isMixed && i == negCount - 1) {
                currentX += zeroGap;
            } else {
                currentX += m_spacing;
            }
        }
    }

    if (isMixed) {
        qreal zeroX = negRightX + 7.0;
        qreal zeroY = (height() - 8.0) / 2.0;
        QRectF zeroRect(zeroX, zeroY, 2.0, 8.0);

        painter.save();
        painter.setPen(Qt::NoPen);
        QColor shadowColor(0, 170, 255, 200);
        QRectF shadowRect = zeroRect.adjusted(-1.5, -1.5, 1.5, 1.5);
        painter.setBrush(shadowColor);
        painter.drawRoundedRect(shadowRect, 1.5, 1.5);

        QLinearGradient grad(zeroRect.topRight(), zeroRect.topLeft());
        grad.setColorAt(0.0, QColor(0x3c, 0xbe, 0xff));
        grad.setColorAt(1.0, QColor(0x00, 0x91, 0xda));
        painter.setBrush(grad);
        painter.drawRect(zeroRect);

        painter.restore();
    }
}*/
//分两种模式，一种是0~正数。一种是负数~正数，方块2px圆角,选中渐变、阴影
void GearSlider::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    int min = minimum();
    int max = maximum();
    bool isMixed = (min < 0 && max > 0);

    int totalBlocks;
    if (isMixed) {
        totalBlocks = -min + max;
    } else {
        totalBlocks = max - min;
    }
    if (totalBlocks <= 0) return;

    // const qreal blockHeight = isEnabled() ? m_blockHeight : 16.0;//修改高度逻辑
    const qreal blockHeight = m_blockHeight;

    const qreal shadowMargin = 6.0;

    int totalSpacing;
    const int zeroGap = 16;
    if (isMixed && totalBlocks >= 2) {
        totalSpacing = (totalBlocks - 2) * m_spacing + zeroGap;
    } else {
        totalSpacing = (totalBlocks - 1) * m_spacing;
    }

    // 动态计算方块宽度，填满控件宽度，增加了可用不可用条件判断
   qreal blockWidth;
// if (!isEnabled()) {
//     blockWidth = 82.0;
// } else {
    qreal availWidth = width() - 2 * shadowMargin - totalSpacing;
    if (availWidth <= 0) availWidth = width();
    blockWidth = availWidth / totalBlocks;
// }

    qreal currentX = shadowMargin;

    const qreal radius = 2.0;
    int currentVal = value();
    int negCount = isMixed ? -min : 0;
    qreal negRightX = 0;

    for (int i = 0; i < totalBlocks; ++i) {
        int realVal = 0;
        bool lit = false;
        if (isMixed) {
            if (i < negCount) realVal = min + i;
            else realVal = i - negCount + 1;

            if (currentVal > 0 && realVal > 0 && realVal <= currentVal) lit = true;
            else if (currentVal < 0 && realVal < 0 && realVal >= currentVal) lit = true;
        } else {
            if (i < currentVal - min) lit = true;
        }

        QRectF blockRect(currentX, (height() - blockHeight) / 2.0, blockWidth, blockHeight);

        // 外发光阴影（点亮时）：4层模拟 blur=4，修改判断条件，增加了可用不可用条件判断
        if (lit && isEnabled()) {
            painter.save();
            painter.setPen(Qt::NoPen);
            for (int s = 4; s >= 1; --s) {
                QRectF shadowRect = blockRect.adjusted(-s, -s, s, s);
                QLinearGradient grad(shadowRect.topRight(), shadowRect.topLeft());
                // s=1→15%(38), s=2→10%(26), s=3→5%(13), s=4→2%(5); 内浓外淡
                int alphas[] = {0, 38, 26, 13, 5};
                int alpha = alphas[s];
                grad.setColorAt(0.0, QColor(0x39, 0xb6, 0xf5, alpha));
                grad.setColorAt(1.0, QColor(0x00, 0x91, 0xda, alpha));
                painter.setBrush(grad);
                painter.drawRoundedRect(shadowRect, radius + s, radius + s);
            }
            painter.restore();
        }

        painter.setPen(Qt::NoPen);//增加可用不可用判断
if (lit) {
    if (isEnabled()) {
        QLinearGradient grad(blockRect.topRight(), blockRect.topLeft());
        grad.setColorAt(0.0, QColor(0x39, 0xb6, 0xf5));
        grad.setColorAt(1.0, QColor(0x00, 0x91, 0xda));
        painter.setBrush(grad);
    } else {
        QLinearGradient grad(blockRect.topRight(), blockRect.topLeft());
        grad.setColorAt(0.0, QColor(0x12, 0x8B, 0xC9));
        grad.setColorAt(1.0, QColor(0x04, 0x6F, 0xA5));
        painter.setBrush(grad);
    }
} else {
    painter.setBrush(QColor(0, 0, 0, 51));
}
        painter.drawRoundedRect(blockRect, radius, radius);

        if (isMixed && i == negCount - 1)
            negRightX = blockRect.right();

        currentX += blockWidth;
        if (i < totalBlocks - 1) {
            if (isMixed && i == negCount - 1) currentX += zeroGap;
            else currentX += m_spacing;
        }
    }

    if (isMixed) {
        qreal zeroX = negRightX + 7.0;
        qreal zeroY = (height() - 8.0) / 2.0;
        QRectF zeroRect(zeroX, zeroY, 2.0, 8.0);
        painter.save();
        painter.setPen(Qt::NoPen);
        QColor shadowColor(0, 170, 255, 200);
        QRectF shadowRect = zeroRect.adjusted(-1.5, -1.5, 1.5, 1.5);
        painter.setBrush(shadowColor);
        painter.drawRoundedRect(shadowRect, 1.5, 1.5);
        QLinearGradient grad(zeroRect.topRight(), zeroRect.topLeft());
        grad.setColorAt(0.0, QColor(0x3c, 0xbe, 0xff));
        grad.setColorAt(1.0, QColor(0x00, 0x91, 0xda));
        painter.setBrush(grad);
        painter.drawRect(zeroRect);
        painter.restore();
    }
}


int GearSlider::valueFromPos(const QPoint &pos) const
{
    int min = minimum();
    int max = maximum();
    bool isMixed = (min < 0 && max > 0);

    int totalBlocks;
    if (isMixed) {
        totalBlocks = -min + max;
    } else {
        totalBlocks = max - min;
    }
    if (totalBlocks <= 0) return min;

    const qreal shadowMargin = 6.0;
    const int   zeroGap      = 16;

    int totalSpacing;
    if (isMixed && totalBlocks >= 2) {
        totalSpacing = (totalBlocks - 2) * m_spacing + zeroGap;
    } else {
        totalSpacing = (totalBlocks - 1) * m_spacing;
    }

    qreal availWidth = width() - 2 * shadowMargin - totalSpacing;
    if (availWidth <= 0) availWidth = width();
    const qreal blockWidth = availWidth / totalBlocks;

    int negCount = isMixed ? -min : 0;
    int x = pos.x();

    double minDist = 1e9;
    int idx = 0;
    qreal cursor = shadowMargin;

    for (int i = 0; i < totalBlocks; ++i) {
        double centerX = cursor + blockWidth / 2.0;
        double dist = qAbs(x - centerX);
        if (dist < minDist) {
            minDist = dist;
            idx = i;
        }
        cursor += blockWidth;
        if (i < totalBlocks - 1) {
            if (isMixed && i == negCount - 1)
                cursor += zeroGap;
            else
                cursor += m_spacing;
        }
    }

    if (isMixed) {
        int currentVal = value();
        // 正数侧 toggle：点最右侧亮块 → 减一熄灭
        if (currentVal > 0 && idx == negCount + currentVal - 1)
            return currentVal - 1;
        // 负数侧 toggle：点最右侧亮块（即数值最大的负数块）→ 加一熄灭
        if (currentVal < 0 && idx == currentVal - min)
            return currentVal + 1;

        if (idx < negCount)
            return min + idx;
        else
            return idx - negCount + 1;
    } else {
        int currentVal = value();
        int newVal;
        if (currentVal > min && idx == currentVal - min - 1) {
            newVal = currentVal - 1;
        } else {
            newVal = min + idx + 1;
        }
        return qBound(min, newVal, max);
    }
}

void GearSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int newVal = valueFromPos(event->pos());
        qDebug("newVal:%d\n", newVal);
        setValue(newVal);
        event->accept();
    } else {
        QSlider::mousePressEvent(event);
    }
}

void GearSlider::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void GearSlider::wheelEvent(QWheelEvent *event)
{
    if (wheelEnabled) {
        int delta = event->angleDelta().y();
        if (delta > 0)
            setValue(value() + 1);
        else if (delta < 0)
            setValue(value() - 1);
        event->accept();
    } else {
        event->ignore();
    }
}

void GearSlider::focusInEvent(QFocusEvent *event)
{
    wheelEnabled = true;
    QSlider::focusInEvent(event);
}

void GearSlider::focusOutEvent(QFocusEvent *event)
{
    wheelEnabled = false;
    QSlider::focusOutEvent(event);
}
