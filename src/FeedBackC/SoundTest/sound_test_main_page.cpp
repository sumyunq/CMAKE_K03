#include "FeedBackC/SoundTest/sound_test_main_page.h"
#include "data/api_global.h"
#include "ui_sound_test_main_page.h"
#include "network/http_client.h"
#include "network/request_options.h"
#include "network/server_router.h"

SoundTestMainPage::SoundTestMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SoundTestMainPage)
{
    ui->setupUi(this);

    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽

    /// 请求网络数据,校对本地视频配置信息,更新滑动区域显示
    echoes_number_.store(0);
    /// 发送请求，遍历所有 scene key 获取网络数据
    for (const QString &scene : scene_keys_) {
        DeSheng::AuditionsListRequest req;
        req.scene = scene;
        req.device_type = "headset";
        req.device_name = "";
        sendAuditionsListRequest(req);
    }
}

SoundTestMainPage::~SoundTestMainPage()
{
    delete ui;
}

void SoundTestMainPage::updateUIFirst()
{
    ///调用该函数前，需确保本地视频配置文件已正常更新()
    if (!cl_video_data_info_) {
        return;
    }

    if (!cl_game_type_scrollArea_hBox) {
        return;
    }

    /// 试听视频类型按键组更新
    QString gameName;
    for (int i = 0; i < scene_keys_.size(); ++i) {
        int size = cl_video_data_info_->cl_video_config_local_
                       .xhub_videos_grouped_[scene_keys_.at(i)]
                       .size();

        /// 对应 场景标识 存在试听视频
        if (size != 0) {
            gameName = cl_video_data_info_->cl_video_config_local_
                           .xhub_videos_grouped_[scene_keys_.at(i)]
                           .at(0)
                           ->sceneName;

            game_type_map_[scene_keys_.at(i)] = gameName; ///更新键值对应名称
            /// 更新 对应键值 所属按键 状态信息
            cl_game_type_scrollArea_hBox->cl_all_game_type_.at(i)->setCl_game_name(gameName);
            cl_game_type_scrollArea_hBox->cl_all_game_type_.at(i)->setCl_is_show(true);

            /// 更新网格布局对应的内部小控件信息
            for (int j = 0; j < cl_video_data_info_->cl_video_config_local_
                                    .xhub_videos_grouped_[scene_keys_.at(i)]
                                    .size();
                 ++j) {
                /// id 唯一,两个布局都复制一份数据,对布局中的操作，需要更新到 cl_video_data_info_ 中
                /// 后续更新时，以 cl_video_data_info_ 为准

                /// 网格布局
                SingleVideoInfo *singleVideoInfo_grid = new SingleVideoInfo(ScrollAreaDisplayMode::GridDisplay,
                                                                            this);
                singleVideoInfo_grid->cl_video_item_ = cl_video_data_info_->cl_video_config_local_
                                                           .xhub_videos_grouped_[scene_keys_.at(i)]
                                                           .at(j);
                singleVideoInfo_grid->setInfoText(singleVideoInfo_grid->cl_video_item_->title);
                singleVideoInfo_grid->setCoverLocalPath(
                    singleVideoInfo_grid->cl_video_item_->coverLocalPath);

                cl_video_scrollArea_grid_map_[scene_keys_.at(i)]->cl_all_videos_.append(
                    singleVideoInfo_grid);

                /// 垂直布局
                SingleVideoInfo *singleVideoInfo_vBox
                    = new SingleVideoInfo(ScrollAreaDisplayMode::SingleColumnDisplay, this);
                singleVideoInfo_vBox->cl_video_item_ = cl_video_data_info_->cl_video_config_local_
                                                           .xhub_videos_grouped_[scene_keys_.at(i)]
                                                           .at(j); ///与网格布局共享一份数据
                singleVideoInfo_vBox->setInfoText(singleVideoInfo_vBox->cl_video_item_->title);
                singleVideoInfo_vBox->setCoverLocalPath(
                    singleVideoInfo_grid->cl_video_item_->coverLocalPath);

                cl_video_scrollArea_vBox_map_[scene_keys_.at(i)]->cl_all_videos_.append(
                    singleVideoInfo_vBox);

                /// 连接一下信号
                /// 网格布局
                QObject::connect(singleVideoInfo_grid,
                                 &SingleVideoInfo::requestDownLoadVideoFile,
                                 this,
                                 &SoundTestMainPage::dealwithVideoDownLoadRequest,
                                 Qt::UniqueConnection);
                QObject::connect(
                    singleVideoInfo_grid,
                    &SingleVideoInfo::openFile,
                    this,
                    [=](QString fileName) {
                        /// 跳转至播放页面 进行播放
                        cl_ffmpeg_main_page_->onMinWidgetSlots(false); ///非小窗口模式进行播放

                        ui->stackedWidget_soundTest_main->setCurrentWidget(ui->page_video_playing);
                        ui->label_description->setText(
                            singleVideoInfo_grid->cl_video_item_->videoDesc);
                        cl_ffmpeg_main_page_->open(fileName.toStdString().c_str());
                    },
                    Qt::UniqueConnection);
                QObject::connect(
                    singleVideoInfo_grid,
                    &SingleVideoInfo::openFileWithMinWidget,
                    this,
                    [=](QString fileName) {
                        /// 启用 画中画模式进行播放

                        ui->label_description->setText(
                            singleVideoInfo_grid->cl_video_item_->videoDesc);
                        cl_ffmpeg_main_page_->open(fileName.toStdString().c_str());

                        cl_ffmpeg_main_page_->onMinWidgetSlots(true); ///小窗口模式进行播放

                        emit minWidget();
                    },
                    Qt::UniqueConnection);

                /// 垂直布局
                QObject::connect(singleVideoInfo_vBox,
                                 &SingleVideoInfo::requestDownLoadVideoFile,
                                 this,
                                 &SoundTestMainPage::dealwithVideoDownLoadRequest,
                                 Qt::UniqueConnection);
                QObject::connect(
                    singleVideoInfo_vBox,
                    &SingleVideoInfo::openFile,
                    this,
                    [=](QString fileName) {
                        ui->label_description->setText(
                            singleVideoInfo_vBox->cl_video_item_->videoDesc);
                        cl_ffmpeg_main_page_->open(fileName.toStdString().c_str());
                    },
                    Qt::UniqueConnection);
                QObject::connect(
                    singleVideoInfo_vBox,
                    &SingleVideoInfo::openFileWithMinWidget,
                    this,
                    [=](QString fileName) {
                        /// 启用 画中画模式进行播放

                        /// 跳转至网格布局页面
                        ui->label_description->setText(
                            singleVideoInfo_vBox->cl_video_item_->videoDesc);

                        ui->stackedWidget_videos->setCurrentWidget(ui->page_videos_viewing);

                        cl_ffmpeg_main_page_->open(fileName.toStdString().c_str());
                        cl_ffmpeg_main_page_->onMinWidgetSlots(true); ///小窗口模式进行播放
                        emit minWidget();
                    },
                    Qt::UniqueConnection);
            }

        } else {
            /// 对应场景 不存在试听视频, 隐藏场景按键
            cl_game_type_scrollArea_hBox->cl_all_game_type_.at(i)->setCl_game_name(
                scene_keys_.at(i));
            cl_game_type_scrollArea_hBox->cl_all_game_type_.at(i)->setCl_is_show(false);
        }

        ///更新 试听视频网格滚动区域
        cl_video_scrollArea_grid_map_[scene_keys_.at(i)]->updateView();

        ///更新 试听视频垂直滚动区域
        cl_video_scrollArea_vBox_map_[scene_keys_.at(i)]->updateView();
    }

    cl_game_type_scrollArea_hBox->updateView(); ///更新 试听视频类型按键组更新
    cl_game_type_scrollArea_hBox->cl_all_games_type_buttons_->button(0)->setChecked(
        true); ///默认选中第一个
}

