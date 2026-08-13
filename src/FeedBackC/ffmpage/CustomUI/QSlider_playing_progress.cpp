// QSliderPlayingProgress.cpp
#include "FeedBackC/ffmpage/CustomUI/QSlider_playing_progress.h"
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>

QSliderPlayingProgress::QSliderPlayingProgress(QWidget *parent)
    : QSlider(parent)
{
    InitUIInformation();
    InitMember();
    InitConnect();
    update();
    setStatus(true); ///默认
}

void QSliderPlayingProgress::InitUIInformation()
{
    setMouseTracking(true); /// 启用鼠标追踪

    setMinimumHeight(30); ///最小高度
    setMinimumWidth(120);

    setRange(0, 100);

    // 或者设置固定大小
    // setFixedSize(574, 12);

    // 或者设置大小策略
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setOrientation(Qt::Horizontal);

    // cl_handle_height_= 16;      ///滑块高度
    // cl_handle_width_ = 6;        ///滑块宽度
    cl_groove_height_ = 12;      ///整体滑槽绘制高度
    cl_left_groove_height_ = 4;  ///左侧滑槽绘制高度
    cl_right_groove_height_ = 4; ///右侧滑槽绘制高度

    cl_handle_radius_ = 6.0;       ///滑块圆角
    cl_groove_radius_ = 6.0;       ///整体滑槽圆角
    cl_left_groove_radius_ = 2.0;  ///左侧滑槽圆角
    cl_right_groove_radius_ = 2.0; ///右侧滑槽圆角

    cl_left_groove_start_ = 15;
    cl_left_groove_end_ = 15;

    //     setStyleSheet(R"(

    // )");

    setAttribute(Qt::WA_TranslucentBackground,true);

    setStyleSheet("QSlider {"
                  ""
                  "}"
                  "QSlider::groove:horizontal {"
                  "  margin-left: 10px;"  // groove 的外边距,限制滑块可滑动距离
                  "  margin-right: 10px;" // groove 的外边距,
                  "  background-color: #E0E0E0;"
                  "}"
                  "QSlider::handle:horizontal {"
                  "  margin: -10px 0;"
                  "  width: 11px;"
                  "  height: 16px;"
                  "  background-color: #FF0000;"
                  "}");

    update();
}

void QSliderPlayingProgress::InitMember()
{
    cl_handle_anim_ = std::make_unique<QVariantAnimation>(this);
    cl_left_groove_anim_ = std::make_unique<QVariantAnimation>(this);
    cl_right_groove_anim_ = std::make_unique<QVariantAnimation>(this);
    cl_groove_anim_ = std::make_unique<QVariantAnimation>(this);

    cl_handle_anim_->setDuration(1000); ///渐变时间
    cl_handle_anim_->setDuration(1000);
    cl_handle_anim_->setDuration(1000);
    cl_handle_anim_->setDuration(1000);

    clp_text_label_ = std::make_unique<QLabel>(this);
    clp_text_label_->setText("");
    clp_text_label_->setStyleSheet(R"(
    QLabel {
        position: absolute;
        left: 0px;
        top: 0px;
        width: 30px;
        height: 22px;
        border-radius: 4px;
        opacity: 1;
        background: #12161D;
        color: #FFFFFF;
        font-family: "Microsoft YaHei";
        font-size: 12px;
        font-weight: normal;
        text-align: center;
        line-height: 22px;
    }
)");
    clp_text_label_->setFixedSize(30, 22);
    clp_text_label_->setAlignment(Qt::AlignCenter);
    clp_text_label_->hide(); ///默认隐藏
}

void QSliderPlayingProgress::InitConnect()
{
    connect(cl_handle_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_handle_current_color = value.value<QColor>();
                update();
            });
    connect(cl_left_groove_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_left_groove_current_color = value.value<QColor>();
                update();
            });
    connect(cl_right_groove_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_right_groove_current_color = value.value<QColor>();
                update();
            });
    connect(cl_groove_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_groove_current_color = value.value<QColor>();
                update();
            });
}

