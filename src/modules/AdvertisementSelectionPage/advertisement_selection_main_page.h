#ifndef ADVERTISEMENT_SELECTION_MAIN_PAGE_H
#define ADVERTISEMENT_SELECTION_MAIN_PAGE_H

#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QLabel>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QPainterPath>
#include <QTimer>
#include <QVariantAnimation>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

#include "data/api_global.h"
#include "modules/AdvertisementSelectionPage/AdvertisementSelectionPageCustomUI/custom_QScrollarea_for_advertisement_pushbutton.h" ///< 子部件：底部按键滑动区域

namespace Ui {
class AdvertisementSelectionMainPage;
}

/// \brief 广告选择页面
/// 子控件：
///     - CustomQScrollAreaForAdvertisementPushButton: 广告按键滚动区域
class AdvertisementSelectionMainPage : public QWidget
{
    Q_OBJECT
public:
    explicit AdvertisementSelectionMainPage(QWidget *parent = nullptr);
    ~AdvertisementSelectionMainPage();

    void updateAdvertisementList(); ///< 更新广告列表

signals:
    void advertisementListReady(); ///< 广告列表及图片全部下载完成

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
protected slots:
    void updateAdvertisementIndex(int index); ///< 显示下一页广告

private:
    void downloadAdImages(int t_idx); ///< 递归下载广告图片

public:
    QList<std::shared_ptr<DeSheng::AdvertisementItem>> cl_advertisement_list_; ///< 可查看广告列表
    QTimer *cl_change_timer_ = nullptr;                                        ///< 定时变换
    std::atomic<int> cl_current_index_ = 0; ///< 当前显示广告的 索引
    QString cl_cache_path_;                 ///< 缓存目录

    CustomQScrollAreaForAdvertisementPushButton *cl_advertisement_pushButton_scrollArea_
        = nullptr; ///< 底部按键

    QPixmap cl_background_image_; ///< 广告图片
    QPixmap cl_next_background_image_; ///< 正在淡入的广告图片

private:
    Ui::AdvertisementSelectionMainPage *ui;

    int cl_image_download_index_ = 0; ///< 当前正在下载的图片索引
    qreal cl_image_transition_progress_ = 1.0; ///< 图片切换透明度进度
    QVariantAnimation *cl_image_transition_anim_ = nullptr; ///< 图片切换动画

    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // ADVERTISEMENT_SELECTION_MAIN_PAGE_H
