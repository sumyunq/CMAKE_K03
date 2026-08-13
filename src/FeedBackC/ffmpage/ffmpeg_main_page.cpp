#include "FeedBackC/ffmpage/ffmpeg_main_page.h"
#include "./ui_ffmpeg_main_page.h"

#include <QCoreApplication>
#include <limits>

FFmpegMainPage::FFmpegMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FFmpegMainPage)
{
    ui->setupUi(this);
    InitUIInformation();
    InitMember();
    InitConnect();
}

FFmpegMainPage::~FFmpegMainPage()
{
    delete ui;
}

void FFmpegMainPage::InitUIInformation()
{
    /// 上 -> 下
    /// 左 -> 右
    this->setMinimumSize(minWidth_, minHeight_);

    /// 小窗按键    当前固定为 32*32
    {
        pBn_min_widget_ = new QPushButton(this);
        pBn_min_widget_->setFixedSize(pBn_min_widget_minWidth_, pBn_min_widget_minHeight_);
        // pBn_min_widget_->setText("小窗按键"); /// 仅图标
        pBn_min_widget_->setStyleSheet(R"(
    QPushButton {
        border-image: url(:/Skin/Images/soundTest/video_play_page_min_widget.png);
        border: none;

    }
    QPushButton:hover {
        border-image: url(:/Skin/Images/soundTest/video_play_page_min_widget_hover.png);
        border: none;
    }
    QPushButton:pressed {
        border-image: url(:/Skin/Images/soundTest/video_play_page_min_widget_press.png);
        border: none;
    }

)");
        pBn_min_widget_x_ = this->geometry().width() - pBn_min_widget_->width()
                            - 20; /// 离右侧 20px
        pBn_min_widget_y_ = 12;   ///离上侧 12px
        pBn_min_widget_->move(pBn_min_widget_x_, pBn_min_widget_y_);

        pBn_min_widget_->setCursor(Qt::PointingHandCursor); /// 手型光标
    }

    /// 自定义进度条
    {
        slider_playing_progress_ = new QSliderPlayingProgress(this);
        // slider_playing_progress_->setParent(ui->widget);
        slider_playing_progress_->setEnabled(true);

        slider_playing_progress_->setFixedSize(this->geometry().width(),
                                               slider_playing_progress_minWidth_);
        slider_playing_progress_x_ = 0;
        slider_playing_progress_y_ = this->geometry().height() - slider_playing_progress_minWidth_
                                     - 54 + 2 - slider_playing_progress_minWidth_ / 2;
        slider_playing_progress_->move(slider_playing_progress_x_, slider_playing_progress_y_);

        slider_playing_progress_->setCursor(Qt::PointingHandCursor); /// 手型光标
    }

    /// 暂停按键    当前固定为 14*18
    {
        pBn_request_pause_ = new QPushButton(this);
        pBn_request_pause_->setFixedSize(pBn_request_pause_minWidth_, pBn_request_pause_minHeight_);
        // pBn_request_pause_->setText("暂停按键"); /// 仅图标
        pBn_request_pause_->setStyleSheet(R"(

    QPushButton {
        border-image: url(:/Skin/Images/soundTest/playing_btn.png);
        border: none;

    }
)");
        pBn_request_pause_x_ = 24;
        pBn_request_pause_y_ = this->geometry().height() - pBn_request_pause_->height() - 18;
        pBn_request_pause_->move(pBn_request_pause_x_, pBn_request_pause_y_);

        pBn_request_pause_->setCursor(Qt::PointingHandCursor); /// 手型光标
    }

    //// 播放时间 label
    {
        label_play_time_ = new QLabel(this);
        // label_play_time_->setText("播放时间");
        label_play_time_->setFixedSize(label_play_time_minWidth_, label_play_time_minHeight_);
        label_play_time_->setStyleSheet(R"(

    QLabel {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 12px;
        color: #C9CFD1;
    }

)");
        label_play_time_x_ = 54;
        label_play_time_y_ = this->geometry().height() - label_play_time_->height() - 19;
        label_play_time_->move(label_play_time_x_, label_play_time_y_);
    }

    /// 取消全屏 按键
    {
        pBn_exit_full_screen_ = new QPushButton(this);
        pBn_exit_full_screen_->setFixedSize(pBn_exit_full_screen_minWidth_,
                                            pBn_exit_full_screen_minHeight_);
        // pBn_exit_full_screen_->setText("取消全屏"); /// 仅图标
        pBn_exit_full_screen_->setStyleSheet(R"(

    QPushButton {
        border-image: url(:/Skin/Images/soundTest/video_play_page_exit_full_screen.png);
        border: none;

    }
    QPushButton:hover {
        border-image: url(:/Skin/Images/soundTest/video_play_page_exit_full_screen_hover.png);
        border: none;
    }
)");
        pBn_exit_full_screen_->setCursor(QCursor(Qt::PointingHandCursor)); //鼠标变成手型

        pBn_exit_full_screen_x_ = this->geometry().width() - pBn_exit_full_screen_->width()
                                  - 22; ///靠右侧22px
        pBn_exit_full_screen_y_ = this->geometry().height() - pBn_exit_full_screen_->height() - 16;
        pBn_exit_full_screen_->move(pBn_exit_full_screen_x_, pBn_exit_full_screen_y_);
        pBn_exit_full_screen_->hide(); /// 默认隐藏，全屏时显示
    }

    /// 全屏按键
    {
        pBn_full_screen_ = new QPushButton(this);
        pBn_full_screen_->setFixedSize(pBn_full_screen_minWidth_, pBn_full_screen_minHeight_);
        // pBn_full_screen_->setText("全屏按键"); /// 仅图标
        pBn_full_screen_->setStyleSheet(R"(

    QPushButton {
        border-image: url(:/Skin/Images/soundTest/video_play_page_full_screen.png);
        border: none;

    }
    QPushButton:hover {
        border-image: url(:/Skin/Images/soundTest/video_play_page_full_screen_hover.png);
        border: none;
    }

)");
        pBn_full_screen_x_ = this->geometry().width() - pBn_full_screen_->width()
                             - 22; ///靠右侧 22px
        pBn_full_screen_y_ = this->geometry().height() - pBn_full_screen_->height() - 16;
        pBn_full_screen_->move(pBn_full_screen_x_, pBn_full_screen_y_);
        pBn_full_screen_->setCursor(Qt::PointingHandCursor); /// 手型光标
    }
}

