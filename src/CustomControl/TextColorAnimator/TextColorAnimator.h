#ifndef TEXTCOLORANIMATOR_H
#define TEXTCOLORANIMATOR_H

#include <QObject>
#include <QComboBox>

class TextColorAnimator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
public:
    explicit TextColorAnimator(QComboBox *combo, QObject *parent = nullptr)
        : QObject(parent), m_combo(combo)
    {
        // 保存基础样式表（不含颜色）
        m_baseStyleSheet = "QComboBox { border: none; }"   // 你可以根据需要保留边框等
                           "QComboBox::drop-down { height: 0px; width: 0px; }";
        // 初始化颜色
        m_color = QColor("#c0c3c8");
    }

    QColor textColor() const { return m_color; }
    void setTextColor(const QColor &color) {
        if (m_color == color) return;
        m_color = color;
        QString style = m_baseStyleSheet + QString("QComboBox { color: %1; }").arg(color.name());
        m_combo->setStyleSheet(style);
    }

private:
    QComboBox *m_combo;
    QColor m_color;
    QString m_baseStyleSheet;
};

#endif // TEXTCOLORANIMATOR_H
