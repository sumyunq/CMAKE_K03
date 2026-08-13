#include "FeedBackC/SoundTest/SoundTestCustomUI/single_video_info.h"
#include "ui_single_video_info.h"

SingleVideoInfo::SingleVideoInfo(ScrollAreaDisplayMode showMode, QWidget *parent)
    : cl_showMode(showMode)
    , QWidget(parent)
    , ui(new Ui::SingleVideoInfo)
{
    ui->setupUi(this);

    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

SingleVideoInfo::~SingleVideoInfo()
{
    delete ui;
}

void SingleVideoInfo::InitUIInformation()
{
    videoCover = new QWidgetVideoInfo(this);
    videoInfo = new QLabel(this);

    UpdateUI(cl_showMode); /// 根据模式去更新内部控件位置
}

void SingleVideoInfo::InitMember() {}

void SingleVideoInfo::InitConnect()
{
    ///  绑定 videoCover 内部的 clp_video_mask_->cl_minView_pBn_ 的点击事件
    QObject::connect(videoCover->clp_video_mask_->cl_minView_pBn_,
                     &QPushButton::clicked,
                     this,
                     [=]() {
                         /// 启用小窗口播放
                         emit openFileWithMinWidget(cl_video_item_->localPath);
                     });
}

void SingleVideoInfo::drawUNDownLoad(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.restore();
}

void SingleVideoInfo::drawDownLoading(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.restore();
}

void SingleVideoInfo::drawDownLoaded(QPainter &painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.restore();
}

void SingleVideoInfo::UpdateUI(ScrollAreaDisplayMode showMode)
{
    ///滑动区域 网格模式显示
    if (showMode == ScrollAreaDisplayMode::GridDisplay) {
        // qDebug() << "网格显示";

        ///整体
        minWidth = 238;
        minHeight = 174;
        maxWidth = 238;
        maxHeight = 174;

        ///显示widget
        videoCover_x = 0;
        videoCover_y = 0;
        videoCover_width = 238;
        videoCover_height = 134;
        videoCover_radius = 10;
        videoCover->styleSheet().clear();
        videoCover->setStyleSheet(
            R"(
QWidget{

border-radius: 10px;

}
)");

        ////显示label
        videoInfo_x = 20;
        videoInfo_y = 148;
        videoInfo_width = 100;
        videoInfo_height = 20;
        videoInfo->setStyleSheet(
            R"(
    QLabel{
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 14px;
        color: #A1A8B3;
    }

            )");
    }

    if (showMode == ScrollAreaDisplayMode::SingleColumnDisplay) {
        // qDebug() << "单列显示";
        ///整体
        minWidth = 212;
        minHeight = 155;
        maxWidth = 212;
        maxHeight = 155;

        ///显示widget
        videoCover_x = 0;
        videoCover_y = 0;
        videoCover_width = 212;
        videoCover_height = 120;
        videoCover_radius = 8;
        videoCover->styleSheet().clear();
        videoCover->setStyleSheet(
            R"(
QWidget{

border-radius: 8px;

}
)");

        ///信息label
        videoInfo_x = 0;
        videoInfo_y = 124;
        videoInfo_width = 100;
        videoInfo_height = 17;
        videoInfo->setStyleSheet(
            R"(
                QLabel{
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;

                }
            )");
    }

    ///整体
    this->setMinimumSize(minWidth, minHeight);
    this->setGeometry(QRect(0, 0, minWidth, minHeight));
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); /// 自动扩张

    ///显示widget
    videoCover->setImages_radius(videoCover_radius);
    videoCover->setGeometry(QRect(videoCover_x, videoCover_y, videoCover_width, videoCover_height));

    videoInfo->setGeometry(
        QRect(videoInfo_x, videoInfo_y, videoInfo_width, videoInfo_height)); ///默认宽度100，高20
    videoInfo->setText(infoText);
}

void SingleVideoInfo::setInfoText(const QString &newInfoText)
{
    infoText = newInfoText;
    videoInfo->setText(infoText);
}

int SingleVideoInfo::getMinWidth()
{
    return minWidth;
}

int SingleVideoInfo::getMinHeight()
{
    return minHeight;
}

