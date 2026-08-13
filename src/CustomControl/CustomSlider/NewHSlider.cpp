#include "CustomControl/CustomSlider/NewHSlider.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPropertyAnimation>

NewHSlider::NewHSlider(QWidget *parent)
    : QSlider(Qt::Horizontal, parent)
    , m_parentWidget(parent)
    , m_sliderDisplayLabelContainer(NULL)
    , m_sliderDisplayLabel(NULL)
    , wheelEnabled(false)
{
    styleType = 0;
    trackHeight = 8;
    fgTrackHeight = 8; // 填充轨道高度（更细，居中）

    setMouseTracking(true);
    setAttribute(Qt::WA_NoSystemBackground);
    // 延迟连接，防止构造过程中 valueChanged 触发
    QMetaObject::invokeMethod(
        this,
        [this]() {
            connect(this, &QSlider::valueChanged, this, [this](int value) {
                update();
                if (m_sliderDisplayLabelContainer && m_sliderDisplayLabelContainer->isVisible())
                    updateDisplayLabelPosition(value);
            });
        },
        Qt::QueuedConnection);

    // 初始颜色设为 #0091C6（可调）
    m_fillColor = QColor("#009FEF");
    m_handleColor = QColor("#FFFFFF");
    updateFullPixmap();
}

NewHSlider::~NewHSlider()
{
    delete m_sliderDisplayLabelContainer;
}

void NewHSlider::updateFullPixmap()
{
    // 创建一个 1x8 大小的透明 pixmap，用当前颜色填充
    m_fullPixmap = QPixmap(1, 8);
    m_fullPixmap.fill(m_fillColor);
}
void NewHSlider::setFillColor(const QColor &color)
{
    if (m_fillColor != color) {
        m_fillColor = color;
        updateFullPixmap();
        update(); // 触发重绘
    }
}
void NewHSlider::setHandleColor(const QColor &color)
{
    if (m_handleColor != color) {
        m_handleColor = color;
        update(); // 滑块颜色变了，重绘
    }
}
//滑动条颜色变化
void NewHSlider::animateFillColor(const QColor &from, const QColor &to, int duration)
{
    // 如果已有动画在运行，先停止
    // 简单方式：创建一个新动画，设置为自动删除
    auto *anim = new QPropertyAnimation(this, "fillColor");
    anim->setDuration(duration);
    anim->setStartValue(from);
    anim->setEndValue(to);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}


//设置滑块颜色渐变
void NewHSlider::animateHandleColor(const QColor &from, const QColor &to, int duration)
{
    auto *anim = new QPropertyAnimation(this, "handleColor");
    anim->setDuration(duration);
    anim->setStartValue(from);
    anim->setEndValue(to);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
QPixmap NewHSlider::generateHandlePixmap(const QColor &baseColor, bool hover, bool pressed) const
{
    int w = 6;  // 普通宽度
    int h = 18; // 手柄高度（比轨道 10px 高，上下会自然超出）
    int radius = 3;
    if (hover || pressed) {
        w = 10; // 悬浮/按下时变宽
        h = 26;
    }

    QPixmap pix(w, h);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);

    QColor color = baseColor;
    if (pressed) {
        color = QColor("#6DD3FF");
        radius = 6;
    } else if (hover) {
        color = color.lighter(110); // 悬浮变亮
        radius = 6;
    }

    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(0, 0, w, h), radius, radius); // 圆角半径 6px
    p.end();
    return pix;
}

//样式类型 ,背景高度，值背景高度,是否开启动画,是否当鼠标悬浮到滑块上方时显示数组
void NewHSlider::setType(int type, int bgTrackHeight, int TfgTrackHeight, bool HAnimationEn,bool ShowTooltip)
{
    styleType = type;//(0：直接使用m_fullPixmap；1：最小值与最大值，分别与左右边缘间隔3，带圆弧；2:直接调用样式表；3：最小值与最大值，分别与左右边缘无间隔，带圆弧)
    trackHeight = bgTrackHeight;
    fgTrackHeight = TfgTrackHeight;
    AnimationEn = HAnimationEn;
    ShowTooltipEn = ShowTooltip;
}
void NewHSlider::setMargin(int value)
{
    margin = value;
}

