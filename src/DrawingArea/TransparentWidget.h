#ifndef TRANSPARENTWIDGET_H
#define TRANSPARENTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>

class TransparentWidget : public QWidget {
    Q_OBJECT
public:
    explicit TransparentWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
    }

    // explicit TransparentWidget(QWidget *w2, QPushButton *btn, QWidget *w3, QWidget *parent = nullptr)
    //     : QWidget(parent)
    // {
    //     setAttribute(Qt::WA_TranslucentBackground);
    //     setup(w2, btn, w3);
    // }

    void setup(QWidget *w2, QPushButton *btn, QWidget *w3)
    {
        w2_ = w2;
        btn_ = btn;
        w3_ = w3;
        buildRegion();
    }

    void setButtonBottomRadius(int radius)
    {
        btnRadius_ = radius;
        update();
    }

    void setSideRadius(int w2Right, int w3Left) // 可选，控制左右圆角
    {
        w2Radius_ = w2Right;
        w3Radius_ = w3Left;
        update();
    }

private:
    void buildRegion()
    {
        if (!w2_ || !btn_ || !w3_) return;
        int maxH = qMax(w2_->height(), qMax(btn_->height(), w3_->height()));
        bottomY_ = btn_->height();
        bottomH_ = maxH - bottomY_;
    }

protected:
    void paintEvent(QPaintEvent *) override {
        if (!w2_ || !btn_ || !w3_) return;
        int btnBottom = btn_->geometry().bottom();  // 按钮底部在父控件中的 y 坐标
        int maxH = height();  // TransparentWidget 的高度应与容器一致
        int bottomH = maxH - btnBottom;
        if (bottomH <= 0) return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPainterPath redPath;

        // 1. 只在按钮正下方画矩形，宽度为按钮宽度
        QRect btnRect = btn_->geometry();
        redPath.addRect(btnRect.left(), btnBottom+1, btnRect.width(), bottomH-1);

        // 2. w2 右下角扇形
        if (w2Radius_ > 0) {
            QRectF r = w2_->geometry();
            qreal rad = w2Radius_;
            // 注意：这里仍然是之前的方法，但要注意 w2 右下角扇形应该覆盖超出按钮左侧的部分
            // 之前的扇形画法是从右下角点开始 arcTo，需要检查是否正确
            // 之前代码是：redPath.moveTo(r.right(), r.bottom()); redPath.arcTo(... 0, -90)...
            // 但这样形成的路径和 addRect 合并，可能会产生多余的连接。需要单独构建 QPainterPath 再 addPath
            QPainterPath corner;
            corner.moveTo(r.right(), r.bottom());
            corner.arcTo(r.right() - 2*rad, r.bottom() - 2*rad, 2*rad, 2*rad, 0, -90);
            corner.closeSubpath();
            redPath.addPath(corner);
        }
        // 3. w3 左下角扇形
        if (w3Radius_ > 0) {
            QRectF r = w3_->geometry();
            qreal rad = w3Radius_;
            QPainterPath corner;
            corner.moveTo(r.left(), r.bottom());
            corner.arcTo(r.left(), r.bottom() - 2*rad, 2*rad, 2*rad, 180, 90);
            corner.closeSubpath();
            redPath.addPath(corner);
        }
        // 4. 按钮下角扇形（与之前相同）
        if (btnRadius_ > 0) {
            QRectF r = btn_->geometry();
            qreal rad = btnRadius_;
            // 左下角
            {
                QPainterPath corner;
                corner.moveTo(r.left(), r.bottom() - rad);
                corner.arcTo(r.left(), r.bottom() - 2*rad, 2*rad, 2*rad, 180, 90);
                corner.lineTo(r.left(), r.bottom());
                corner.closeSubpath();
                redPath.addPath(corner);
            }
            // 右下角
            {
                QPainterPath corner;
                corner.moveTo(r.right() - rad, r.bottom());
                corner.arcTo(r.right() - 2*rad, r.bottom() - 2*rad, 2*rad, 2*rad, 270, 90);
                corner.lineTo(r.right(), r.bottom());
                corner.closeSubpath();
                redPath.addPath(corner);
            }
        }

        painter.fillPath(redPath, QColor(81, 96, 122, 51)); // 或其他颜色
    }


    QWidget *w2_ = nullptr;
    QPushButton *btn_ = nullptr;
    QWidget *w3_ = nullptr;
    int bottomY_ = 0;
    int bottomH_ = 0;
    int btnRadius_ = 10;    // 按钮底部圆角半径
    int w2Radius_ = 16;     // w2 右下角半径
    int w3Radius_ = 16;     // w3 左下角半径
};

#endif // TRANSPARENTWIDGET_H
