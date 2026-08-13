#include "VideoHover.h"
#include <QMouseEvent>
#include "ui_VideoHover.h"

VideoHover::VideoHover(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoHover)
{
    ui->setupUi(this);

    InitUIInformation();
}

VideoHover::~VideoHover()
{
    //on_pBt_close_clicked();
    delete ui;
}

void VideoHover::on_pBt_close_clicked()
{
    closeRequested();
    // emit videoClose();
    // close();
    // deleteLater();
}

void VideoHover::updateMinWidget_Frame(const QImage &img)
{
    /// 视频切换过渡期间拒绝接收帧
    if (!accepting_frames_.load()) {
        return;
    }
    /// 图片帧为空，就显示默认图片/默认封面
    if (img.isNull()) {
        // currentImage = img;
    } else {
        currentImage = img;
    }

    update();
}

//返回视频播放页面
void VideoHover::on_pBt_back_clicked()
{
    emit restoreRequested();
    // emit videoBack();
}
//播放/暂停
void VideoHover::on_pBt_play_clicked()
{
    /// 按钮为 checkable，点击后 checked 状态已自动切换
    /// checked = 暂停状态, unchecked = 播放状态
    emit video_pause_requested(ui->pBt_play->isChecked());
}
void VideoHover::setPlayEn(bool en)
{
    ui->pBt_play->blockSignals(true);
    ui->pBt_play->setChecked(en);
    ui->pBt_play->blockSignals(false);
}

void VideoHover::requestRestore()
{
    emit restoreRequested();
}

void VideoHover::set_is_pause(bool is_pause)
{
    ui->pBt_play->setChecked(is_pause);
}

void VideoHover::setAcceptingFrames(bool accept)
{
    accepting_frames_.store(accept);
    if (!accept) {
        currentImage = QImage();
    }
}

void VideoHover::InitUIInformation()
{
    hoverWav = ui->widget_wav;
    ui->widget_wav->move(0, 32);
    hoverWav->move(0, 32);

    // 启用悬停事件检测
    setAttribute(Qt::WA_Hover, true);
    // 启用样式表背景
    setAttribute(Qt::WA_StyledBackground, true);
    // 启用鼠标跟踪
    //setMouseTracking(true);
    installEventFilter(this);

    // setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    // setAttribute(Qt::WA_DeleteOnClose);
    // // 默认大小
    // resize(400, 225);
    // // 启用鼠标追踪用于拖动
    // setMouseTracking(true);

    /// 取消画中画按键样式
    ui->pBt_back->setStyleSheet(R"(
    QPushButton
    {
        border-image: url(:/Skin/Images/soundTest/min_widget_pBt_back.png);
        border: none;
    }
    QPushButton:hover {
        border-image: url(:/Skin/Images/soundTest/min_widget_pBt_back_hover.png);
        border: none;
    }
    QPushButton:pressed
    {
        border-image: url(:/Skin/Images/soundTest/min_widget_pBt_back_press.png);
        border: none;
    }
)");
    ui->pBt_back->setCursor(QCursor(Qt::PointingHandCursor));//鼠标变成手型

    /// 关闭按键样式
    ui->pBt_close->setStyleSheet(R"(

    QPushButton
    {
        border-image: url(:/Skin/Images/soundTest/min_widget_close_btn.png);
        border: none;
    }
    QPushButton:hover {
        border-image: url(:/Skin/Images/soundTest/min_widget_close_btn_hover.png);
        border: none;
    }
    QPushButton:pressed
    {
        border-image: url(:/Skin/Images/soundTest/min_widget_close_btn_press.png);
        border: none;
    }
)");
    ui->pBt_close->setCursor(QCursor(Qt::PointingHandCursor));//鼠标变成手型

    /// 播放按键样式
    ui->pBt_play->setStyleSheet(R"(

    QPushButton
    {
        border-image: url(:/Skin/Images/soundTest/playing_btn.png);
        border: none;
    }
    QPushButton:checked
    {
        border-image: url(:/Skin/Images/soundTest/pause_btn.png);
        border: none;
    }
)");
    ui->pBt_play->setCursor(QCursor(Qt::PointingHandCursor));//鼠标变成手型

    /// 背景样式
    this->setObjectName("VideoHover");
    setStyleSheet(R"(

    QWidget#VideoHover
    {
        background-color: #0D0F14;
        /*border: 2px solid gray;*/
        border-radius: 8px;
    }
)");


}