void QSliderPlayingProgress::setStatus(bool status)
{
    /// 停止当前动画
    stopAllAnim();

    ///启用状态渐变
    if (status) {
        // 在 0.0 ~ 1.0 之间设置多个关键帧
        cl_handle_anim_->setKeyValues({{0.0, QColor("#c7c7c7")}, {1.0, QColor("#FFFFFF")}});

        cl_left_groove_anim_->setKeyValues({{0.0, QColor("#006184")}, {1.0, QColor("#0091DA")}});

        // cl_right_groove_anim_->setKeyValues({{0.0, QColor(Qt::red)},
        //                                      {0.3, QColor(Qt::yellow)},
        //                                      {0.7, QColor(Qt::green)},
        //                                      {1.0, QColor(Qt::blue)}});

        cl_groove_anim_->setKeyValues({{0.0, QColor("#FFFFFF")}, {1.0, QColor("#FFFFFF")}});
        ///禁用状态渐变
    } else {
        // 在 0.0 ~ 1.0 之间设置多个关键帧
        cl_handle_anim_->setKeyValues({{0.0, QColor("#FFFFFF")}, {1.0, QColor("#C7C7C7")}});

        cl_left_groove_anim_->setKeyValues({{0.0, QColor("#0091DA")}, {1.0, QColor("#006184")}});

        // cl_right_groove_anim_->setKeyValues({{0.0, QColor(Qt::blue)},
        //                                      {0.3, QColor(Qt::yellow)},
        //                                      {0.7, QColor(Qt::green)},
        //                                      {1.0, QColor(Qt::red)}});

        cl_groove_anim_->setKeyValues({{0.0, QColor("##FFFFFF")}, {1.0, QColor("##FFFFFF")}});
    }

    cl_handle_anim_->start();
    cl_left_groove_anim_->start();
    // cl_right_groove_anim_->start();
    cl_groove_anim_->start();

    setEnabled(status);
    // update();
}

void QSliderPlayingProgress::paintEvent(QPaintEvent *event)
{
    // QSlider::paintEvent(event);
    // qDebug() << "当前值：" << value();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /// 如果为禁用状态
    if (!isEnabled()) {
        drawDisabledStyle(painter);
    } else {
        drawEnableStatus(painter);
    }
}

void QSliderPlayingProgress::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    update(); // 触发重绘
}

void QSliderPlayingProgress::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    update(); // 触发重绘
}

void QSliderPlayingProgress::mousePressEvent(QMouseEvent *event)
{
    /// 只接受滑槽内部的点击
    if (grooveRect_actual.contains(event->pos())) {
        if (event->button() == Qt::LeftButton) {
            // 1. 获取滑块的样式信息
            QStyleOptionSlider opt;
            initStyleOption(&opt);

            // 2. 获取滑块柄区域
            QRect handleRect = style()->subControlRect(QStyle::CC_Slider,
                                                       &opt,
                                                       QStyle::SC_SliderHandle,
                                                       this);

            // 3. 关键判断：如果点击位置在滑块柄内，则交给基类处理（实现拖动）
            if (handleRect.contains(event->pos())) {
                cl_handle_current_color = QColor("#4D6DD3FF");
                // 让基类QSlider处理后续的拖动逻辑
                QSlider::mousePressEvent(event);
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
                int clickX = event->pos().x() - trackRect.left();
                ratio = static_cast<double>(clickX) / trackRect.width();
                ratio = qMax(0.0, qMin(1.0, ratio));
            } else {
                // 备用方法：使用整个滑块的宽度
                ratio = static_cast<double>(event->pos().x()) / width();
                ratio = qMax(0.0, qMin(1.0, ratio));
            }

            // 根据反转设置调整比例
            if (invertedAppearance()) {
                ratio = 1.0 - ratio;
            }

            // 计算新值
            int range = maximum() - minimum();
            int newValue = minimum() + static_cast<int>(range * ratio + 0.5);

            // 设置值
            setValue(newValue);

            // // 发出信号（注意：setValue内部也会触发valueChanged，可能会造成重复）
            emit sliderReleased();
            // emit sliderMoved(newValue);
            // emit valueChanged(newValue);

            event->accept();
            // 注意：这里不再调用基类的mousePressEvent，因为我们已实现点击跳转
            return;
        }
        // // 对于非左键点击，依然交给基类处理
        // QSlider::mousePressEvent(event);
        QSlider::mousePressEvent(event);
    } else {
        event->ignore();
    }
}

