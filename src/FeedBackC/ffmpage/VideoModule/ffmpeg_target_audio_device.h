#ifndef FFMPEG_TARGET_AUDIO_DEVICE_H
#define FFMPEG_TARGET_AUDIO_DEVICE_H

#include <QObject>

#include "FeedBackC/ffmpage/public_space.h"

#include "FeedBackC/ffmpage/VideoModule/ffmpeg_iodevice.h"

///
/// \brief The FFmpegTargetAudioDevice class
/// 流媒体文件音频 输出到 目标音频设备
class FFmpegTargetAudioDevice : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegTargetAudioDevice(FFmpegGlobal *target,QObject *parent = nullptr);
    ~FFmpegTargetAudioDevice() override;


    /// QT音频相关（用于处理音频流信息）
    QList<QAudioDeviceInfo> devices;

    /// 音频流目标格式(具体取决与输出设备)
    mutable QMutex targetFormat_Mutex_;
    QAudioFormat targetFormat; ///输出设备的最佳匹配格式

    /// 具体音频输出设备(必须全局唯一)
    mutable QMutex targetDevice_Mutex_;
    QAudioDeviceInfo targetDevice;
    mutable QMutex targetAudioOutput_Mutex_;
    QAudioOutput *targetAudioOutput = nullptr;
    mutable QMutex targetAudioDevice_Mutex_;
    QIODevice *targetAudioDevice = nullptr;

    std::unique_ptr<FFmpegIODevice> cl_ffmpegIODevice_ = nullptr; ///

public slots:
    void initAudio();


signals:

private:
    void InitMember();  ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

private:
    FFmpegGlobal *target_ffmpeg_global_
        = nullptr; ///该对象需访问 FFmpegGlobal 对象的部分资源(构造时指定，该对象内部不负责析构)

};

#endif // FFMPEG_TARGET_AUDIO_DEVICE_H
