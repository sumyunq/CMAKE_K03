#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"

// std::unique_ptr<FFmpegGlobal> g_FFmpeg_global_ = std::make_unique<FFmpegGlobal>();

FFmpegGlobal::FFmpegGlobal(QObject *parent)
{
    InitMember();
    InitConnect();
    // 线程在 open() → startAllThread() 中按需启动，
    // 不再在构造时就启动空闲线程
}

FFmpegGlobal::~FFmpegGlobal()
{
    qDebug() << "========== FFmpegGlobal 析构开始 ==========";

    //  1. 设置停止标志
    cl_is_stop_.store(true);
    cl_is_pause_.store(false);

    //  2. 唤醒所有被暂停阻塞的线程
    {
        QMutexLocker pauseLocker(&cl_pause_Mutex_);
        cl_pause_Cond_.wakeAll();
    }

    //  3. 停止所有队列（唤醒在 dequeue 中等待的线程）
    {
        QMutexLocker locker(&video_AVPacket_Queue_Mutex_);
        if (video_AVPacket_Queue_) {
            video_AVPacket_Queue_->stop();
            video_AVPacket_Queue_->flush();
        }
    }
    {
        QMutexLocker locker(&audio_AVPacket_Queue_Mutex_);
        if (audio_AVPacket_Queue_) {
            audio_AVPacket_Queue_->stop();
            audio_AVPacket_Queue_->flush();
        }
    }
    {
        QMutexLocker locker(&video_AVFrame_Queue_Mutex_);
        if (video_AVFrame_Queue_) {
            video_AVFrame_Queue_->stop();
            video_AVFrame_Queue_->flush();
        }
    }
    {
        QMutexLocker locker(&audio_AVFrame_Queue_Mutex_);
        if (audio_AVFrame_Queue_) {
            audio_AVFrame_Queue_->stop();
            audio_AVFrame_Queue_->flush();
        }
    }

    //  4. 等待所有工作线程退出（必须在成员析构之前，
    //     否则 mutex 会在线程仍运行时被销毁，导致 QMutex 异常）
    auto quitAndWait = [](std::unique_ptr<QThread> &t) {
        if (t && t->isRunning()) {
            t->quit();
            t->wait(3000);
        }
    };
    quitAndWait(clp_sync_video_thread_);
    quitAndWait(clp_sync_audio_thread_);
    quitAndWait(clp_decoder_video_thread_);
    quitAndWait(clp_decoder_audio_thread_);
    quitAndWait(clp_FFmpeg_unpackage_thread_);

    qDebug() << "========== FFmpegGlobal 析构完成 ==========";
}

void FFmpegGlobal::InitMember()
{
    clp_FFmpeg_unpackage_ = std::make_unique<FFmpegUnpackageThread>(this);
    clp_decoder_audio_ = std::make_unique<FFmpegDecoderThread>(this);
    clp_decoder_video_ = std::make_unique<FFmpegDecoderThread>(this);
    clp_sync_audio_ = std::make_unique<FFmpegSyncThread>(this);
    clp_sync_video_ = std::make_unique<FFmpegSyncThread>(this);

    clp_FFmpeg_unpackage_thread_ = std::make_unique<QThread>(); /// 解码数据包线程
    clp_decoder_audio_thread_ = std::make_unique<QThread>();    /// 音频流处理线程
    clp_decoder_video_thread_ = std::make_unique<QThread>();    /// 视频处理线程
    clp_sync_audio_thread_ = std::make_unique<QThread>();       /// 音频帧同步线程
    clp_sync_video_thread_ = std::make_unique<QThread>();       /// 视频帧同步线程

    clp_FFmpeg_unpackage_->moveToThread(clp_FFmpeg_unpackage_thread_.get());
    clp_decoder_audio_->moveToThread(clp_decoder_audio_thread_.get());
    clp_decoder_video_->moveToThread(clp_decoder_video_thread_.get());
    clp_sync_audio_->moveToThread(clp_sync_audio_thread_.get());
    clp_sync_video_->moveToThread(clp_sync_video_thread_.get());

    clp_FFmpegTargetAudioDeviceInfo_ = std::make_unique<FFmpegTargetAudioDevice>(this);
    // QAudioOutput 留在主线程初始化，避免 worker 线程 WASAPI/COM 死锁
    // initAudio 在 startAllThread() 中直接调用

    aud_clk_ = std::make_unique<Clock>(); ///音频时钟
    vid_clk_ = std::make_unique<Clock>(); ///视频时钟
    ext_clk_ = std::make_unique<Clock>(); ///外部时钟

    video_AVPacket_Queue_ = std::make_unique<AVPacketQueue>(); /// 压缩数据包队列(视频)
    audio_AVPacket_Queue_ = std::make_unique<AVPacketQueue>(); /// 压缩数据包队列(音频)

    video_AVFrame_Queue_ = std::make_unique<AVFrameQueue>(); /// 缓存 帧队列(视频、图像)
    audio_AVFrame_Queue_ = std::make_unique<AVFrameQueue>(); /// 缓存 帧队列(音频)

    cl_decoder_audio_ = std::make_unique<Decoder>();
    cl_decoder_video_ = std::make_unique<Decoder>();
}

void FFmpegGlobal::InitConnect()
{
    /// 连接一下相关信号用于控制线程

    // connect(this, &FFmpegGlobal::updateUnpackageThread,
    //         clp_FFmpeg_unpackage_.get(), &FFmpegUnpackageThread::startUnpackage);
}

