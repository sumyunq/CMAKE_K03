#include "FeedBackC/ffmpage/publicStruct/AVFrame_queue.h"

AVFrameQueue::AVFrameQueue(int size)
{
    this->max_size_ = size;
}

AVFrameQueue::~AVFrameQueue()
{
    flush();
}

bool AVFrameQueue::enqueue(MyAVFrame *Frame) {
    QMutexLocker locker(&mutex_);
    while (queue_.size() >= max_size_ && !stopped_)
        condNotFull_.wait(&mutex_);

    if (stopped_) return false;

    queue_.enqueue(Frame);
    condNotEmpty_.wakeOne();
    return true;
}


bool AVFrameQueue::dequeue(MyAVFrame **frame, int timeout_ms) {
    QMutexLocker locker(&mutex_);

    while (queue_.isEmpty() && !stopped_) {
        if (timeout_ms < 0) {
            // 无限等待
            condNotEmpty_.wait(&mutex_);
        } else {
            if (!condNotEmpty_.wait(&mutex_, timeout_ms)) {
                qWarning() << "缓存帧队列 等待数据超时（" << timeout_ms
                           << "ms），可能解码已结束或卡住。" << this;
                // return false;
            }
        }

    }
    if (stopped_) return false;
    *frame = queue_.dequeue();
    condNotFull_.wakeOne();
    return true;
}


void AVFrameQueue::flush() {
    QMutexLocker locker(&mutex_);
    while (!queue_.isEmpty()) {
        MyAVFrame *frame = queue_.dequeue();
        delete frame;  // 释放帧内存
    }

    condNotFull_.wakeAll();
}
void AVFrameQueue::start() {
    QMutexLocker locker(&mutex_);
    stopped_ = false;
    condNotEmpty_.wakeAll();
    condNotFull_.wakeAll();
}
void AVFrameQueue::stop() {
    QMutexLocker locker(&mutex_);
    stopped_ = true;
    condNotEmpty_.wakeAll();
    condNotFull_.wakeAll();
}

int AVFrameQueue::size()
{
    QMutexLocker locker(&mutex_);
    return queue_.size();
}