void SoundTestMainPage::InitUIInformation()
{
    /// 试听类型 选择区域
    cl_game_type_scrollArea_hBox = new GameTypeSelectedScrollArea(
        ui->widget_game_type_list); /// 游戏类型 按键滑动区域(水平)
    ui->widget_game_type_list->layout()->addWidget(cl_game_type_scrollArea_hBox);

    /// ffmpeg 播放区域
    cl_ffmpeg_main_page_ = std::make_unique<FFmpegMainPage>(
        ui->widget_for_ffmpegPlaying); ///音视频播放器界面（ffmpeg）
    ui->widget_for_ffmpegPlaying->layout()->addWidget(cl_ffmpeg_main_page_.get());

    /// 网格滑动区域 / 垂直滑动区域 的显示取决与具体的配置信息
    /// 这边默认加载十个与场景键值一一对应的
    for (int i = 0; i < scene_keys_.size(); ++i) {
        VideoScrollArea *grid_scroll = new VideoScrollArea(ScrollAreaDisplayMode::GridDisplay, this);

        VideoScrollArea *column_scroll = new VideoScrollArea(ScrollAreaDisplayMode::SingleColumnDisplay, this);

        cl_video_scrollArea_grid_map_[scene_keys_.at(i)] = grid_scroll;
        cl_video_scrollArea_vBox_map_[scene_keys_.at(i)] = column_scroll;
    }

    /// 动态更新(默认第一个)
    ui->page_videos_1->layout()->addWidget(cl_video_scrollArea_grid_map_[scene_keys_.at(0)]);
    ui->widget_other_videos->layout()->addWidget(cl_video_scrollArea_vBox_map_[scene_keys_.at(0)]);

    /// 设置 UI 样式
    this->setObjectName("cl_sound_test_main_page_");
    /// 控制指定对象 背景样式
    setStyleSheet(R"(
    QWidget#cl_sound_test_main_page_ {
        background: transparent;
        border: 2px solid gray;
        border-radius: 10px;
    }
)");

    ui->page_videos_1->setObjectName("page_videos_1");
    ui->page_videos_1->setAutoFillBackground(true);
    // qDebug() << "objectName:" << ui->page_videos_1->objectName();
    ui->page_videos_1->setStyleSheet(R"(
    QWidget#page_videos_1 {

    }
)");

    /// 试听文字样式
    ui->label->setText(tr("试听"));
    ui->label->setStyleSheet(R"(
        QLabel {
            font-family: "Noto Sans S Chinese";
            font-weight: 500;
            font-size: 16px;
            color: #FFFFFF;
        }
)");
    /// 视频网格布局页面 关闭按键
    ui->pBt_close->setStyleSheet(R"(
QPushButton {
    border-image: url(:/Skin/Images/soundTest/close_btn.png);
    border: none;
}
QPushButton:hover {
    border-image: url(:/Skin/Images/soundTest/close_btn_hover.png);
    border: none;
}
)");
    ui->pBt_close->setCursor(QCursor(Qt::PointingHandCursor)); //鼠标变成手型

    /// 视频播放页面 关闭按键
    ui->pBt_close_2->setStyleSheet(R"(
QPushButton {
    border-image: url(:/Skin/Images/soundTest/close_btn.png);
    border: none;
}
QPushButton:hover {
    border-image: url(:/Skin/Images/soundTest/close_btn_hover.png);
    border: none;
}
)");
    ui->pBt_close_2->setCursor(QCursor(Qt::PointingHandCursor)); //鼠标变成手型

    /// 视频播放界面描述
    ui->label_description->setStyleSheet(R"(
        QLabel {
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 18px;
            color: #A1A8B3;
        }
)");

    /// 返回按键
    ui->pushButton_return_videosViewingPage->setStyleSheet(R"(
QPushButton {
    border-image: url(:/Skin/Images/soundTest/video_playing_page_return_btn.png);
    border: none;
}
QPushButton:hover {
    border-image: url(:/Skin/Images/soundTest/video_playing_page_return_btn_hover.png);
    border: none;
}

)");
    ui->pushButton_return_videosViewingPage->setCursor(
        QCursor(Qt::PointingHandCursor)); //鼠标变成手型

    /// 文字：其他试听视频
    ui->label_other_videos_text->setText(tr("其他试听视频"));
    ui->label_other_videos_text->setStyleSheet(R"(
QLabel{
    font-family: "Noto Sans S Chinese";
    font-weight: 500;
    font-size: 16px;
    color: #A1A8B3;
}

)");

    ui->stackedWidget_soundTest_main->setCurrentWidget(ui->page_videos_viewing); ///默认显示网格布局
}