void FFmpegGlobal::debugInfo()
{
    // av_dump_format(cl_fmt_, 0, url, 0); ///打印视频文件元数据
    {
        ///函数测试用
        // 还可以获取更多容器信息
        const AVInputFormat *fmt = cl_avFormatContext_->iformat;

        qDebug() << "封装格式名:" << fmt->name;
        qDebug() << "封装格式全称:" << fmt->long_name;
        qDebug() << "支持的扩展名:" << fmt->extensions; // 如 "mov,mp4,m4a,3gp,3g2,mj2"
        qDebug() << "MIME 类型:" << fmt->mime_type;     // 如 "video/quicktime"
        qDebug() << "私有编解码器标签数:" << fmt->priv_class;
        ///仅测试用
        if (cl_avFormatContext_->duration != AV_NOPTS_VALUE) {
            // 总时长（微秒）
            int64_t duration_us = cl_avFormatContext_->duration;
            // 转换为秒 (double)
            // cl_total_duration_us_.store((double) duration_us / AV_TIME_BASE);
            // qDebug() << "容器 cl_fmt_ 中视频总时长:" << cl_total_duration_us_ << "秒";

            // 转换为毫秒 (qint64)
            qint64 duration_ms = duration_us / 1000;
            qDebug() << "容器 cl_fmt_ 中视频总时长:" << duration_ms << "毫秒";

        } else {
            // 容器未提供时长信息，需要使用备选方案
            qDebug() << "无法从容器 cl_fmt_ 获取总时长";
        }
    }
}


