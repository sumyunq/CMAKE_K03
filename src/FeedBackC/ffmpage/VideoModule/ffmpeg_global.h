#ifndef FFMPEG_GLOBAL_H
#define FFMPEG_GLOBAL_H

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QObject>

#include "FeedBackC/ffmpage/public_space.h"

#include "FeedBackC/ffmpage/publicStruct/ffmpeg_public_struct.h"

#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_decoder_thread.h"
#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_sync_thread.h"
#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_unpackage_thread.h"

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_target_audio_device.h"

///
/// \brief The FFmpegGlobal class
/// 优先保证线程安全，供FFmpeg工作子线程访问
/// 内部对象:
///     视频流压缩数据帧队列
///     音频流压缩数据帧队列
///     视频流解码器
///     音频流解码器
///     音频同步时钟
///     ///暂定     视频流帧队列(已处理完成图像帧队列,可直接用于显示)
///     ///暂定     音频流帧队列(已处理完成音频帧队列,可直接用于播放)
class FFmpegGlobal : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegGlobal(QObject *parent = nullptr);
    ~FFmpegGlobal();

    bool open(const char *url);    ///打开流媒体文件
    void seek(double seconds);     ///跳转
    void pause(bool requestPause); ///暂停

    /// 同步时钟
    double get_master_clock();                        ///获取主时钟同步时钟
    void update_master_clock(double pts, int serial); ///更新主时钟同步时钟

    bool cl_is_stop() const;
    void set_cl_is_stop(bool stop);

    bool cl_is_pause() const;

    double get_total_time() const;    /// 获取总时长

    QString current_media_filename() const;


signals:
    // void updateUnpackageThread(bool isStart);
    // void startAllThread(); ///启动所有线程
    // void stopAllThread(); ///停止所有线程

    // void startAllPlaying(); ///开始播放
    // void pauseAllPlaying(); ///暂停播放
    // void stopAllPlay();  ///停止播放

public slots:
    void startAllThread(); ///启动所有线程
    void stopAllThread();  ///停止所有线程

    void startAllPlaying(); ///开始播放
    void pauseAllPlaying(); ///暂停播放
    void stopAllPlay();     ///停止播放

private:
    void InitMember();  ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

    void debugInfo();