void SoundTestMainPage::retranslateTexts()
{
    ui->retranslateUi(this);
}

void SoundTestMainPage::InitMember()
{
    cl_video_data_info_ = std::make_unique<VideoData>();

    // QString configPath = QApplication::applicationDirPath() + "/videoData/videoData.json";    ///旧版
    QString configPath = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/videoData/videoData.json";
    local_config_loaded_ = loadLocalConfigurationFile(configPath);
    if (local_config_loaded_) {
        qDebug() << "本地视频配置文件加载成功";
        validateLocalFiles(); ///验证本地配置是否有效
    } else {
        qDebug() << "本地视频配置文件加载失败（首次运行或无配置）";
    }
}

void SoundTestMainPage::InitConnect()
{
    /// 点击返回，小窗口恢复，视频暂停
    QObject::connect(ui->pushButton_return_videosViewingPage, &QPushButton::clicked, this, [=]() {
        /// 检查是否为小窗口模式
        if (cl_ffmpeg_main_page_.get()->is_minView_.load()) {
            {
                cl_ffmpeg_main_page_.get()
                    ->minView.get()
                    ->requestRestore(); ///    小窗口主动请求恢复至主窗口
                cl_ffmpeg_main_page_.get()->is_minView_.store(false);
            }
        } else {
        }

        /// 如果视频正在播放中，则暂停
        if (!cl_ffmpeg_main_page_->cl_ffmpeg_global_->cl_is_pause()) {
            cl_ffmpeg_main_page_->pause(); /// 视频暂停
        }

        ui->stackedWidget_soundTest_main->setCurrentWidget(ui->page_videos_viewing);
    });

    /// 连接 cl_ffmpeg_main_page_ 发出的全屏信号
    QObject::connect(cl_ffmpeg_main_page_.get(),
                     &FFmpegMainPage::fullScreen,
                     this,
                     [=](bool isFullScreen) {
                         if (isFullScreen) {
                         } else {
                             cl_ffmpeg_main_page_.get()->setParent(ui->widget_for_ffmpegPlaying);
                             ui->widget_for_ffmpegPlaying->layout()->addWidget(
                                 cl_ffmpeg_main_page_.get());
                         }
                     });

    /// 连接 cl_ffmpeg_main_page_ 发出的 画中画 信号
    QObject::connect(cl_ffmpeg_main_page_.get(),
                     &FFmpegMainPage::EnableSmallWindowMode,
                     this,
                     [=](bool isEnable) {
                         ///如果启用画中画模式，跳转到试听主界面（模拟）
                         if (isEnable) {
                             if (ui->stackedWidget_soundTest_main->currentWidget()
                                 != ui->page_videos_viewing)
                                 this->ui->stackedWidget_soundTest_main->setCurrentWidget(
                                     ui->page_videos_viewing);

                             emit minWidget();
                         } else {
                             ///回到试听视频播放页面
                             if (ui->stackedWidget_soundTest_main->currentWidget()
                                 != ui->page_video_playing)
                                 this->ui->stackedWidget_soundTest_main->setCurrentWidget(
                                     ui->page_video_playing);
                             emit minWidget(false);
                         }
                     });

    /// 试听游戏类型更换
    QObject::connect(cl_game_type_scrollArea_hBox,
                     &GameTypeSelectedScrollArea::changeGameTypeVideos,
                     this,
                     [=](int index) {
                         /// 更换 滑动区域

                         ///视频内容主要在网格 scrollArea_grid_ 中，所以只需要切换到指定的 cl_video_scrollArea_grid_
                         if (index == this->current_page_index_) {
                         } else {
                             /// 清除布局（但不会删除 widget，widget 会变成悬浮孤儿）
                             QLayout *grid_layout = ui->page_videos_1->layout();
                             QLayout *vBox_layout = ui->widget_other_videos->layout();

                             /// 先隐藏所有旧的 widget，再添加新的
                             /// 处理网格布局
                             if (grid_layout) {
                                 /// 移除所有 item，但不删除 widget
                                 while (QLayoutItem *item = grid_layout->takeAt(0)) {
                                     if (item->widget()) {
                                         item->widget()->hide(); /// 隐藏而非删除
                                     }
                                     delete item; /// 删除 layout item，不删 widget
                                 }

                                 grid_layout->addWidget(
                                     cl_video_scrollArea_grid_map_[scene_keys_.at(index)]);
                                 cl_video_scrollArea_grid_map_[scene_keys_.at(index)]->show();
                             }
                             /// 处理垂直布局
                             if (vBox_layout) {
                                 /// 移除所有 item，但不删除 widget
                                 while (QLayoutItem *item = vBox_layout->takeAt(0)) {
                                     if (item->widget()) {
                                         item->widget()->hide(); /// 隐藏而非删除
                                     }
                                     delete item; /// 删除 layout item，不删 widget
                                 }

                                 vBox_layout->addWidget(
                                     cl_video_scrollArea_vBox_map_[scene_keys_.at(index)]);
                                 cl_video_scrollArea_vBox_map_[scene_keys_.at(index)]->show();
                             }

                             this->current_page_index_ = index;
                         }
                     });

    /// 进度条按下（暂停播放并 seek 到按下位置）
    QObject::connect(cl_ffmpeg_main_page_.get()->slider_playing_progress(),
                     &QSliderPlayingProgress::sliderPressed,
                     this,
                     [=]() {
                         if (!cl_ffmpeg_main_page_->cl_ffmpeg_global_->cl_is_pause()) {
                             cl_ffmpeg_main_page_->pause();
                         }
                         double sec = cl_ffmpeg_main_page_.get()->slider_playing_progress()->value()
                                      / 1000.0;
                         cl_ffmpeg_main_page_->seek(sec);
                     });

    /// 进度条拖动中：仅更新缩略图预览（由 FFmpegMainPage::onSliderDrag 处理），不做 seek
    /// seek 太重（stopAllThread + flush + avformat_seek + startAllThread），
    /// 每像素触发一次会导致 UI 卡死且无法实时显示画面

    /// 进度条释放（跳转到指定进度）
    QObject::connect(cl_ffmpeg_main_page_.get()->slider_playing_progress(),
                     &QSliderPlayingProgress::sliderReleased,
                     this,
                     [=]() {
                         double sec = cl_ffmpeg_main_page_.get()->slider_playing_progress()->value()
                                      / 1000.0;
                         cl_ffmpeg_main_page_->seek(sec);

                         ///如果处于暂停状态，恢复播放
                         if (cl_ffmpeg_main_page_->cl_ffmpeg_global_->cl_is_pause()) {
                             cl_ffmpeg_main_page_->pause(); /// 视频暂停
                         }
                     });

    /// 向服务器请求网络数据（API 请求统一走 ApiClient，见 sendAuditionsListRequest）

    /// pBt_close_2 和 pBt_close
    QObject::connect(ui->pBt_close,
                     &QPushButton::clicked,
                     this,
                     &SoundTestMainPage::dealwithColseRequest);
    QObject::connect(ui->pBt_close_2,
                     &QPushButton::clicked,
                     this,
                     &SoundTestMainPage::dealwithColseRequest);
}