void FFmpegMainPage::InitMember()
{
    cl_ffmpeg_global_ = std::make_unique<FFmpegGlobal>();
    // minView = std::make_unique<VideoHover>();
}

void FFmpegMainPage::InitConnect()
{
    ///开始处理

    // /// 测试打开文件
    // QObject::connect(pBn_exit_full_screen_, &QPushButton::clicked, this, [=]() {
    //     QString file = QFileDialog::getOpenFileName(this, "Open Video");
    //     if (file.isEmpty())
    //         return;
    //     open(file.toUtf8());

    //     ///暂停状态下请求 ===》 恢复播放
    //     if (cl_ffmpeg_global_->cl_is_pause()) {
    //         cl_ffmpeg_global_.get()->pause(true);
    //     }
    // });

    /// 视频帧显示
    connect(cl_ffmpeg_global_->clp_sync_video_.get(),
            &FFmpegSyncThread::frameReady,
            this,
            &FFmpegMainPage::updateMainPage_Frame);

    /// 画中画（小窗口模式）
    QObject::connect(pBn_min_widget_, &QPushButton::clicked, this, [=]() {
        onMinWidgetSlots(true);
    });

    /// 暂停请求（同步小窗口按钮状态）
    QObject::connect(pBn_request_pause_, &QPushButton::clicked, this, [=]() {
        cl_ffmpeg_global_.get()->pause(true);

        if (is_minView_.load()) {
            minView->set_is_pause(cl_ffmpeg_global_->cl_is_pause());
        }
    });

    /// 更新播放时间
    QObject::connect(cl_ffmpeg_global_.get()->clp_sync_audio_.get(),
                     &FFmpegSyncThread::currentPlayingTimes,
                     this,
                     [=](double time) {
                         current_frame_pts_ = time;
                         slider_playing_progress_->setValue(static_cast<int>(time * 1000));
                         label_play_time_->setText(
                             GlobalTool::formatTime(time, cl_ffmpeg_global_->get_total_time()));
                         last_pts_elapsed_.restart();
                     });

    /// 进度条拖动时实时更新主画面（暂停/seek/恢复 由外部 SoundTestMainPage 处理）
    QObject::connect(slider_playing_progress_, &QSlider::sliderPressed, this, [=]() {
        is_slider_dragging_ = true;
        saved_playback_frame_ = currentImage;
    });

    QObject::connect(slider_playing_progress_,
                     &QSlider::sliderMoved,
                     this,
                     &FFmpegMainPage::onSliderDrag);

    QObject::connect(slider_playing_progress_, &QSlider::sliderReleased, this, [=]() {
        is_slider_dragging_ = false;
    });

    /// 循环播放检测：播放到末尾后自动从头开始
    loop_check_timer_ = new QTimer(this);
    loop_check_timer_->setInterval(600);
    QObject::connect(loop_check_timer_, &QTimer::timeout, this, [=]() {
        if (is_slider_dragging_ || cl_ffmpeg_global_->cl_is_pause()
            || cl_ffmpeg_global_->cl_is_stop()) {
            return;
        }
        if (!last_pts_elapsed_.isValid())
            return;
        /// PTS 超过 0.5 秒未更新，判定为播放结束，循环到开头
        if (last_pts_elapsed_.elapsed() > 500) {
            qDebug() << "检测到播放结束，循环播放";
            cl_ffmpeg_global_->seek(0);
        }
    });
    loop_check_timer_->start();

    /// 取消全屏
    QObject::connect(pBn_exit_full_screen_, &QPushButton::clicked, this, [=]() {
        // 恢复窗口标志（保留可调整大小和关闭按钮）
        setWindowFlags(Qt::Widget);
        // 重新显示窗口
        showNormal();
        emit fullScreen(false);

        pBn_exit_full_screen_->setEnabled(false);
        pBn_exit_full_screen_->hide();

        pBn_full_screen_->setEnabled(true);
        pBn_full_screen_->show();

        pBn_min_widget_->show();
    });

    /// 全屏显示
    QObject::connect(pBn_full_screen_, &QPushButton::clicked, this, [=]() {
        showNormal();
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        showFullScreen();
        emit fullScreen(true);
        pBn_exit_full_screen_->setEnabled(true);
        pBn_full_screen_->setEnabled(false);

        pBn_min_widget_->hide();
        pBn_full_screen_->hide();

        pBn_exit_full_screen_->show();
    });
}