///
/// \brief FFmpegGlobal::open
/// \param url 要打开的目标流媒体文件的地址
/// \return
/// 打开流媒体文件后需要重置大部分 ffmpeg 相关部件
/// 打开时，需检查是否需要重置解码器状态（音频解码器、视频解码器），刷新对应数据包队列，缓存帧队列，重置时钟状态等
/// 重采样器、图片转换上下文 这些交由对应解码线程去执行初始化，一些必要的参数可以在打开时进行初始化
/// 打开时，可以返回一些必要的元数据信息给UI线程（比如总时长）
/// 打开时，需确保锁正常持有，来避免一些潜在的异常问题
bool FFmpegGlobal::open(const char *url)
{
    if (!url)
        return false;

    {
        QMutexLocker locker(&current_media_filename_Mutex_);
        /// 如果正在播放同一个文件，直接返回
        if (current_media_filename_ == QString::fromUtf8(url)) {
            qDebug() << "已经是同一个文件，跳过";
            return true;
        }
        /// 记录新文件名
        current_media_filename_ = QString::fromUtf8(url);
    }

    /// 先停止所有线程
    stopAllThread();

    /// 重置必要部件
    {
        /// 测试数据重置
        {
            ///打印一下前一个视频的帧记录
            qDebug() << " ts_allAudioAVPacketCount " << ts_allAudioAVPacketCount.load();
            qDebug() << " ts_allVideoAVPacketCount " << ts_allVideoAVPacketCount.load();
            qDebug() << " ts_allAudioFrameCount " << ts_allAudioFrameCount.load();
            qDebug() << " allVideoFrameCount " << ts_allVideoFrameCount.load();
            qDebug() << " allSlots " << ts_allSlots.load();

            /// 重置测试数据
            ts_allAudioAVPacketCount.store(0); ///视频压缩数据包总计
            ts_allVideoAVPacketCount.store(0); ///音频压缩数据包总计
            ts_allAudioFrameCount.store(0);    ///视频缓存帧总计
            ts_allVideoFrameCount.store(0);    ///音频缓存帧总计
            ts_allSlots.store(0);              ///槽触发
        }

        ///刷新对应数据包队列、缓存帧队列
        {
            ///视频
            {
                QMutexLocker locker(&video_AVPacket_Queue_Mutex_);
                if (!video_AVPacket_Queue_) {
                    video_AVPacket_Queue_ = std::make_unique<AVPacketQueue>(64);
                } else {
                    video_AVPacket_Queue_->flush();
                }
            }
            {
                QMutexLocker locker(&video_AVFrame_Queue_Mutex_);
                if (!video_AVFrame_Queue_) {
                    video_AVFrame_Queue_ = std::make_unique<AVFrameQueue>(64);
                } else {
                    video_AVFrame_Queue_->flush();
                }
            }
            /// 音频
            {
                QMutexLocker locker(&audio_AVPacket_Queue_Mutex_);
                if (!audio_AVPacket_Queue_) {
                    audio_AVPacket_Queue_ = std::make_unique<AVPacketQueue>(64);
                } else {
                    audio_AVPacket_Queue_->flush();
                }
            }
            {
                QMutexLocker locker(&audio_AVFrame_Queue_Mutex_);
                if (!audio_AVFrame_Queue_) {
                    audio_AVFrame_Queue_ = std::make_unique<AVFrameQueue>(64);
                } else {
                    audio_AVFrame_Queue_->flush();
                }
            }
        }
        ///重置时钟（init_clock 后显式设为 0.0，避免 NAN 导致初期视频帧被跳过）
        {
            {
                QMutexLocker locker(&aud_clk_Mutex_);
                aud_clk_->init_clock();
                aud_clk_->set_clock(0.0, -1);
            }
            {
                QMutexLocker locker(&vid_clk_Mutex_);
                vid_clk_->init_clock();
                vid_clk_->set_clock(0.0, -1);
            }
            {
                QMutexLocker locker(&ext_clk_Mutex_);
                ext_clk_->init_clock();
                ext_clk_->set_clock(0.0, -1);
            }
        }
    }

    /// 获取必要的锁，来保证打开文件后，能够正常运行，该操作持有锁的时间会长一点
    QMutexLocker locker(&cl_avFormatContext_Mutex_);

    ///如果正在播放文件，先关闭
    if (cl_avFormatContext_) {
        avformat_close_input(&cl_avFormatContext_);
        cl_avFormatContext_ = nullptr;
    }

    /// 打开流媒体文件
    if (avformat_open_input(&cl_avFormatContext_, url, nullptr, nullptr) != 0) {
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << " avformat_open_input failed";
        return false;
    }
    if (avformat_find_stream_info(cl_avFormatContext_, nullptr) < 0) {
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << " avformat_find_stream_info failed";
        avformat_close_input(&cl_avFormatContext_);
        cl_avFormatContext_ = nullptr;
        return false;
    }

    /// 文件打开后，序列号自动加一
    cl_serial_.fetch_add(1);
    seek_target_pts_.store(-1.0); /// 清除遗留的 seek 目标

    debugInfo(); ///调试用

    ///更新相关部件（解码器冲刷、缓存帧队列清理、压缩数据包队列清理、同步时钟重置、）
    has_video_stream_.store(false);
    has_audio_stream_.store(false);
    for (unsigned i = 0; i < cl_avFormatContext_->nb_streams; i++) {
        AVCodecParameters *params = cl_avFormatContext_->streams[i]->codecpar; ///无需手动释放

        switch (cl_avFormatContext_->streams[i]->codecpar->codec_type) {
        case AVMediaType::AVMEDIA_TYPE_UNKNOWN: {
            /// 未知类型流，通常当作 AVMEDIA_TYPE_DATA 处理，用于未识别或未指定的流
            /// 不处理
            qDebug() << "未知流类型 == AVMediaType:AVMEDIA_TYPE_UNKNOWN";
            break;
        }
        case AVMediaType::AVMEDIA_TYPE_VIDEO: {
            /// 视频流，存储压缩后的图像序列（如 H.264、HEVC、MPEG-4）
            {
                // clp_FFmpeg_unpackage_.get ()->cl_avFormatContext_->streams[i]->discard;
                // * @返回 成功时返回非负的流编号，找不到具有请求类型流的情况返回 AVERROR_STREAM_NOT_FOUND，找到流但无解码器的情况返回 AVERROR_DECODER_NOT_FOUND
                video_index_.store(av_find_best_stream(cl_avFormatContext_,
                                                       AVMEDIA_TYPE_VIDEO,
                                                       -1,
                                                       -1,
                                                       NULL,
                                                       0)); ///查找最合适的流
                if (video_index_.load() < 0) {
                    qDebug() << "未找到可用视频流，跳过视频初始化";
                    break;
                }
                // qDebug() << "最佳 视频流 的索引号：" << video_index_;
            }
            {
                QMutexLocker locker_decoder(&cl_decoder_video_Mutex_);
                {
                    QMutexLocker locker_vCtx(&cl_decoder_video_->avctx_Mutex_);
                    AVCodecContext *old_ctx = cl_decoder_video_->avctx_.release();
                    if (old_ctx) {
                        avcodec_flush_buffers(old_ctx);
                        avcodec_free_context(&old_ctx);
                    }

                    AVCodec *codec = const_cast<AVCodec *>(avcodec_find_decoder(
                        cl_avFormatContext_->streams[video_index_.load()]->codecpar->codec_id));
                    if (!codec) {
                        qDebug() << "未找到视频解码器";
                        avformat_close_input(&cl_avFormatContext_);
                        cl_avFormatContext_ = nullptr;
                        return false;
                    }
                    AVCodecContext *newCtx = avcodec_alloc_context3(codec);
                    if (!newCtx) {
                        qDebug() << "视频解码器上下文分配失败";
                        avformat_close_input(&cl_avFormatContext_);
                        cl_avFormatContext_ = nullptr;
                        return false;
                    }
                    avcodec_parameters_to_context(newCtx,
                                                  cl_avFormatContext_->streams[video_index_.load()]
                                                      ->codecpar);
                    if (avcodec_open2(newCtx, codec, nullptr) < 0) {
                        qDebug() << "视频解码器打开失败";
                        avcodec_free_context(&newCtx);
                        avformat_close_input(&cl_avFormatContext_);
                        cl_avFormatContext_ = nullptr;
                        return false;
                    }
                    cl_decoder_video_->avctx_.reset(newCtx);

                    // vCtx->lowres = codec->max_lowres;
                    // vCtx->flags2 |= AV_CODEC_FLAG2_FAST;
                    // vCtx->hw_device_ctx
                    {
                        src_width_.store(cl_decoder_video_->avctx_->width);
                        src_height_.store(cl_decoder_video_->avctx_->height);
                        qDebug() << "解码器w-h" << src_width_.load() << " - " << src_height_.load();

                        // 预分配RGB缓冲区（重用可以提升性能）
                        num_bytes_.store(av_image_get_buffer_size(AV_PIX_FMT_RGB24,
                                                                  src_width_.load(),
                                                                  src_height_.load(),
                                                                  1));

                        {
                            QMutexLocker locker(&buffer_Mutex_);
                            if (buffer_) {
                                av_free(buffer_);
                            }

                            /// 重新分配
                            buffer_ = (uint8_t *) av_malloc(num_bytes_.load());
                            if (!buffer_) {
                                qDebug() << "buffer_  内存分配错误";
                            }
                            qDebug() << "buffer_  内存分配成功 大小：" << num_bytes_.load();
                        }

                        {
                            QMutexLocker locker_vSream(&video_stream_Mutex_);
                            video_stream_ = cl_avFormatContext_
                                                ->streams[video_index_.load()]; ///无需手动释放
                            if (video_stream_->duration != AV_NOPTS_VALUE) {
                                double duration_sec = video_stream_->duration
                                                      * av_q2d(video_stream_->time_base);
                                qDebug() << "视频流 总时长:" << duration_sec << "秒";
                            }
                        }

                        {
                            QMutexLocker locker_vTimeBase(&video_TimeBase_Mutex_);
                            video_TimeBase_ = cl_avFormatContext_->streams[video_index_.load()]
                                                  ->time_base; ///更新视频流 时间基
                            /// 校验测试
                            qDebug() << "校准测试 视频流 总时长:"
                                     << video_stream_->duration * av_q2d(video_TimeBase_) << " 秒";
                            video_total_time_.store( video_stream_->duration * av_q2d(video_TimeBase_));
                        }
                    }
                }

                ///随同更新 图像转换上下文 SwsContext
                {
                    QMutexLocker locker(&sub_convert_ctx_Mutex_);
                    if (sub_convert_ctx_) {
                        sws_freeContext(sub_convert_ctx_);
                        sub_convert_ctx_ = nullptr;
                    }

                    sub_convert_ctx_ = sws_getContext(
                        cl_decoder_video_->avctx_->width,
                        cl_decoder_video_->avctx_->height,
                        cl_decoder_video_->avctx_
                            ->pix_fmt, // 输入：图像宽、高、像素格式（来自解码器）
                        cl_decoder_video_->avctx_->width,
                        cl_decoder_video_->avctx_->height,
                        AV_PIX_FMT_RGB24, // 输出：目标宽、高、像素格式（固定为RGB24）
                        // SWS_LANCZOS,  // 兰佐斯算法，质量最高
                        SWS_BICUBIC | SWS_ACCURATE_RND, // 双三次插值，质量较好且较快
                        // SWS_AREA,      // 适合缩小时使用
                        nullptr,
                        nullptr,
                        nullptr // 使用双线性算法进行缩放
                    );

                    if (!sub_convert_ctx_) {
                        qDebug() << " sub_convert_ctx_ 初始化失败";
                    }
                }
            }
            has_video_stream_.store(true);
            break;
        }
        case AVMediaType::AVMEDIA_TYPE_AUDIO: {
            /// 音频流，存储压缩后的声音数据（如 AAC、MP3、FLAC）
            {
                // audio_index_.store(i);   ///方式一
                audio_index_.store(av_find_best_stream(cl_avFormatContext_,
                                                       AVMEDIA_TYPE_AUDIO,
                                                       -1,
                                                       -1,
                                                       NULL,
                                                       0)); ///查找最合适的流
                if (audio_index_.load() < 0) {
                    qDebug() << "未找到可用音频流，跳过音频初始化";
                    break;
                }
            }
            {
                QMutexLocker locker_decoder_audio(&cl_decoder_audio_Mutex_);
                {
                    QMutexLocker locker_vCtx(&cl_decoder_audio_->avctx_Mutex_);
                    AVCodecContext *old_ctx = cl_decoder_audio_->avctx_.release();
                    if (old_ctx) {
                        avcodec_flush_buffers(old_ctx);
                        avcodec_free_context(&old_ctx);
                    }

                    AVCodec *codec = const_cast<AVCodec *>(avcodec_find_decoder(
                        cl_avFormatContext_->streams[audio_index_.load()]->codecpar->codec_id));
                    if (!codec) {
                        qDebug() << "未找到音频解码器";
                        avformat_close_input(&cl_avFormatContext_);
                        cl_avFormatContext_ = nullptr;
                        return false;
                    }
                    AVCodecContext *newCtx = avcodec_alloc_context3(codec);
                    if (!newCtx) {
                        qDebug() << "音频解码器上下文分配失败";
                        avformat_close_input(&cl_avFormatContext_);
                        cl_avFormatContext_ = nullptr;
                        return false;
                    }
                    avcodec_parameters_to_context(newCtx,
                                                  cl_avFormatContext_->streams[audio_index_.load()]
                                                      ->codecpar);
                    if (avcodec_open2(newCtx, codec, nullptr) < 0) {
                        qDebug() << "音频解码器打开失败";
                        avcodec_free_context(&newCtx);
                        avformat_close_input(&cl_avFormatContext_);
                        cl_avFormatContext_ = nullptr;
                        return false;
                    }
                    cl_decoder_audio_->avctx_.reset(newCtx);

                    {
                        QMutexLocker locker_aString(&audio_stream_Mutex_);
                        audio_stream_ = cl_avFormatContext_->streams[audio_index_.load()];

                        if (audio_stream_->duration != AV_NOPTS_VALUE) {
                            double duration_sec = audio_stream_->duration
                                                  * av_q2d(audio_stream_->time_base);
                            qDebug() << "音频流 总时长:" << duration_sec << "秒";
                        }
                        {
                            QMutexLocker locker_aTimeBase(&audio_TimeBase_Mutex_);
                            audio_TimeBase_ = cl_avFormatContext_->streams[audio_index_.load()]
                                                  ->time_base; ///更新音频流 时间基
                            /// 校验测试
                            qDebug() << "校准测试 音频流 总时长:"
                                     << audio_stream_->duration * av_q2d(audio_TimeBase_) << " 秒";
                            audio_total_time_.store(audio_stream_->duration
                                                    * av_q2d(audio_TimeBase_));
                        }
                    }

                    {
                        { ///初始化重采样器
                            QMutexLocker locker_aCtx(&swr_ctx_Mutex_);
                            if (swr_ctx_) {
                                swr_free(&swr_ctx_);
                                swr_ctx_ = nullptr;
                            }

                            AVChannelLayout out_ch_layout;                // 或从设备参数构造
                            av_channel_layout_default(&out_ch_layout, 2); // 2 声道默认布局

                            /// 从解码器上下文中获取输入通道数
                            AVChannelLayout in_ch_layout = cl_decoder_audio_->avctx_->ch_layout;

                            int ret = swr_alloc_set_opts2(
                                &swr_ctx_,
                                &out_ch_layout,
                                AV_SAMPLE_FMT_S16,
                                clp_FFmpegTargetAudioDeviceInfo_->targetFormat.sampleRate(),
                                &in_ch_layout,
                                cl_decoder_audio_->avctx_->sample_fmt,
                                cl_decoder_audio_->avctx_->sample_rate,
                                0,
                                nullptr);

                            qDebug() << "swr_alloc_set_opts2 结果：" << ret;

                            // cl_swrCtx_ = swr_alloc_set_opts(nullptr,
                            //                                 AV_CH_LAYOUT_STEREO,
                            //                                 AV_SAMPLE_FMT_S16,
                            //                                 44100,
                            //                                 aCtx->channel_layout,
                            //                                 aCtx->sample_fmt,
                            //                                 aCtx->sample_rate,
                            //                                 0,
                            //                                 nullptr);
                            //av_opt_set_int()
                            // av_opt_set_dict()
                            ret = swr_init(swr_ctx_);

                            qDebug() << "重采样器初始化结果：" << ret;

                            if (!swr_ctx_) {
                                qDebug() << " swr_ctx_ 初始化失败";
                            }
                        }
                    }
                    /// 发射信号去更新 UI 类中的音频设置 / 集成到这个类里
                }
            }
            has_audio_stream_.store(true);
            break;
        }
        case AVMediaType::AVMEDIA_TYPE_DATA: {
            /// 数据流，不透明的连续数据，如某些封装格式中的自定义二进制数据块、日志流、或已弃用的字幕格式
            break;
        }
        case AVMediaType::AVMEDIA_TYPE_SUBTITLE: {
            /// 字幕流，文本或图形字幕（如 ASS、SRT、PGS）
            break;
        }
        case AVMediaType::AVMEDIA_TYPE_ATTACHMENT: {
            /// 附件流，稀疏的不透明数据，通常用于存储内嵌文件（如 MP3 的专辑封面图片、字体文件）
            break;
        }
        case AVMediaType::AVMEDIA_TYPE_NB: {
            /// 类型计数，不是真正的媒体类型，仅用于统计枚举元素个数（NB = Number of）
            break;
        }
        default: {
            qDebug() << "格式错误";
            break;
        }
        }
    }

    QMetaObject::invokeMethod(this,
                              "startAllThread",
                              Qt::QueuedConnection); /// 后续需要改为只触发执行对应的单条流程的线程
    // startAllThread();
    return true;
}

