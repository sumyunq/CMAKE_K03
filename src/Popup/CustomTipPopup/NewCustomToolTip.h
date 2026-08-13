#ifndef NEWCUSTOMTOOLTIP_H
#define NEWCUSTOMTOOLTIP_H
#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QScreen>

class NewCustomToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit NewCustomToolTip(QWidget *parent = nullptr);

    void AddToolTip(QWidget *target,const QString &text,Qt::Alignment align);

    void showToolTipBelow();

    void setLabelStyle(int idx);//设置样式(0:聊天气泡样式（带三角），1：2.4G链接时弹窗，2：方案描述的弹窗)

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;   // 新增


private:
    QVBoxLayout *layout;
    QLabel *label;
    QWidget *targetWidget;
    Qt::Alignment CurrentAlign;
    int m_currentStyle = 0;   // 默认样式为0(0:聊天气泡样式（带三角），1：2.4G链接时弹窗，2：方案描述的弹窗)

    // 箭头参数
    static constexpr int arrowHeight = 8;   // 箭头高度
    static constexpr int arrowWidth  = 12;  // 箭头底边宽度
    static constexpr int borderRadius = 4;  // 气泡圆角半径
    static constexpr int shadowBlur   = 8;  // 阴影模糊半径
    static constexpr int shadowOffsetY = 4; // 阴影垂直偏移
    static const QColor bgColor;
};
#endif