void NewHSlider::hideDisplayLabel()
{
    if (!m_sliderDisplayLabelContainer)
        return;

    delete m_sliderDisplayLabelContainer;
    m_sliderDisplayLabelContainer = nullptr;
    m_sliderDisplayLabel = nullptr;
}

void NewHSlider::ensureDisplayLabel()
{
    if (m_sliderDisplayLabelContainer)
        return;

    m_sliderDisplayLabelContainer = new QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint);
    m_sliderDisplayLabelContainer->setAttribute(Qt::WA_TranslucentBackground);
    m_sliderDisplayLabelContainer->setFixedSize(30, 22);

    m_sliderDisplayLabel = new QLabel(m_sliderDisplayLabelContainer);
    m_sliderDisplayLabel->setGeometry(0, 0, 30, 22);
    m_sliderDisplayLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_sliderDisplayLabel->setStyleSheet("QLabel {"
                                        "  background-color: #12161D;"
                                        "  border-radius: 4px;"
                                        "  color: #FFFFFF;"
                                        "  font-family: \"Noto Sans S Chinese\";"
                                        "  font-weight: 500;"
                                        "  font-size: 12px;"
                                        "}");
}

int NewHSlider::sliderCenterForValue(int displayValue) const
{
    const int range = maximum() - minimum();
    if (range <= 0)
        return 0;

    QStyleOptionSlider opt;
    initStyleOption(&opt);
    const int length = orientation() == Qt::Horizontal ? width() : height();
    const int edgeOffset = style()->pixelMetric(QStyle::PM_SliderTickmarkOffset, &opt, this);
    const int span = qMax(0, length - edgeOffset * 2);
    return edgeOffset + QStyle::sliderPositionFromValue(minimum(), maximum(), displayValue,
                                                        span, opt.upsideDown);
}

void NewHSlider::updateDisplayLabelPosition(int displayValue)
{
    if (!m_sliderDisplayLabelContainer || !m_sliderDisplayLabel)
        return;

    m_sliderDisplayLabel->setText(QString::number(displayValue));
    const QPoint sliderGlobalTopLeft = mapToGlobal(QPoint(0, 0));
    const int center = sliderCenterForValue(displayValue);

    if (orientation() == Qt::Horizontal) {
        m_sliderDisplayLabelContainer->move(
            sliderGlobalTopLeft.x() + center - m_sliderDisplayLabelContainer->width() / 2,
            sliderGlobalTopLeft.y() - 14);
    } else {
        m_sliderDisplayLabelContainer->move(
            sliderGlobalTopLeft.x() - 20,
            sliderGlobalTopLeft.y() + center - m_sliderDisplayLabelContainer->height() / 2);
    }
}

void NewHSlider::cancelActiveInteraction()
{
    m_emitSliderReleasedOnMouseRelease = false;
    if (isSliderDown())
        setSliderDown(false);
    if (QWidget::mouseGrabber() == this)
        releaseMouse();
    setPressed(false);
    setHover(false);
    hideDisplayLabel();
}

