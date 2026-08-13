#include "CustomControl/CustomSpaceWidget/SpaceControl.h"
#include <QPainter>
#include <QPen>
#include <QtMath>

SpaceControl::SpaceControl(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(422, 422);
    initButtons();
}

SpaceControl::~SpaceControl()
{
}

void SpaceControl::initButtons()
{
    // 7个环绕按钮在模式2下的参考中心点（用于计算固定角度）
    const QPointF centersMode2[7] = {
        QPointF(211, 60),    // 上
        QPointF(104, 104),   // 左上
        QPointF(60, 211),    // 左中
        QPointF(104, 318),   // 左下
        QPointF(318, 318),   // 右下
        QPointF(362, 211),   // 右中
        QPointF(318, 104)    // 右上
    };

    const QPointF controlCenter(211, 211);
    for (int i = 0; i < 7; ++i) {
        // 计算固定角度
        qreal dx = centersMode2[i].x() - controlCenter.x();
        qreal dy = centersMode2[i].y() - controlCenter.y();
        m_angles.append(qAtan2(dy, dx));

        // 创建按钮（初始大小24）
        QPushButton *btn = new QPushButton(this);
        btn->setFixedSize(24, 24);
        btn->setEnabled(false);

        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(30);
        shadow->setOffset(0, 0);
        shadow->setColor(QColor("#00AAFF"));
        shadow->setEnabled(false);
        btn->setGraphicsEffect(shadow);

        m_buttons.append(btn);
        m_shadows.append(shadow);
    }

    // 图标文件名（按方位顺序）
    m_disIcons << "front-dis.png" << "LeftTop-dis.png" << "Left-dis.png"
               << "LeftBottom-dis.png" << "RightBottom-dis.png"
               << "Right-dis.png" << "RightTop-dis.png";
    m_noIcons  << "front-no.png"  << "LeftTop-no.png"  << "Left-no.png"
              << "LeftBottom-no.png" << "RightBottom-no.png"
              << "Right-no.png"  << "RightTop-no.png";

    // 中心按钮（固定大小，居中）
    m_centerBtn = new QPushButton(this);
    m_centerBtn->setFixedSize(CENTER_BTN_SIZE, CENTER_BTN_SIZE);
    // 中心点定位：控件中心(211,211) → 左上角(193,193)
    m_centerBtn->move(211 - CENTER_BTN_SIZE/2, 211 - CENTER_BTN_SIZE/2);
    m_centerBtn->setEnabled(false);
    m_centerBtn->setStyleSheet(
        "QPushButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: " + QString::number(CENTER_BTN_SIZE/2) + "px;"
                                                 "  border-image: url(:/Skin/Images/Headphones/Space/space.png);"
                                                 "}"
        );

    m_centerShadow = new QGraphicsDropShadowEffect(this);
    m_centerShadow->setBlurRadius(20);           // 对应 20px 模糊
    m_centerShadow->setOffset(0, 0);             // 偏移 0 0
    m_centerShadow->setColor(QColor("#DCDFE4")); // 颜色 #DCDFE4
    m_centerShadow->setEnabled(false);           // 初始禁用
    m_centerBtn->setGraphicsEffect(m_centerShadow);  // 安装效果到按钮

    // 将中心按钮加入列表（索引7）
    m_buttons.append(m_centerBtn);
    m_shadows.append(m_centerShadow);

    // 应用默认模式（2）的位置
    updateButtonPositionsByMode();
    // 应用初始全局缩放（大小24）
    setGlobalScale(0);
}

void SpaceControl::updateButtonPositionsByMode()
{
    int margin;
    switch (m_currentMode) {
    case 0: margin = 125; break;
    case 1: margin = 93;  break;
    case 2:
    default: margin = 60;  break;
    }

    int diameter = width() - 2 * margin;  // 422 - 2*margin
    int radius = diameter / 2;
    QPointF center(width() / 2.0, height() / 2.0);

    for (int i = 0; i < 7; ++i) {
        qreal angle = m_angles.at(i);
        QPointF newCenter(center.x() + radius * qCos(angle),
                          center.y() + radius * qSin(angle));
        m_centers.append(newCenter);  // 首次调用已预留空间，不会重复追加

        QPushButton *btn = m_buttons.at(i);
        int btnSize = btn->width();
        btn->move(newCenter.x() - btnSize/2, newCenter.y() - btnSize/2);
    }
    update(); // 重绘虚线圆
}

