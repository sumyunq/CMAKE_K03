#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_sync_thread.h"

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"

FFmpegSyncThread::FFmpegSyncThread(FFmpegGlobal *target, QObject *parent)
    : target_ffmpeg_global_(target)
    , QObject{parent}
{
    InitMember();
    InitConnect();
}

FFmpegSyncThread::~FFmpegSyncThread() {}

void FFmpegSyncThread::InitMember() {}

void FFmpegSyncThread::InitConnect() {}

void FFmpegSyncThread::startSyncVideo()
{
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "startSyncVideo start";

    if (!target_ffmpeg_global_) {
        qDebug() << "target_ffmpeg_global_ is nullptr";
        return;
    }

    AVFrame *video_frame = av_frame_alloc();
    AVFrame *video_rgbFrame = av_frame_alloc();

    if (!video_frame || !video_rgbFrame) {
        av_frame_free(&video_frame);
        av_frame_free(&video_rgbFrame);
        qDebug() << "av_frame_alloc is failed";
        return;
    }

    while (!target_ffmpeg_global_->cl_is_stop()) {
        MyAVFrame *Frame = nullptr;

        if (target_ffmpeg_global_->video_AVFrame_Queue_->dequeue(&Frame, 3000)) {
            // 1. 计算视频帧PTS（加锁读取时间基）
            double vpts = 0.0;
            {
                QMutexLocker tbLocker(&target_ffmpeg_global_->video_TimeBase_Mutex_);
                if (Frame->frame_->best_effort_timestamp != AV_NOPTS_VALUE) {
                    vpts = Frame->frame_->best_effort_timestamp
                           * av_q2d(target_ffmpeg_global_->video_TimeBase_);
                } else if (Frame->frame_->pts != AV_NOPTS_VALUE) {
                    vpts = Frame->frame_->pts * av_q2d(target_ffmpeg_global_->video_TimeBase_);
                }
            }

            // 2. seek 后丢弃关键帧之前的旧帧
            double seek_target = target_ffmpeg_global_->seek_target_pts_.load();
            if (seek_target >= 0.0
                && Frame->serial_.load() == target_ffmpeg_global_->seek_serial_.load()
                && vpts < seek_target - 0.1) {
                delete Frame;
                Frame = nullptr;
                av_frame_unref(video_frame);
                continue;
            }
            if (seek_target >= 0.0 && vpts >= seek_target) {
                target_ffmpeg_global_->seek_target_pts_.store(-1.0);
            }

            // 3. 获取音频时钟
            double audio_clock = target_ffmpeg_global_->get_master_clock();

            // 3. 计算差值
            double diff = vpts - audio_clock;

            qDebug() << "视频PTS:" << vpts << " 音频时钟:" << audio_clock << " diff:" << diff
                     << Qt::endl;

            // 4. 同步策略
            const double SYNC_THRESHOLD = 0.01;   // 10ms内认为同步
            const double NOSYNC_THRESHOLD = 10.0; // 超过10秒不同步

            if (fabs(diff) < NOSYNC_THRESHOLD) {
                if (diff > SYNC_THRESHOLD) {
                    // 视频超前：等待
                    int waitMs = static_cast<int>(diff * 1000);
                    waitMs = qMin(waitMs, 100); // 最多等100ms
                    if (waitMs > 1) {
                        QThread::msleep(waitMs);
                        audio_clock = target_ffmpeg_global_->get_master_clock();
                        diff = vpts - audio_clock;
                    }
                }

                if (diff < -SYNC_THRESHOLD) {
                    // 视频落后：检查是否落后太多需要丢帧
                    if (diff < -0.1) { // 落后超过100ms，丢帧
                        delete Frame;
                        Frame = nullptr;
                        av_frame_unref(video_frame);
                        continue;
                    }
                }

                // 5. 格式转换
                {
                    QMutexLocker locker(&target_ffmpeg_global_->buffer_Mutex_);
                    av_image_fill_arrays(video_rgbFrame->data,
                                         video_rgbFrame->linesize,
                                         target_ffmpeg_global_->buffer_,
                                         AV_PIX_FMT_RGB24,
                                         target_ffmpeg_global_->src_width_,
                                         target_ffmpeg_global_->src_height_,
                                         1);
                }
                {
                    QMutexLocker locker(&target_ffmpeg_global_->sub_convert_ctx_Mutex_);

                    sws_scale(target_ffmpeg_global_->sub_convert_ctx_,
                              Frame->frame_->data,
                              Frame->frame_->linesize,
                              0,
                              target_ffmpeg_global_->src_height_,
                              video_rgbFrame->data,
                              video_rgbFrame->linesize);
                }
                // 6. 创建QImage并发送
                QImage img(video_rgbFrame->data[0],
                           target_ffmpeg_global_->src_width_,
                           target_ffmpeg_global_->src_height_,
                           video_rgbFrame->linesize[0],
                           QImage::Format_RGB888);

                img = img.copy(0,
                               0,
                               target_ffmpeg_global_->src_width_,
                               target_ffmpeg_global_->src_height_);

                emit frameReady(img.copy());
                target_ffmpeg_global_->ts_allVideoFrameCount.fetch_add(1);
            }

            delete Frame;
            Frame = nullptr;
            av_frame_unref(video_frame);
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

    av_frame_free(&video_frame);
    av_frame_free(&video_rgbFrame);
}

void FFmpegSyncThread::stopSyncVideo() {}

void FFmpegSyncThread::startSyncAudio()
{
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "startSyncAudio start";

    if (!target_ffmpeg_global_) {
        qDebug() << "target_ffmpeg_global_ is nullptr";
        return;
    }

    while (!target_ffmpeg_global_->cl_is_stop()) {
        MyAVFrame *Frame = nullptr;

        if (target_ffmpeg_global_->audio_AVFrame_Queue_->dequeue(&Frame, 3000)) {
            AVFrame *af = Frame->frame_.get();
            if (!af || !af->data[0] || af->nb_samples <= 0) {
                delete Frame;
                continue;
            }

            // 1. 计算帧 PTS
            double frame_pts = 0.0;
            {
                QMutexLocker tbLocker(&target_ffmpeg_global_->audio_TimeBase_Mutex_);
                if (af->pts != AV_NOPTS_VALUE)
                    frame_pts = af->pts * av_q2d(target_ffmpeg_global_->audio_TimeBase_);
                else if (af->best_effort_timestamp != AV_NOPTS_VALUE)
                    frame_pts = af->best_effort_timestamp
                                * av_q2d(target_ffmpeg_global_->audio_TimeBase_);
            }

            double frame_duration = (double)af->nb_samples / af->sample_rate;

            // 2. seek 后丢弃关键帧之前的旧音频帧
            double audio_seek_target = target_ffmpeg_global_->seek_target_pts_.load();
            if (audio_seek_target >= 0.0
                && Frame->serial_.load() == target_ffmpeg_global_->seek_serial_.load()
                && frame_pts < audio_seek_target - 0.1) {
                delete Frame;
                continue;
            }
            if (audio_seek_target >= 0.0 && frame_pts >= audio_seek_target) {
                target_ffmpeg_global_->seek_target_pts_.store(-1.0);
            }

            // 3. 重采样
            int out_sample_rate;
            int out_channels;
            {
                QMutexLocker swrLocker(&target_ffmpeg_global_->swr_ctx_Mutex_);
                SwrContext *swr = target_ffmpeg_global_->swr_ctx_;
                if (!swr) {
                    delete Frame;
                    continue;
                }

                QAudioFormat &fmt =
                    target_ffmpeg_global_->clp_FFmpegTargetAudioDeviceInfo_->targetFormat;
                out_sample_rate = fmt.sampleRate();
                out_channels = fmt.channelCount();
                AVSampleFormat out_fmt;
                if (fmt.sampleSize() == 16)
                    out_fmt = AV_SAMPLE_FMT_S16;
                else if (fmt.sampleType() == QAudioFormat::Float)
                    out_fmt = AV_SAMPLE_FMT_FLT;
                else
                    out_fmt = AV_SAMPLE_FMT_S32;

                int64_t delay = swr_get_delay(swr, af->sample_rate);
                int maxOutSamples = av_rescale_rnd(delay + af->nb_samples,
                                                   out_sample_rate,
                                                   af->sample_rate,
                                                   AV_ROUND_UP);

                int outBufferSize = av_samples_get_buffer_size(
                    nullptr, out_channels, maxOutSamples, out_fmt, 1);
                uint8_t *outBuffer = (uint8_t *)av_malloc(outBufferSize);
                if (!outBuffer) {
                    delete Frame;
                    continue;
                }

                int outSamples = swr_convert(swr,
                                             &outBuffer,
                                             maxOutSamples,
                                             (const uint8_t **)af->extended_data,
                                             af->nb_samples);

                if (outSamples > 0) {
                    int real_bytes = av_samples_get_buffer_size(
                        nullptr, out_channels, outSamples, out_fmt, 1);

                    // 3. 写入音频设备（推模式：initAudio 已在主线程同步完成，设备就绪）
                    QAudioOutput *audioOutput =
                        target_ffmpeg_global_->clp_FFmpegTargetAudioDeviceInfo_->targetAudioOutput;
                    QIODevice *audioDevice =
                        target_ffmpeg_global_->clp_FFmpegTargetAudioDeviceInfo_->targetAudioDevice;
                    if (audioOutput && audioDevice
                        && audioOutput->state() != QAudio::StoppedState
                        && audioOutput->state() != QAudio::SuspendedState) {

                        // 等待缓冲区有空间
                        while (audioOutput->bytesFree() < real_bytes
                               && !target_ffmpeg_global_->cl_is_stop()) {
                            QThread::msleep(5);
                        }

                        if (!target_ffmpeg_global_->cl_is_stop()) {
                            audioDevice->write((const char *)outBuffer, real_bytes);

                            // 4. 更新音频时钟（减去硬件缓冲延迟）
                            int bufferSize = audioOutput->bufferSize();
                            int bytesFree = audioOutput->bytesFree();
                            int bufferedBytes = bufferSize - bytesFree;
                            int sampleSize = av_get_bytes_per_sample(out_fmt);
                            double bufferedDuration =
                                (double)bufferedBytes
                                / (out_sample_rate * out_channels * sampleSize);

                            double audio_clock = (frame_pts + frame_duration)
                                                 - bufferedDuration;
                            if (audio_clock < 0.0)
                                audio_clock = 0.0;

                            target_ffmpeg_global_->update_master_clock(
                                audio_clock, Frame->serial_.load());

                            /// 进度条显示用（不减去硬件缓冲延迟，保证与总时长对齐）
                            double display_time = frame_pts + frame_duration;
                            if (display_time < 0.0)
                                display_time = 0.0;
                            emit currentPlayingTimes(display_time);

                        }
                    } else {
                        // QAudioOutput 未就绪，不消耗此帧也不更新时钟
                        //（正常情况 initAudio 已在 startAllThread 中同步完成，不应走到这里）
                        qWarning() << "QAudioOutput 未就绪，丢弃音频帧";
                    }
                }

                av_free(outBuffer);
            }

            delete Frame;

            // 暂停等待
            {
                QMutexLocker locker(&target_ffmpeg_global_->cl_pause_Mutex_);
                while (target_ffmpeg_global_->cl_is_pause()
                       && !target_ffmpeg_global_->cl_is_stop()) {
                    target_ffmpeg_global_->cl_pause_Cond_.wait(
                        &target_ffmpeg_global_->cl_pause_Mutex_, 50);
                }
            }
        }
    }

    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << "startSyncAudio end";
}

void FFmpegSyncThread::stopSyncAudio() {}