bool SoundTestMainPage::loadLocalConfigurationFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "配置文件不存在:" << filePath;
        QDir dir = QFileInfo(filePath).absoluteDir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        if (file.open(QIODevice::WriteOnly)) {
            DeSheng::videoConfig defaultJson;
            QJsonDocument doc(defaultJson.toJson());

            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
            qDebug() << "已创建配置文件:" << filePath;
            return loadLocalConfigurationFile(filePath);
        } else {
            qWarning() << "创建配置文件失败:" << filePath;
            return false;
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << file.errorString();
        return false;
    }

    QByteArray rawData = file.readAll();
    file.close();

    if (rawData.isEmpty()) {
        qWarning() << "配置文件为空";
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        QFile::copy(filePath, filePath + ".bak");
        return false;
    }

    cl_video_data_info_->cl_video_config_local_ = DeSheng::videoConfig::fromJson(
        doc.object()); ///保存本地视频配置信息

    return true;
}

void SoundTestMainPage::sendAuditionsListRequest(const DeSheng::AuditionsListRequest &req)
{
    QUrlQuery query;
    QString errorMsg;
    if (!DeSheng::buildAuditionsListQuery(req, query, errorMsg)) {
        qDebug() << "errorMsg :" << errorMsg;
        return;
    }

    // 统一走新栈 ApiClient（URL 路由由 ServerRouter 按 tag "audition" 解析）
    QNetworkReply *t_reply = HttpClient::instance().get("/auditions",
        RequestOptions{}.withQuery(query).withTag("audition"));
    connect(t_reply, &QNetworkReply::finished, this,
            [this, t_reply]() { netWorkFinished(t_reply); });

    qDebug() << "URL:" << ServerRouter::instance().resolveUrl("/auditions", "", "audition") + "?" + query.toString();
}