public:
    /// 控制位
    std::atomic<bool> cl_is_pause_ = false; ///是否暂停
    mutable QMutex cl_pause_Mutex_;         ///用于暂停后唤醒
    mutable QWaitCondition cl_pause_Cond_;  ///用于暂停后唤醒

    std::atomic<bool> cl_is_stop_ = false; ///播放是否停止
    std::atomic<bool> cl_is_seeking_{false}; ///是否正在seek，防止重入

    /// 流可用标志（由 open() 设置，startAllThread() 用于按需启动线程）
    std::atomic<bool> has_video_stream_{false};
    std::atomic<bool> has_audio_stream_{false};

    /// seek 目标 PTS，用于 seek 后丢弃关键帧之前的旧帧
    std::atomic<double> seek_target_pts_{-1.0};
    std::atomic<int> seek_serial_{-1};

    /// FFmpeg
    /// 子线程对象
    std::unique_ptr<FFmpegUnpackageThread> clp_FFmpeg_unpackage_ = nullptr; /// 解码数据包对象
    std::unique_ptr<FFmpegDecoderThread> clp_decoder_audio_ = nullptr;      /// 音频流处理对象
    std::unique_ptr<FFmpegDecoderThread> clp_decoder_video_ = nullptr;      /// 视频处理对象
    std::unique_ptr<FFmpegSyncThread> clp_sync_audio_ = nullptr;            /// 视频帧同步处理对象
    std::unique_ptr<FFmpegSyncThread> clp_sync_video_ = nullptr;            /// 视频帧同步处理对象

    std::unique_ptr<QThread> clp_FFmpeg_unpackage_thread_ = nullptr; /// 解码数据包线程
    std::unique_ptr<QThread> clp_decoder_audio_thread_ = nullptr;    /// 音频流处理线程
    std::unique_ptr<QThread> clp_decoder_video_thread_ = nullptr;    /// 视频处理线程
    std::unique_ptr<QThread> clp_sync_audio_thread_ = nullptr;       /// 视频帧同步线程
    std::unique_ptr<QThread> clp_sync_video_thread_ = nullptr;       /// 视频帧同步线程

    /// 音频回调线程
    std::unique_ptr<FFmpegTargetAudioDevice> clp_FFmpegTargetAudioDeviceInfo_ = nullptr;
    std::unique_ptr<QThread> clp_sync_audioDevice_thread_ = nullptr;

    /// 音视频时钟同步对象(向音频同步)
    std::atomic<ForFFmpeg::AVSyncType> av_sync_type_{
        ForFFmpeg::AVSyncType::AudioMaster}; /// 初始化为音频主时钟
    mutable QMutex aud_clk_Mutex_;
    mutable QMutex vid_clk_Mutex_;
    mutable QMutex ext_clk_Mutex_;
    std::unique_ptr<Clock> aud_clk_ = nullptr; ///音频时钟
    std::unique_ptr<Clock> vid_clk_ = nullptr; ///视频时钟
    std::unique_ptr<Clock> ext_clk_ = nullptr; ///外部时钟

    std::atomic<int> cl_serial_
        = -1; ///有效序列号,一：用于音频同步;二：校验数据包队列、缓存帧队列是否有效 ///切换流媒体文件时自动+1、seek后自动+1

    /**************************************** 格式上下文/封装格式上下文/媒体格式上下文 *************************************************************/
    /// 流媒体上下文
    mutable QMutex cl_avFormatContext_Mutex_;
    AVFormatContext *cl_avFormatContext_ = nullptr;

    const AVInputFormat *cl_avInFormatContext_ = nullptr;



    /**************************************** 视频流处理相关 *************************************************************/
    /// 解码器
    mutable QMutex cl_decoder_video_Mutex_;
    std::unique_ptr<Decoder> cl_decoder_video_ = nullptr;

    ///视频流索引
    std::atomic<int> video_index_ = -1;

    ///视频流
    mutable QMutex video_stream_Mutex_;
    AVStream *video_stream_ = nullptr;

    /// 视频流时间基
    mutable QMutex video_TimeBase_Mutex_;
    AVRational
        video_TimeBase_; ///视频流时间基准  ///用于 pts * time_base拿到 预期播放时间 /// pts = av_frame_get_best_effort_timestamp(video_frame)
    std::atomic<double> video_total_time_;  ///视频流总时长(单位 秒)

    /// 压缩数据包队列（视频）
    mutable QMutex video_AVPacket_Queue_Mutex_; /// video_AVPacket_Queue 的互斥锁
    std::unique_ptr<AVPacketQueue> video_AVPacket_Queue_ = nullptr; /// 压缩数据包队列(视频)

    /// 处理完成的帧队列(也称缓存队列)
    mutable QMutex video_AVFrame_Queue_Mutex_;
    std::unique_ptr<AVFrameQueue> video_AVFrame_Queue_ = nullptr; /// 缓存 帧队列(视频、图像)

    /// 图像转换上下文 SwsContext
    mutable QMutex sub_convert_ctx_Mutex_;
    struct SwsContext *sub_convert_ctx_ = nullptr;

    /// 视频流参数(输入、输出)
    std::atomic<int> src_width_ = 0;     ///原格式宽度
    std::atomic<int> src_height_ = 0;    ///原格式高度
    std::atomic<int> target_width_ = 0;  ///目标格式宽度
    std::atomic<int> target_height_ = 0; ///目标格式高度

    std::atomic<int> num_bytes_ = 0; /// 计算存储一张RGB24图像所需的最小字节数

    /// 具体的buffer
    mutable QMutex buffer_Mutex_;
    uint8_t *buffer_ = {0}; /// 根据计算出的字节数，从堆中分配内存

    /**************************************** 音频流处理相关 *************************************************************/
    /// 解码器
    mutable QMutex cl_decoder_audio_Mutex_;
    std::unique_ptr<Decoder> cl_decoder_audio_ = nullptr;

    ///音频流索引
    std::atomic<int> audio_index_ = -1;

    ///音频流
    mutable QMutex audio_stream_Mutex_;
    AVStream *audio_stream_ = nullptr;

    ///音频流时间基准  ///同时，用于更新音频流时钟
    mutable QMutex audio_TimeBase_Mutex_;
    AVRational audio_TimeBase_;
    std::atomic<double> audio_total_time_; /// 音频流总时长(单位 秒)

    /// 压缩数据包队列（视频）
    mutable QMutex audio_AVPacket_Queue_Mutex_; /// audio_AVPacket_Queue_ 的互斥锁
    std::unique_ptr<AVPacketQueue> audio_AVPacket_Queue_ = nullptr; /// 压缩数据包队列(音频)

    /// 处理完成的帧队列(也称缓存队列)
    mutable QMutex audio_AVFrame_Queue_Mutex_;
    std::unique_ptr<AVFrameQueue> audio_AVFrame_Queue_ = nullptr; /// 缓存 帧队列(音频)

    /// 重采样器
    mutable QMutex swr_ctx_Mutex_;
    SwrContext *swr_ctx_ = nullptr;

    /// 音频流参数(输入、输出)
    /// 单个 音频帧 输入格式
    std::atomic<int> in_nb_samples_;
    std::atomic<int> in_sample_rate_;
    std::atomic<int> in_channels_;
    std::atomic<int64_t> in_layout_;
    AVSampleFormat in_fmt_;

    /// 音频输出格式
    AVSampleFormat out_fmt_;
    std::atomic<int> out_sample_rate_;
    std::atomic<int> out_channels_;
    std::atomic<int64_t> out_channel_layout_;

    /// qt音频输出设备格式
    QAudioFormat cl_format_;



    /**************************************** 字幕流处理相关 *************************************************************/

    /**************************************** 其他 *************************************************************/
    mutable QMutex current_media_filename_Mutex_;
    QString current_media_filename_; ///正在播放的文件名字

    /// 测试用数据,统计 压缩数据包解码数量、缓存帧处理数量 是否匹配
    std::atomic<int> ts_allAudioAVPacketCount = 0; ///视频压缩数据包总计
    std::atomic<int> ts_allVideoAVPacketCount = 0; ///音频压缩数据包总计
    std::atomic<int> ts_allAudioFrameCount = 0;    ///视频缓存帧总计
    std::atomic<int> ts_allVideoFrameCount = 0;    ///音频缓存帧总计
    std::atomic<int> ts_allSlots = 0;              ///音频槽函数触次数,应和缓存帧同步
};

