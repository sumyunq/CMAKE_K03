#include "FeedBackC/ffmpage/ThreadModule/ffmpeg_decoder_thread.h"

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_global.h"

FFmpegDecoderThread::FFmpegDecoderThread(FFmpegGlobal *target, QObject *parent)
    : target_ffmpeg_global_(target)
{
    InitUIInformation();
    InitMember();
    InitConnect();
}

FFmpegDecoderThread::~FFmpegDecoderThread()
{
}

void FFmpegDecoderThread::InitUIInformation() {}

void FFmpegDecoderThread::InitMember() {}

void FFmpegDecoderThread::InitConnect() {}

void FFmpegDecoderThread::startDecoderVideo()
{
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this << "startDecoderVideo start";

    while (!target_ffmpeg_global_->cl_is_stop()) {
        MyAVPacket *avPacket = nullptr;

        if (target_ffmpeg_global_->video_AVPacket_Queue_->dequeue(&avPacket, 3000)) {
            AVFrame *video_frame = av_frame_alloc();
            if (!video_frame) {
                delete avPacket;
                continue;
            }

            {
                QMutexLocker locker(&target_ffmpeg_global_->cl_decoder_video_Mutex_);
                Decoder *decoder = target_ffmpeg_global_->cl_decoder_video_.get();
                if (!decoder || !decoder->avctx_) {
                    qDebug() << __FILE__ << __FUNCTION__ << "video decoder or avctx is nullptr";
                    delete avPacket;
                    av_frame_free(&video_frame);
                    continue;
                }

                int send_ret = avcodec_send_packet(decoder->avctx_.get(),
                                                    avPacket->pkt_.get());
                // 处理 EAGAIN：需先收帧再重发
                if (send_ret == AVERROR(EAGAIN)) {
                    while (avcodec_receive_frame(decoder->avctx_.get(), video_frame) >= 0) {
                        MyAVFrame *t_AVFrame = new MyAVFrame();
                        t_AVFrame->frame_.reset(av_frame_clone(video_frame));
                        if (t_AVFrame->frame_) {
                            t_AVFrame->serial_.store(avPacket->serial_.load());
                            if (!target_ffmpeg_global_->video_AVFrame_Queue_->enqueue(t_AVFrame)) {
                                delete t_AVFrame;
                            }
                        } else {
                            delete t_AVFrame;
                        }
                        av_frame_unref(video_frame);
                    }
                    // 重试发送
                    send_ret = avcodec_send_packet(decoder->avctx_.get(),
                                                    avPacket->pkt_.get());
                }
                if (send_ret >= 0) {
                    while (avcodec_receive_frame(decoder->avctx_.get(), video_frame) >= 0) {
                        MyAVFrame *t_AVFrame = new MyAVFrame();
                        t_AVFrame->frame_.reset(av_frame_clone(video_frame));
                        if (!t_AVFrame->frame_) {
                            delete t_AVFrame;
                            av_frame_unref(video_frame);
                            continue;
                        }
                        t_AVFrame->serial_.store(avPacket->serial_.load());
                        if (!target_ffmpeg_global_->video_AVFrame_Queue_->enqueue(t_AVFrame)) {
                            delete t_AVFrame;
                            qWarning() << "视频帧入队失败";
                        } else {
                            target_ffmpeg_global_->ts_allVideoFrameCount.fetch_add(1);
                        }
                        av_frame_unref(video_frame);
                    }
                }
            }

            av_frame_free(&video_frame);
            delete avPacket;

            // 暂停等待（条件变量）
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

    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this << "startDecoderVideo end";
}

void FFmpegDecoderThread::stopDecoderVideo() {}

void FFmpegDecoderThread::startDecoderAudio()
{
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this << "startDecoderAudio start";

    while (!target_ffmpeg_global_->cl_is_stop()) {
        MyAVPacket *avPacket = nullptr;

        if (target_ffmpeg_global_->audio_AVPacket_Queue_->dequeue(&avPacket, 3000)) {
            AVFrame *audio_frame = av_frame_alloc();
            if (!audio_frame) {
                delete avPacket;
                continue;
            }

            {
                Decoder *decoder = target_ffmpeg_global_->cl_decoder_audio_.get();
                if (!decoder || !decoder->avctx_) {
                    qDebug() << __FILE__ << __FUNCTION__ << "audio decoder or avctx is nullptr";
                    delete avPacket;
                    av_frame_free(&audio_frame);
                    continue;
                }

                QMutexLocker locker(&decoder->avctx_Mutex_);

                int send_ret = avcodec_send_packet(decoder->avctx_.get(),
                                                    avPacket->pkt_.get());
                if (send_ret == AVERROR(EAGAIN)) {
                    while (avcodec_receive_frame(decoder->avctx_.get(), audio_frame) >= 0) {
                        MyAVFrame *t_AVFrame = new MyAVFrame();
                        t_AVFrame->frame_.reset(av_frame_clone(audio_frame));
                        if (t_AVFrame->frame_) {
                            t_AVFrame->serial_.store(avPacket->serial_.load());
                            if (!target_ffmpeg_global_->audio_AVFrame_Queue_->enqueue(t_AVFrame)) {
                                delete t_AVFrame;
                            }
                        } else {
                            delete t_AVFrame;
                        }
                        av_frame_unref(audio_frame);
                    }
                    send_ret = avcodec_send_packet(decoder->avctx_.get(),
                                                    avPacket->pkt_.get());
                }
                if (send_ret >= 0) {
                    while (avcodec_receive_frame(decoder->avctx_.get(), audio_frame) >= 0) {
                        MyAVFrame *t_AVFrame = new MyAVFrame();
                        t_AVFrame->frame_.reset(av_frame_clone(audio_frame));
                        if (!t_AVFrame->frame_) {
                            delete t_AVFrame;
                            av_frame_unref(audio_frame);
                            continue;
                        }
                        t_AVFrame->serial_.store(avPacket->serial_.load());
                        if (!target_ffmpeg_global_->audio_AVFrame_Queue_->enqueue(t_AVFrame)) {
                            delete t_AVFrame;
                            qWarning() << "音频帧入队失败";
                        } else {
                            target_ffmpeg_global_->ts_allAudioFrameCount.fetch_add(1);
                        }
                        av_frame_unref(audio_frame);
                    }
                }
            }

            av_frame_free(&audio_frame);
            delete avPacket;

            // 暂停等待（条件变量）
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

    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this << "startDecoderAudio end";
}

void FFmpegDecoderThread::stopDecoderAudio() {}