void FFmpegGlobal::seek(double seconds)
{
    /// 防止重入：上一次 seek 未完成时直接跳过
    bool expected = false;
    if (!cl_is_seeking_.compare_exchange_strong(expected, true)) {
        qDebug() << "seek 重入，跳过";
        return;
    }

    /// 先停止所有线程
    stopAllThread();

    {
        /// 1. 暂停解码(1解码线程、2压缩数据包处理线程、2缓存帧处理线程)

        /// 跳转前检查 准备条件
        /// 先检查是否是暂停状态，如果是，则解除，然后让线程进入跳转阻塞状态，等待跳转完成后唤醒
        /// 如果不是暂停状态，那就直接让线程进入跳转状态，等待跳转完成后唤醒

        {
            QMutexLocker locker(&cl_avFormatContext_Mutex_); ///
            if (!cl_avFormatContext_) {
                cl_is_seeking_.store(false);
                return;
            }
        }
        {
            ///打印一下跳转前的帧记录
            qDebug() << " ts_allAudioAVPacketCount " << ts_allAudioAVPacketCount.load();
            qDebug() << " ts_allVideoAVPacketCount " << ts_allVideoAVPacketCount.load();
            qDebug() << " ts_allAudioFrameCount " << ts_allAudioFrameCount.load();
            qDebug() << " allVideoFrameCount " << ts_allVideoFrameCount.load();
            qDebug() << " allSlots " << ts_allSlots.load();
            /// 重置帧记录 测试数据
            ts_allAudioAVPacketCount.store(0); ///视频压缩数据包总计
            ts_allVideoAVPacketCount.store(0); ///音频压缩数据包总计
            ts_allAudioFrameCount.store(0);    ///视频缓存帧总计
            ts_allVideoFrameCount.store(0);    ///音频缓存帧总计
            ts_allSlots.store(0);              ///音频缓存帧总计
        }

        ///刷新对应数据包队列、缓存帧队列(先刷新帧队列，再刷新解码器)
        {
            ///视频
            {
                QMutexLocker locker(&video_AVPacket_Queue_Mutex_);
                if (!video_AVPacket_Queue_) {
                    video_AVPacket_Queue_ = std::make_unique<AVPacketQueue>(64);
                } else {
                    video_AVPacket_Queue_->flush();
                }
            }
            {
                QMutexLocker locker(&video_AVFrame_Queue_Mutex_);
                if (!video_AVFrame_Queue_) {
                    video_AVFrame_Queue_ = std::make_unique<AVFrameQueue>(64);
                } else {
                    video_AVFrame_Queue_->flush();
                }
            }
            /// 音频
            {
                QMutexLocker locker(&audio_AVPacket_Queue_Mutex_);
                if (!audio_AVPacket_Queue_) {
                    audio_AVPacket_Queue_ = std::make_unique<AVPacketQueue>(64);
                } else {
                    audio_AVPacket_Queue_->flush();
                }
            }
            {
                QMutexLocker locker(&audio_AVFrame_Queue_Mutex_);
                if (!audio_AVFrame_Queue_) {
                    audio_AVFrame_Queue_ = std::make_unique<AVFrameQueue>(64);
                } else {
                    audio_AVFrame_Queue_->flush();
                }
            }
        }

        /// 执行跳转操作
        {
            QMutexLocker locker(&cl_avFormatContext_Mutex_);
            int64_t seekTarget = seconds * AV_TIME_BASE;
            qDebug() << "跳转seek开始,目标pts: " << seekTarget;
            // AVSEEK_FLAG_BACKWARD：定位到目标时间或之前最近的关键帧
            int ret = avformat_seek_file(cl_avFormatContext_, -1,
                                          INT64_MIN, seekTarget, seekTarget,
                                          AVSEEK_FLAG_BACKWARD);
            if (ret < 0) {
                qDebug() << "seek 失败，恢复播放";
                seek_target_pts_.store(-1.0);
                startAllThread();
                cl_is_seeking_.store(false);
                return;
            }
            cl_serial_.fetch_add(1); ///序列号+1
            // 记录 seek 目标，供 sync 线程丢弃关键帧之前的旧帧
            seek_target_pts_.store(seconds);
            seek_serial_.store(cl_serial_.load());
            qDebug() << "跳转seek完成,目标pts: " << seekTarget;
        }

        ///刷新解码器内部缓存、上下文   刷新 != 重置
        {
            {
                QMutexLocker locker_decoder_video(&cl_decoder_video_Mutex_);
                {
                    QMutexLocker locker_vCtx(&cl_decoder_video_->avctx_Mutex_);
                    if (cl_decoder_video_->avctx_) {
                        avcodec_flush_buffers(cl_decoder_video_->avctx_.get ());
                    }
                }
            }
            {
                QMutexLocker locker_decoder_audio(&cl_decoder_audio_Mutex_);
                {
                    QMutexLocker locker_vCtx(&cl_decoder_audio_->avctx_Mutex_);
                    if (cl_decoder_audio_->avctx_) {
                        avcodec_flush_buffers(cl_decoder_audio_->avctx_.get());
                    }
                }
            }

            ///更新主时钟
            {
                qDebug() << "更新主时钟开始";
                update_master_clock(
                    seconds,
                    cl_serial_.load()); ///如果 跳转成功，那么 seconds 就在跳转后 第一帧的 pts 附近
                qDebug() << "更新主时钟完成";
            }

            /// 跳转无需更新 图片转换上下文、重采样器等
            /// 刷新图片转换上下文 参数
            /// 重置重采样器 参数

            /// 跳转完成
            // cl_is_seeking_.store(false);
            /// 唤醒等待的子线程
            // QMutexLocker lockerSeek(&cl_seek_Mutex);
            // cl_seek_Cond.wakeAll();
        }
    }

    startAllThread();
    cl_is_seeking_.store(false);
}