void SoundTestMainPage::checkLocalConfigurationFile()
{
    if (cl_video_data_info_->cl_video_config_netWork_.isEmpty()) {
        qDebug() << "网络配置为空，跳过本地校对";
        return;
    }

    /// 本地向网络服务器合并(更新到服务器配置，然后写入服务器配置到本地，再同步本地配置)
    for (const QString &key : scene_keys_) {
        for (int i = 0;
             i < cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key].size();
             ++i) {
            for (int j = 0;
                 j < cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key].size();
                 ++j) {
                if (cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key].at(i)->id
                        == cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                               .at(j)
                               ->id
                    && cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                               .at(j)
                               ->localStatus
                           == DeSheng::VideoStatus::Downloaded) {
                    /// id相等 且 本地状态为已下载
                    /// 检查视频url 以及封面url 是否修改，如果未修改，更新服务器配置状态，否则（其中任意一项有修改），执行本地清除处理，并重置为未下载状态（不做修改）
                    if (cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key]
                                .at(i)
                                ->videoUrl
                            == cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                   .at(j)
                                   ->oldVideoUrl
                        && cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key]
                                   .at(i)
                                   ->imgUrl
                               == cl_video_data_info_->cl_video_config_local_
                                      .xhub_videos_grouped_[key]
                                      .at(j)
                                      ->imgUrl) {
                        /// 更新状态
                        cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key]
                            .at(i)
                            ->localStatus
                            = cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                  .at(j)
                                  ->localStatus;
                        ///更新本地视频路径
                        cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key]
                            .at(i)
                            ->localPath
                            = cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                  .at(j)
                                  ->localPath;
                        ///更新封面路径
                        cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key]
                            .at(i)
                            ->coverLocalPath
                            = cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                  .at(j)
                                  ->coverLocalPath;
                        ///更新旧视频url路径
                        cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_[key]
                            .at(i)
                            ->oldVideoUrl
                            = cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                  .at(j)
                                  ->oldVideoUrl;

                    } else {
                        /// 以服务器为准，不做修改

                        /// 执行本地清理操作
                        if (!cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                 .at(j)
                                 ->localPath.isEmpty()) {
                            /// 删除本地视频文件
                            QFile::remove(cl_video_data_info_->cl_video_config_local_
                                              .xhub_videos_grouped_[key]
                                              .at(j)
                                              ->localPath);
                            qDebug() << "移除本地视频文件:"
                                     << cl_video_data_info_->cl_video_config_local_
                                            .xhub_videos_grouped_[key]
                                            .at(j)
                                            ->localPath
                                     << Qt::endl;
                        }

                        if (!cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key]
                                 .at(j)
                                 ->coverLocalPath.isEmpty()) {
                            /// 删除本地封面文件
                            QFile::remove(cl_video_data_info_->cl_video_config_local_
                                              .xhub_videos_grouped_[key]
                                              .at(j)
                                              ->coverLocalPath);
                            qDebug() << "移除本地封面文件:"
                                     << cl_video_data_info_->cl_video_config_local_
                                            .xhub_videos_grouped_[key]
                                            .at(j)
                                            ->coverLocalPath
                                     << Qt::endl;
                        }
                    }
                } else {
                    /// 其他情况，以服务器为准
                }
            }
        }
    }
    /// 至此，cl_video_config_netWork_ 已 同步本地配置数据，执行本地清理操作

    /// 清理本地已不存在的视频条目（校对id,网络服务器不存在的id，本地直接删除并清理）

    /// 记录网络服务器存在的id
    QSet<QString> netIdSceneSet;
    for (const auto &item : cl_video_data_info_->cl_video_config_netWork_.getAllVideos()) {
        netIdSceneSet.insert(QString("%1").arg(item->id));
    }

    /// 遍历本地的每一个item
    for (const QString &key : cl_video_data_info_->cl_video_config_local_.SCENE_KEYS) {
        auto &list = cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key];

        /// 对需要移除的元素执行操作
        for (auto &local : list) {
            if (!netIdSceneSet.contains(QString::number(local->id))) {
                /// 服务器已不存在该条目
                if (!local->localPath.isEmpty()) {
                    /// 删除本地视频文件
                    QFile::remove(local->localPath);
                    qDebug() << "移除本地视频文件:" << local->localPath << Qt::endl;
                }

                if (!local->coverLocalPath.isEmpty()) {
                    /// 删除本地封面文件
                    QFile::remove(local->coverLocalPath);
                    qDebug() << "移除本地封面文件:" << local->coverLocalPath << Qt::endl;
                }

                /// 记录日志
                /// 发射信号通知UI
            }
        }
    }

    /// 配置更新前，需要确保 cl_video_data_info_->cl_video_config_netWork_ 信息正确
    QString configPath =  QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/videoData/videoData.json";
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(cl_video_data_info_->cl_video_config_netWork_.toJson());
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "本地配置文件已更新:" << configPath;
    } else {
        qWarning() << "无法写入配置文件:" << configPath;
    }

    /// 更新本地配置（二次加载，确保本地配置的正确性）
    local_config_loaded_ = loadLocalConfigurationFile(configPath);
    if (local_config_loaded_) {
        qDebug() << "本地视频配置文件加载成功";
        validateLocalFiles(); ///验证本地配置是否有效
    } else {
        qDebug() << "本地视频配置文件加载失败（首次运行或无配置）";
    }

    updateUIFirst(); ///校验并更新本地视频配置文件后，根据本地配置信息去更新UI
}