void NewHSlider::paintEvent(QPaintEvent *event)
{
    if (styleType == 2)
    {
        // 直接调用父类 QSlider 的 paintEvent，使用原始样式绘制
        QSlider::paintEvent(event);
        return;
    } else {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);
        //painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿

        // 提前加载并缩放图片到固定高度 10px
        static const QPixmap emptyPixmap = QPixmap(":/Skin/Images/Slider/empty.png")
                                               .scaled(1,
                                                       8,
                                                       Qt::IgnoreAspectRatio,
                                                       Qt::SmoothTransformation);

        if (emptyPixmap.isNull() || m_fullPixmap.isNull()) {
            return;
        }

        const int sliderMin = style()->pixelMetric(QStyle::PM_SliderTickmarkOffset, nullptr, this);
        const int sliderMax = width() - sliderMin;

        const int range = maximum() - minimum(); // 总范围
        if (range <= 0) {
            return;
        }
        const double scale = static_cast<double>(sliderMax - sliderMin) / range;
        const int centerPos = sliderMin + (0 - minimum()) * scale;        // 0值对应的位置
        const int currentPos = sliderMin + (value() - minimum()) * scale; // 当前值位置

        // 固定滑轨高度为 10px
        // const int trackHeight = 8;
        // const int fgTrackHeight = 4;     // 填充轨道高度（更细，居中）
        const int y = (height() - trackHeight) / 2; // 垂直居中
        const int trackWidth = sliderMax - sliderMin;

        //const qreal radius = 4.0; // 圆角半径（约等于 border-radius: 5px）
        const qreal radius = trackHeight / 2;    // 圆角半径（约等于 border-radius: 5px）
        const qreal radius2 = fgTrackHeight / 2; // 圆角半径（约等于 border-radius: 5px）

        // === 绘制背景（空轨道，带圆角 + 平铺）===
        {
            QPainterPath clipPath;
            clipPath.addRoundedRect(QRectF(sliderMin, y, trackWidth, trackHeight), radius, radius);
            painter.save();
            painter.setClipPath(clipPath);

            // 现在在这个裁剪区域内进行平铺绘制
            // painter.drawTiledPixmap(QRect(sliderMin, y, trackWidth, trackHeight), emptyPixmap);

            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#4D000000"));
            painter.drawRect(QRect(sliderMin, y, trackWidth, trackHeight));

            // painter.setBrush(Qt::green);
            // painter.drawRect (QRect(sliderMin, y, trackWidth, trackHeight));
            painter.restore(); // 恢复原始状态，不影响后续绘制
        }

        // === 绘制已填充部分（从 左侧 到 current，带圆角）===
        if (value() != 0)
        {
            QRect fillRect;
            if (value() < 0) {
                int w = centerPos - currentPos;
                fillRect = QRect(currentPos, y, w, trackHeight);
            } else {
                int w = currentPos - centerPos;
                fillRect = QRect(centerPos, y, w, trackHeight);
            }

            if (styleType == 0)
            {
                painter.drawTiledPixmap(fillRect, m_fullPixmap);
            } else if (styleType == 1)
            {
                // 计算前景填充区域：高度变小，垂直居中于背景
                int fy = y + (trackHeight - fgTrackHeight) / 2; // 垂直居中偏移
                QRect fillRect2(fillRect.x() + margin,
                                fy,
                                fillRect.width() - margin,
                                fgTrackHeight); //最小值与最大值，分别与左右边缘间隔margin

                //带圆角
                QPainterPath fillPath;
                fillPath.addRoundedRect(fillRect2, radius2, radius2);
                painter.save();
                painter.setClipPath(fillPath);
                painter.drawPixmap(fillRect2, m_fullPixmap);
                painter.restore();
            } else if (styleType == 3)
            {
                // 计算前景填充区域：高度变小，垂直居中于背景
                int fy = y + (trackHeight - fgTrackHeight) / 2; // 垂直居中偏移
                QRect fillRect2(fillRect.x(),
                                fy,
                                fillRect.width(),
                                fgTrackHeight); //最小值与最大值，分别与左右边缘无间隔

                //带圆角
                QPainterPath fillPath;
                fillPath.addRoundedRect(fillRect2, radius2, radius2);
                painter.save();
                painter.setClipPath(fillPath);
                painter.drawPixmap(fillRect2, m_fullPixmap);
                painter.restore();
            }
        }

        if (!AnimationEn) {
            // === 绘制滑块手柄（Handle）===
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.subControls = QStyle::SC_SliderHandle; //只绘制手柄部分
            opt.sliderPosition = value();
            opt.sliderValue = value();
            style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);
        } else {
            // === 绘制滑块手柄（动态颜色，使用系统计算的矩形）===
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            opt.subControls = QStyle::SC_SliderHandle; // 仅计算手柄
            // // 获取手柄的正确矩形（会自动考虑边界限制）
            // QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
            QPixmap handlePix = generateHandlePixmap(m_handleColor, m_hover, m_pressed);
            int handleW = handlePix.width();
            int handleH = handlePix.height();
            // 中心点移动范围：手柄左边缘不能小于 sliderMin，右边缘不能大于 sliderMax
            double halfW = handleW / 2.0;
            int minCenterX = sliderMin + halfW;
            int maxCenterX = sliderMax - halfW;
            int centerX = sliderMin + (value() - minimum()) * scale;
            centerX = qBound(minCenterX, centerX, maxCenterX);

            int handleX = centerX - halfW;
            int handleY = (height() - handleH) / 2;
            painter.drawPixmap(handleX, handleY, handlePix);
        }
    }
}