void SpaceControl::updateButtonStyle(int index)
{
    if (index < 0 || index >= m_buttons.size())
        return;

    QPushButton *btn = m_buttons.at(index);
    int size = btn->width(); // 按钮为正方形，宽高相等
    QString iconName;
    const QString basePath = ":/Skin/Images/Headphones/Space/";

    if (index < 7) {
        bool enabled = btn->isEnabled();
        iconName = enabled ? m_noIcons.at(index) : m_disIcons.at(index);
    } else {
        iconName = "space.png"; // 中心按钮固定图标
    }

    btn->setStyleSheet(
        QString(
            "QPushButton {"
            "  background-color: transparent;"
            "  border: none;"
            "  border-image: url(%2%3);"
            "}"
            ).arg(basePath, iconName)
        );
}
void SpaceControl::updateSurroundShadows()
{
    for (int i = 0; i < 7; ++i) {
        QPushButton *btn = m_buttons.at(i);
        QGraphicsDropShadowEffect *shadow = m_shadows.at(i);
        // 只有全局缩放值不为0 且 按钮启用 时才显示阴影
        bool shouldEnable = (m_globalScaleValue != 0) && btn->isEnabled();
        shadow->setEnabled(shouldEnable);
    }
}
void SpaceControl::setGlobalScale(int value)
{
    value = qBound(0, value, 100);
    m_globalScaleValue = value;

    // 线性映射：24 ~ 36
    int size = 24 + value * (36 - 24) / 100;
    size = qBound(24, size, 36);

    for (int i = 0; i < 7; ++i) {
        QPushButton *btn = m_buttons.at(i);
        QPointF center = m_centers.at(i);

        btn->setFixedSize(size, size);
        btn->move(center.x() - size/2, center.y() - size/2);
        updateButtonStyle(i); // 刷新图标
    }
    // 根据新缩放值刷新所有环绕阴影状态
    updateSurroundShadows();
}

void SpaceControl::setMode(int mode)
{
    if (mode < 0 || mode > 2 || m_currentMode == mode)
        return;

    m_currentMode = mode;
    m_centers.clear(); // 清空旧中心点，updateButtonPositionsByMode 会重新填充
    updateButtonPositionsByMode();

    // 重新应用全局缩放，确保大小和位置一致
    setGlobalScale(m_globalScaleValue);
    update();
}

void SpaceControl::setButtonEnabled(bool enabled)
{
    // 只操作环绕按钮（索引 0~6）
    for (int i = 0; i < 7; ++i) {
        QPushButton *btn = m_buttons.at(i);
        btn->setEnabled(enabled);
        updateButtonStyle(i);   // 切换 -no / -dis 图标
    }
    // 根据新的启用状态刷新阴影
    updateSurroundShadows();
}

QPushButton* SpaceControl::buttonAt(int index) const
{
    if (index < 0 || index >= m_buttons.size())
        return nullptr;
    return m_buttons.at(index);
}

void SpaceControl::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 根据当前模式计算虚线圆半径
    int margin;
    switch (m_currentMode) {
    case 0: margin = 125; break;
    case 1: margin = 93;  break;
    case 2:
    default: margin = 60;  break;
    }
    int radius = (width() - 2 * margin) / 2;
    QPointF center(width() / 2.0, height() / 2.0);

    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    pen.setColor(QColor(220, 223, 228, 77)); // rgba(220,223,228,0.3)
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);
}

void SpaceControl::setCenterShadowEnabled(bool enabled)
{
    if (m_centerShadow) {
        m_centerShadow->setEnabled(enabled);
    }
}
