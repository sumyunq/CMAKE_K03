#include "FeedBackC/SoundTest/SoundTestCustomUI/QWidget_video_info.h"
#include "ui_QWidget_video_info.h"

QWidgetVideoInfo::QWidgetVideoInfo(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QWidgetVideoInfo)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

QWidgetVideoInfo::~QWidgetVideoInfo()
{
    delete ui;
}

void QWidgetVideoInfo::DownLoadVideoStatusChange(DeSheng::VideoStatus newCl_downLoadVideoStatus)
{
    clp_video_mask_->cl_video_downLoad_pending_->setCl_downLoadVideoStatus(newCl_downLoadVideoStatus);
    update();
}

void QWidgetVideoInfo::DownloadProgressChange(double downloadProgerss)
{
    clp_video_mask_->cl_video_downLoad_pending_->setCl_download_progress(downloadProgerss);
}

void QWidgetVideoInfo::InitUIInformation()
{
    clp_video_mask_ = new QWidgetVideoMask(this); /// 内部遮罩，悬停时显示
    clp_video_mask_->resize(rect().width(), rect().height()); /// 内部遮罩，悬停时显示

}

void QWidgetVideoInfo::InitMember()
{
    cl_background_enlarge_anim_ = std::make_unique<QVariantAnimation>(this);    ///背景放大动画
    cl_background_enlarge_anim_->setDuration(300); ///动画时间

}

void QWidgetVideoInfo::InitConnect()
{
    /// 连接一下 clp_video_mask_ 部件中 cl_video_downLoad_pending_ 的 DownLoadVideoStatusChanged(DownLoadVideoStatus newCl_downLoadVideoStatus) 信号
    QObject::connect(clp_video_mask_->cl_video_downLoad_pending_,
                     &DownLoadVideoPending::DownLoadVideoStatusChanged,
                     this,
                     &QWidgetVideoInfo::DownLoadVideoStatusChange);

    ///
    connect(cl_background_enlarge_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                scaling_factor_ = value.value<double>();
                update();
            });
}

void QWidgetVideoInfo::drawUNDownLoad(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    // painter.setOpacity(1);
    painter.setBrush(QColor("#FFFFFF"));
    painter.drawRoundedRect (rect(),10,10);

    /// 未下载状态，默认显示遮罩（内部状态图标自行绘制）
    clp_video_mask_->show();

    painter.restore();
}

void QWidgetVideoInfo::drawDownLoading(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    // painter.setOpacity(1);
    painter.setBrush(QColor("#FFFFFF"));
    painter.drawRoundedRect (rect(),10,10);

    /// 下载中状态，默认显示遮罩（内部状态图标自行绘制）
    clp_video_mask_->show();

    painter.restore();
}

void QWidgetVideoInfo::drawDownLoaded(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), images_radius_, images_radius_);
    painter.setClipPath(clipPath);

    /// 下载完成状态，默认不显示遮罩（内部状态图标自行绘制）,显示背景封面图
    /// 悬停时显示遮罩，并执行放大背景图动画，底部不变，向三周放大
    if (is_entering) {
        clp_video_mask_->show();
    } else {
        clp_video_mask_->hide();
    }

    if (!clp_video_pixmap_.isNull()) {

        // 居中填满()
        QPixmap scaled = clp_video_pixmap_.scaled(size() * scaling_factor_,
                                               Qt::IgnoreAspectRatio,
                                               Qt::SmoothTransformation);
        int x = (width() - scaled.width()) / 2;

        int y = (height() - scaled.height());   ///底部对齐
        painter.drawPixmap(x, y, scaled);
    }

    // if (!clp_video_pixmap_->isNull()) {
    //     // 缩放填满（可能拉伸变形）
    //     // painter.drawPixmap(rect(), clp_video_pixmap_->scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    //     // 或保持比例居中()
    //     QPixmap scaled = scaled_pixmap_.scaled(size() * scaling_factor_,
    //                                                Qt::KeepAspectRatio,
    //                                                Qt::SmoothTransformation);
    //     int x = (width() - scaled.width()) / 2;

    //     int y = (height() - scaled.height());   ///底部对齐

    //     painter.drawPixmap(x, y, scaled);
    // }

    painter.restore();
}

QPixmap QWidgetVideoInfo::clp_video_pixmap() const
{
    return clp_video_pixmap_;
}

void QWidgetVideoInfo::setClp_video_pixmap(const QPixmap &newClp_video_pixmap)
{
    clp_video_pixmap_ = newClp_video_pixmap;
}

void QWidgetVideoInfo::setImages_radius(int newImages_radius)
{
    images_radius_ = newImages_radius;
    /// 同步更新遮罩的圆角
    clp_video_mask_->setRadius(newImages_radius);
}

void QWidgetVideoInfo::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    /// 根据 clp_video_mask_ 部件中 cl_video_downLoad_pending_ 的中间的视频状态图标 的内置状态 进行绘制
    switch (clp_video_mask_->cl_video_downLoad_pending_->cl_downLoadVideoStatus()) {
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
    QWidget::paintEvent(event);
}

void QWidgetVideoInfo::resizeEvent(QResizeEvent *event) {

    clp_video_mask_->resize(rect().width(), rect().height()); /// 内部遮罩，悬停时显示

}

void QWidgetVideoInfo::enterEvent(QEvent *event)
{
    is_entering = true;
    cl_background_enlarge_anim_->setKeyValues({{0.0, default_scaling_factor_}, {1.0, target_scaling_factor_}});
    cl_background_enlarge_anim_->start();
    update();
    QWidget::enterEvent(event);
}

void QWidgetVideoInfo::leaveEvent(QEvent *event)
{
    is_entering = false;
    cl_background_enlarge_anim_->setKeyValues({{0.0, target_scaling_factor_}, {1.0, default_scaling_factor_}});
    cl_background_enlarge_anim_->start();
    update();
    QWidget::leaveEvent(event);
}