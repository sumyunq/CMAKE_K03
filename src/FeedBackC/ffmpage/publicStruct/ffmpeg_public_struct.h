#ifndef FFMPEG_PUBLIC_STRUCT_H
#define FFMPEG_PUBLIC_STRUCT_H

#include "FeedBackC/ffmpage/public_space.h"

///
/// ffmpeg 相关结构体封装
///

extern "C" {
#include <libavcodec/avcodec.h>

#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavutil/avstring.h"
#include "libavutil/bprint.h"
#include "libavutil/channel_layout.h"
#include "libavutil/dict.h"
#include "libavutil/fifo.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"
#include "libavutil/pixdesc.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/tx.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>

#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
}

#include "FeedBackC/ffmpage/publicStruct/AVFrame_queue.h"
#include "FeedBackC/ffmpage/publicStruct/AVPacket_queue.h"


#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE \
    FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct AudioParams
{
    int freq;
    AVChannelLayout ch_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

typedef struct FrameData
{
    std::atomic<int64_t> pkt_pos;
} FrameData;

struct AVCodecContextDeleter
{
    void operator()(AVCodecContext *ptr) const
    {
        // qDebug() << " AVCodecContextDeleter 析构";
        if (ptr)
            avcodec_free_context(&ptr);
    }
};
using AVCodecContextUniquePtr = std::unique_ptr<AVCodecContext, AVCodecContextDeleter>;

typedef struct Decoder
{
    AVPacketUniquePtr pkt_ = nullptr;
    std::unique_ptr<AVPacketQueue> queue_ = nullptr;

    mutable QMutex avctx_Mutex_;
    AVCodecContextUniquePtr avctx_ = nullptr;
    std::atomic<int> pkt_serial_;
    std::atomic<int> finished_;
    std::atomic<int> packet_pending_;
    // QWaitCondition *empty_queue_cond_;
    std::atomic<int64_t> start_pts_;
    AVRational start_pts_tb_;
    std::atomic<int64_t> next_pts_;
    AVRational next_pts_tb_;

} Decoder;

namespace ForFFmpeg {
///暂时 用于实现 FFmpeg 多线程,这个位置用于存放共享变量

// 音频时钟（单位：秒），由音频播放线程更新
// std::atomic<double> m_audioClock{0.0};

// 同步阈值
const double AV_SYNC_THRESHOLD_MIN = 0.04; // 40ms
const double AV_SYNC_THRESHOLD_MAX = 0.1; // 100ms
const double AV_SYNC_FRAMEDUP_THRESHOLD = 0.1; // 100ms
const double AV_NOSYNC_THRESHOLD = 10.0; // 10s，

/// 时钟同步类型
enum class AVSyncType {
    AudioMaster, // 0:音频作为主时钟
    VideoMaster, // 1:视频作为主时钟
    ExternalClock // 2：外部时钟作为主时钟，一般是系统时钟
};

// enum {
//     AV_SYNC_AUDIO_MASTER, /* default choice */
//     AV_SYNC_VIDEO_MASTER,
//     AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
// };

} // namespace ForFFmpeg

///
/// \brief The Clock class
/// 原子操作去保证数据一致性
class Clock
{
public:
    double get_clock()
    {
        QMutexLocker locker(&clock_mutex);
        if (paused_.load()) {
            return pts_.load();
        } else {
            double time = av_gettime_relative() / 1000000.0;
            return pts_drift_.load() + time - (time - last_updated_.load()) * (1.0 - speed_.load());
        }
    }

    void set_clock_at(double pts, int serial, double time)
    {
        QMutexLocker locker(&clock_mutex);
        pts_.store(pts);
        last_updated_.store(time);
        pts_drift_.store(pts_.load() - time);
        serial_.store(serial);
    }

    void set_clock(double pts, int serial)
    {
        double time = av_gettime_relative() / 1000000.0;
        this->set_clock_at(pts, serial, time);
    }

    void set_clock_speed(double speed)
    {
        set_clock(get_clock(), serial_.load());
        speed_.store(speed);
    }

    void init_clock()
    {
        speed_.store(1.0);
        paused_.store(0);
        // c->queue_serial = queue_serial;
        set_clock(NAN, -1); /// 初始化序列号为 -1
    }
    ///同步到指定时钟
    void sync_clock_to_slave(Clock *slave)
    {
        double clock = get_clock();
        double slave_clock = slave->get_clock();
        if (!isnan(slave_clock)
            && (isnan(clock) || fabs(clock - slave_clock) > ForFFmpeg::AV_NOSYNC_THRESHOLD))
            set_clock(slave_clock, slave->serial_);
    }
    ///确定主时钟类型
    // int get_master_sync_type(VideoState *is)
    // {
    //     if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
    //         if (is->video_st)
    //             return AV_SYNC_VIDEO_MASTER;
    //         else
    //             return AV_SYNC_AUDIO_MASTER;
    //     } else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
    //         if (is->audio_st)
    //             return AV_SYNC_AUDIO_MASTER;
    //         else
    //             return AV_SYNC_EXTERNAL_CLOCK;
    //     } else {
    //         return AV_SYNC_EXTERNAL_CLOCK;
    //     }
    // }

    /* get the current master clock value */
    // static double get_master_clock(VideoState *is)
    // {
    //     double val;

    //     switch (get_master_sync_type(is)) {
    //     case AV_SYNC_VIDEO_MASTER:
    //         val = get_clock(&is->vidclk);
    //         break;
    //     case AV_SYNC_AUDIO_MASTER:
    //         val = get_clock(&is->audclk);
    //         break;
    //     default:
    //         val = get_clock(&is->extclk);
    //         break;
    //     }
    //     return val;
    // }

    // static void check_external_clock_speed(VideoState *is) {
    //     if (is->video_stream >= 0 && is->videoq.nb_packets <= ForFFmpeg::EXTERNAL_CLOCK_MIN_FRAMES ||
    //         is->audio_stream >= 0 && is->audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES) {
    //         set_clock_speed(&is->extclk, FFMAX(EXTERNAL_CLOCK_SPEED_MIN, is->extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));
    //     } else if ((is->video_stream < 0 || is->videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) &&
    //                (is->audio_stream < 0 || is->audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES)) {
    //         set_clock_speed(&is->extclk, FFMIN(EXTERNAL_CLOCK_SPEED_MAX, is->extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));
    //     } else {
    //         double speed = is->extclk.speed;
    //         if (speed != 1.0)
    //             set_clock_speed(&is->extclk, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
    //     }
    // }

    void setPaused(bool is_paused)
    {
        paused_.store(is_paused);
        set_clock(pts_.load(), serial_.load());
    }

private:
    QMutex clock_mutex; ///用于保证数据一致性
    std::atomic<double> pts_{0.0}; ///时钟基准,对标 pts
    std::atomic<double> pts_drift_{0.0}; ///时钟基准减去上次更新时钟的时间
    std::atomic<double> last_updated_{0.0}; ///上次更新时间
    std::atomic<double> speed_{0.0}; /// 时间流逝速度，可能用于加速
    std::atomic<int> serial_{
        0}; ///对齐序列号,校验当前的压缩数据包队列、缓存帧队列是否有效     ///时钟基于具有此序列号的数据包
    std::atomic<bool> paused_{false}; ///时钟是否暂停，用于实现暂停操作

    // int *queue_serial;    ///这个暂时不需要,这边可以确保全局唯一 /* 指向当前数据包队列序列号的指针，用于检测过时的时钟 */

    /// ffplay参考时钟
    // typedef struct Clock {
    //     double pts;           /* 时钟基准,标准 pts */
    //     double pts_drift;     /* 时钟基准减去上次更新时钟的时间 */
    //     double last_updated;
    //     double speed;
    //     int serial;           /* 时钟基于具有此序列号的数据包 */
    //     int paused;
    //     int *queue_serial;    /* 指向当前数据包队列序列号的指针，用于检测过时的时钟 */
    // } Clock;

    // typedef struct Clock
    // {
    //     double pts;       /* clock base */
    //     double pts_drift; /* clock base minus time at which we updated the clock */
    //     double last_updated;
    //     double speed;
    //     int serial; /* clock is based on a packet with this serial */
    //     int paused;
    //     int *queue_serial; /* pointer to the current packet queue serial, used for obsolete clock detection */
    // } Clock;
};


typedef struct MyVideoState
{
    // SDL_Thread *read_tid;
    const AVInputFormat *iformat;
    std::atomic<int> abort_request;
    std::atomic<int> force_refresh;
    std::atomic<int> paused;
    std::atomic<int> last_paused;
    std::atomic<int> queue_attachments_req;
    std::atomic<int> seek_req;
    std::atomic<int> seek_flags;
    std::atomic<int64_t> seek_pos;
    std::atomic<int64_t> seek_rel;
    std::atomic<int> read_pause_return;
    // AVFormatContext *ic;
    std::atomic<int> realtime;

    // std::unique_ptr<Clock> audclk = nullptr;
    // std::unique_ptr<Clock> vidclk = nullptr;
    // std::unique_ptr<Clock> extclk = nullptr;

    // std::unique_ptr<AVFrameQueue> pictq = nullptr;
    // std::unique_ptr<AVFrameQueue> subpq = nullptr;
    // std::unique_ptr<AVFrameQueue> sampq = nullptr;

    // std::unique_ptr<Decoder> auddec = nullptr;
    // std::unique_ptr<Decoder> viddec = nullptr;
    // std::unique_ptr<Decoder> subdec = nullptr;

    std::atomic<int> audio_stream;

    std::atomic<int> av_sync_type;

    std::atomic<double> audio_clock;
    std::atomic<int> audio_clock_serial;
    std::atomic<double> audio_diff_cum; /* used for AV difference average computation */
    std::atomic<double> audio_diff_avg_coef;
    std::atomic<double> audio_diff_threshold;
    std::atomic<int> audio_diff_avg_count;
    // AVStream *audio_st;
    // std::unique_ptr<AVPacketQueue> audioq;
    std::atomic<int> audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    std::atomic<unsigned int> audio_buf_size; /* in bytes */
    std::atomic<unsigned int> audio_buf1_size;
    std::atomic<int> audio_buf_index; /* in bytes */
    std::atomic<int> audio_write_buf_size;
    std::atomic<int> audio_volume;
    std::atomic<int> muted;
    struct AudioParams audio_src;
    struct AudioParams audio_filter_src;
    struct AudioParams audio_tgt;
    // struct SwrContext *swr_ctx;
    std::atomic<int> frame_drops_early;
    std::atomic<int> frame_drops_late;

    enum ShowMode {
        SHOW_MODE_NONE = -1,
        SHOW_MODE_VIDEO = 0,
        SHOW_MODE_WAVES,
        SHOW_MODE_RDFT,
        SHOW_MODE_NB
    } show_mode;
    std::atomic<int16_t> sample_array[SAMPLE_ARRAY_SIZE];
    std::atomic<int> sample_array_index;
    std::atomic<int> last_i_start;
    AVTXContext *rdft;
    av_tx_fn rdft_fn;
    std::atomic<int> rdft_bits;
    float *real_data;
    AVComplexFloat *rdft_data;
    std::atomic<int> xpos;
    std::atomic<double> last_vis_time;

    // RenderParams render_params;
    // SDL_Texture *vis_texture;
    // SDL_Texture *sub_texture;
    // SDL_Texture *vid_texture;

    std::atomic<int> subtitle_stream;
    AVStream *subtitle_st;
    std::unique_ptr<AVPacketQueue> subtitleq;

    std::atomic<double> frame_timer;
    std::atomic<double> frame_last_returned_time;
    std::atomic<double> frame_last_filter_delay;
    std::atomic<int> video_stream;
    // AVStream *video_st;
    // std::unique_ptr<AVPacketQueue> videoq;
    std::atomic<double>
        max_frame_duration; // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    // struct SwsContext *sub_convert_ctx;
    std::atomic<int> eof;

    char *filename;
    std::atomic<int> width, height, xleft, ytop;
    std::atomic<int> step;

    std::atomic<int> vfilter_idx;
    AVFilterContext *in_video_filter; // the first filter in the video chain
    AVFilterContext *out_video_filter; // the last filter in the video chain
    AVFilterContext *in_audio_filter; // the first filter in the audio chain
    AVFilterContext *out_audio_filter; // the last filter in the audio chain
    AVFilterGraph *agraph; // audio filter graph

    std::atomic<int> last_video_stream, last_audio_stream, last_subtitle_stream;

    // SDL_cond *continue_read_thread;
}MyVideoState;

#endif // FFMPEG_PUBLIC_STRUCT_H
