#ifndef AUDITION_API_H
#define AUDITION_API_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QUrlQuery>
#include <memory>

#include <QDebug>

namespace DeSheng {

/// 路径常量
namespace ApiPaths {
inline constexpr const char *kAuditionList   = "/auditions";
inline constexpr const char *kAuditionDetail = "/auditions/%1";
} // namespace ApiPaths


/*************************************************************************************  试听视频相关  ************************************************************************************************/
/// 下载状态（本地字段，不来自 API）
enum class VideoStatus : quint8 { UnDownloaded = 0, Downloading = 1, Downloaded = 2 };

/// 视频条目
struct VideoItem
{
    /// 网络相关信息
    int id = 0;         ///id
    QString title;      ///标题
    QString scene;      ///场景标识
    QString sceneName;  ///场景中文名
    QString videoDesc;  ///详细描述
    QString imgUrl;     ///图片url
    QString videoUrl;   ///视频url
    QString deviceName; ///设备名称
    QString deviceType; ///设备类型
    QString status;     ///状态
    QString createdAt;  ///创建时间
    QString updatedAt;  ///最近更新时间

    /// 本地配置信息
    VideoStatus localStatus = VideoStatus::UnDownloaded; ///下载状态(默认未下载)
    QString localPath = "";                              ///本地视频文件路径
    QString coverLocalPath = "";                         ///本地封面图片路径
    QString oldVideoUrl
        = ""; /// 下载到本地后记录Url（id + oldVideoUrl == id + videoUrl 确定视频状态） ///校正标识

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["id"] = id;
        obj["title"] = title;
        obj["scene"] = scene;
        obj["scene_name"] = sceneName;
        obj["video_desc"] = videoDesc;
        obj["img_url"] = imgUrl;
        obj["video_url"] = videoUrl;
        obj["device_name"] = deviceName;
        obj["device_type"] = deviceType;
        obj["status"] = status;
        obj["created_at"] = createdAt;
        obj["updated_at"] = updatedAt;
        obj["localStatus"] = (int) localStatus;
        obj["localPath"] = localPath;
        obj["coverLocalPath"] = coverLocalPath;
        obj["oldVideoUrl"] = oldVideoUrl;
        return obj;
    }

    static VideoItem fromJson(const QJsonObject &obj)
    {
        VideoItem item;
        item.id = obj["id"].toInt();
        item.title = obj["title"].toString();
        item.scene = obj["scene"].toString();
        item.sceneName = obj["scene_name"].toString();
        item.videoDesc = obj["video_desc"].toString();
        item.imgUrl = obj["img_url"].toString();
        item.videoUrl = obj["video_url"].toString();
        item.deviceName = obj["device_name"].toString();
        item.deviceType = obj["device_type"].toString();
        item.status = obj["status"].toString();
        item.createdAt = obj["created_at"].toString();
        item.updatedAt = obj["updated_at"].toString();
        item.localStatus = (VideoStatus) obj["localStatus"].toInt(0);
        item.localPath = obj["localPath"].toString();
        item.coverLocalPath = obj["coverLocalPath"].toString();
        item.oldVideoUrl = obj["oldVideoUrl"].toString();
        return item;
    }
};

