#pragma once

#include <QObject>
#include <QThread>
#include <qglobal.h>

#include "FeedBackC/ffmpage/public_space.h"

#include "FeedBackC/ffmpage/publicStruct/ffmpeg_public_struct.h"

class FFmpegGlobal;

///
/// @brief The FFmpegUnpackageThread class
/// 该类负责根据流类型交给对应的流处理线程（音频/视频）
class FFmpegUnpackageThread : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegUnpackageThread(FFmpegGlobal *target, QObject *parent = nullptr);
    ~FFmpegUnpackageThread();



public slots:
    void startUnpackage(); ///开始解包
    void stopUnpackage(); ///停止解包

private:
    void InitMember(); ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

private:
    FFmpegGlobal *target_ffmpeg_global_ = nullptr;    ///该对象需访问 FFmpegGlobal 对象的部分资源(构造时指定，该对象内部不负责析构)

};
