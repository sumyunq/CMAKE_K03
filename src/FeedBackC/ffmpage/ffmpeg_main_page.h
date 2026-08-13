#ifndef FFMPEG_MAIN_PAGE_H
#define FFMPEG_MAIN_PAGE_H

#include <QPushButton>
#include <QWidget>

#include "VideoHover.h" ///小窗口

#include "FeedBackC/ffmpage/CustomUI/QSlider_playing_progress.h" ///进度条

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"
#include "FeedBackC/ffmpage/public_space.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class FFmpegMainPage;
}
QT_END_NAMESPACE

class FFmpegMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit FFmpegMainPage(QWidget *parent = nullptr);
    ~FFmpegMainPage() override;

    void open(const char *url);
    void seek(double seconds);
    void pause();

public slots:
    void updateMainPage_Frame(const QImage &img);
    void onSliderDrag(int value);
    void onMinWidgetSlots(bool isMinWidgetShow = true);    ///是否启用 画中画(默认是)


signals:
    void fullScreen(bool isFullScreen);        ///是否全屏
    void EnableSmallWindowMode(bool isEnable); ///是否小窗口模式(用于播放视口切换至均衡器界面)

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    std::atomic<bool> is_minView_{false};   /// 默认非小窗模式
    std::shared_ptr<VideoHover> minView = nullptr; /// 小窗口播放

    std::unique_ptr<FFmpegGlobal> cl_ffmpeg_global_ = nullptr;

    QSliderPlayingProgress *slider_playing_progress() const;

private:
    Ui::FFmpegMainPage *ui;

    QImage currentImage;
    double current_frame_pts_ = 0.0; ///当前Frame的PTS

    /// 缩略图缓存 (key: 毫秒数, value: 缩略图)
    QMap<int, QPixmap> thumbnail_cache_;
    int last_sample_ms_ = -1;                          ///上次采样的毫秒数(避免重复采样)
    static constexpr int THUMBNAIL_INTERVAL_MS = 1000; ///采样间隔(毫秒)
    bool is_slider_dragging_ = false;                  ///是否正在拖动进度条
    QImage saved_playback_frame_;                      ///拖动前保存的播放画面，用于释放时恢复

    /// 循环播放检测
    QTimer *loop_check_timer_ = nullptr;
    QElapsedTimer last_pts_elapsed_;

    /// 视频切换过渡期间拒绝接收帧（防止前一个视频的延迟信号污染画面）
    std::atomic<bool> transitioning_{false};

    ///********************************************************** UI 相关 **********************************************************///
    /// 整体widget属性
    std::atomic<int> minWidth_ = 751;  ///最小宽度
    std::atomic<int> minHeight_ = 422; ///最小高度
    std::atomic<int> maxWidth_ = 751;  ///最大宽度
    std::atomic<int> maxHeight_ = 422; ///最大高度

    /// 小窗按键 固定 32*32
    QPushButton *pBn_min_widget_ = nullptr;          ///实例化,后期可能需要动态效果，直接替换构造类
    std::atomic<int> pBn_min_widget_x_ = 0;          ///x
    std::atomic<int> pBn_min_widget_y_ = 0;          ///y
    std::atomic<int> pBn_min_widget_minWidth_ = 32;  ///最小宽度
    std::atomic<int> pBn_min_widget_minHeight_ = 32; ///最小高度
    std::atomic<int> pBn_min_widget_maxWidth_ = 32;  ///最大宽度
    std::atomic<int> pBn_min_widget_maxHeight_ = 32; ///最大高度

    /// 进度条
    QSliderPlayingProgress *slider_playing_progress_ = nullptr;
    std::atomic<int> slider_playing_progress_x_ = 0;         ///x
    std::atomic<int> slider_playing_progress_y_ = 0;         ///y
    std::atomic<int> slider_playing_progress_minWidth_ = 30; ///固定高度
    // std::atomic<int> slider_playing_progress_minHeight_ = 90; ///最小高度
    // std::atomic<int> slider_playing_progress_maxWidth_ = 90; ///最大宽度
    // std::atomic<int> slider_playing_progress_maxHeight_ = 90; ///最大高度

    /// 暂停按键
    QPushButton *pBn_request_pause_ = nullptr;
    std::atomic<int> pBn_request_pause_x_ = 0;          ///x
    std::atomic<int> pBn_request_pause_y_ = 0;          ///y
    std::atomic<int> pBn_request_pause_minWidth_ = 14;  ///最小宽度
    std::atomic<int> pBn_request_pause_minHeight_ = 18; ///最小高度
    std::atomic<int> pBn_request_pause_maxWidth_ = 14;  ///最大宽度
    std::atomic<int> pBn_request_pause_maxHeight_ = 18; ///最大高度

    /// 播放时间 label
    QLabel *label_play_time_ = nullptr;
    std::atomic<int> label_play_time_x_ = 0;          ///x
    std::atomic<int> label_play_time_y_ = 0;          ///y
    std::atomic<int> label_play_time_minWidth_ = 53;  ///最小宽度
    std::atomic<int> label_play_time_minHeight_ = 17; ///最小高度
    std::atomic<int> label_play_time_maxWidth_ = 53;  ///最大宽度
    std::atomic<int> label_play_time_maxHeight_ = 17; ///最大高度

    /// 取消全屏 按键
    QPushButton *pBn_exit_full_screen_ = nullptr;
    std::atomic<int> pBn_exit_full_screen_x_ = 0;          ///x
    std::atomic<int> pBn_exit_full_screen_y_ = 0;          ///y
    std::atomic<int> pBn_exit_full_screen_minWidth_ = 22;  ///最小宽度
    std::atomic<int> pBn_exit_full_screen_minHeight_ = 22; ///最小高度
    std::atomic<int> pBn_exit_full_screen_maxWidth_ = 22;  ///最大宽度
    std::atomic<int> pBn_exit_full_screen_maxHeight_ = 22; ///最大高度

    /// 全屏按键
    QPushButton *pBn_full_screen_ = nullptr;
    std::atomic<int> pBn_full_screen_x_ = 0;          ///x
    std::atomic<int> pBn_full_screen_y_ = 0;          ///y
    std::atomic<int> pBn_full_screen_minWidth_ = 22;  ///最小宽度
    std::atomic<int> pBn_full_screen_minHeight_ = 22; ///最小高度
    std::atomic<int> pBn_full_screen_maxWidth_ = 22;  ///最大宽度
    std::atomic<int> pBn_full_screen_maxHeight_ = 22; ///最大高度

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};
#endif // FFMPEG_MAIN_PAGE_H
