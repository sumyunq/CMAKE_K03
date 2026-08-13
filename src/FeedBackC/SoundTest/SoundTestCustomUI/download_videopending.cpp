#include "FeedBackC/SoundTest/SoundTestCustomUI/download_videopending.h"
#include "ui_download_videopending.h"

DownLoadVideoPending::DownLoadVideoPending(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DownLoadVideoPending)
{
    ui->setupUi(this);

    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

DownLoadVideoPending::~DownLoadVideoPending()
{
    delete ui;
}

DeSheng::VideoStatus DownLoadVideoPending::cl_downLoadVideoStatus() const
{
    return cl_downLoadVideoStatus_;
}

void DownLoadVideoPending::setCl_downLoadVideoStatus(DeSheng::VideoStatus newCl_downLoadVideoStatus)
{
    cl_downLoadVideoStatus_ = newCl_downLoadVideoStatus;
    update();
}

void DownLoadVideoPending::InitUIInformation()
{
    // setMinimumSize(QSize(40, 40));
    setFixedSize(QSize(40, 40));

    cl_label_icon_ = new QLabel(this);
    cl_label_icon_->setMinimumSize(QSize(40, 40));
    cl_label_icon_->move(0, 0);
    // cl_label_icon_->setStyleSheet("background-color: rgb(255, 0, 0);border-color: rgb(255, 0, 0);");

    cl_label_icon_->setScaledContents(true);
    cl_label_icon_->setCursor(Qt::PointingHandCursor);  // 手型光标

    cl_movie_ = std::make_unique<QMovie>(":/Skin/Images/listen/DownLoadVideoPending.gif");

}

void DownLoadVideoPending::InitMember()
{
    // 创建定时器
    // cl_download_timer_ = new QTimer(this);

    // cl_downLoadVideoStatus_ = DownLoadVideoStatus::DownLoaded;

    // 连接信号槽
    // connect(cl_download_timer_, &QTimer::timeout, this, [=]() {
    //     if (cl_download_progress_ > 100) {
    //         cl_download_progress_ = 0.0;
    //         ///状态更新
    //         if (cl_downLoadVideoStatus_ == DownLoadVideoStatus::UNDownLoad) {
    //             cl_downLoadVideoStatus_ = DownLoadVideoStatus::DownLoading;
    //         } else if (cl_downLoadVideoStatus_ == DownLoadVideoStatus::DownLoading) {
    //             cl_downLoadVideoStatus_ = DownLoadVideoStatus::DownLoaded;
    //         } else if(cl_downLoadVideoStatus_ == DownLoadVideoStatus::DownLoaded){
    //             cl_downLoadVideoStatus_ = DownLoadVideoStatus::UNDownLoad;
    //         }

    //         emit DownLoadVideoStatusChanged(cl_downLoadVideoStatus_);
    //     } else {
    //         cl_download_progress_ += 1;
    //     }
    //     update();
    // });

    // 设置间隔（毫秒），例如每 100ms 增加 1
    // cl_download_timer_->setInterval(10);

    // 启动定时器
    // cl_download_timer_->start();
}

void DownLoadVideoPending::InitConnect() {}

void DownLoadVideoPending::drawUNDownLoad(QPainter &painter)
{
    /// 默认状态,绘制默认背景和下载图标
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    // /// 绘制圆的背景)
    // painter.setPen(Qt::NoPen);
    // painter.setBrush(cl_default_background_color_);
    // painter.drawEllipse(rect());

    /// 绘制中间图标(更新图标)
    cl_label_icon_->setStyleSheet("");
    cl_label_icon_->setStyleSheet (R"(
    /* 默认图标 */
    QLabel {
        image: url(:/Skin/Images/soundTest/waiting_downloaded.png);
    }
    /* 悬停图标 */
    QLabel:hover {
        image: url(:/Skin/Images/soundTest/waiting_downloaded_hover.png);
    }
)");   ///未下载时默认图标和悬停图标

    /// 关闭动画 ///先不关闭，如果用户出现图标重叠，即表示网络问题
    // if (cl_movie_->state() == QMovie::Running) {
    //     cl_movie_->stop();
    // }

    painter.restore();
}

void DownLoadVideoPending::drawDownLoading(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    /// 内圆
    QRect targetRect = rect().adjusted(cl_lineWidth_,
                                       cl_lineWidth_,
                                       -cl_lineWidth_,
                                       -cl_lineWidth_); ///用于绘制进度圆弧

    /// 绘制圆的背景)
    QPainterPath path_background;
    path_background.moveTo(QPointF(rect().center()));                 ///移动到中心
    path_background.lineTo(QPointF(rect().center().x(), rect().y())); ///直线到顶部
    path_background.arcTo(rect(), 90, 361);
    path_background.lineTo(QPointF(rect().center())); ///直线到中心
    /// 闭合填充
    painter.setPen(Qt::NoPen);
    painter.setBrush(cl_default_background_color_);
    painter.drawPath(path_background);

    /// 绘制扇形圆弧进度
    QPainterPath path;
    path.moveTo(QPointF(rect().center()));                 ///移动到中心
    path.lineTo(QPointF(rect().center().x(), rect().y())); ///直线到顶部
    if (cl_download_progress_ > 0) {
        path.arcTo(rect(), 90, -(cl_download_progress_ * 3.6));
    }
    // path.arcTo(rect(), 90, -(cl_download_progress_ * 3.6)); ///圆弧计算未完成的百分比
    path.lineTo(QPointF(rect().center())); ///直线到中心
    /// 闭合填充
    painter.setPen(Qt::NoPen);
    painter.setBrush(cl_fill_color_);
    painter.drawPath(path);

    /// 绘制内圆
    painter.setBrush(cl_default_background_color_);
    painter.drawPie(targetRect, 90, 360 * 16);

    /// 绘制中间图标(更新图标)
    /// 清空样式表
    cl_label_icon_->setStyleSheet("");

    cl_label_icon_->setMovie(cl_movie_.get ());
    cl_movie_->setScaledSize(cl_label_icon_->size());
    // cl_movie_->setSpeed(50); // 50% = 0.5 倍数
    /// 启动动画
    if (cl_movie_->state() != QMovie::Running) {
        cl_movie_->start();
    }

    painter.restore();
}

void DownLoadVideoPending::drawDownLoaded(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    /// 清空样式表
    cl_label_icon_->setStyleSheet("");
    cl_label_icon_->setMovie(nullptr); // 或 ui->label_icon->clear()
    /// 关闭动画
    if (cl_movie_->state() == QMovie::Running) {
        cl_movie_->stop();
    }

    /// 绘制中间图标(更新图标)
    /// 交由父级窗口取绘制

    // cl_label_icon_->setText("已下载");

    painter.restore();
}

double DownLoadVideoPending::cl_download_progress() const
{
    return cl_download_progress_;
}

void DownLoadVideoPending::setCl_download_progress(double newCl_download_progress)
{
    cl_download_progress_ = newCl_download_progress;
    update(); /// 更新绘制
}

void DownLoadVideoPending::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    switch (cl_downLoadVideoStatus_) {
    case DeSheng::VideoStatus::Downloading: {
        drawDownLoading(painter);
        break;
    }
    case DeSheng::VideoStatus::Downloaded: {
        drawDownLoaded(painter);
        break;
    }
    case DeSheng::VideoStatus::UnDownloaded:
    default: {
        drawUNDownLoad(painter);
        break;
    }
    }
}

void DownLoadVideoPending::resizeEvent(QResizeEvent *event)
{
    // /// 图标label
    // cl_label_width_ = this->geometry().width() / 2;
    // cl_label_height_ = this->geometry().height() / 2;
    // cl_label_x_ = (this->geometry().width() - cl_label_width_) / 2;
    // cl_label_y_ = (this->geometry().height() - cl_label_height_) / 2;

    // /// 图标label
    // cl_label_icon_->setGeometry(QRect(cl_label_x_, cl_label_y_, cl_label_width_, cl_label_height_));


}