#ifndef AVPACKET_QUEUE_H
#define AVPACKET_QUEUE_H

#include <QDebug>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

#include "FeedBackC/ffmpage/public_space.h"

struct AVPacketDeleter
{
    void operator()(AVPacket *ptr) const
    {
        if (ptr)
            av_packet_free(&ptr); ///默认堆对象
    }
};
using AVPacketUniquePtr = std::unique_ptr<AVPacket, AVPacketDeleter>;

///封装 AVPacket 为自定义的、带序列号的包
typedef struct MyAVPacket
{
    AVPacketUniquePtr pkt_;
    std::atomic<int> serial_;
} MyAVPacket;
using MyAVPacketUniquePtr = std::unique_ptr<MyAVPacket>;


class AVPacketQueue
{
public:
    AVPacketQueue(int size = 64);
    ~AVPacketQueue();

    bool enqueue(MyAVPacket *pkt); /// 解包线程放数据入队
    bool dequeue(MyAVPacket **pkt,
                int timeout_ms
                = 30000); /// 解码线程取数据，并自行释放取出的数据包(默认没数据时 30s 打印一次提醒)
    void flush();         /// 清空队列（Seek 时使用）
    void start();         /// 启动队列（用于启动所有线程）
    void stop();          /// 停止队列（释放所有等待线程）
    int size();           /// 获取当前队列数据量



private:

    QQueue<MyAVPacket *> cl_AVPacket_queue_;
    QMutex mutex_;
    QWaitCondition condNotEmpty_; // 队列非空信号
    QWaitCondition condNotFull_;  // 队列非满信号（可选）

    std::atomic<bool> stopped_ = false;
    std::atomic<int> max_size_ = 64;
    std::atomic<int> nb_packets_;
    std::atomic<int> size_;
    std::atomic<int64_t> duration_;
    std::atomic<int> abort_request_;
    std::atomic<int> serial_;

    // AVFifo *pkt_list;
    // int nb_packets;
    // int size;
    // int64_t duration;
    // int abort_request;
    // int serial;
    // SDL_mutex *mutex;
    // SDL_cond *cond;
};

#endif // AVPACKET_QUEUE_H
