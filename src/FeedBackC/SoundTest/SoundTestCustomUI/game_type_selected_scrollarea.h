#ifndef GAME_TYPE_SELECTED_SCROLLAREA_H
#define GAME_TYPE_SELECTED_SCROLLAREA_H

#include <QButtonGroup>
#include <QDir>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

#include "FeedBackC/SoundTest/SoundTestCustomUI/QPushButton_single_game_type.h" /// 子部件（单个按键）

#include "FeedBackC/SoundTest/SoundTestDataStruct/video_data.h"  /// 视频状态配置信息


///
/// \brief The GameTypeSelectedScrollArea class
/// 游戏视频类型选择,默认水平布局排列
class GameTypeSelectedScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    GameTypeSelectedScrollArea(QWidget *parent = nullptr);
    ~GameTypeSelectedScrollArea();

    void updateView();

    void onGameTypeClicked(int index); ///处理对应点击的按钮（切换 可播放视频区域 ）

signals:
    void changeGameTypeVideos(int index);   ///发送信息体或者index
    // void changeGameTypeVideos(QString gameType);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QList<QPushButtonSingleGameType *> cl_all_game_type_; /// 从网络服务器获取到的可试听视频 的 游戏类型
    QButtonGroup *cl_all_games_type_buttons_ = nullptr; ///游戏类型按键组

private:
    QWidget *cl_content_widget_ = nullptr; /// 内容显示区域
    QHBoxLayout *cl_hBoxLayout_ = nullptr; ///水平布局

    ///布局属性
    int left_margin_ = 0;
    int top_margin_ = 0;
    int right_margin_ = 0;
    int bottom_margin_ = 0;
    int spacing_ = 4; ///内部部件见间距

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // GAME_TYPE_SELECTED_SCROLLAREA_H