void SingleVideoInfo::resizeEvent(QResizeEvent *event)
{
    ///忽略整体,只改变内部控件

    if (cl_showMode == ScrollAreaDisplayMode::GridDisplay) {
        ///显示widget
        videoCover_x = 0;
        videoCover_y = 0;
        videoCover_width = this->geometry().width();
        videoCover_height = this->geometry().height() - 40;

        ////显示label
        videoInfo_x = 20;
        videoInfo_y = this->geometry().height() - 40 + 14;
        videoInfo_width = 200;
        videoInfo_height = 20;
    }

    if (cl_showMode == ScrollAreaDisplayMode::SingleColumnDisplay) {
        ///显示widget
        videoCover_x = 0;
        videoCover_y = 0;
        videoCover_width = this->geometry().width();
        videoCover_height = this->geometry().height() - 35;

        ////显示label
        videoInfo_x = 0;
        videoInfo_y = this->geometry().height() - 35 + 4;
        videoInfo_width = 200;
        videoInfo_height = 20;
    }

    ///显示widget
    videoCover->setGeometry(QRect(videoCover_x, videoCover_y, videoCover_width, videoCover_height));

    ///显示label
    videoInfo->setGeometry(
        QRect(videoInfo_x, videoInfo_y, videoInfo_width, videoInfo_height)); ///默认宽度100，高20
}

void SingleVideoInfo::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 圆角裁剪路径
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect(), videoCover_radius, videoCover_radius);
    painter.setClipPath(clipPath);

    switch (cl_video_item_->localStatus) {
    case DeSheng::VideoStatus::Downloaded: {
        videoCover->DownLoadVideoStatusChange(cl_video_item_->localStatus);
        if (!cl_video_item_->coverLocalPath.isEmpty() && videoCover->clp_video_pixmap().isNull())
            videoCover->setClp_video_pixmap(QPixmap(cl_video_item_->coverLocalPath));

        drawDownLoaded(painter); ///绘制已下载状态
        break;
    }
    case DeSheng::VideoStatus::Downloading: {
        videoCover->DownLoadVideoStatusChange(cl_video_item_->localStatus);
        drawDownLoading(painter); ///绘制下载中状态
        break;
    }
    case DeSheng::VideoStatus::UnDownloaded:
    default: {
        videoCover->DownLoadVideoStatusChange(cl_video_item_->localStatus);
        drawUNDownLoad(painter); ///绘制未下载状态
        break;
    }
    }

    // if (fileName.isEmpty()) {

    //     /// 文件名为空：绘制默认矩形背景
    //     painter.fillRect(rect(), QColor(255,255, 255));
    //     painter.setPen(QPen(QColor(255, 255, 255), 2));

    // } else {
    //     /// 文件名不为空：显示背景图
    //     if (!image.isNull()) {
    //         // 绘制背景图，根据需求选择缩放模式
    //         // 拉伸填充（会变形）
    //         // painter.drawPixmap(rect(), cl_background_pixmap_);
    //         // 保持宽高比居中裁剪（覆盖）
    //         QPixmap scaled = image.scaled(size(),
    //                                                       Qt::KeepAspectRatioByExpanding,
    //                                                       Qt::SmoothTransformation);
    //         int x = (width() - scaled.width()) / 2;
    //         int y = (height() - scaled.height()) / 2;
    //         painter.drawPixmap(x, y, scaled);

    //     } else {
    //         /// 图片加载失败时的默认显示
    //         // painter.fillRect(rect(), QColor(200, 200, 200));
    //         // painter.drawText(rect(), Qt::AlignCenter, tr("加载失败"));
    //     }
    // }
}

void SingleVideoInfo::mouseReleaseEvent(QMouseEvent *event)
{
    /// 左键，且视频文件已下载（进行播放）
    if (!cl_video_item_->localPath.isEmpty() && event->button() == Qt::LeftButton) {
        ///区域检测(中心点 40x40)
        QRect targetRect(videoCover->rect().center().x() - 20,
                         videoCover->rect().center().y() - 20,
                         40,
                         40);
        if (targetRect.contains(event->pos())) {
            emit openFile(cl_video_item_->localPath);
        }

        /// 小窗口播放点位
        // // rect().width() - cl_minView_pBn_->width() - 6, rect().y() + 6
        // QRect targetRect_min(videoCover->rect ().width () - 32 - 6,videoCover->rect ().y () + 6, 32, 32);
        // if (targetRect_min.contains (event->pos ())) {
        //     emit openFile(cl_video_item_->localPath);
        // }
    }

    /// 左键，且视频文件未下载（请求下载）
    if (cl_video_item_->localPath.isEmpty() && event->button() == Qt::LeftButton) {
        ///区域检测(中心点 40x40)
        QRect targetRect(videoCover->rect().center().x() - 20,
                         videoCover->rect().center().y() - 20,
                         40,
                         40);
        if (targetRect.contains(event->pos())) {
            emit requestDownLoadVideoFile();
        }
    }

    QWidget::mousePressEvent(event);
}

void SingleVideoInfo::setDownloadProgress(double downloadProgress)
{
    videoCover->DownloadProgressChange(downloadProgress); ///进度更新
}

void SingleVideoInfo::setCoverLocalPath(QString coverLocalPath)
{
    videoCover->setClp_video_pixmap(QPixmap(coverLocalPath));
}