void QSliderPlayingProgress::mouseMoveEvent(QMouseEvent *event)
{
    // qDebug() << "move :" << event->pos();
    /// 如果鼠标在滑块范围内
    if (handleRect_actual.contains(event->pos())) {
        // qDebug() << "悬停";
        cl_isHover_.store(true);
        update();
    } else {
        // qDebug() << "非悬停";
        cl_isHover_.store(false);
        update();
    }

    QSlider::mouseMoveEvent(event);
}

void QSliderPlayingProgress::mouseReleaseEvent(QMouseEvent *event)
{
    // cl_handle_current_color = QColor("#FF0000");
    cl_handle_current_color = QColor("#FFFFFF");

    QSlider::mouseReleaseEvent(event);
}

void QSliderPlayingProgress::resizeEvent(QResizeEvent *event)
{
    QSlider::resizeEvent(event);
}

void QSliderPlayingProgress::wheelEvent(QWheelEvent *event)
{
    if (cl_is_wheelEnabled_) {
        ///判断是否在滑槽内
        if (grooveRect_actual.contains(event->position().toPoint())) {
            int numSteps = event->angleDelta().y() / 120;
            int newValue = value() - numSteps;
            newValue = qBound(minimum(), newValue, maximum());
            setValue(newValue);
            event->accept();
        } else {
            event->ignore();
        }
        // // 允许滚轮控制时才执行操作
        // QSlider::wheelEvent(event);
    } else {
        // 忽略滚动事件
        event->ignore();
    }
}

void QSliderPlayingProgress::focusInEvent(QFocusEvent *event)
{
    cl_is_wheelEnabled_.store(true);
    QSlider::focusInEvent(event);
}

void QSliderPlayingProgress::focusOutEvent(QFocusEvent *event)
{
    cl_is_wheelEnabled_.store(false);
    QSlider::focusInEvent(event);
}

void QSliderPlayingProgress::drawGrooveRect(
    QPainter &painter, const QRectF &rect, qreal tl, qreal tr, qreal br, qreal bl)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    // 顺时针方向绘制
    path.moveTo(rect.left() + tl, rect.top());

    // 上边和右上角
    path.lineTo(rect.right() - tr, rect.top());
    if (tr > 0)
        path.arcTo(QRectF(rect.right() - tr * 2, rect.top(), tr * 2, tr * 2), 90, -90);

    // 右边和右下角
    path.lineTo(rect.right(), rect.bottom() - br);
    if (br > 0)
        path.arcTo(QRectF(rect.right() - br * 2, rect.bottom() - br * 2, br * 2, br * 2), 0, -90);

    // 下边和左下角
    path.lineTo(rect.left() + bl, rect.bottom());
    if (bl > 0)
        path.arcTo(QRectF(rect.left(), rect.bottom() - bl * 2, bl * 2, bl * 2), -90, -90);

    // 左边和左上角
    path.lineTo(rect.left(), rect.top() + tl);
    if (tl > 0)
        path.arcTo(QRectF(rect.left(), rect.top(), tl * 2, tl * 2), -180, -90);

    path.closeSubpath();
    // painter.drawPath(path);
    painter.fillPath(path, painter.brush());

    painter.restore();
}

