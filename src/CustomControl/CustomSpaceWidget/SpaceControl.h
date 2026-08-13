#ifndef SPACECONTROL_H
#define SPACECONTROL_H

#include <QWidget>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QList>
#include <QVector>
#include <QPointF>
#include <QStringList>

class SpaceControl : public QWidget
{
    Q_OBJECT
public:
    explicit SpaceControl(QWidget *parent = nullptr);
    ~SpaceControl();

    // 全局缩放，value: 0~100 映射到按钮尺寸 24~36
    void setGlobalScale(int value);

    // 设置模式：0/1/2 对应虚线圆距控件边缘 125/93/60
    void setMode(int mode);

    // 启用/禁用某个按钮（自动切换图标和阴影）
    void setButtonEnabled(bool enabled);

    // 获取按钮指针
    QPushButton* buttonAt(int index) const;

    void setCenterShadowEnabled(bool enabled); //独立控制中心阴影

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initButtons();
    void updateButtonPositionsByMode();  // 根据当前模式重新计算并移动按钮
    void updateButtonStyle(int index);   // 刷新按钮样式（大小、图标、圆角）
    void updateSurroundShadows();              // 刷新所有环绕按钮阴影状态

    // 环绕按钮数据（索引0~6）
    QList<QPushButton*> m_buttons;
    QList<QGraphicsDropShadowEffect*> m_shadows;
    QVector<QPointF> m_centers;          // 当前模式下的按钮中心坐标
    QVector<qreal> m_angles;             // 固定角度（弧度），以圆心(211,211)为原点
    QStringList m_disIcons;              // 禁用图标文件名
    QStringList m_noIcons;               // 启用图标文件名

    // 中心按钮（索引7）
    QPushButton *m_centerBtn = nullptr;
    QGraphicsDropShadowEffect *m_centerShadow = nullptr;

    int m_currentMode = 2;               // 当前模式，默认2
    int m_globalScaleValue = 0;          // 当前全局缩放值(0~100)
    static constexpr int CENTER_BTN_SIZE = 36; // 中心按钮固定尺寸
};

#endif // SPACECONTROL_H
