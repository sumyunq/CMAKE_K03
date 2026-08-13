#ifndef DOWNLOAD_VIDEOPENDING_H
#define DOWNLOAD_VIDEOPENDING_H

#include <QBrush>
#include <QLabel>
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTimer>
#include <QWidget>


#include "data/api_global.h"


namespace Ui {
class DownLoadVideoPending;
}

///
/// \brief The DownLoadVideoPending class
/// 剩余：点击事件判断状态，发射对应的信号
class DownLoadVideoPending : public QWidget
{
    Q_OBJECT

public:
    explicit DownLoadVideoPending(QWidget *parent = nullptr);
    ~DownLoadVideoPending();

    DeSheng::VideoStatus cl_downLoadVideoStatus() const;
    void setCl_downLoadVideoStatus(DeSheng::VideoStatus newCl_downLoadVideoStatus);

    double cl_download_progress() const;
    void setCl_download_progress(double newCl_download_progress);

signals:
    void DownLoadVideoStatusChanged(DeSheng::VideoStatus newCl_downLoadVideoStatus);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    void drawUNDownLoad(QPainter &painter);  ///绘制未下载状态
    void drawDownLoading(QPainter &painter); ///绘制下载中状态
    void drawDownLoaded(QPainter &painter);  ///绘制已下载状态

private:
    Ui::DownLoadVideoPending *ui;
    /*********************************************************************** UI相关 ***********************************************************************/
    DeSheng::VideoStatus  cl_downLoadVideoStatus_
        = DeSheng::VideoStatus::UnDownloaded;       /// 下载状态, 默认未下载
    std::unique_ptr<QMovie> cl_movie_ = nullptr; /// 下载时动画(.gif)
    double cl_download_progress_ = 0.0;          ///下载进度

    /// 整体widget属性
    inline static int minWidth = 40;
    inline static int minHeight = 40;
    inline static int maxWidth = 40;
    inline static int maxHeight = 40;

    /// 进度填充区域相关
    std::atomic<int> cl_lineWidth_ = 3;      /// 填充宽度(默认固定为 3)
    std::atomic<int> outer_radius_ = 20;     /// 外圈半径(最小20，默认为整体的一般宽度)
    std::atomic<int> inner_radius_ = 20 - 3; /// 内圈圆弧半径(外圈半径 - 填充宽度)

    /// 内部图标属性
    QLabel *cl_label_icon_ = nullptr;
    std::atomic<int> cl_label_x_ = 10;
    std::atomic<int> cl_label_y_ = 10;
    std::atomic<int> cl_label_width_ = 20;
    std::atomic<int> cl_label_height_ = 20;

    QColor cl_default_background_color_ = QColor("#4c4c4c"); ///默认背景色(仅圆内)
    QColor cl_hover_background_color_ = QColor("#5e5e5e");   ///悬停背景色(仅圆内)
    QColor cl_fill_color_ = QColor("#0091da");               ///下载进度填充区域

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // DOWNLOAD_VIDEOPENDING_H
