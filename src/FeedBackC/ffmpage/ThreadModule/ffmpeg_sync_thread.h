#ifndef FFMPEG_SYNC_THREAD_H
#define FFMPEG_SYNC_THREAD_H

#include <QObject>

#include "FeedBackC/ffmpage/public_space.h"

#include "FeedBackC/ffmpage/publicStruct/ffmpeg_public_struct.h"

class FFmpegGlobal;

///
/// \brief The FFmpegSyncThread class
/// 同步显示线程,操作  视频缓存帧 队列的数据, 进行图像显示
class FFmpegSyncThread : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegSyncThread(FFmpegGlobal *target, QObject *parent = nullptr);
    ~FFmpegSyncThread();

signals:
    void frameReady(const QImage &img);
    void currentPlayingTimes(double times);

public slots:
    /// 若为视频同步线程
    void startSyncVideo(); ///开始同步视频流
    void stopSyncVideo();  ///停止同步视频流
    /// 若为音频同步线程
    void startSyncAudio(); ///开始同步音频流
    void stopSyncAudio();  ///开始同步音频流
private:
    void InitMember();  ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

private:
    FFmpegGlobal *target_ffmpeg_global_
        = nullptr; ///该对象需访问 FFmpegGlobal 对象的部分资源(构造时指定，该对象内部不负责析构)
};

#endif // FFMPEG_SYNC_THREAD_H
