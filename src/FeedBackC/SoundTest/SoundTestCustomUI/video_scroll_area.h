#ifndef VIDEO_SCROLL_AREA_H
#define VIDEO_SCROLL_AREA_H

#include <QObject>
#include <QScrollArea>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QDir>



#include "FeedBackC/SoundTest/SoundTestCustomUI/single_video_info.h"  ///单个子部件

class VideoScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    VideoScrollArea(ScrollAreaDisplayMode showMode = ScrollAreaDisplayMode::GridDisplay, QObject *parent = nullptr);
    ~VideoScrollArea();

    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember(); ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

    void updateView();

    void dealwithSingleVideo(QString fileName);

signals:
    void openVideo(QString videoFile);

public:
    QList<SingleVideoInfo *>
        cl_all_videos_; ///所有的已下载的的视频（也可以从服务器获取全部可下载的列表。先获取网络列表，再扫描本地路径来确定如何显示）

private:
    QWidget *cl_content_widget_ = nullptr; /// 内容显示区域
    QGridLayout *cl_gridLayout_ = nullptr; ///网格布局
    QHBoxLayout *cl_hBoxLayout_ = nullptr; ///水平布局
    QVBoxLayout *cl_vBoxLayout_ = nullptr; ///垂直布局

    ScrollAreaDisplayMode cl_showMode = ScrollAreaDisplayMode::GridDisplay; ///默认网格布局
    int cl_columnCount_; ///子部件行数

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // VIDEO_SCROLL_AREA_H