void SoundTestMainPage::validateLocalFiles()
{
    bool changed = false;
    for (const QString &key : cl_video_data_info_->cl_video_config_local_.SCENE_KEYS) {
        auto &list = cl_video_data_info_->cl_video_config_local_.xhub_videos_grouped_[key];
        for (auto &item : list) {
            if (item->localStatus != DeSheng::VideoStatus::Downloaded)
                continue;

            bool fileMissing = false;

            /// 校验视频文件
            if (!item->localPath.isEmpty()) {
                if (!QFile::exists(item->localPath)) {
                    qDebug() << "视频文件不存在:" << item->localPath;
                    fileMissing = true;
                }
            } else {
                fileMissing = true;
            }

            /// 校验封面文件
            if (!item->coverLocalPath.isEmpty()) {
                if (!QFile::exists(item->coverLocalPath)) {
                    qDebug() << "封面文件不存在:" << item->coverLocalPath;
                }
            }

            /// 视频文件缺失则重置为未下载状态
            if (fileMissing) {
                item->localStatus = DeSheng::VideoStatus::UnDownloaded;
                item->localPath = "";
                item->coverLocalPath = "";
                changed = true;
                qDebug() << "重置下载状态:" << item->title << "(scene:" << item->scene << ")";
            }
        }
    }

    if (changed) {
        QString configPath =  QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/videoData/videoData.json";
        QFile file(configPath);
        if (file.open(QIODevice::WriteOnly)) {
            QJsonDocument doc(cl_video_data_info_->cl_video_config_local_.toJson());
            file.write(doc.toJson(QJsonDocument::Indented));
            file.close();
            qDebug() << "本地配置文件已修正:" << configPath;
        }
    }
}

