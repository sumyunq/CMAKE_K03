#include "FeedBackC/SoundTest/SoundTestCustomUI/QWidget_video_mask.h"
#include "ui_QWidget_video_mask.h"

QWidgetVideoMask::QWidgetVideoMask(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWidgetVideoMask)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

QWidgetVideoMask::~QWidgetVideoMask()
{
    delete ui;
}

void QWidgetVideoMask::InitUIInformation()
{
    cl_minView_pBn_ = new QPushButton(this); ///右上角小窗口播放按键，下载完成后显示
    cl_minView_pBn_->setFixedSize(32, 32);
    // cl_minView_pBn_->setText("小窗口播放");   ///仅显示图标
    cl_minView_pBn_->move(rect().width() - cl_minView_pBn_->width() - 6, rect().y() + 6);
    cl_minView_pBn_->setStyleSheet(R"(

    QPushButton {
        border-image: url(:/Skin/Images/soundTest/single_video_info_btn.png);
        border: none;
    }
    QPushButton:hover {
        border-image: url(:/Skin/Images/soundTest/single_video_info_btn_hover.png);
        border: none;
    }
    QPushButton:pressed {
        border-image: url(:/Skin/Images/soundTest/single_video_info_btn_press.png);
        border: none;
    }

    )");
    // cl_minView_pBn_->setScaledContents(true);
    cl_minView_pBn_->setCursor(Qt::PointingHandCursor); // 手型光标
    cl_minView_pBn_->hide();

    /// 加载图标
    cl_video_downLoad_pending_ = new DownLoadVideoPending(this); /// 中间的视频状态图标(默认未下载)
    cl_video_downLoad_pending_->move(rect().center().x() - cl_video_downLoad_pending_->width() / 2,
                                     rect().center().y() - cl_video_downLoad_pending_->height() / 2);
}

void QWidgetVideoMask::InitMember() {}

void QWidgetVideoMask::InitConnect() {}

void QWidgetVideoMask::drawUNDownLoad(QPainter &painter)
{
    /// 未下载状态：整体绘制背景阴影、绘制静态下载图标（正中心，其内部自行绘制）
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    /// 绘制背景阴影)
    painter.setPen(Qt::NoPen);

    // painter.setOpacity(background_opacity_unDownLoad); ///遮罩背景透明度
    // painter.setBrush(cl_shadow_color_);
    // painter.drawRoundedRect(rect(), radius, radius); ///绘制圆角矩形
    // painter.setOpacity(1.0);                         /// 恢复

    QPixmap icon(":/Skin/Images/soundTest/waiting_downloaded_mask.png");
    painter.drawPixmap(rect(), icon);

    /// 按键隐藏
    cl_minView_pBn_->hide();

    painter.restore();
}

void QWidgetVideoMask::drawDownLoading(QPainter &painter)
{
    /// 下载中状态：整体绘制背景阴影、绘制动态下载图标（正中心，其内部自行绘制）
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    // /// 绘制背景阴影)
    // painter.setPen(Qt::NoPen);
    // painter.setOpacity(background_opacity_downLoading);
    // painter.setBrush(cl_shadow_color_);
    // painter.drawRoundedRect(rect(), radius, radius); ///绘制圆角矩形
    // painter.setOpacity(1.0);                         /// 恢复

    QPixmap icon(":/Skin/Images/soundTest/waiting_downloaded_mask.png");
    painter.drawPixmap(rect(), icon);

    /// 按键隐藏
    cl_minView_pBn_->hide();
    painter.restore();
}

void QWidgetVideoMask::drawDownLoaded(QPainter &painter)
{
    /// 已下载：整体绘制背景阴影、绘制播放图标（正中心，其内部自行绘制）,右上角小窗口按键显示
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    // /// 绘制背景阴影)
    // painter.setPen(Qt::NoPen);
    // painter.setOpacity(background_opacity_downLoaded);
    // painter.setBrush(cl_shadow_color_);
    // painter.drawRoundedRect(rect(), radius, radius); ///绘制圆角矩形
    // painter.setOpacity(1.0);                         /// 恢复

    QPixmap icon(":/Skin/Images/soundTest/video_downloadeded_hover_mask_grid.png");
    painter.drawPixmap(rect(), icon);

    /// 显示小窗口播放按键
    cl_minView_pBn_->show();

    painter.restore();
}

void QWidgetVideoMask::setRadius(int newRadius)
{
    radius = newRadius;
}

void QWidgetVideoMask::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    /// 根据 中间的视频状态图标 的内置状态 进行绘制
    switch (cl_video_downLoad_pending_->cl_downLoadVideoStatus()) {
    case DeSheng::VideoStatus::Downloaded: {
        drawDownLoaded(painter);
        break;
    }
    case DeSheng::VideoStatus::Downloading: {
        drawDownLoading(painter);
        break;
    }
    case DeSheng::VideoStatus::UnDownloaded:
    default: {
        drawUNDownLoad(painter);
        break;
    }
    }
}

void QWidgetVideoMask::resizeEvent(QResizeEvent *event)
{
    cl_minView_pBn_->move(rect().width() - cl_minView_pBn_->width() - 6, rect().y() + 6);
    cl_video_downLoad_pending_->move(rect().center().x() - cl_video_downLoad_pending_->width() / 2,
                                     rect().center().y() - cl_video_downLoad_pending_->height() / 2);
}