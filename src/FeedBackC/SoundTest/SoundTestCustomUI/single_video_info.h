#ifndef SINGLE_VIDEO_INFO_H
#define SINGLE_VIDEO_INFO_H

#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QWidget>

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>

#include <QApplication>
#include <QFileDialog>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>
#include <mutex>

#include "FeedBackC/SoundTest/SoundTestCustomUI/QWidget_video_info.h" ///单个视频封面 widget

#include "data/api_global.h" /// 数据结构



namespace Ui {
class SingleVideoInfo;
}

class SingleVideoInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SingleVideoInfo(ScrollAreaDisplayMode showMode = ScrollAreaDisplayMode::GridDisplay,
                             QWidget *parent = nullptr);
    ~SingleVideoInfo();

    void UpdateUI(ScrollAreaDisplayMode showMode = ScrollAreaDisplayMode::GridDisplay); /// 根据模式去更新内部控件位置

    void setInfoText(const QString &newInfoText);
    void setDownloadProgress(double downloadProgress);
    void setCoverLocalPath(QString coverLocalPath); ///设置封面


    static int getMinWidth();
    static int getMinHeight();

signals:
    void openFile(QString fileName); ///打开指定本地文件
    void openFileWithMinWidget(QString fileName); ///以小窗口模式打开本地文件

    void requestDownLoadVideoFile(); ///下载指定视频文件到本地

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    void drawUNDownLoad(QPainter &painter);  ///绘制未下载状态
    void drawDownLoading(QPainter &painter); ///绘制下载中状态
    void drawDownLoaded(QPainter &painter);  ///绘制已下载状态

public:
    /*********************************************************************** 视频相关 ***********************************************************************/
    std::shared_ptr<DeSheng::VideoItem>
        cl_video_item_; ///单个视频信息 ///UI 显示详细取决于该成员变量   ///共享指针，指向 SoundTestMainPage 中 cl_video_data_info_->cl_video_config_local_->xhub_videos_grouped_["xhub_0x"].at(i)

private:
    Ui::SingleVideoInfo *ui;

    /*********************************************************************** UI相关 ***********************************************************************/
    ScrollAreaDisplayMode cl_showMode = ScrollAreaDisplayMode::GridDisplay; ///默认网格显示

    inline static int minWidth = 238;
    inline static int minHeight = 174;
    inline static int maxWidth = 238;
    inline static int maxHeight = 174;

    QWidgetVideoInfo *videoCover = nullptr; ///视频封面
    int videoCover_x = 0;
    int videoCover_y = 0;
    int videoCover_width = 238;
    int videoCover_height = 134;
    int videoCover_radius = 10; /// 封面圆角 (网格 10 px 、垂直 8px)

    QLabel *videoInfo = nullptr; ///视频描述
    QString infoText;            ///具体信息
    int videoInfo_x = 20;
    int videoInfo_y = 148;
    int videoInfo_width = 200;
    int videoInfo_height = 20;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // SINGLE_VIDEO_INFO_H
