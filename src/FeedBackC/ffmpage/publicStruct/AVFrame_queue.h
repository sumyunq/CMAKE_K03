#ifndef AVFRAME_QUEUE_H
#define AVFRAME_QUEUE_H

#include <QDebug>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "FeedBackC/ffmpage/public_space.h"

#include "FeedBackC/ffmpage/publicStruct/AVPacket_queue.h"


/// AVFrame 还需要更新为自定义的、带序列号的帧
struct AVFrameDeleter
{
    void operator()(AVFrame *ptr) const
    {
        if (ptr)
            av_frame_free(&ptr); ///默认堆对象
    }
};
using AVFrameUniquePtr = std::unique_ptr<AVFrame, AVFrameDeleter>;

/// 重定义 FFmpeg 数据帧结构
typedef struct MyAVFrame
{
    AVFrameUniquePtr frame_ = {nullptr};
    AVSubtitle sub_;
    std::atomic<int> serial_;
    std::atomic<double> pts_;      /* presentation timestamp for the frame */
    std::atomic<double> duration_; /* estimated duration of the frame */
    std::atomic<int64_t> pos_;     /* byte position of the frame in the input file */
    std::atomic<int> width_;
    std::atomic<int> height_;
    std::atomic<int> format_;
    AVRational sar_;
    std::atomic<int> uploaded_;
    std::atomic<int> flip_v_;
} MyAVFrame;

class AVFrameQueue
{
public:
    AVFrameQueue(int size = 64);
    ~AVFrameQueue();

    bool enqueue(MyAVFrame *Frame); ///解码线程放 帧数据 入队
    bool dequeue(MyAVFrame **Frame,
                 int timeout_ms); ///同步线程取 帧数据，并自行释放内部帧数据
    void flush();
    void start();
    void stop();
    int size();

private:
    QQueue<MyAVFrame *> queue_;
    QMutex mutex_;
    QWaitCondition condNotEmpty_; // 队列非空信号
    QWaitCondition condNotFull_;  // 队列非满信号（可选）
    bool stopped_ = false;
    int max_size_ = 64; // 防止内存爆炸

    std::atomic<int> rindex_;
    std::atomic<int> windex_;
    std::atomic<int> size_;
    std::atomic<int> keep_last_;
    std::atomic<int> rindex_shown_;

    std::unique_ptr<AVPacketQueue> pktq_ = nullptr;
};



#endif // AVFRAME_QUEUE_H

/// 参考结构
// /* Common struct for handling all types of decoded data and allocated render buffers. */
// typedef struct Frame
// {
//     AVFrame *frame;
//     AVSubtitle sub;
//     int serial;
//     double pts;      /* presentation timestamp for the frame */
//     double duration; /* estimated duration of the frame */
//     int64_t pos;     /* byte position of the frame in the input file */
//     int width;
//     int height;
//     int format;
//     AVRational sar;
//     int uploaded;
//     int flip_v;
// } Frame;

// typedef struct FrameQueue
// {
//     Frame queue[FRAME_QUEUE_SIZE];
//     int rindex;
//     int windex;
//     int size;
//     int max_size;
//     int keep_last;
//     int rindex_shown;
//     QMutex *mutex;
//     QWaitCondition *cond;
//     PacketQueue *pktq;
// } FrameQueue;