//鼠标点击到滑道上则跳转
void NewHSlider::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        const int range = maximum() - minimum();
        if (AnimationEn)
        {
            setPressed(true);
        }
        if (ShowTooltipEn)
        {
            ensureDisplayLabel();
            updateDisplayLabelPosition(value());
            m_sliderDisplayLabelContainer->setVisible(true);
            m_sliderDisplayLabelContainer->raise();   // 强制置顶
        }

        // 1. 获取滑块的样式信息
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        // 2. 获取滑块柄区域
        QRect handleRect = style()->subControlRect(QStyle::CC_Slider,
                                                   &opt,
                                                   QStyle::SC_SliderHandle,
                                                   this);

        // 3. 关键判断：如果点击位置在滑块柄内，则交给基类处理（实现拖动）
        if (handleRect.contains(ev->pos())) {
            // 让基类QSlider处理后续的拖动逻辑
            QSlider::mousePressEvent(ev);
            return;
        }

        // 4. 如果点击在滑块柄之外（通常是轨道上），则执行您的点击跳转逻辑
        // 获取轨道区域
        QRect grooveRect = style()->subControlRect(QStyle::CC_Slider,
                                                   &opt,
                                                   QStyle::SC_SliderGroove,
                                                   this);

        // 计算实际可点击的轨道区域（减去滑块柄宽度）
        QRect trackRect = grooveRect;
        int handleWidth = handleRect.width();

        if (trackRect.isValid() && handleWidth > 0) {
            // 调整轨道区域，去掉滑块柄占用的空间
            trackRect.adjust(handleWidth / 2, 0, -handleWidth / 2, 0);
        }

        // 计算点击位置在轨道中的比例
        double ratio = 0.0;

        if (trackRect.isValid() && trackRect.width() > 0) {
            // 计算相对于轨道左边缘的位置
            int clickX = ev->pos().x() - trackRect.left();
            ratio = static_cast<double>(clickX) / trackRect.width();
            ratio = qMax(0.0, qMin(1.0, ratio));
        } else {
            // 备用方法：使用整个滑块的宽度
            ratio = static_cast<double>(ev->pos().x()) / width();
            ratio = qMax(0.0, qMin(1.0, ratio));
        }

        // 根据反转设置调整比例
        if (invertedAppearance()) {
            ratio = 1.0 - ratio;
        }

        // 计算新值
        if (range <= 0) {
            QSlider::mousePressEvent(ev);
            return;
        }
        int newValue = minimum() + static_cast<int>(range * ratio + 0.5);

        // 设置值
        setValue(newValue);

        m_emitSliderReleasedOnMouseRelease = true;

        ev->accept();
        // 注意：这里不再调用基类的mousePressEvent，因为我们已实现点击跳转
        return;
    }

    // 对于非左键点击，依然交给基类处理
    QSlider::mousePressEvent(ev);
}
//鼠标滚动事件
void NewHSlider::wheelEvent(QWheelEvent *event)
{
    if (wheelEnabled) {
        // 允许滚轮控制时才执行操作
        QSlider::wheelEvent(event);
    } else {
        // 忽略滚动事件
        event->ignore();
    }
}
//得到焦点
void NewHSlider::focusInEvent(QFocusEvent *event)
{
    wheelEnabled = true;
    QSlider::focusInEvent(event);
}
//失去焦点
void NewHSlider::focusOutEvent(QFocusEvent *event)
{
    wheelEnabled = false;
    QSlider::focusOutEvent(event);
}