/// 视频类型映射
struct VideosType
{
    QString xhub_01;
    QString xhub_02;
    QString xhub_03;
    QString xhub_04;
    QString xhub_05;
    QString xhub_06;
    QString xhub_07;
    QString xhub_08;
    QString xhub_09;
    QString xhub_10;

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["xhub_01"] = xhub_01;
        obj["xhub_02"] = xhub_02;
        obj["xhub_03"] = xhub_03;
        obj["xhub_04"] = xhub_04;
        obj["xhub_05"] = xhub_05;
        obj["xhub_06"] = xhub_06;
        obj["xhub_07"] = xhub_07;
        obj["xhub_08"] = xhub_08;
        obj["xhub_09"] = xhub_09;
        obj["xhub_10"] = xhub_10;
        return obj;
    }

    QString nameForKey(const QString &key) const
    {
        if (key == "xhub_01")
            return xhub_01;
        if (key == "xhub_02")
            return xhub_02;
        if (key == "xhub_03")
            return xhub_03;
        if (key == "xhub_04")
            return xhub_04;
        if (key == "xhub_05")
            return xhub_05;
        if (key == "xhub_06")
            return xhub_06;
        if (key == "xhub_07")
            return xhub_07;
        if (key == "xhub_08")
            return xhub_08;
        if (key == "xhub_09")
            return xhub_09;
        if (key == "xhub_10")
            return xhub_10;
        return {};
    }

    static VideosType fromJson(const QJsonObject &obj)
    {
        VideosType vt;
        vt.xhub_01 = obj["xhub_01"].toString();
        vt.xhub_02 = obj["xhub_02"].toString();
        vt.xhub_03 = obj["xhub_03"].toString();
        vt.xhub_04 = obj["xhub_04"].toString();
        vt.xhub_05 = obj["xhub_05"].toString();
        vt.xhub_06 = obj["xhub_06"].toString();
        vt.xhub_07 = obj["xhub_07"].toString();
        vt.xhub_08 = obj["xhub_08"].toString();
        vt.xhub_09 = obj["xhub_09"].toString();
        vt.xhub_10 = obj["xhub_10"].toString();
        return vt;
    }
};

/// 配置文件根结构
struct videoConfig
{
    const QStringList SCENE_KEYS = {"xhub_01",
                                    "xhub_02",
                                    "xhub_03",
                                    "xhub_04",
                                    "xhub_05",
                                    "xhub_06",
                                    "xhub_07",
                                    "xhub_08",
                                    "xhub_09",
                                    "xhub_10"};

    double version = 1.0;
    VideosType videosType;
    QMap<QString, QList<std::shared_ptr<VideoItem>>> xhub_videos_grouped_;

    videoConfig()
    {
        for (const QString &key : SCENE_KEYS) {
            xhub_videos_grouped_[key] = QList<std::shared_ptr<VideoItem>>();
        }
    }

    videoConfig &operator=(const videoConfig &other)
    {
        version = other.version;
        videosType = other.videosType;
        xhub_videos_grouped_ = other.xhub_videos_grouped_; // shared_ptr共享所有权，引用计数+1
        return *this;
    }

    void clear()
    {
        for (const QString &key : SCENE_KEYS) {
            xhub_videos_grouped_[key].clear();
        }
    }

    void fillVideoConfig(const QList<VideoItem> &netWork_Return_info)
    {
        clear();
        for (const auto &item : netWork_Return_info) {
            if (xhub_videos_grouped_.contains(item.scene)) {
                auto data = std::make_shared<VideoItem>(item); // 拷贝一份到堆上
                xhub_videos_grouped_[item.scene].append(data);
            } else {
                qWarning() << "未知的 scene:" << item.scene;
            }
        }
    }

    void mergeFromNetwork(const QList<std::shared_ptr<VideoItem>> &netWorkItems)
    {
        for (const auto &netItem : netWorkItems) {
            if (!xhub_videos_grouped_.contains(netItem->scene)) {
                qWarning() << "未知的 scene:" << netItem->scene;
                continue;
            }

            auto &localList = xhub_videos_grouped_[netItem->scene];

            /// localList 存的是 shared_ptr，比较的是解引用后的对象
            auto it = std::find_if(localList.begin(),
                                   localList.end(),
                                   [&](const std::shared_ptr<VideoItem> &local) {
                                       return local->id == netItem->id
                                              && local->scene == netItem->scene;
                                   });

            if (it != localList.end()) {
                /// 直接修改 shared_ptr 指向的对象，UI 端自动同步
                (*it)->title = netItem->title;
                (*it)->sceneName = netItem->sceneName;
                (*it)->videoDesc = netItem->videoDesc;
                (*it)->imgUrl = netItem->imgUrl;
                (*it)->videoUrl = netItem->videoUrl;
                (*it)->deviceName = netItem->deviceName;
                (*it)->deviceType = netItem->deviceType;
                (*it)->status = netItem->status;
                (*it)->createdAt = netItem->createdAt;
                (*it)->updatedAt = netItem->updatedAt;
            } else {
                /// 新增的也包装成 shared_ptr
                localList.append(netItem);
            }
        }
    }