#endif // FFMPEG_GLOBAL_H

// typedef struct VideoState {
// SDL_Thread *read_tid;
// const AVInputFormat *iformat;
// int abort_request;
// int force_refresh;
// int paused;
// int last_paused;
// int queue_attachments_req;
// int seek_req;
// int seek_flags;
// int64_t seek_pos;
// int64_t seek_rel;
// int read_pause_return;
// AVFormatContext *ic;
// int realtime;

// Clock audclk;
// Clock vidclk;
// Clock extclk;

// FrameQueue pictq;
// FrameQueue subpq;
// FrameQueue sampq;

// Decoder auddec;
// Decoder viddec;
// Decoder subdec;

// int audio_stream;

// int av_sync_type;

// double audio_clock;
// int audio_clock_serial;
// double audio_diff_cum; /* used for AV difference average computation */
// double audio_diff_avg_coef;
// double audio_diff_threshold;
// int audio_diff_avg_count;
// AVStream *audio_st;
// PacketQueue audioq;
// int audio_hw_buf_size;
// uint8_t *audio_buf;
// uint8_t *audio_buf1;
// unsigned int audio_buf_size; /* in bytes */
// unsigned int audio_buf1_size;
// int audio_buf_index; /* in bytes */
// int audio_write_buf_size;
// int audio_volume;
// int muted;
// struct AudioParams audio_src;
// struct AudioParams audio_filter_src;
// struct AudioParams audio_tgt;
// struct SwrContext *swr_ctx;
// int frame_drops_early;
// int frame_drops_late;

// enum ShowMode {
//     SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
// } show_mode;
// int16_t sample_array[SAMPLE_ARRAY_SIZE];
// int sample_array_index;
// int last_i_start;
// AVTXContext *rdft;
// av_tx_fn rdft_fn;
// int rdft_bits;
// float *real_data;
// AVComplexFloat *rdft_data;
// int xpos;
// double last_vis_time;
// RenderParams render_params;
// SDL_Texture *vis_texture;
// SDL_Texture *sub_texture;
// SDL_Texture *vid_texture;

// int subtitle_stream;
// AVStream *subtitle_st;
// PacketQueue subtitleq;

// double frame_timer;
// double frame_last_returned_time;
// double frame_last_filter_delay;
// int video_stream;
// AVStream *video_st;
// PacketQueue videoq;
// double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
// struct SwsContext *sub_convert_ctx;
// int eof;

// char *filename;
// int width, height, xleft, ytop;
// int step;

// int vfilter_idx;
// AVFilterContext *in_video_filter;   // the first filter in the video chain
// AVFilterContext *out_video_filter;  // the last filter in the video chain
// AVFilterContext *in_audio_filter;   // the first filter in the audio chain
// AVFilterContext *out_audio_filter;  // the last filter in the audio chain
// AVFilterGraph *agraph;              // audio filter graph

// int last_video_stream, last_audio_stream, last_subtitle_stream;

// SDL_cond *continue_read_thread;
// }