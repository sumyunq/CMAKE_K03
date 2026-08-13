#ifndef CUSTOM_QWIDGET_PRODUCT_DISPLAY_H
#define CUSTOM_QWIDGET_PRODUCT_DISPLAY_H

#include <QLabel>
#include <QParallelAnimationGroup>
#include <QRect>
#include <QTimer>
#include <QWidget>

#include "Popup/CustomTipPopup/NewCustomToolTip.h"           ///< 自定义提示按键 头文件
#include "modules/HomePage/HomePageCustomUI/custom_QPushbutton_hover_leftward.h" ///< 按键子部件头文件

#include "modules/GlobalCustomUI/cumtom_QWidget_global_base.h"

namespace Ui {
class CustomQWidgetProductDisplay;
}
///
/// \brief The CustomQWidgetProductDisplay class
/// 产品展示页面
/// 子部件：
///     机型选择按键
///     声音设置按键
///     说明书按键
///     背景图片
class CustomQWidgetProductDisplay : public CumtomQWidgetGlobalBase
{
    Q_OBJECT

public:
    explicit CustomQWidgetProductDisplay(QWidget *parent = nullptr);
    ~CustomQWidgetProductDisplay();

    void retranslateTexts();
    void UpdateBackgroundImage(const QString &imagePath,
                               QPoint globalStartPoint,
                               QSize startSize,
                               QPoint globalTargetPoint,
                               QSize targetSize); ///< 更新产品背景图片、动态效果
    void UpdateBackgroundImageImmediately(const QString &imagePath);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    QRect productImageRect() const;
    QPixmap scaledProductPixmap(const QPixmap &pixmap) const;
    void updateProductPixmapLabel();
public:
    /******************** UI ********************/
    CustomQPushButtonHoverLeftward *cl_pBt_devSel_ = nullptr; ///< 选择机型
    QPoint cl_pBt_devSel_default_point_ = QPoint(471, 24);    ///< 选择机型 按键默认位置

    CustomQPushButtonHoverLeftward *cl_pBt_SysVloSet_ = nullptr; ///< 声音设置
    QPoint cl_pBt_SysVloSet_default_point_ = QPoint(471, 66);    ///< 声音设置 按键默认位置

    CustomQPushButtonHoverLeftward *cl_pBt_UserGuide_ = nullptr; ///< 说明书
    QPoint cl_pBt_UserGuide_default_point_ = QPoint(471, 108);   ///< 说明书 按键默认位置

    QPushButton *cl_pBt_explain_ = nullptr;                ///< 说明按键
    NewCustomToolTip *clp_tip_explain_ = nullptr;          ///< 自定义提示控件
    QPoint cl_pBt_explain_default_point_ = QPoint(23, 24); ///< 说明按键 默认位置
    QSize cl_pBt_explain_default_size_ = QSize(25, 25);    ///< 说明按键 默认位置

    QLabel *cl_pixmap_label_ = nullptr; ///< 背景图片
    QPixmap cl_background_pixmap_;      ///背景图片
    QString cl_images_path_;            ///< 背景图片路径
    QLabel *flyLabel;                   ///< 动画标签：从 原始位置 放大到 目标位置，然后隐藏

    QPropertyAnimation *posAnim = nullptr;        ///< 位置动画
    QPropertyAnimation *sizeAnim = nullptr;       ///< 尺寸动画
    QParallelAnimationGroup *animGroup = nullptr; ///< 动画组（包含 中心位置、尺寸 变化）

private:
    Ui::CustomQWidgetProductDisplay *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    // virtual void paintEvent(QPaintEvent *event) override;
};

#endif // CUSTOM_QWIDGET_PRODUCT_DISPLAY_H
