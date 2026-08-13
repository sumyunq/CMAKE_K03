#include "FeedBackC/ffmpage/publicStruct/AVPacket_queue.h"

AVPacketQueue::AVPacketQueue(int size)
{
    max_size_.store (size);
    cl_AVPacket_queue_.clear();

}

AVPacketQueue::~AVPacketQueue()
{
    flush();
}

bool AVPacketQueue::enqueue(MyAVPacket *pkt)
{
    QMutexLocker locker(&mutex_);
    while (cl_AVPacket_queue_.size() >= max_size_ && !stopped_) {
        condNotFull_.wait(&mutex_);
    }

    if (stopped_)
        return false;

    cl_AVPacket_queue_.enqueue(pkt);
    condNotEmpty_.wakeOne();
    return true;
}

bool AVPacketQueue::dequeue(MyAVPacket **pkt,int timeout_ms)
{
    QMutexLocker locker(&mutex_);

    while (cl_AVPacket_queue_.isEmpty() && !stopped_) {
        if (!condNotEmpty_.wait(&mutex_, timeout_ms)) {
            qWarning() << "缓存帧队列 等待数据超时（" << timeout_ms
                       << "ms），可能解码已结束或卡住。" << this;
            return false;
        }
    }

    if (stopped_)
        return false;

    *pkt = cl_AVPacket_queue_.dequeue();

    condNotFull_.wakeOne();
    return true;
}

void AVPacketQueue::flush()
{
    QMutexLocker locker(&mutex_);
    qWarning() << "压缩数据包队列 大小：" << cl_AVPacket_queue_.size();


    while (!cl_AVPacket_queue_.isEmpty()) {
        MyAVPacket *pkt = cl_AVPacket_queue_.dequeue();
        delete pkt;
    }

    serial_.fetch_add(1, std::memory_order_release);
    condNotFull_.wakeAll();
}

void AVPacketQueue::start()
{
    QMutexLocker locker(&mutex_);
    stopped_ = false;
    condNotEmpty_.wakeAll();
    condNotFull_.wakeAll();
}

void AVPacketQueue::stop()
{
    QMutexLocker locker(&mutex_);
    stopped_ = true;
    condNotEmpty_.wakeAll();
    condNotFull_.wakeAll();
}

int AVPacketQueue::size()
{
    QMutexLocker locker(&mutex_);
    return cl_AVPacket_queue_.size();
}