QSliderPlayingProgress *FFmpegMainPage::slider_playing_progress() const
{
    return slider_playing_progress_;
}

void FFmpegMainPage::open(const char *url)
{
    /// 标记过渡状态：拒绝所有 frameReady 信号，防止前一个视频的延迟信号污染画面和缓存
    transitioning_.store(true);
    minView->setAcceptingFrames(false);

    /// 如果是正在播放的文件, 跳转至0开始播放
    if (cl_ffmpeg_global_->current_media_filename() == QString::fromUtf8(url)) {
        cl_ffmpeg_global_->seek(0);
    } else {
        cl_ffmpeg_global_->open(url);
    }

    /// 消费主线程事件队列中积压的前一个视频的 frameReady 信号（transitioning_ 为 true，会被拒绝）
    QCoreApplication::processEvents();

    /// 清空上一视频残留的当前帧 & 缩略图缓存 & 重置循环检测
    currentImage = QImage();
    saved_playback_frame_ = QImage();
    thumbnail_cache_.clear();
    last_sample_ms_ = -1;
    last_pts_elapsed_.invalidate();

    /// 更新 进度条（毫秒精度）
    slider_playing_progress_->setRange(0,
                                       static_cast<int>(cl_ffmpeg_global_->get_total_time() * 1000));

    ///暂停状态下请求 ===》 恢复播放
    if (cl_ffmpeg_global_->cl_is_pause()) {
        cl_ffmpeg_global_.get()->pause(true);
    }

    /// 解除过渡状态，开始接收新视频的帧
    minView->setAcceptingFrames(true);
    transitioning_.store(false);
}

void FFmpegMainPage::seek(double seconds)
{
    cl_ffmpeg_global_.get()->seek(seconds);
    // 不再自动恢复播放，由调用方（sliderPressed/Released 等）决定是否 resume
}

void FFmpegMainPage::pause()
{
    cl_ffmpeg_global_.get()->pause(true);
    if (is_minView_.load()) {
        minView->set_is_pause(cl_ffmpeg_global_->cl_is_pause());
    }
}

