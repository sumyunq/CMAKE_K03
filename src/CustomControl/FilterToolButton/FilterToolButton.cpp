#include "CustomControl/FilterToolButton/FilterToolButton.h"
#include "qgraphicseffect.h"
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QStyle>


FilterToolButton::FilterToolButton(QWidget *parent)
    : QToolButton(parent)
{
}

void FilterToolButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOptionToolButton option;
    initStyleOption(&option);

    QString origText = option.text;
    QIcon origIcon = option.icon;
    option.text = QString();
    option.icon = QIcon();

    // 绘制背景（所有伪状态由样式表控制），不画图标和文字
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &option, &painter, this);
    // === 从调色板/样式表读取当前状态下的文字颜色 ===
    QColor textColor = palette().color(QPalette::ButtonText);
    if (isChecked()) {
        textColor = QColor("#FFFFFF");
    } else {
        textColor = QColor("#A1A8B3");
    }

    option.text = origText;
    option.icon = origIcon;



    // 如果图标为空，只绘制文本（并提前返回，避免空指针）
    if (icon().isNull()) {
        painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, text());
        return;
    }

    // 只画一次
    //获取图标实际尺寸（兼容 24x13、24x15 等非正方形）
    QList<QSize> sizes = icon().availableSizes();
    QSize iconSize;
    if (!sizes.isEmpty()) {
        iconSize = sizes.first();
    } else {
        // 备选方案：从 pixmap 获取
        QPixmap pix = icon().pixmap(QSize(24, 24));
        iconSize = pix.size();
        if (iconSize.isEmpty()) iconSize = QSize(24, 16); // 保底默认值
    }

    const int leftMargin = m_iconLeftMargin;   // 图标距左
    const int spacing = m_textSpacing;     // 图标与文字间距
    QRect rect = this->rect();

    //绘制图标（垂直居中，保持原始宽高）
    int iconX = leftMargin;
    int iconY = (rect.height() - iconSize.height()) / 2;
    QRect iconRect(iconX, iconY, iconSize.width(), iconSize.height());
    painter.drawPixmap(iconRect, icon().pixmap(iconSize));

    //绘制文本（在图标右侧，垂直居中）
    int textX = leftMargin + iconSize.width() + spacing;
    QRect textRect(textX, 0, rect.width() - textX - 2, rect.height());
    painter.setPen(textColor);



    QFont labelFont;
    labelFont.setFamily("Noto Sans S Chinese");   // 字体族名称
    labelFont.setWeight(QFont::Medium);
    labelFont.setPixelSize(10);


    painter.setFont(labelFont);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
}
