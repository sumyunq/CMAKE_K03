#ifndef FFMPEG_DECODER_THREAD_H
#define FFMPEG_DECODER_THREAD_H

#include <QObject>
#include <QThread>

#include "FeedBackC/ffmpage/public_space.h"

#include "FeedBackC/ffmpage/publicStruct/ffmpeg_public_struct.h"

class FFmpegGlobal;

///
/// \brief The FFmpegDecoderThread class
/// 处理对应的压缩包数据队列
class FFmpegDecoderThread : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegDecoderThread(FFmpegGlobal *target, QObject *parent = nullptr);
    ~FFmpegDecoderThread();

signals:

public slots:
    /// 若为视频解码线程
    void startDecoderVideo(); ///开始解码视频流
    void stopDecoderVideo();  ///停止解码视频流
    /// 若为视频解码线程
    void startDecoderAudio(); ///开始解码音频流
    void stopDecoderAudio();  ///开始解码音频流

private:
    void InitUIInformation(); ///< 初始化UI的默认信息(有的话)
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

private:
    FFmpegGlobal *target_ffmpeg_global_
        = nullptr; ///该对象需访问 FFmpegGlobal 对象的部分资源(构造时指定，该对象内部不负责析构)
};

#endif // FFMPEG_DECODER_THREAD_H
