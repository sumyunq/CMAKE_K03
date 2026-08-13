#ifndef FILTERTOOLBUTTON_H
#define FILTERTOOLBUTTON_H

#include <QToolButton>

class FilterToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit FilterToolButton(QWidget *parent = nullptr);

    // 设置间距的
    void setIconLeftMargin(int margin) { m_iconLeftMargin = margin; update(); }
    void setTextSpacing(int spacing) { m_textSpacing = spacing; update(); }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_iconLeftMargin = 4;   // 图标距左
    int m_textSpacing = 16;     // 图标与文本间距
};

#endif // FILTERTOOLBUTTON_H