void SoundTestMainPage::netWorkFinished(QNetworkReply *reply)
{
    if (!reply)
        return;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << QString("网络错误: %1").arg(reply->errorString());
        reply->deleteLater();
        return;
    }

    // qint64 totalSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    qDebug() << "当前：" << echoes_number_.load();
    qint64 totalSize = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
    qDebug() << "视频大小：" << totalSize;

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_01")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_01 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_01"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_01
                    = ret_info.data.list.at(0).sceneName;

            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }

        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }

        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_02")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_02 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_02"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_02
                    = ret_info.data.list.at(0).sceneName;

            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }

        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_03")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_03 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_03"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_03
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }

        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_04")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_04 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_04"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_04
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }

        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_05")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_05 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_05"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_05
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }

        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_06")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_06 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_06"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_06
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }
        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_07")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_07 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_07"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_07
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }
        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_08")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_08 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_08"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_08
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }
        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_09")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_09 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_09"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_09
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }
        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }

    if (reply->url().toString().contains("/api/v1/auditions?scene=xhub_10")) {
        QByteArray responseData = reply->readAll();
        qDebug() << "xhub_10 回显原始数据:" << QString::fromUtf8(responseData);

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << QString("JSON解析错误: %1").arg(parseError.errorString());
            reply->deleteLater();
            return;
        }

        DeSheng::AuditionsListResponse ret_info; ///回显信息
        if (DeSheng::ProcessAuditionsListResult(ret_info, jsonDoc)) {
            qDebug() << QString("回显JSON解析成功:   video list total :%1").arg(ret_info.total);

            // /// 确定显示顺序
            // std::sort(ret_info.data.list.begin(),
            //           ret_info.data.list.end(),
            //           [](const DeSheng::VideoItem &a, const DeSheng::VideoItem &b) {
            //               return a.scene.toLower() < b.scene.toLower();
            //           });

            /// 加到对应的
            for (int i = 0; i < ret_info.data.list.size(); ++i) {
                cl_video_data_info_->cl_video_config_netWork_.xhub_videos_grouped_["xhub_10"].append(
                    std::make_shared<DeSheng::VideoItem>(ret_info.data.list.at(i)));
            }
            if (ret_info.data.list.size() > 0)
                cl_video_data_info_->cl_video_config_netWork_.videosType.xhub_10
                    = ret_info.data.list.at(0).sceneName;
            echoes_number_.fetch_add(1);
        } else {
            qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                            .arg(ret_info.code)
                            .arg(ret_info.message);
        }
        if (echoes_number_.load() == 10) {
            /// 执行 ==》校对本地视频配置文件 ==》更新UI操作
            checkLocalConfigurationFile();
        }
        reply->deleteLater();
    }
}

