#ifndef QWIDGET_VIDEO_MASK_H
#define QWIDGET_VIDEO_MASK_H

#include <QPushButton>
#include <QWidget>

#include "FeedBackC/SoundTest/SoundTestCustomUI/download_videopending.h" /// 试听视频状态（未下载/下载中/已下载）

namespace Ui {
class QWidgetVideoMask;
}

///
/// \brief The QWidgetVideoMask class
/// 视频内容遮罩,内置视频状态部件
class QWidgetVideoMask : public QWidget
{
    Q_OBJECT

public:
    explicit QWidgetVideoMask(QWidget *parent = nullptr);
    ~QWidgetVideoMask();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    void drawUNDownLoad(QPainter &painter);  ///绘制未下载状态
    void drawDownLoading(QPainter &painter); ///绘制下载中状态
    void drawDownLoaded(QPainter &painter);  ///绘制已下载状态

public:
    QPushButton *cl_minView_pBn_ = nullptr; ///右上角小窗口播放按键，下载完成后显示
    DownLoadVideoPending *cl_video_downLoad_pending_ = nullptr; /// 中间的视频状态图标

    void setRadius(int newRadius);

private:
    Ui::QWidgetVideoMask *ui;
    /*********************************************************************** UI相关 ***********************************************************************/
    // QColor cl_shadow_color_ = "#999999";         ///阴影颜色
    QColor cl_shadow_color_ = "#FF0000";         ///阴影颜色
    int radius = 5;                              ///默认圆角
    double background_opacity_unDownLoad = 0.3;  /// 遮罩透明度(未下载状态)
    double background_opacity_downLoading = 0.3; /// 遮罩透明度(下载中状态)
    double background_opacity_downLoaded = 0.3;  /// 遮罩透明度(已下载状态)

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // QWIDGET_VIDEO_MASK_H
