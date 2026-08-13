#include "Popup/CustomTipPopup/NewCustomToolTip.h"
#include <QPainter>
#include <QPainterPath>

const QColor NewCustomToolTip::bgColor("#0D0F14");

NewCustomToolTip::NewCustomToolTip(QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint),
      targetWidget(nullptr)
{
    // 1. 窗口透明，才能在 paintEvent 中画出自定义形状
    setAttribute(Qt::WA_TranslucentBackground, true);

    label = new QLabel(this);
    layout = new QVBoxLayout(this);

    // 2. 设置阴影效果（box-shadow: 0px 4px 8px 0px rgba(0,0,0,0.5)）
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(label);
    shadow->setColor(QColor(0, 0, 0, 128));          // rgba(0,0,0,0.5)
    shadow->setBlurRadius(8);                        // 模糊半径 8px
    shadow->setOffset(0, 4);                         // Y 偏移 4px
    label->setGraphicsEffect(shadow);



    // // 3. 创建 label，背景透明，文字样式不变
    // label = new QLabel(this);
    // label->setStyleSheet(R"(
    //     font-family: "Noto Sans S Chinese";
    //             font-weight: 500;
    //     font-size: 10px;
    //     color: #A1A8B3;
    //     background: transparent;   /* 背景透明，由父窗口绘制底色 */
    // )");
    // label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    // label->setWordWrap(true);

    // // 4. 布局，顶部留出箭头高度 (arrowHeight = 8px)
    // layout = new QVBoxLayout(this);
    // layout->setContentsMargins(6, 6 + arrowHeight, 6, 6); // 上边距加上箭头高度
    // layout->addWidget(label);
    // setLayout(layout);

    // // 禁止 widget 自身的样式表背景
    // setStyleSheet("background: transparent; border: none;");

    // setMinimumSize(1, 1 + arrowHeight);

}
void NewCustomToolTip::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(m_currentStyle != 0)
    {
        return;
    }

    // 完全复制 paintEvent 中构造路径的逻辑
    QPainterPath path;
    int arrowCenterX = width() / 2;
    int aw = arrowWidth;
    int ah = arrowHeight;
    QRect bubbleRect = rect().adjusted(0, ah, 0, 0);
    int r = borderRadius;

    path.moveTo(arrowCenterX - aw/2, ah);
    path.lineTo(arrowCenterX, 0);
    path.lineTo(arrowCenterX + aw/2, ah);

    path.lineTo(bubbleRect.right() - r, bubbleRect.top());
    path.arcTo(bubbleRect.right() - 2*r, bubbleRect.top(), 2*r, 2*r, 90, -90);
    path.lineTo(bubbleRect.right(), bubbleRect.bottom() - r);
    path.arcTo(bubbleRect.right() - 2*r, bubbleRect.bottom() - 2*r, 2*r, 2*r, 0, -90);
    path.lineTo(bubbleRect.left() + r, bubbleRect.bottom());
    path.arcTo(bubbleRect.left(), bubbleRect.bottom() - 2*r, 2*r, 2*r, 270, -90);
    path.lineTo(bubbleRect.left(), bubbleRect.top() + r);
    path.arcTo(bubbleRect.left(), bubbleRect.top(), 2*r, 2*r, 180, -90);
    path.lineTo(bubbleRect.left(), bubbleRect.top());
    path.lineTo(arrowCenterX - aw/2, bubbleRect.top());

    path.closeSubpath();

    // 将窗口形状设置为该路径
    setMask(path.toFillPolygon().toPolygon());
}
// 新增 paintEvent，绘制气泡主体和箭头
void NewCustomToolTip::paintEvent(QPaintEvent *event)
{
    if(m_currentStyle != 0)
    {
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    // 背景色 #0D0F14
    QColor bgColor("#0D0F14");
    painter.setBrush(bgColor);

    // 获取可用绘制区域（去掉阴影效果的边距，避免箭头被裁切）
    QRect contentRect = rect().adjusted(0, 0, 0, 0); // 可直接用 rect()
    // 箭头参数
    int arrowTop = 0;
    int arrowCenterX = width() / 2;


    // 气泡主体：圆角矩形，顶部需要从箭头以下开始
    QRect bubbleRect = contentRect.adjusted(0, arrowHeight, 0, 0);
    int radius = 4; // 圆角半径

    // 绘制主体（带圆角的矩形，顶部被箭头“吃掉”一块，但可以用 Path 合并）
    QPainterPath path;
    // 从箭头左侧底部开始
    path.moveTo(arrowCenterX - arrowWidth / 2, arrowHeight);
    // 画到箭头尖端（向上）
    path.lineTo(arrowCenterX, 0);
    // 画到箭头右侧底部
    path.lineTo(arrowCenterX + arrowWidth / 2, arrowHeight);

    // 接着画圆角矩形的上边框（从箭头右侧底部开始往右）
    path.lineTo(bubbleRect.right() - radius, bubbleRect.top()); // 注意 bubbleRect.top() = arrowHeight
    // 右上角圆角
    path.arcTo(bubbleRect.right() - 2*radius, bubbleRect.top(), 2*radius, 2*radius, 90, -90);
    // 右边框
    path.lineTo(bubbleRect.right(), bubbleRect.bottom() - radius);
    // 右下角圆角
    path.arcTo(bubbleRect.right() - 2*radius, bubbleRect.bottom() - 2*radius, 2*radius, 2*radius, 0, -90);
    // 下边框
    path.lineTo(bubbleRect.left() + radius, bubbleRect.bottom());
    // 左下角圆角
    path.arcTo(bubbleRect.left(), bubbleRect.bottom() - 2*radius, 2*radius, 2*radius, 270, -90);
    // 左边框
    path.lineTo(bubbleRect.left(), bubbleRect.top() + radius);
    // 左上角圆角
    path.arcTo(bubbleRect.left(), bubbleRect.top(), 2*radius, 2*radius, 180, -90);
    // 连接到箭头左侧底部
    path.lineTo(bubbleRect.left(), bubbleRect.top());
    path.lineTo(arrowCenterX - arrowWidth / 2, bubbleRect.top()); // 其实就是 arrowHeight 处

    path.closeSubpath();
    painter.drawPath(path);
}

void NewCustomToolTip::AddToolTip(QWidget *target,const QString &text,Qt::Alignment align)
{
    if(text.isEmpty())
    {
        return;
    }
    // if (!target || target == targetWidget)
    //     return;
    if (!target)
        return;

    label->setText(text);
    label->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); // 重置缓存大小
    adjustSize(); // 重新根据文本内容调整大小
    // 限制最大宽度（例如不超过 136px）
    int maxWidth = 136;
    if(m_currentStyle == 0)
    {
        maxWidth = 136;
    }else if(m_currentStyle == 1)
    {
        maxWidth = 120;
    }else if(m_currentStyle == 2)
    {
        maxWidth = 166;
    }
    if (label->width() > maxWidth) {
        label->setMaximumWidth(maxWidth);
        label->setWordWrap(true);
        adjustSize();
    }

    // 移除旧控件的事件过滤器
    if (this->targetWidget) {
        this->targetWidget->removeEventFilter(this);
    }

    this->targetWidget = target;
    this->targetWidget->installEventFilter(this);

    CurrentAlign = align;
}