void SoundTestMainPage::dealwithVideoDownLoadRequest()
{
    /// 确定信号发送来源，更新对应的内部信息
    SingleVideoInfo *single_video_info = qobject_cast<SingleVideoInfo *>(sender());
    if (single_video_info) {
        /// 转换成功
        /// 更新其 cl_video_item_ 来改变其内部状态

        if (single_video_info->cl_video_item_->videoUrl.isEmpty()) {
            qWarning() << "视频 URL 为空，无法下载:" << single_video_info->cl_video_item_->videoUrl;
            return;
        }

        const QString downloadDir =  QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/downloads/videos/"
                                    + single_video_info->cl_video_item_->scene + "/";
        QDir dir;
        if (!dir.mkpath(downloadDir)) {
            qWarning() << "无法创建下载目录:" << downloadDir;
            return;
        }

        const QString baseName = QFileInfo(QUrl(single_video_info->cl_video_item_->videoUrl).path())
                                     .fileName();
        if (baseName.isEmpty()) {
            qWarning() << "无法从 URL 提取文件名:" << single_video_info->cl_video_item_->videoUrl;
            return;
        }
        const QString localPath = downloadDir + baseName;

        /// 更新状态为下载中
        single_video_info->cl_video_item_->localStatus = DeSheng::VideoStatus::Downloading;

        const auto saveConfig = [this]() {
            const QString cfgPath = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/videoData/videoData.json";
            QFile cfgFile(cfgPath);
            if (cfgFile.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(this->cl_video_data_info_->cl_video_config_local_.toJson());
                cfgFile.write(doc.toJson(QJsonDocument::Indented));
                cfgFile.close();
            }
        };

        /// 下载视频
        auto *dlManager = new QNetworkAccessManager(this);
        QNetworkReply *reply = dlManager->get(
            QNetworkRequest(QUrl(single_video_info->cl_video_item_->videoUrl)));

        /// 下载进度
        QObject::connect(reply,
                         &QNetworkReply::downloadProgress,
                         this,
                         [single_video_info](qint64 received, qint64 total) {
                             if (total > 0) {
                                 double pct = 100.0 * received / total;
                                 if (single_video_info)
                                     single_video_info->setDownloadProgress(pct);
                             }
                         });

        QObject::connect(reply, &QNetworkReply::finished, this, [=]() mutable {
            reply->deleteLater();
            dlManager->deleteLater();

            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "视频下载失败:" << reply->errorString();
                single_video_info->cl_video_item_->localStatus = DeSheng::VideoStatus::UnDownloaded;
                saveConfig(); ///保存一下配置
                return;
            }

            QFile file(localPath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                qDebug() << "视频下载完成:" << localPath;
            }

            single_video_info->cl_video_item_->localStatus = DeSheng::VideoStatus::Downloaded;
            /// 更新本地路径已就绪
            single_video_info->cl_video_item_->localPath = localPath;
            single_video_info->cl_video_item_->oldVideoUrl = single_video_info->cl_video_item_
                                                                 ->videoUrl;

            saveConfig();
        });

        /// 下载封面
        if (!single_video_info->cl_video_item_->imgUrl.isEmpty()) {
            const QString coverPath = downloadDir
                                      + single_video_info->cl_video_item_->imgUrl.section("/", -1);

            auto *coverManager = new QNetworkAccessManager(this);
            QNetworkReply *coverReply = coverManager->get(
                QNetworkRequest(QUrl(single_video_info->cl_video_item_->imgUrl)));

            QObject::connect(coverReply, &QNetworkReply::finished, this, [=]() mutable {
                coverReply->deleteLater();
                coverManager->deleteLater();

                if (coverReply->error() == QNetworkReply::NoError) {
                    QFile cf(coverPath);
                    if (cf.open(QIODevice::WriteOnly)) {
                        cf.write(coverReply->readAll());
                        cf.close();
                    }
                }

                ///状态
                // single_video_info->cl_video_item_->localStatus = DeSheng::VideoStatus::Downloaded;
                single_video_info->cl_video_item_->coverLocalPath = coverPath;
                single_video_info->setCoverLocalPath(coverPath); ///设置封面

                saveConfig();

                // emit videoData->localConfigUpdated();
            });
        } else {
            // emit videoData->localConfigUpdated();
        }

    } else {
        qWarning() << "转换失败，发送者不是 SingleVideoInfo 类型";
    }
}

void SoundTestMainPage::dealwithColseRequest()
{
    /// 检查小窗口状态
    if (cl_ffmpeg_main_page_->is_minView_.load()) {
        /// 关闭小窗
        cl_ffmpeg_main_page_->minView->hide();
    }

    /// 检查播放状态
    if (!cl_ffmpeg_main_page_->cl_ffmpeg_global_->cl_is_pause_.load()) {
        ///使其暂停
        cl_ffmpeg_main_page_->cl_ffmpeg_global_->pause(true);
    }

    emit closeSoundTest(); ///跳转至方案库页面
}
