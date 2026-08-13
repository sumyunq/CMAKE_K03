#include "CustomControl/CustomScrollArea/SnapScrollArea.h"

SnapScrollArea::SnapScrollArea(QWidget *parent)
    : QScrollArea(parent), m_rowHeight(78)
{
    // 隐藏水平滚动条，只保留竖直滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 关键：限制垂直滚动条的范围为 0 ~ 1
    verticalScrollBar()->setRange(0, 1);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setPageStep(1);
}
void SnapScrollArea::scrollContentsBy(int dx, int dy)
{
    int value = verticalScrollBar()->value();
    // 吸附：value 只能是 0 或 m_rowHeight
    int target = qRound(static_cast<double>(value) / m_rowHeight) * m_rowHeight;
    if (target != value) {
        verticalScrollBar()->setValue(target);
        return; // 避免重复刷新
    }
    QScrollArea::scrollContentsBy(dx, target - value);
}
