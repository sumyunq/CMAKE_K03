#ifndef SOUND_TEST_MAIN_PAGE_H
#define SOUND_TEST_MAIN_PAGE_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QWidget>
#include <QStandardPaths>

#include "FeedBackC/SoundTest/SoundTestCustomUI/game_type_selected_scrollarea.h"
#include "FeedBackC/SoundTest/SoundTestCustomUI/video_scroll_area.h"

#include "FeedBackC/ffmpage/ffmpeg_main_page.h"

namespace Ui {
class SoundTestMainPage;
}

/// 试听视频主页,先加载本地的配置信息，再通过http请求获取网络上的视频列表信息，来更新本地配置文件
class SoundTestMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit SoundTestMainPage(QWidget *parent = nullptr);
    ~SoundTestMainPage();

    void updateUIFirst(); ///本地配置信息更新完成后，第一次更新UI信息
    void retranslateTexts();

signals:
    void closeSoundTest(); ///关闭按键信号，跳转到方案库
    void minWidget(bool isMinWidgetShow = true); ///小窗信号，跳转到方案库

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    bool loadLocalConfigurationFile(const QString &filePath); /// 加载本地配置文件
    void sendAuditionsListRequest(
        const DeSheng::AuditionsListRequest &req); ///发送试听视频列表请求(按场景值)
    void checkLocalConfigurationFile(); ///根据网络服务器回显信息 校对本地配置文件 以服务器为准
    void validateLocalFiles();          ///校验本地文件正确性


protected slots:
    void netWorkFinished(QNetworkReply *reply);
    void dealwithVideoDownLoadRequest(); ///处理视频下载请求

    void dealwithColseRequest();///处理关闭请求

public:
    /************************************************************************ ffmpeg 播放器 ************************************************************************/
    std::unique_ptr<FFmpegMainPage> cl_ffmpeg_main_page_ = nullptr; /// 音视频播放器界面（ffmpeg）


private:
    Ui::SoundTestMainPage *ui;

    /************************************************************************ 自定义UI相关 ************************************************************************/
    GameTypeSelectedScrollArea *cl_game_type_scrollArea_hBox
        = nullptr; /// 游戏类型 按键滑动区域(水平) ///ui->widget_game_type_list

    QMap<QString, VideoScrollArea *>
        cl_video_scrollArea_grid_map_; ///对应游戏类型的网格区域和垂直滚动区域    ///ui->page_videos_1
    QMap<QString, VideoScrollArea *>
        cl_video_scrollArea_vBox_map_; ///对应游戏类型的网格区域和垂直滚动区域    ///ui->widget_other_videos


    /************************************************************************ 本地配置数据 ************************************************************************/
    /// UI 基于配置数据进行渲染
    std::unique_ptr<VideoData> cl_video_data_info_ = nullptr;             ///试听视频本地配置信息
    std::atomic<int> echoes_number_
        = 0; ///记录回显次数（开始会发送10次请求，请求全部完成后，更新更新UI数据）
    std::atomic<bool> local_config_loaded_ = false; ///本地配置文件是否加载
    std::atomic<int> current_page_index_ = 0;       ///当前界面所属按键 id
    const QList<QString> scene_keys_ = {"xhub_01",
                                        "xhub_02",
                                        "xhub_03",
                                        "xhub_04",
                                        "xhub_05",
                                        "xhub_06",
                                        "xhub_07",
                                        "xhub_08",
                                        "xhub_09",
                                        "xhub_10"};
    QMap<QString, QString> game_type_map_; ///根据 场景键值 保存 对应的 场景名称
};

#endif // SOUND_TEST_MAIN_PAGE_H