double FFmpegGlobal::get_master_clock()
{
    double current_pts; ///同步时间
    /// 根据当前 主时钟类型 来返回对应的同步时钟
    switch (av_sync_type_.load()) {
    case ForFFmpeg::AVSyncType::AudioMaster: {
        /// 以音频为基准同步
        QMutexLocker locker(&aud_clk_Mutex_);
        current_pts = aud_clk_->get_clock();
        break;
    }
    case ForFFmpeg::AVSyncType::VideoMaster: {
        /// 以视频为基准同步
        QMutexLocker locker(&vid_clk_Mutex_);
        current_pts = vid_clk_->get_clock();
        break;
    }
    case ForFFmpeg::AVSyncType::ExternalClock:
    default: {
        /// 默认采用外部时钟为基准同步
        QMutexLocker locker(&ext_clk_Mutex_);
        current_pts = ext_clk_->get_clock();
        break;
    }
    }
    return current_pts;
}

void FFmpegGlobal::update_master_clock(double pts, int serial)
{
    /// 根据当前 主时钟类型  更新对应的同步时钟
    switch (av_sync_type_.load()) {
    case ForFFmpeg::AVSyncType::AudioMaster: {
        /// 主时钟为 音频时钟
        QMutexLocker locker(&aud_clk_Mutex_);
        aud_clk_->set_clock(pts, serial);
        ///其他两个向主时钟同步
        {
            QMutexLocker locker(&vid_clk_Mutex_);
            vid_clk_->sync_clock_to_slave(aud_clk_.get());
        }
        {
            QMutexLocker locker(&ext_clk_Mutex_);
            ext_clk_->sync_clock_to_slave(aud_clk_.get());
        }
        break;
    }
    case ForFFmpeg::AVSyncType::VideoMaster: {
        /// 主时钟为 视频时钟
        QMutexLocker locker(&vid_clk_Mutex_);
        vid_clk_->set_clock(pts, serial);
        ///其他两个向主时钟同步
        {
            QMutexLocker locker(&aud_clk_Mutex_);
            aud_clk_->sync_clock_to_slave(vid_clk_.get());
        }
        {
            QMutexLocker locker(&ext_clk_Mutex_);
            ext_clk_->sync_clock_to_slave(vid_clk_.get());
        }
        break;
    }
    case ForFFmpeg::AVSyncType::ExternalClock:
    default: {
        /// 主时钟为 外部时钟
        QMutexLocker locker(&ext_clk_Mutex_);
        ext_clk_->set_clock(pts, serial);
        ///其他两个向主时钟同步
        {
            QMutexLocker locker(&aud_clk_Mutex_);
            aud_clk_->sync_clock_to_slave(ext_clk_.get());
        }
        {
            QMutexLocker locker(&vid_clk_Mutex_);
            vid_clk_->sync_clock_to_slave(ext_clk_.get());
        }
        break;
    }
    }
}