void NewCustomToolTip::showToolTipBelow()
{
    if (!targetWidget) return;

    QPoint globalPos = targetWidget->mapToGlobal(QPoint(0, 0));
    int x = globalPos.x();//左侧对齐  , 默认 AlignLeft 就是 x 不变
    int y = globalPos.y() + targetWidget->height() + 4;

    if (CurrentAlign & Qt::AlignRight)//右侧对齐
    {
        x = globalPos.x() + targetWidget->width() - width();
    }
    else if (CurrentAlign & Qt::AlignHCenter)//中心对齐
    {
        x = globalPos.x() + (targetWidget->width() - width()) / 2;
    }


    QScreen *screen = QGuiApplication::screenAt(globalPos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    QRect avail = screen->availableGeometry();

    x = qBound(avail.left(), x, avail.right() - width());
    if (y + height() <= avail.bottom()) {
        // 正常下方显示
    } else {
        y = globalPos.y() - height() - 4;
        y = qBound(avail.top(), y, avail.bottom() - height());
    }

    // if (x < avail.left()) x = avail.left();
    // if (x + width() > avail.right()) x = avail.right() - width();

    // if (y + height() > avail.bottom()) {
    //     y = globalPos.y() - height() - 4;
    //     if (y < avail.top()) y = avail.top();
    // }



   // adjustSize(); // 再次确保尺寸正确
    move(x, y);

    // 显示前强制 layout 更新和 repaint
    raise();
    //repaint(); // 触发一次安全重绘
    show();
}


bool NewCustomToolTip::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == targetWidget) {
        switch (event->type())
        {
            case QEvent::Enter:
            {
                showToolTipBelow();
                return false;
            }
            case QEvent::MouseButtonPress:
            {
                showToolTipBelow();
                return false;
            }
            case QEvent::Leave:
            {
                hide();
                return false;
            }
            case QEvent::MouseMove:
            {
                // // 持续更新 tooltip 位置（如果需要跟随）
                // if (isVisible())
                // {
                //     QPoint pos = targetWidget->mapToGlobal(QPoint(0, targetWidget->height()));
                //     move(pos);
                // }
                return false;
            }
            default:
                break;
            }
    }
    return QWidget::eventFilter(obj, event);
}
//设置样式(0:聊天气泡样式（带三角），1：2.4G链接时弹窗，2：方案描述的弹窗)
void NewCustomToolTip::setLabelStyle(int idx)
{
    m_currentStyle = idx;
    switch(idx)
    {
    case 1:
    {
        this->setStyleSheet(R"(
        background-color: transparent;
        border: 4px;
        )");
        this->resize(120, 54);
        label->resize(120, 54);
        label->setWordWrap(true);                 //自动换行
        // layout->setContentsMargins(0, 0, 0, 0);
        layout->setContentsMargins(0, 4, 12, 12); // 留出阴影空间
        layout->addWidget(label);
        setLayout(layout);
        label->setStyleSheet(R"(
                border-radius: 4px;
                padding: 6px 8px;
                background: rgba(0, 0, 0, 0.5);
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 10px;
                color: #5D646E;
                border: none;
        )");
        break;
    }
    case 2:
    {
        this->setStyleSheet(R"(background-color: transparent;border: 6px;)");
        label->setStyleSheet(
            "QLabel {"
            "   background-color: #0D0F14;"
            "   color: #454D57;"
            "   border: none;"
            "   padding: 6px 8px;"
            "   border-radius: 6px;"
            "  font-family: \"Noto Sans S Chinese\";"
            "  font-weight: 500;"
            "  font-size: 10px;"
            "}"
            );
        label->setAttribute(Qt::WA_StyledBackground, true);  // 圆角必需
        label->setWordWrap(true);
        label->setFixedWidth(166);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);

        // 强制 label 根据当前文本和宽度计算理想高度
        label->adjustSize();

        // layout->setContentsMargins(0, 0, 0, 0);
        layout->setContentsMargins(0, 4, 12, 12); // 留出阴影空间
        layout->addWidget(label);
        setLayout(layout);

        setFixedWidth(166);
        // setMinimumSize(166, 1);  // 防止尺寸为0

        break;
    }
    case 0:
    default:
    {
        //label，背景透明，文字样式不变

        label->setStyleSheet(R"(
        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 10px;
        color: #A1A8B3;
        background: transparent;   /* 背景透明，由父窗口绘制底色 */
        )");
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        label->setWordWrap(true);

        // 布局，顶部留出箭头高度 (arrowHeight = 8px)
        layout->setContentsMargins(6, 6 + arrowHeight, 6, 6); // 上边距加上箭头高度
        layout->addWidget(label);
        setLayout(layout);

        // 禁止 widget 自身的样式表背景
        setStyleSheet("background: transparent; border: none;");

        setMinimumSize(1, 1 + arrowHeight);
        break;
    }
    }
}
