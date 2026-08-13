#pragma once
///
/// @brief 该头文件仅用于声明公共库、公共枚举、公共函数等
///

/// windows库
#include <tchar.h>
#include <windows.h> // Windows API
#include <winnt.h>

/// c/c++库/**************************************************************/

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/fifo.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
}
/// Qt库 /**************************************************************/

///<    Core模块
#include <QDir>
#include <QMessageBox>
#include <QMetaObject>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QWaitCondition>

#include <QFileDialog>
#include <QMessageBox>
#include <QMutexLocker>
#include <QTimer>

#include <QApplication>
#include <QDrag>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QIODevice>
#include <QMimeData>
#include <QMovie>
#include <QObject>
#include <QPainter>
#include <QReadWriteLock>
#include <QScrollBar>
#include <QWheelEvent>
#include <QPushButton>

///<    Sql模块
// #include <QSqlDatabase>
// #include <QSqlError>
// #include <QSqlQuery>
// #include <QStandardPaths>

#include <QDebug>
#include <QPixmap>
#include <QString>
#include <QVector>

#include <QMetaType>

///<    音视频模块
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QAudioOutput>

/// 自定义枚举/数据结构/函数指针/公共命名空间 /**************************************************************/
/// 自定义枚举

/// 自定义数据结构

/// 自定义函数指针

/// 公共命名空间
namespace GlobalTool {
    ///  视频播放进度格式
    QString formatTime(double currentSeconds, double totalSeconds);
}