bool FFmpegGlobal::cl_is_stop() const
{
    return cl_is_stop_.load();
}

void FFmpegGlobal::set_cl_is_stop(bool stop)
{
    cl_is_stop_.store(stop);
}

bool FFmpegGlobal::cl_is_pause() const
{
    return cl_is_pause_.load();
}

double FFmpegGlobal::get_total_time() const
{
    if (has_audio_stream_.load()) {
        return audio_total_time_.load();
    }

    if (has_video_stream_.load()) {
        return video_total_time_.load();
    }

    return 0;
}

QString FFmpegGlobal::current_media_filename() const
{
    QMutexLocker locker(&current_media_filename_Mutex_);
    return current_media_filename_;
}

void FFmpegGlobal::startAllThread()
{
    qDebug() << __FUNCTION__ << "开始启动工作线程... 视频流:"
             << has_video_stream_.load() << " 音频流:" << has_audio_stream_.load();

    cl_is_stop_.store(false);

    // 1. 重启所有队列
    {
        QMutexLocker locker(&video_AVPacket_Queue_Mutex_);
        if (video_AVPacket_Queue_) video_AVPacket_Queue_->start();
    }
    {
        QMutexLocker locker(&audio_AVPacket_Queue_Mutex_);
        if (audio_AVPacket_Queue_) audio_AVPacket_Queue_->start();
    }
    {
        QMutexLocker locker(&video_AVFrame_Queue_Mutex_);
        if (video_AVFrame_Queue_) video_AVFrame_Queue_->start();
    }
    {
        QMutexLocker locker(&audio_AVFrame_Queue_Mutex_);
        if (audio_AVFrame_Queue_) audio_AVFrame_Queue_->start();
    }

    auto startThread = [](std::unique_ptr<QThread> &t) {
        if (t && !t->isRunning()) t->start();
    };

    // 2. 音频设备直接在主线程初始化（QAudioOutput 需主线程 COM/消息泵）
    if (has_audio_stream_.load()) {
        clp_FFmpegTargetAudioDeviceInfo_->initAudio();
    }

    // 3. 启动解码和同步线程（事件循环就绪）
    if (has_video_stream_.load()) {
        startThread(clp_decoder_video_thread_);
        startThread(clp_sync_video_thread_);
    }
    if (has_audio_stream_.load()) {
        startThread(clp_decoder_audio_thread_);
        startThread(clp_sync_audio_thread_);
    }

    // 4. 最后启动解包（确保下游线程的事件循环已就绪，再开始生产数据）
    startThread(clp_FFmpeg_unpackage_thread_);
    QMetaObject::invokeMethod(clp_FFmpeg_unpackage_.get(), "startUnpackage", Qt::QueuedConnection);

    // 5. 投递工作任务
    if (has_video_stream_.load()) {
        QMetaObject::invokeMethod(clp_decoder_video_.get(), "startDecoderVideo", Qt::QueuedConnection);
        QMetaObject::invokeMethod(clp_sync_video_.get(), "startSyncVideo", Qt::QueuedConnection);
    }
    if (has_audio_stream_.load()) {
        QMetaObject::invokeMethod(clp_decoder_audio_.get(), "startDecoderAudio", Qt::QueuedConnection);
        QMetaObject::invokeMethod(clp_sync_audio_.get(), "startSyncAudio", Qt::QueuedConnection);
    }

    qDebug() << __FUNCTION__ << "工作线程启动完成";
}

