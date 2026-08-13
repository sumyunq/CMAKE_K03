#pragma once

#include <QIODevice >
#include <QObject>
#include <qglobal.h>

#include "FeedBackC/ffmpage/public_space.h"

class FFmpegGlobal;

class FFmpegIODevice : public QIODevice
{
    Q_OBJECT
public:
    FFmpegIODevice(FFmpegGlobal *target);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    FFmpegGlobal *target_ffmpeg_global_ = nullptr;

    // 音频输出参数（从 QAudioFormat / FFmpegGlobal 初始化）
    int out_sample_rate_ = 44100;
    int out_channels_ = 2;
    int64_t out_channel_layout_ = 0;
    AVSampleFormat out_fmt_ = AV_SAMPLE_FMT_S16;

    // 内部缓冲区
    QByteArray m_buffer;
    mutable QMutex m_mutex;
};