void NewHSlider::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange && !isEnabled()) {
        cancelActiveInteraction();
    }
    QSlider::changeEvent(event);
}

//鼠标离开时
void NewHSlider::leaveEvent(QEvent *e)
{
    //离开控件时删除悬浮标签
    if (ShowTooltipEn)
    {
        hideDisplayLabel();
    }

    //取消 hover 状态
    if (AnimationEn)
    {
        setHover(false);
    }
    QSlider::leaveEvent(e);
}
//鼠标释放时
void NewHSlider::mouseReleaseEvent(QMouseEvent *e)
{
    // //释放鼠标时删除悬浮标签
    // if (ShowTooltipEn)
    // {
    //     if (m_sliderDisplayLabelContainer) {
    //         delete m_sliderDisplayLabelContainer;
    //         m_sliderDisplayLabelContainer = nullptr;
    //         m_sliderDisplayLabel = nullptr;
    //     }
    // }

    //取消按下状态
    if (AnimationEn)
    {
        setPressed(false);
    }
    if (m_emitSliderReleasedOnMouseRelease && e->button() == Qt::LeftButton) {
        m_emitSliderReleasedOnMouseRelease = false;
        emit sliderReleased();
    }
    QSlider::mouseReleaseEvent(e);
    if (e->button() == Qt::LeftButton) {
        cancelActiveInteraction();
    }
}
//鼠标移动时（实时监听鼠标移动，无论鼠标从哪儿进入控件，只要经过手柄区域就能触发）
void NewHSlider::mouseMoveEvent(QMouseEvent *ev)
{

    // 获取滑块手柄区域（
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect handleRect = style()->subControlRect(QStyle::CC_Slider,
                                               &opt,
                                               QStyle::SC_SliderHandle,
                                               this);

    // 将滑块区域转换为全局坐标
    QPoint globalPos = mapToGlobal(handleRect.topLeft());
    handleRect.moveTo(globalPos);
    handleRect.setWidth(16);
    handleRect.setHeight(16);

    QPoint cursorPos = QCursor::pos();// 当前鼠标的全局坐标
    bool hoverHandle = handleRect.contains(cursorPos);
    //qDebug("handleRect.x():%d,handleRect.y():%d,handleRect.width():%d,handleRect.height():%d,\n a.x():%d,a.y():%d\n",handleRect.x(),handleRect.y(),handleRect.width(),handleRect.height(),a.x(),a.y());
    // 动画
    if (AnimationEn)
    {
        if (hoverHandle)
            setHover(true);
        else
            setHover(false); // 注意：原先离开时在 leaveEvent 里设为 false，这里可以补全
    }
    // 悬浮数值显示
    if (ShowTooltipEn)
    {
        if (hoverHandle || isSliderDown() || m_pressed)
        {
            ensureDisplayLabel();
            updateDisplayLabelPosition(value());
            m_sliderDisplayLabelContainer->setVisible(true);

        } else
        {
            // 鼠标不在手柄上，但可能仍在控件内（比如轨道），可以选择隐藏或保留
            // 按照你原先的逻辑，不删除，只隐藏或更新位置
            if (m_sliderDisplayLabelContainer) {
                m_sliderDisplayLabelContainer->hide(); // 或 setVisible(false)
            }
        }

    }
    return QSlider::mouseMoveEvent(ev); // 最后调用了基类
}
void NewHSlider::setHover(bool hover)
{
    if (m_hover == hover)
        return;
    m_hover = hover;
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void NewHSlider::setPressed(bool pressed)
{
    if (m_pressed == pressed)
        return;
    m_pressed = pressed;
    style()->unpolish(this);
    style()->polish(this);
    update();
}
