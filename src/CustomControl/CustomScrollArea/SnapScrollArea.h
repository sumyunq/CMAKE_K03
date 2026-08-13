#ifndef SNAPSCROLLAREA_H
#define SNAPSCROLLAREA_H

#include <QObject>
#include <QScrollArea>
#include <QScrollBar>

class SnapScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit SnapScrollArea(QWidget *parent = nullptr);

protected:
    // 拦截滚动事件，确保位置吸附到整数倍行高
    void scrollContentsBy(int dx, int dy) override;

private:
    int m_rowHeight;
};

#endif // SNAPSCROLLAREA_H