    QList<std::shared_ptr<VideoItem>> getAllVideos() const
    {
        QList<std::shared_ptr<VideoItem>> all;
        for (const QString &key : SCENE_KEYS) {
            all.append(xhub_videos_grouped_.value(key));
        }
        return all;
    }

    bool isEmpty() const
    {
        for (const QString &key : SCENE_KEYS) {
            if (!xhub_videos_grouped_.value(key).isEmpty())
                return false;
        }
        return true;
    }

    QJsonObject toJson() const
    {
        QJsonObject root;
        root["version"] = version;
        root["videosType"] = videosType.toJson();

        for (const QString &key : SCENE_KEYS) {
            QJsonArray arr;
            const auto &list = xhub_videos_grouped_[key];
            for (const auto &itemPtr : list) {
                arr.append(itemPtr->toJson()); // 解引用 shared_ptr
            }
            root[key] = arr;
        }

        return root;
    }

    static videoConfig fromJson(const QJsonObject &root)
    {
        videoConfig config;
        config.version = root["version"].toDouble(1.0);
        config.videosType = VideosType::fromJson(root["videosType"].toObject());

        for (const QString &key : config.SCENE_KEYS) {
            const QJsonArray arr = root[key].toArray();
            QList<std::shared_ptr<VideoItem>> list;
            for (const auto &val : arr) {
                // 从JSON解析后包装成 shared_ptr
                list.append(std::make_shared<VideoItem>(VideoItem::fromJson(val.toObject())));
            }
            config.xhub_videos_grouped_[key] = list;
        }

        return config;
    }
};

/// 试听列表请求结构
typedef struct AuditionsListRequest
{
    QString scene;       ///场景标识，如：mouse_try、keyboard_demo（必填）
    QString device_type; ///设备类型：mouse/keyboard/headset（可选）
    QString device_name; ///设备名称，精确匹配（非必填）
    int page = 1;        ///页码，默认 1（非必填）
    int page_size = 10;  ///每页数量，默认 10，最大 100（非必填）

} AuditionsListRequest;

/// 试听列表应答结构（也做本地配置）
typedef struct AuditionsListResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QList<VideoItem> list;
    } returnData;
    returnData data;
    int total;
    int page;
    int page_size;
} AuditionsListResponse;

bool ProcessAuditionsListResult(DeSheng::AuditionsListResponse &responseData,
                                QJsonDocument &jsonDocument);

QJsonObject AuditionsListRequestToJson(const AuditionsListRequest &req);

bool buildAuditionsListQuery(const AuditionsListRequest &req, QUrlQuery &query, QString &error);

/// GetAuditionDetailRequest 获取试听详情 请求结构体
typedef struct GetAuditionDetailRequest
{
    int id; ///< 试听 ID（路径参数，必填）
} GetAuditionDetailRequest;

/// GetAuditionDetailResponse 获取试听详情 应答结构体
typedef struct GetAuditionDetailResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        int id;              ///< 试听 ID
        QString title;       ///< 标题
        QString scene;       ///< 场景标识
        QString scene_name;  ///< 场景中文名
        QString video_desc;  ///< 视频描述
        QString img_url;     ///< 封面图 URL
        QString video_url;   ///< 视频 URL
        QString device_name; ///< 设备名称
        QString device_type; ///< 设备类型
        QString status;      ///< 状态
        QString created_at;  ///< 创建时间
        QString updated_at;  ///< 更新时间
    } ReturnData;
    ReturnData data;
} GetAuditionDetailResponse;

bool ProcessGetAuditionDetailResult(GetAuditionDetailResponse &responseData,
                                    QJsonDocument &jsonDocument);

QJsonObject GetAuditionDetailRequestToJson(const GetAuditionDetailRequest &req);

bool buildGetAuditionDetailQuery(const GetAuditionDetailRequest &req,
                                  QUrlQuery &query,
                                  QString &error);