void QSliderPlayingProgress::drawEnableStatus(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QStyleOptionSlider opt;
    initStyleOption(&opt);

    /// 整体滑槽矩形
    QRect grooveRect = style()->subControlRect(QStyle::CC_Slider,
                                               &opt,
                                               QStyle::SC_SliderGroove,
                                               this);

    ///先绘制一下整体背景
    painter.setPen(Qt::NoPen);
    // painter.setBrush(QColor("#303949"));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(grooveRect, 0, 0);

    // 完全透明背景（不清除背景）
    // painter.setCompositionMode(QPainter::CompositionMode_Source);
    // painter.fillRect(rect(), Qt::transparent);

    // painter.setPen(Qt::NoPen);
    // // painter.setBrush(cl_groove_current_color);
    // painter.setBrush(Qt::green);
    grooveRect_actual.setRect(grooveRect.left() + 5,
                              grooveRect.top() + grooveRect.height() / 2 - cl_groove_height_ / 2,
                              grooveRect.width() - 20,
                              cl_groove_height_); ///实际绘制形状
    // painter.drawRoundedRect(grooveRect_actual, cl_groove_radius_, cl_groove_radius_);

    /// 滑块矩形
    QRect handleRect = style()->subControlRect(QStyle::CC_Slider,
                                               &opt,
                                               QStyle::SC_SliderHandle,
                                               this);
    // qDebug() << handleRect;
    // painter.setPen(Qt::NoPen);
    // painter.setBrush(QColor("#0000ff"));
    // painter.drawRoundedRect(handleRect, 0, 0);

    // 滑块位置比例
    qreal handlePosRatio = (qreal) (value() - minimum()) / (maximum() - minimum());
    // 已走过部分宽度
    int filledWidth = static_cast<int>(grooveRect.width() * handlePosRatio);

    ///绘制左侧滑槽(填充区域)
    {
        // QRect filledRect(grooveRect.left(), grooveRect.top(), filledWidth, grooveRect.height());

        // 绘制左侧已走过部分
        painter.setPen(Qt::NoPen);
        painter.setBrush(cl_left_groove_current_color);

        QRect filledRect_actual; ///左侧填充滑槽
        ///对于水平形态
        if (handleRect.center().x() < cl_left_groove_start_) {
            ///无需绘制左侧滑槽

        } else {
            filledRect_actual.setRect(cl_left_groove_start_, ///起始绘制位置
                                      grooveRect.top() + grooveRect.height() / 2
                                          - cl_left_groove_height_ / 2,
                                      handleRect.center().x()
                                          - cl_left_groove_start_, ///滑块中心 - 起始绘制点
                                      cl_left_groove_height_);
        }
        drawGrooveRect(painter,
                       filledRect_actual,
                       cl_left_groove_radius_,
                       0,
                       0,
                       cl_left_groove_radius_);
    }

    ///绘制右侧滑槽(非填充区域)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#66FFFFFF"));

        QRect filledRect_actual_right;                           ///右侧空白滑槽
                                                                 ///对于水平形态
        filledRect_actual_right.setRect(handleRect.center().x(), ///滑块中心 - 起始绘制点
                                        grooveRect.top() + grooveRect.height() / 2
                                            - cl_left_groove_height_ / 2,
                                        grooveRect.width() - cl_left_groove_end_
                                            - handleRect.center().x(),
                                        cl_left_groove_height_);
        drawGrooveRect(painter,
                       filledRect_actual_right,
                       0,
                       cl_right_groove_radius_,
                       cl_right_groove_radius_,
                       0);
    }

    ///绘制滑块(最后绘制)
    {
        ///如果是悬停状态
        if (cl_isHover_.load()) {
            ///放大滑块
            cl_handle_height_.store(26); ///滑块高度
            cl_handle_width_.store(10);  ///滑块宽度
            cl_handle_radius_ = 6;

            clp_text_label_->setText(QString::number(value()));
            clp_text_label_->move(handleRect.center().x() - (clp_text_label_->width() / 2),
                                  handleRect.center().y() - cl_handle_height_.load() / 2 - 10
                                      - (clp_text_label_->height()));
            // clp_text_label_->show(); ///不启用（视频播放模块）
            clp_text_label_->hide();
        } else {
            cl_handle_height_.store(16); ///滑块高度
            cl_handle_width_.store(6);   ///滑块宽度
            cl_handle_radius_ = 3;
            clp_text_label_->hide();
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(cl_handle_current_color);
        handleRect_actual.setRect(handleRect.center().x() - cl_handle_width_.load() / 2,
                                  handleRect.center().y() - cl_handle_height_.load() / 2,
                                  cl_handle_width_.load(),
                                  cl_handle_height_.load());
        // painter.drawRect(handleRect_actual);
        drawGrooveRect(painter,
                       handleRect_actual,
                       cl_handle_radius_,
                       cl_handle_radius_,
                       cl_handle_radius_,
                       cl_handle_radius_);

        // if (cl_isHover_.load()) {
        //     clp_text_label_->show();
        // } else {
        //     clp_text_label_->hide();
        // }
    }

    painter.restore();
}

void QSliderPlayingProgress::drawDisabledStyle(QPainter &painter)
{
    drawEnableStatus(painter); ///目前没区别
}

void QSliderPlayingProgress::stopAllAnim()
{
    cl_handle_anim_->stop();
    cl_left_groove_anim_->stop();
    cl_right_groove_anim_->stop();
    cl_groove_anim_->stop();
}
