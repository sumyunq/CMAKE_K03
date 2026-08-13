#ifndef VIDEO_DATA_STRUCT_H
#define VIDEO_DATA_STRUCT_H

#include <QApplication>
#include <QString>
#include <memory>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include "data/api_global.h"

///
/// \brief 视频数据管理类
///
class VideoData : public QObject
{
    Q_OBJECT
public:
    VideoData(QObject *parent = nullptr);
    ~VideoData();

public:
    DeSheng::videoConfig cl_video_config_local_;   ///视频配置信息(本地)
    DeSheng::videoConfig cl_video_config_netWork_; ///视频配置信息(网络)

private:
    void InitMember(); ///< 初始化内部成员
    void InitConnect();

private:
};

#endif // VIDEO_DATA_STRUCT_H