void FFmpegMainPage::onSliderDrag(int value)
{
    if (!is_slider_dragging_) {
        return;
    }

    QImage displayFrame;

    if (!thumbnail_cache_.isEmpty()) {
        /// value 是毫秒，查找最近的缩略图
        int nearestMs = -1;
        int minDiff = std::numeric_limits<int>::max();

        for (auto it = thumbnail_cache_.begin(); it != thumbnail_cache_.end(); ++it) {
            int diff = qAbs(it.key() - value);
            if (diff < minDiff) {
                minDiff = diff;
                nearestMs = it.key();
            }
        }

        if (nearestMs >= 0 && minDiff <= 3000) {
            displayFrame = thumbnail_cache_[nearestMs].toImage();
        }
    }

    /// 若无缩略图命中，回退到拖动前保存的画面
    if (displayFrame.isNull() && !saved_playback_frame_.isNull()) {
        displayFrame = saved_playback_frame_;
    }

    if (!displayFrame.isNull()) {
        currentImage = displayFrame;
        update();
    }
}

void FFmpegMainPage::onMinWidgetSlots(bool isMinWidgetShow)
{
    bool is_min_view = is_minView_.load(); ///记录一下当前状态

    if (isMinWidgetShow == is_min_view) {
        if (is_min_view) {
            minView->show();
            minView->raise();

        } else {
            minView->hide();
        }

        return; ///目标状态一致，不做任何处理
    } else {
        /// 目标状态不一致,根据 is_min_view 状态，向isMinWidgetShow做出改变
        if (is_min_view) {
            /// 小窗口模式下，向非小窗口模式改变
            QObject::connect(this->cl_ffmpeg_global_.get()->clp_sync_video_.get(),
                             &FFmpegSyncThread::frameReady,
                             this,
                             &FFmpegMainPage::updateMainPage_Frame);

            QObject::disconnect(this->cl_ffmpeg_global_.get()->clp_sync_video_.get(),
                                &FFmpegSyncThread::frameReady,
                                minView.get(),
                                &VideoHover::updateMinWidget_Frame);

            this->updateMainPage_Frame(minView->currentImage); ///更新最新帧
            minView->hide();
            is_minView_.store(false);

            emit EnableSmallWindowMode(false); ///非小窗口播放模式

        } else {
            /// 非小窗口模式下，向小窗口模式改变
            /// 更换图片帧显示窗口（this --> minView）
            QObject::disconnect(this->cl_ffmpeg_global_.get()->clp_sync_video_.get(),
                                &FFmpegSyncThread::frameReady,
                                this,
                                &FFmpegMainPage::updateMainPage_Frame);

            QObject::connect(this->cl_ffmpeg_global_.get()->clp_sync_video_.get(),
                             &FFmpegSyncThread::frameReady,
                             minView.get(),
                             &VideoHover::updateMinWidget_Frame);

            minView->updateMinWidget_Frame(currentImage); ///更新最新帧
            minView->set_is_pause(
                this->cl_ffmpeg_global_->cl_is_pause()); ///根据当前播放状态更新小窗口暂停按键状态
            minView->show();
            minView->raise();

            /// 恢复至主窗口播放
            QObject::connect(minView.get(), &VideoHover::restoreRequested, this, [=]() {
                QObject::connect(this->cl_ffmpeg_global_.get()->clp_sync_video_.get(),
                                 &FFmpegSyncThread::frameReady,
                                 this,
                                 &FFmpegMainPage::updateMainPage_Frame);

                QObject::disconnect(this->cl_ffmpeg_global_.get()->clp_sync_video_.get(),
                                    &FFmpegSyncThread::frameReady,
                                    minView.get(),
                                    &VideoHover::updateMinWidget_Frame);

                this->updateMainPage_Frame(minView->currentImage); ///更新最新帧
                minView->hide();
                is_minView_.store(false);

                emit EnableSmallWindowMode(false); ///非小窗口播放模式
            });
            /// 小窗口 控制暂停/播放（只在状态不一致时才 toggle）
            QObject::connect(
                minView.get(),
                &VideoHover::video_pause_requested,
                this,
                [=](bool is_pause) {
                    if (cl_ffmpeg_global_->cl_is_pause() != is_pause) {
                        cl_ffmpeg_global_->pause(true);
                    }
                },
                Qt::UniqueConnection);
            /// 小窗口关闭请求
            QObject::connect(
                minView.get(),
                &VideoHover::closeRequested,
                this,
                [=]() {
                    /// 非暂停状态，则暂停播放
                    if (!cl_ffmpeg_global_->cl_is_pause()) {
                        cl_ffmpeg_global_->pause(true);
                    }
                    minView->hide();
                },
                Qt::UniqueConnection);

            /// 更新 is_minView_
            is_minView_.store(true);
            emit EnableSmallWindowMode(true); ///小窗口播放模式,
        }
    }
}

