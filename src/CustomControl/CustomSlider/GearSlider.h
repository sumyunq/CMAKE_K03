#ifndef GEARSLIDER_H
#define GEARSLIDER_H

#include <QSlider>

class GearSlider : public QSlider
{
    Q_OBJECT
public:
    explicit GearSlider(QWidget *parent = nullptr);

    void setBlockHeight(qreal height) {
        m_blockHeight = height;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    int valueFromPos(const QPoint &pos) const;

    bool wheelEnabled;

    qreal m_blockHeight = 10;

    int m_spacing = 5;
};

#endif // GEARSLIDER_H
