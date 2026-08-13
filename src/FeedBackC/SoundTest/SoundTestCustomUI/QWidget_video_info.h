#ifndef QWIDGET_VIDEO_INFO_H
#define QWIDGET_VIDEO_INFO_H

#include <QVariantAnimation>
#include <QWidget>

#include "FeedBackC/SoundTest/SoundTestCustomUI/QWidget_video_mask.h" ///内部的遮罩部件

namespace Ui {
class QWidgetVideoInfo;
}

///
/// \brief The QWidgetVideoInfo class
/// 内置一个遮罩部件，悬停时，根据内部视频状态进行对应显示
/// 一个背景封面,悬停时放大（动画效果）
class QWidgetVideoInfo : public QWidget
{
    Q_OBJECT

public:
    explicit QWidgetVideoInfo(QWidget *parent = nullptr);
    ~QWidgetVideoInfo();

    void setImages_radius(int newImages_radius);

    QPixmap clp_video_pixmap() const;
    void setClp_video_pixmap(const QPixmap &newClp_video_pixmap);

public slots:
    void DownLoadVideoStatusChange(
        DeSheng::VideoStatus newCl_downLoadVideoStatus);  ///video状态改变,触发一次重绘
    void DownloadProgressChange(double downloadProgerss); ///下载进度更新，触发一次重绘

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    void drawUNDownLoad(QPainter &painter);  ///绘制未下载状态
    void drawDownLoading(QPainter &painter); ///绘制下载中状态
    void drawDownLoaded(QPainter &painter);  ///绘制已下载状态

public:
    QWidgetVideoMask *clp_video_mask_ = nullptr; /// 内部遮罩，悬停时显示

private:
    Ui::QWidgetVideoInfo *ui;
    /*********************************************************************** UI相关 ***********************************************************************/
    QPixmap clp_video_pixmap_; /// 内部背景封面图片，悬停时放大

    double scaling_factor_ = 1.0;         /// 实际缩放系数（重绘时真实的计算值）
    double default_scaling_factor_ = 1.0; /// 默认缩放系数
    double target_scaling_factor_ = 1.08; /// 目标放大系数
    int images_radius_ = 0;               ///封面圆角

    bool is_entering = false; ///鼠标是否进入该widget内部（悬停）

    std::unique_ptr<QVariantAnimation> cl_background_enlarge_anim_ = nullptr; ///背景放大动画

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

#endif // QWIDGET_VIDEO_INFO_H