void FFmpegMainPage::updateMainPage_Frame(const QImage &img)
{
    /// 视频切换过渡期间拒绝接收帧，防止前一个视频的延迟信号污染画面
    if (transitioning_.load()) {
        return;
    }

    int currentMs = static_cast<int>(current_frame_pts_ * 1000);

    /// 拖动进度条时不更新画面（由 onSliderDrag 控制显示）
    if (is_slider_dragging_) {
        /// 但仍然采样缩略图
        if (!img.isNull()) {
            if (currentMs - last_sample_ms_ >= THUMBNAIL_INTERVAL_MS) {
                last_sample_ms_ = currentMs;
                if (!thumbnail_cache_.contains(currentMs)) {
                    thumbnail_cache_[currentMs] = QPixmap::fromImage(
                        img.scaled(480, 270, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
        }
        return;
    }

    /// 图片帧为空，就显示默认图片/默认封面
    if (img.isNull()) {
        // currentImage = img;
    } else {
        currentImage = img;
    }

    /// 采样缩略图：每隔 THUMBNAIL_INTERVAL_MS 毫秒采样一帧
    if (currentMs - last_sample_ms_ >= THUMBNAIL_INTERVAL_MS) {
        last_sample_ms_ = currentMs;
        if (!img.isNull() && !thumbnail_cache_.contains(currentMs)) {
            thumbnail_cache_[currentMs] = QPixmap::fromImage(
                img.scaled(480, 270, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    update();
}

void FFmpegMainPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!currentImage.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(currentImage);
        painter.drawPixmap(rect(), pixmap);
    } else {
        painter.fillRect(rect(), Qt::black);
    }
    /// 改变图标
    if (cl_ffmpeg_global_->cl_is_pause()) {
        pBn_request_pause_->setStyleSheet(R"(

    QPushButton {
        border-image: url(:/Skin/Images/soundTest/pause_btn.png);
        border: none;

    }
)");
    } else {
        pBn_request_pause_->setStyleSheet(R"(

    QPushButton {
        border-image: url(:/Skin/Images/soundTest/playing_btn.png);
        border: none;

    }
)");
    }

    QWidget::paintEvent(event);
}

void FFmpegMainPage::resizeEvent(QResizeEvent *event)
{
    /// 更新一下对应的位置信息
    /// 小窗按键    当前固定为 32*32
    {
        pBn_min_widget_x_ = this->geometry().width() - pBn_min_widget_->width()
                            - 20; /// 离右侧 20px
        pBn_min_widget_y_ = 12;   ///离上侧 12px
        pBn_min_widget_->move(pBn_min_widget_x_, pBn_min_widget_y_);
    }

    /// 自定义进度条
    {
        slider_playing_progress_->setFixedSize(this->geometry().width(),
                                               slider_playing_progress_minWidth_);
        slider_playing_progress_x_ = 0;
        slider_playing_progress_y_ = this->geometry().height() - 54 + 2
                                     - slider_playing_progress_minWidth_ / 2; ///暂定居中看效果
        slider_playing_progress_->move(slider_playing_progress_x_, slider_playing_progress_y_);
    }

    /// 暂停按键    当前固定为 14*18
    {
        pBn_request_pause_x_ = 24;
        pBn_request_pause_y_ = this->geometry().height() - pBn_request_pause_->height() - 18;
        pBn_request_pause_->move(pBn_request_pause_x_, pBn_request_pause_y_);
    }

    //// 播放时间 label
    {
        label_play_time_x_ = 54;
        label_play_time_y_ = this->geometry().height() - label_play_time_->height() - 19;
        label_play_time_->move(label_play_time_x_, label_play_time_y_);
    }

    /// 取消全屏 按键
    {
        pBn_exit_full_screen_x_ = this->geometry().width() - pBn_exit_full_screen_->width()
                                  - 22; ///靠右侧 22 px
        pBn_exit_full_screen_y_ = this->geometry().height() - pBn_exit_full_screen_->height() - 16;
        pBn_exit_full_screen_->move(pBn_exit_full_screen_x_, pBn_exit_full_screen_y_);
    }

    /// 全屏按键
    {
        pBn_full_screen_x_ = this->geometry().width() - pBn_full_screen_->width()
                             - 22; ///靠右侧 22px
        pBn_full_screen_y_ = this->geometry().height() - pBn_full_screen_->height() - 16;
        pBn_full_screen_->move(pBn_full_screen_x_, pBn_full_screen_y_);
    }
}