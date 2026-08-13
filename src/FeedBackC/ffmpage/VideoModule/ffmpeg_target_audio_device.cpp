#include "FeedBackC/ffmpage/VideoModule/ffmpeg_target_audio_device.h"

#include <QAudioDeviceInfo>
#include <QAudioFormat>

#include <QDebug>

FFmpegTargetAudioDevice::FFmpegTargetAudioDevice(FFmpegGlobal *target, QObject *parent)
    : target_ffmpeg_global_(target),QObject{parent}
{
    InitMember();  ///< 初始化内部成员
    InitConnect(); ///< 连接默认的信号槽
}

FFmpegTargetAudioDevice::~FFmpegTargetAudioDevice()
{
    if (targetAudioOutput) {
        targetAudioOutput->stop();
        delete targetAudioOutput;
        targetAudioOutput = nullptr;
    }
}

void FFmpegTargetAudioDevice::initAudio() {
    qDebug() << "初始化 initAudio (Push Mode)";

    QMutexLocker locker(&targetDevice_Mutex_);
    // 先清理旧设备（可重复调用）
    if (targetAudioOutput) {
        targetAudioOutput->stop();
        delete targetAudioOutput;
        targetAudioOutput = nullptr;
        targetAudioDevice = nullptr;
    }
    targetAudioOutput = new QAudioOutput(targetDevice, targetFormat, nullptr);
    targetAudioOutput->setBufferSize(targetFormat.bytesForDuration(200000));
    // 推模式：start() 返回内部 QIODevice，由 startSyncAudio 线程通过它写入 PCM
    targetAudioDevice = targetAudioOutput->start();
    qDebug() << "初始化 initAudio PASS (Push Mode)";
}

void FFmpegTargetAudioDevice::InitMember()
{
    bool open = true;
    if (open) {
        // 获取所有可用的音频输出设备
        // devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        // for (const QAudioDeviceInfo &deviceInfo : devices) {
        //     qDebug() << "设备名称:" << deviceInfo.deviceName();
        // }
        // targetDevice =devices.at(0);///指定播放设备



        ///默认播放设备
        QMutexLocker locker(&targetDevice_Mutex_);
        targetDevice = QAudioDeviceInfo::defaultOutputDevice();
        if (targetDevice.isNull()) {
            qDebug() << "错误：默认音频输出设备无效！";
        }

        qDebug() << "设备名称:" << targetDevice.deviceName();

        // 获取推荐格式
        QAudioFormat preferredFormat = targetDevice.preferredFormat();
        qDebug() << "推荐采样率:" << preferredFormat.sampleRate();
        qDebug() << "推荐声道数:" << preferredFormat.channelCount();
        qDebug() << "推荐采样大小:" << preferredFormat.sampleSize();

        // 检查是否支持特定格式
        // QAudioFormat format;
        // format.setSampleRate(44100);
        // format.setChannelCount(2);
        // format.setSampleSize(16);
        // format.setCodec("audio/pcm");
        // format.setByteOrder(QAudioFormat::LittleEndian);
        // format.setSampleType(QAudioFormat::SignedInt);

        // if (!targetDevice.isFormatSupported(format)) {
        //     qWarning() << "原始格式不支持，使用推荐格式";
        //     format = targetDevice.nearestFormat(format);
        // }



        /// QT音频相关(已上锁)
        QMutexLocker locker_desFormat(&targetFormat_Mutex_);
        // targetFormat.setSampleRate(targetDevice.preferredFormat().sampleRate());
        // targetFormat.setSampleSize(targetDevice.preferredFormat().sampleSize());
        // targetFormat.setSampleType(targetDevice.preferredFormat().sampleType());
        // targetFormat.setChannelCount(targetDevice.preferredFormat().channelCount());
        // targetFormat.setCodec(targetDevice.preferredFormat().codec());
        // targetFormat.setByteOrder(targetDevice.preferredFormat().byteOrder());


        ///目标输出音频格式
         targetFormat.setSampleRate(44100);
         targetFormat.setChannelCount(2);
         targetFormat.setSampleSize(16);
         targetFormat.setCodec("audio/pcm");
         targetFormat.setByteOrder(QAudioFormat::LittleEndian);
         targetFormat.setSampleType(QAudioFormat::SignedInt);

        if (!targetDevice.isFormatSupported(targetFormat)) {
            targetFormat = targetDevice.nearestFormat(targetFormat);
            qDebug() << "使用最接近的音频格式";
        }

        /// 配置音频重采样器 (SwrContext) 参数,需与 desiredFormat 相匹配
        /// 目标格式：44100Hz，立体声，S16 交错格式
        // outChannelLayout = av_get_default_channel_layout(targetFormat.channelCount());
        // outSampleRate = targetFormat.sampleRate();
        // outSampleFmt = AV_SAMPLE_FMT_S16; // S16 对应 Qt 的 SignedInt
        // outChannels = targetFormat.channelCount();





        // targetAudioDevice =



        // if (!targetAudioDevice) {
        //     ///确定是否有音频设备
        //     qWarning() << "QAudioOutput::start() failed, targetAudioDevice is null";
        // }



    }
}

void FFmpegTargetAudioDevice::InitConnect() {}
