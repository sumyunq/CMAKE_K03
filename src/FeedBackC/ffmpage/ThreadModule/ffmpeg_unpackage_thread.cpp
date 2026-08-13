#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_unpackage_thread.h"

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"

FFmpegUnpackageThread::FFmpegUnpackageThread(FFmpegGlobal *target, QObject *parent)
    : target_ffmpeg_global_(target)
{
    InitMember();
    InitConnect();
}

FFmpegUnpackageThread::~FFmpegUnpackageThread()
{
    //qDebug() << "解码 FFmpegUnpackage 线程析构";
}


void FFmpegUnpackageThread::startUnpackage()
{
    ///开始解码
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this << "Unpackage start";

    if (!target_ffmpeg_global_) {
        qDebug() << "target_ffmpeg_global_ is nullptr";
        return;
    }

    while (!target_ffmpeg_global_->cl_is_stop()) {
        {


            /// 流数据包分发过程
            /// 策略二:压缩数据包读取成功时，通知对应线程做处理，不等待线程处理完成，直接读取下一个压缩数据包，目前无法保证子线程处理速度会快于该解码线程，可能会出现 wake 信号丢失，需要做其他同步策略
            /// 策略二更高效，但需要在实现数据包队列之后才好弄，直接往队列里添加压缩数据包，线程只处理数据包队列
            AVPacket pkt;

            /// 对  cl_fmt 进行上锁操作(防止切换流媒体文件时，av_read_frame 引发异常)
            QMutexLocker locker(&target_ffmpeg_global_->cl_avFormatContext_Mutex_);
            if (!target_ffmpeg_global_->cl_avFormatContext_) {
                qDebug() << "cl_avFormatContext_ is nullptr";
                continue;
            }

            int ret = av_read_frame(target_ffmpeg_global_->cl_avFormatContext_,
                                    &pkt); ///如果在正常运行的话,就不断读取压缩数据包
            if (ret >= 0) {
                ///正常读取到数据包
                MyAVPacket *newPkt = new MyAVPacket();
                // newPkt->pkt_.reset(av_packet_alloc());
                // av_packet_ref(newPkt->pkt_.get(),
                //               &pkt); ///拷贝数据后入队，其交由解码线程（出队并释放）

                AVPacket *cloned = av_packet_clone(&pkt);
                if (!cloned) {
                    delete newPkt;
                    av_packet_unref(&pkt);
                    continue;
                }
                newPkt->pkt_.reset(cloned);  // 交给智能指针管理
                newPkt->serial_.store(target_ffmpeg_global_->cl_serial_.load());


                /// 根据压缩数据包的内容，发送到对应的压缩数据包队列
                AVStream *stream = target_ffmpeg_global_->cl_avFormatContext_->streams[pkt.stream_index];
                switch (
                    stream->codecpar->codec_type) { ///检查压缩数据包所属类型,唤醒对应子线程去处理
                case AVMediaType::AVMEDIA_TYPE_UNKNOWN: {
                    /// 未知类型流，通常当作 AVMEDIA_TYPE_DATA 处理，用于未识别或未指定的流
                    //qDebug() << "唤醒 子线程处理 未知类型流";
                    break;
                }
                case AVMediaType::AVMEDIA_TYPE_VIDEO: {
                    /// 视频流，存储压缩后的图像序列（如 H.264、HEVC、MPEG-4）

                    if (!target_ffmpeg_global_->video_AVPacket_Queue_.get()->enqueue(newPkt)) {
                        delete newPkt;
                        qWarning() << "视频包入队失败，已释放";
                    } else {
                        target_ffmpeg_global_->ts_allVideoAVPacketCount.fetch_add(1);
                    }
                    //qDebug() << "发送 AVPacket 到 videoPacketQueue 队列  解码时间戳："<< newPkt->dts << " 显示时间戳：" << newPkt->pts << "当前 videoPacketQueue 队列大小: " << g_FFmpeg_global_->video_AVPacket_Queue()->size();

                    break;
                }
                case AVMediaType::AVMEDIA_TYPE_AUDIO: {
                    /// 音频流，存储压缩后的声音数据（如 AAC、MP3、FLAC）

                    if (!target_ffmpeg_global_->audio_AVPacket_Queue_.get()->enqueue(newPkt)) {
                        delete newPkt;
                        qWarning() << "音频包入队失败，已释放";
                    } else {
                        target_ffmpeg_global_->ts_allAudioAVPacketCount.fetch_add(1);
                    }

                    //qDebug() << "发送 AVPacket 到 audioPacketQueue 队列  解码时间戳：" << newPkt->dts << " 显示时间戳：" << newPkt->pts << "当前 audioPacketQueue 队列大小: "<< g_FFmpeg_global_->audio_AVPacket_Queue()->size();
                    // g_FFmpeg_global_->audio_AVPacket_Queue()->enqueue(newPkt); ///同上
                    // g_FFmpeg_global_->add1();
                    break;
                }
                case AVMediaType::AVMEDIA_TYPE_DATA: {
                    /// 数据流，不透明的连续数据，如某些封装格式中的自定义二进制数据块、日志流、或已弃用的字幕格式
                    //qDebug() << "唤醒 子线程处理 数据流";
                    break;
                }
                case AVMediaType::AVMEDIA_TYPE_SUBTITLE: {
                    /// 字幕流，文本或图形字幕（如 ASS、SRT、PGS）
                    //qDebug() << "唤醒 子线程处理 字幕流";
                    break;
                }
                case AVMediaType::AVMEDIA_TYPE_ATTACHMENT: {
                    /// 附件流，稀疏的不透明数据，通常用于存储内嵌文件（如 MP3 的专辑封面图片、字体文件）
                    //qDebug() << "唤醒 子线程处理 附件流";
                    break;
                }
                case AVMediaType::AVMEDIA_TYPE_NB: {
                    /// 类型计数，不是真正的媒体类型，仅用于统计枚举元素个数（NB = Number of）

                    break;
                }
                default: {
                    //qDebug() << "格式错误";
                    break;
                }
                }
            }

            av_packet_unref(&pkt);
        }

        {
            QMutexLocker locker(&target_ffmpeg_global_->cl_pause_Mutex_);
            while (target_ffmpeg_global_->cl_is_pause()
                   && !target_ffmpeg_global_->cl_is_stop()) {
                target_ffmpeg_global_->cl_pause_Cond_.wait(
                    &target_ffmpeg_global_->cl_pause_Mutex_, 50);
            }
        }
    }
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this << "Unpackage end";
}

void FFmpegUnpackageThread::stopUnpackage() {}

void FFmpegUnpackageThread::InitMember() {}

void FFmpegUnpackageThread::InitConnect() {}

