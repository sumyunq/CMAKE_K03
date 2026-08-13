#include "FeedBackC/ffmpage/VideoModule/ffmpeg_iodevice.h"

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"

FFmpegIODevice::FFmpegIODevice(FFmpegGlobal *target)
    : target_ffmpeg_global_(target)
{
    auto &fmt =
        target_ffmpeg_global_
            ->clp_FFmpegTargetAudioDeviceInfo_
            ->targetFormat;

    out_sample_rate_ = fmt.sampleRate();
    out_channels_ = fmt.channelCount();
    out_channel_layout_ = av_get_default_channel_layout(out_channels_);

    if (fmt.sampleSize() == 16) {
        out_fmt_ = AV_SAMPLE_FMT_S16;
    } else if (fmt.sampleSize() == 32) {
        if (fmt.sampleType() == QAudioFormat::Float)
            out_fmt_ = AV_SAMPLE_FMT_FLT;
        else
            out_fmt_ = AV_SAMPLE_FMT_S32;
    } else {
        out_fmt_ = AV_SAMPLE_FMT_S16;
    }

    qDebug()
        << "========== Qt REAL Audio Format ==========";
    qDebug()
        << "sampleRate:" << out_sample_rate_;
    qDebug()
        << "channels:" << out_channels_;
    qDebug()
        << "sampleSize:" << fmt.sampleSize();
    qDebug()
        << "sampleType:" << fmt.sampleType();
    qDebug()
        << "==========================================";

    open(QIODevice::ReadOnly);
}

qint64 FFmpegIODevice::readData(char *data, qint64 maxSize)
{
    if (!target_ffmpeg_global_ || !data || maxSize <= 0)
        return 0;

    qint64 copied = 0;

    // ============================================
    // 1. 先消费内部缓存
    // ============================================
    {
        QMutexLocker locker(&m_mutex);
        if (!m_buffer.isEmpty()) {
            copied = qMin(maxSize, (qint64)m_buffer.size());
            memcpy(data, m_buffer.constData(), copied);
            m_buffer.remove(0, copied);
        }
    }

    // ============================================
    // 2. 缓冲不足，解码一帧
    // ============================================
    if (copied < maxSize) {
        MyAVFrame *audio_frame = nullptr;

        // 使用合理超时，避免 0 超时疯狂轮询
        if (target_ffmpeg_global_->audio_AVFrame_Queue_->dequeue(&audio_frame, 100)) {
            if (audio_frame && audio_frame->frame_ && audio_frame->frame_->data[0]
                && audio_frame->frame_->nb_samples > 0) {

                AVFrame *af = audio_frame->frame_.get();

                // ===== 帧 PTS =====
                double frame_pts = 0.0;
                if (af->pts != AV_NOPTS_VALUE) {
                    QMutexLocker tbLocker(&target_ffmpeg_global_->audio_TimeBase_Mutex_);
                    frame_pts = af->pts * av_q2d(target_ffmpeg_global_->audio_TimeBase_);
                } else if (af->best_effort_timestamp != AV_NOPTS_VALUE) {
                    QMutexLocker tbLocker(&target_ffmpeg_global_->audio_TimeBase_Mutex_);
                    frame_pts = af->best_effort_timestamp
                                * av_q2d(target_ffmpeg_global_->audio_TimeBase_);
                }

                // ===== 帧时长（用原始采样率）=====
                double frame_duration = (double)af->nb_samples / af->sample_rate;

                // ===== 重采样 =====
                {
                    QMutexLocker swrLocker(&target_ffmpeg_global_->swr_ctx_Mutex_);

                    SwrContext *swr = target_ffmpeg_global_->swr_ctx_;
                    if (!swr) {
                        delete audio_frame;
                        // 无重采样器，填充静音
                        memset(data + copied, 0, maxSize - copied);
                        return maxSize;
                    }

                    int64_t delay = swr_get_delay(swr, af->sample_rate);
                    int maxOutSamples = av_rescale_rnd(delay + af->nb_samples,
                                                       out_sample_rate_,
                                                       af->sample_rate,
                                                       AV_ROUND_UP);

                    int outBufferSize = av_samples_get_buffer_size(nullptr,
                                                                   out_channels_,
                                                                   maxOutSamples,
                                                                   out_fmt_,
                                                                   1);

                    uint8_t *outBuffer = (uint8_t *)av_malloc(outBufferSize);
                    if (!outBuffer) {
                        delete audio_frame;
                        memset(data + copied, 0, maxSize - copied);
                        return maxSize;
                    }

                    int outSamples = swr_convert(swr,
                                                 &outBuffer,
                                                 maxOutSamples,
                                                 (const uint8_t **)af->extended_data,
                                                 af->nb_samples);

                    if (outSamples > 0) {
                        int real_bytes = av_samples_get_buffer_size(nullptr,
                                                                    out_channels_,
                                                                    outSamples,
                                                                    out_fmt_,
                                                                    1);

                        qint64 remain = maxSize - copied;
                        qint64 write_now = qMin((qint64)real_bytes, remain);

                        // 先满足 Qt 本次请求
                        memcpy(data + copied, outBuffer, write_now);
                        copied += write_now;

                        // 剩余 PCM 存入缓存
                        if (real_bytes > write_now) {
                            QMutexLocker locker(&m_mutex);
                            m_buffer.append((const char *)outBuffer + write_now,
                                            real_bytes - write_now);
                        }

                        // ===== 音频时钟更新：考虑硬件缓冲延迟 =====
                        double provided_ratio = (double)write_now / real_bytes;
                        double now_clock = frame_pts + frame_duration * provided_ratio;
                        if (now_clock < 0.0) now_clock = 0.0;

                        // 减去已提交但尚未播放的硬件缓冲时长，得到实际听到的位置
                        QAudioOutput *audioOutput =
                            target_ffmpeg_global_->clp_FFmpegTargetAudioDeviceInfo_
                                ->targetAudioOutput;
                        if (audioOutput) {
                            int bufferSize = audioOutput->bufferSize();
                            int bytesFree = audioOutput->bytesFree();
                            int bufferedBytes = bufferSize - bytesFree;
                            int sampleSize = av_get_bytes_per_sample(out_fmt_);
                            double bufferedDuration =
                                (double)bufferedBytes
                                / (out_sample_rate_ * out_channels_ * sampleSize);
                            double actualClock = now_clock - bufferedDuration;
                            if (actualClock >= 0.0)
                                now_clock = actualClock;
                        }

                        target_ffmpeg_global_->update_master_clock(
                            now_clock, audio_frame->serial_.load());
                    }

                    av_free(outBuffer);
                }
            }

            delete audio_frame;
        }
    }

    // ============================================
    // 3. 不足补静音
    // ============================================
    if (copied < maxSize) {
        memset(data + copied, 0, maxSize - copied);
    }

    return maxSize;
}

qint64 FFmpegIODevice::writeData(const char *data, qint64 maxSize)
{
    return -1;
}