void FFmpegGlobal::stopAllThread()
{
    qDebug() << __FUNCTION__ << "开始停止所有工作线程...";

    // 1. 设置停止标志
    cl_is_stop_.store(true);

    // 2. 唤醒暂停等待中的线程
    {
        QMutexLocker locker(&cl_pause_Mutex_);
        cl_pause_Cond_.wakeAll();
    }

    // 3. 停止 QAudioOutput（如果正在播放）
    if (clp_FFmpegTargetAudioDeviceInfo_ && clp_FFmpegTargetAudioDeviceInfo_->targetAudioOutput) {
        clp_FFmpegTargetAudioDeviceInfo_->targetAudioOutput->stop();
    }

    // 4. 停止所有队列（唤醒 dequeue 等待中的线程）
    {
        QMutexLocker locker(&video_AVPacket_Queue_Mutex_);
        if (video_AVPacket_Queue_) video_AVPacket_Queue_->stop();
    }
    {
        QMutexLocker locker(&audio_AVPacket_Queue_Mutex_);
        if (audio_AVPacket_Queue_) audio_AVPacket_Queue_->stop();
    }
    {
        QMutexLocker locker(&video_AVFrame_Queue_Mutex_);
        if (video_AVFrame_Queue_) video_AVFrame_Queue_->stop();
    }
    {
        QMutexLocker locker(&audio_AVFrame_Queue_Mutex_);
        if (audio_AVFrame_Queue_) audio_AVFrame_Queue_->stop();
    }

    // 5. 等待所有 QThread 事件循环退出
    auto quitAndWait = [](std::unique_ptr<QThread> &t) {
        if (t && t->isRunning()) {
            t->quit();
            t->wait(3000);
        }
    };
    quitAndWait(clp_sync_audio_thread_);
    quitAndWait(clp_sync_video_thread_);
    quitAndWait(clp_decoder_video_thread_);
    quitAndWait(clp_decoder_audio_thread_);
    quitAndWait(clp_FFmpeg_unpackage_thread_);

    // 6. 流标志不清零（seek() 后需要复用），由下一次 open() 覆盖

    qDebug() << __FUNCTION__ << "所有工作线程已停止";
}

