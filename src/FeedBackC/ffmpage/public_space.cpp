#include "FeedBackC/ffmpage/public_space.h"

// AVFrameSharedPtr createAVFrame()
// {
//     AVFrame *raw = av_frame_alloc();
//     if (!raw)
//         return nullptr;
//     return AVFrameSharedPtr(raw, AVFrameDeleter());
// }

QString GlobalTool::formatTime(double currentSeconds, double totalSeconds) {
    auto toHMS = [](double sec) -> QString {
        int h = (int) (sec) / 3600;
        int m = ((int) (sec) % 3600) / 60;
        int s = (int) (sec) % 60;
        if (h > 0)
            return QString("%1:%2:%3")
                .arg(h, 2, 10, QChar('0'))
                .arg(m, 2, 10, QChar('0'))
                .arg(s, 2, 10, QChar('0'));
        else
            return QString("%1:%2").arg(m, 1, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    };

    return QString("%1/%2").arg(toHMS(currentSeconds), toHMS(totalSeconds));
}