// 获取可移动区域
QRect VideoHover::getMoveArea() const
{
    if (parentWidget()) {
        // 获取父窗口的几何区域
        QRect parentRect = parentWidget()->rect();

        // 根据要求设置移动边界
        // 左侧：父窗口x坐标 + 80px
        // 右侧：父窗口宽度 - 当前窗口宽度（确保不超出右边界）
        // 上侧：父窗口y坐标 + 30px
        // 下侧：父窗口高度 - 当前窗口高度
        return QRect(parentRect.left() + 80,
                     parentRect.top() + 30,
                     parentRect.width() - 80 - this->width(),
                     parentRect.height() - 30 - this->height());
    }
    return QRect();
}

QPointF VideoHover::getRelativePosition() const
{
    return m_relativePosition;
}

void VideoHover::setRelativePosition(const QPointF &relativePos)
{
    m_relativePosition = relativePos;
    // 确保在合理范围内
    m_relativePosition.setX(qBound(0.0, m_relativePosition.x(), 1.0));
    m_relativePosition.setY(qBound(0.0, m_relativePosition.y(), 1.0));
}
// 更新相对位置记录
void VideoHover::updatePositionRelativeToParent()
{
    if (!parentWidget())
        return;

    // 根据相对位置计算绝对位置
    QRect moveArea = getMoveArea();

    // 计算可用区域的宽度和高度
    int availableWidth = moveArea.width();
    int availableHeight = moveArea.height();

    // 根据相对位置计算坐标
    int x = moveArea.left() + availableWidth * m_relativePosition.x();
    int y = moveArea.top() + availableHeight * m_relativePosition.y();

    // 确保位置在移动区域内
    x = qBound(moveArea.left(), x, moveArea.right());
    y = qBound(moveArea.top(), y, moveArea.bottom());

    move(x, y);
}

void VideoHover::updateRelativePosition()
{
    if (!parentWidget())
        return;

    // 计算当前窗口相对于移动区域的相对位置
    QRect moveArea = getMoveArea();
    QPoint currentPos = this->pos();

    // 计算相对位置 (0.0 到 1.0)
    double relativeX = 0.0;
    double relativeY = 0.0;

    if (moveArea.width() > 0) {
        relativeX = (currentPos.x() - moveArea.left()) / (double) moveArea.width();
        relativeX = qBound(0.0, relativeX, 1.0);
    }

    if (moveArea.height() > 0) {
        relativeY = (currentPos.y() - moveArea.top()) / (double) moveArea.height();
        relativeY = qBound(0.0, relativeY, 1.0);
    }

    m_relativePosition = QPointF(relativeX, relativeY);
}
void VideoHover::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        m_dragging = true;
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void VideoHover::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        QPoint newPos = event->globalPos() - m_dragPosition;

        QRect moveArea = getMoveArea();

        if (parentWidget()) {
            if (newPos.x() < moveArea.left()) {
                newPos.setX(moveArea.left());
            } else if (newPos.x() > moveArea.right()) {
                newPos.setX(moveArea.right());
            }

            if (newPos.y() < moveArea.top()) {
                newPos.setY(moveArea.top());
            } else if (newPos.y() > moveArea.bottom()) {
                newPos.setY(moveArea.bottom());
            }
        }

        // 更新位置
        move(newPos);

        // 计算并保存相对位置
        if (parentWidget()) {
            QRect moveArea = getMoveArea();
            if (moveArea.width() > 0 && moveArea.height() > 0) {
                m_relativePosition.setX((newPos.x() - moveArea.left()) / (double) moveArea.width());
                m_relativePosition.setY((newPos.y() - moveArea.top()) / (double) moveArea.height());
            }
        }

        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void VideoHover::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void VideoHover::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    if (!currentImage.isNull()) {
        // 保持宽高比缩放显示
        QPixmap pixmap = QPixmap::fromImage(currentImage);
        QPixmap scaled = pixmap.scaled(ui->widget_wav->size(),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        painter.drawPixmap(0,
                           32,
                           ui->widget_wav->rect().width(),
                           ui->widget_wav->rect().height(),
                           scaled);
    }
}
