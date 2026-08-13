#ifndef AUTORESIZELABEL_H
#define AUTORESIZELABEL_H

#include <QLabel>
#include <QObject>
//自定义QLabel，每次修改文本，就重新计算控件宽度
class AutoResizeLabel : public QLabel {
public:
    explicit AutoResizeLabel(QWidget* parent = nullptr) : QLabel(parent) {
        setMaximumHeight(16);
        setMinimumHeight(16);
        setAlignment(Qt::AlignCenter);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setContentsMargins(8, 0, 8, 0);  // 左右各8px
    }

    void setText(const QString& text) {
        QLabel::setText(text);
        adjustWidth();
    }

private:
    void adjustWidth() {
        // 用 fontMetrics 计算文本宽度，加上左右边距
        int textWidth = fontMetrics().horizontalAdvance(text());
        setFixedWidth(textWidth + contentsMargins().left() + contentsMargins().right());
    }
};

#endif // AUTORESIZELABEL_H