/*************************************************************************************  管理端试听  ************************************************************************************************/

/// 路径常量（管理端）
namespace ApiPaths {
inline constexpr const char *kAdminAuditionList   = "/admin/auditions";
inline constexpr const char *kAdminAuditionDetail = "/admin/auditions/%1";
} // namespace ApiPaths

/// 管理端试听条目（仅 API 字段，不含本地状态）
typedef struct AdminAuditionItem
{
    int id;              ///< 试听ID
    QString title;       ///< 标题
    QString scene;       ///< 场景标识
    QString scene_name;  ///< 场景中文名
    QString video_desc;  ///< 视频描述
    QString img_url;     ///< 封面图URL
    QString video_url;   ///< 视频URL
    QString device_name; ///< 设备名称
    QString device_type; ///< 设备类型
    QString status;      ///< 状态：active/inactive
    QString created_at;  ///< 创建时间
    QString updated_at;  ///< 更新时间
} AdminAuditionItem;

// ---- 管理端试听列表 ----

typedef struct AdminAuditionListRequest
{
    QString scene;       ///< 场景标识（可选）
    QString status;      ///< 状态筛选：active/inactive（可选）
    QString device_type; ///< 设备类型：mouse/keyboard/headset（可选）
    QString device_name; ///< 设备名称，精确匹配（可选）
    QString keyword;     ///< 关键词搜索标题（可选）
    int page = 1;        ///< 页码，默认 1
    int page_size = 10;  ///< 每页数量，默认 10，最大 100
} AdminAuditionListRequest;

typedef struct AdminAuditionListResponse
{
    QString code;
    QString message;
    typedef struct ReturnData
    {
        QList<AdminAuditionItem> list;
        int total;
        int page;
        int page_size;
    } ReturnData;
    ReturnData data;
} AdminAuditionListResponse;

bool ProcessAdminAuditionListResult(AdminAuditionListResponse &responseData,
                                    const QJsonDocument &jsonDocument);
QJsonObject AdminAuditionListRequestToJson(const AdminAuditionListRequest &req);
bool buildAdminAuditionListQuery(const AdminAuditionListRequest &req,
                                 QUrlQuery &query,
                                 QString &error);

// ---- 管理端试听详情 ----

typedef struct AdminAuditionDetailRequest
{
    int id; ///< 试听ID（路径参数，必填）
} AdminAuditionDetailRequest;

typedef struct AdminAuditionDetailResponse
{
    QString code;
    QString message;
    AdminAuditionItem data;
} AdminAuditionDetailResponse;

bool ProcessAdminAuditionDetailResult(AdminAuditionDetailResponse &responseData,
                                      const QJsonDocument &jsonDocument);