void FFmpegGlobal::startAllPlaying()
{
    qDebug() << __FUNCTION__ << "开始播放";

    // 确保线程未停止
    if (cl_is_stop_.load()) {
        qWarning() << "无法开始播放：播放器已停止";
        return;
    }

    // 解除暂停状态
    cl_is_pause_.store(false);

    // 恢复时钟
    {
        QMutexLocker locker_aud(&aud_clk_Mutex_);
        if (aud_clk_) {
            aud_clk_->setPaused(false);
        }
    }
    {
        QMutexLocker locker_vid(&vid_clk_Mutex_);
        if (vid_clk_) {
            vid_clk_->setPaused(false);
        }
    }
    {
        QMutexLocker locker_ext(&ext_clk_Mutex_);
        if (ext_clk_) {
            ext_clk_->setPaused(false);
        }
    }

    // 唤醒所有在暂停等待的线程
    // 注意：这里使用条件变量唤醒比依赖循环检查更高效
    QMutexLocker pauseLocker(&cl_pause_Mutex_);
    cl_pause_Cond_.wakeAll();

    qDebug() << __FUNCTION__ << "播放已开始";
}

void FFmpegGlobal::pauseAllPlaying()
{
    qDebug() << __FUNCTION__ << "暂停播放";

    // 设置暂停标志
    cl_is_pause_.store(true);

    // 暂停所有时钟
    {
        QMutexLocker locker_aud(&aud_clk_Mutex_);
        if (aud_clk_) {
            aud_clk_->setPaused(true);
        }
    }
    {
        QMutexLocker locker_vid(&vid_clk_Mutex_);
        if (vid_clk_) {
            vid_clk_->setPaused(true);
        }
    }
    {
        QMutexLocker locker_ext(&ext_clk_Mutex_);
        if (ext_clk_) {
            ext_clk_->setPaused(true);
        }
    }

    qDebug() << __FUNCTION__ << "暂停完成";
}

void FFmpegGlobal::stopAllPlay()
{
    qDebug() << __FUNCTION__ << "停止播放";

    // 停止所有工作线程
    stopAllThread();

    // 关闭媒体文件
    {
        QMutexLocker locker(&cl_avFormatContext_Mutex_);
        if (cl_avFormatContext_) {
            avformat_close_input(&cl_avFormatContext_);
            cl_avFormatContext_ = nullptr;
        }
    }

    // 清空所有队列
    {
        QMutexLocker locker(&video_AVPacket_Queue_Mutex_);
        if (video_AVPacket_Queue_) {
            video_AVPacket_Queue_->flush();
        }
    }
    {
        QMutexLocker locker(&audio_AVPacket_Queue_Mutex_);
        if (audio_AVPacket_Queue_) {
            audio_AVPacket_Queue_->flush();
        }
    }
    {
        QMutexLocker locker(&video_AVFrame_Queue_Mutex_);
        if (video_AVFrame_Queue_) {
            video_AVFrame_Queue_->flush();
        }
    }
    {
        QMutexLocker locker(&audio_AVFrame_Queue_Mutex_);
        if (audio_AVFrame_Queue_) {
            audio_AVFrame_Queue_->flush();
        }
    }

    // 重置时钟
    {
        QMutexLocker locker(&aud_clk_Mutex_);
        aud_clk_->init_clock();
    }
    {
        QMutexLocker locker(&vid_clk_Mutex_);
        vid_clk_->init_clock();
    }
    {
        QMutexLocker locker(&ext_clk_Mutex_);
        ext_clk_->init_clock();
    }

    // 重置计数器
    ts_allAudioAVPacketCount.store(0);
    ts_allVideoAVPacketCount.store(0);
    ts_allAudioFrameCount.store(0);
    ts_allVideoFrameCount.store(0);
    ts_allSlots.store(0);

    // 清空当前文件名
    {
        QMutexLocker locker(&current_media_filename_Mutex_);
        current_media_filename_.clear();
    }

    qDebug() << __FUNCTION__ << "停止播放完成";
}

void FFmpegGlobal::pause(bool requestPause)
{
    if (requestPause) {
        // 切换暂停/播放状态
        if (!cl_is_pause_.load()) {
            // 当前播放中 → 暂停
            cl_is_pause_.store(true);
            {
                QMutexLocker locker_aud(&aud_clk_Mutex_);
                aud_clk_->setPaused(true);
            }
            {
                QMutexLocker locker_vid(&vid_clk_Mutex_);
                vid_clk_->setPaused(true);
            }
            {
                QMutexLocker locker_ext(&ext_clk_Mutex_);
                ext_clk_->setPaused(true);
            }
            qDebug() << "暂停时钟:" << aud_clk_->get_clock();
        } else {
            // 当前暂停中 → 恢复播放
            cl_is_pause_.store(false);
            {
                QMutexLocker locker_aud(&aud_clk_Mutex_);
                aud_clk_->setPaused(false);
            }
            {
                QMutexLocker locker_vid(&vid_clk_Mutex_);
                vid_clk_->setPaused(false);
            }
            {
                QMutexLocker locker_ext(&ext_clk_Mutex_);
                ext_clk_->setPaused(false);
            }
            // 唤醒所有被暂停阻塞的工作线程
            QMutexLocker pauseLocker(&cl_pause_Mutex_);
            cl_pause_Cond_.wakeAll();
            qDebug() << "恢复播放，时钟:" << aud_clk_->get_clock();
        }
    }
}