QJsonObject AdminAuditionDetailRequestToJson(const AdminAuditionDetailRequest &req);
bool buildAdminAuditionDetailQuery(const AdminAuditionDetailRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

// ---- 管理端创建试听 ----

typedef struct AdminAuditionCreateRequest
{
    QString title;       ///< 试听标题，最多200字符（必填）
    QString scene;       ///< 场景标识（必填）
    QString scene_name;  ///< 场景中文名，最多100字符（必填）
    QString video_desc;  ///< 视频描述，最多500字符（可选）
    QString img_url;     ///< 封面图URL，最多500字符（必填）
    QString video_url;   ///< 视频URL，最多500字符（必填）
    QString device_name; ///< 关联设备名称，最多100字符（可选）
    QString device_type; ///< 设备类型：mouse/keyboard/headset，最多20字符（可选）
    QString status;      ///< 状态：active/inactive，默认 active（可选）
} AdminAuditionCreateRequest;

typedef struct AdminAuditionCreateResponse
{
    QString code;
    QString message;
    AdminAuditionItem data;
} AdminAuditionCreateResponse;

bool ProcessAdminAuditionCreateResult(AdminAuditionCreateResponse &responseData,
                                      const QJsonDocument &jsonDocument);
QJsonObject AdminAuditionCreateRequestToJson(const AdminAuditionCreateRequest &req);
bool buildAdminAuditionCreateQuery(const AdminAuditionCreateRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

// ---- 管理端更新试听 ----

typedef struct AdminAuditionUpdateRequest
{
    int id;              ///< 试听ID（路径参数，必填）
    QString title;       ///< 试听标题，最多200字符（可选）
    QString scene;       ///< 场景标识（可选）
    QString scene_name;  ///< 场景中文名，最多100字符（可选）
    QString video_desc;  ///< 视频描述，最多500字符，传空字符串清空（可选）
    QString img_url;     ///< 封面图URL，最多500字符（可选）
    QString video_url;   ///< 视频URL，最多500字符（可选）
    QString device_name; ///< 关联设备名称，最多100字符（可选）
    QString device_type; ///< 设备类型：mouse/keyboard/headset，最多20字符（可选）
    QString status;      ///< 状态：active/inactive（可选）
} AdminAuditionUpdateRequest;

typedef struct AdminAuditionUpdateResponse
{
    QString code;
    QString message;
    AdminAuditionItem data;
} AdminAuditionUpdateResponse;

bool ProcessAdminAuditionUpdateResult(AdminAuditionUpdateResponse &responseData,
                                      const QJsonDocument &jsonDocument);
QJsonObject AdminAuditionUpdateRequestToJson(const AdminAuditionUpdateRequest &req);
bool buildAdminAuditionUpdateQuery(const AdminAuditionUpdateRequest &req,
                                   QUrlQuery &query,
                                   QString &error);

// ---- 管理端删除试听 ----

typedef struct AdminAuditionDeleteRequest
{
    int id; ///< 试听ID（路径参数，必填）
} AdminAuditionDeleteRequest;

typedef struct AdminAuditionDeleteResponse
{
    QString code;
    QString message;
} AdminAuditionDeleteResponse;

bool ProcessAdminAuditionDeleteResult(AdminAuditionDeleteResponse &responseData,
                                      const QJsonDocument &jsonDocument);
QJsonObject AdminAuditionDeleteRequestToJson(const AdminAuditionDeleteRequest &req);
bool buildAdminAuditionDeleteQuery(const AdminAuditionDeleteRequest &req,
                                   QUrlQuery &query,
                                   QString &error);


} // namespace DeSheng

// 调用示例（新栈）— 获取试听视频列表
// auto &cli = HttpClient::instance();  /// network/http_client.h
// AuditionsListRequest t_req;
// t_req.scene = "xhub_01";
// QUrlQuery t_query;
// QString t_err;
// buildAuditionsListQuery(t_req, t_query, t_err);
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kAuditionList, RequestOptions{}.withQuery(t_query).withTag("audition"));
// connect(r, &QNetworkReply::finished, [r]() {
//     AuditionsListResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r->readAll());
//     if (ProcessAuditionsListResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data.list ...
//     }
//     r->deleteLater();
// });
//
// 调用示例（新栈）— 获取试听视频详情
// GetAuditionDetailRequest t_req2;
// t_req2.id = 1;
// QNetworkReply *r2 = cli.get(QString(DeSheng::ApiPaths::kAuditionDetail).arg(t_req2.id), RequestOptions{}.withTag("audition"));
// connect(r2, &QNetworkReply::finished, [r2]() {
//     GetAuditionDetailResponse t_resp;
//     QJsonDocument t_doc = QJsonDocument::fromJson(r2->readAll());
//     if (ProcessGetAuditionDetailResult(t_resp, t_doc) && t_resp.code == "success") {
//         // t_resp.data ...
//     }
//     r2->deleteLater();
// });
//
// 推荐写法（HttpClient + RequestOptions）
// auto &cli = HttpClient::instance();
// QUrlQuery t_q; t_q.addQueryItem("scene", "xhub_01");
// QNetworkReply *r = cli.get(DeSheng::ApiPaths::kAuditionList, RequestOptions{}.withQuery(t_q).withTag("audition"));
// connect(r, &QNetworkReply::finished, [r]() { ... ; r->deleteLater(); });

Q_DECLARE_METATYPE(DeSheng::VideoItem)         ///< 试听视频数据

#endif // AUDITION_API_H
