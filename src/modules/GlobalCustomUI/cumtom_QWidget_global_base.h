#ifndef CUMTOM_QWIDGET_GLOBAL_BASE_H
#define CUMTOM_QWIDGET_GLOBAL_BASE_H

#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QMutex>
#include <QPainter>
#include <QWidget>

///
/// \brief The CumtomQWidgetGlobalBase class
/// 全局 widget 基类
/// 主要用于透明度、模糊度调节
class CumtomQWidgetGlobalBase : public QWidget
{
    Q_OBJECT
public:
    explicit CumtomQWidgetGlobalBase(QWidget *parent = nullptr);
    ~CumtomQWidgetGlobalBase();

    // ========== 全局控制接口（静态） ==========
    static double s_g_Opacity();
    static void setS_g_Opacity(double newS_g_Opacity);
    static qreal s_g_BlurRadius();
    static void setS_g_BlurRadius(qreal newS_g_BlurRadius);

    // 更新所有实例
    static void updateAllInstances();


signals:

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    void applyEffects(); ///< 应用当前全局效果到当前实例

public:
    // UI 组件
    QWidget *cl_content_widget_ = nullptr;               ///< 内容页面（可调透明度）
    QWidget *cl_background_widget_ = nullptr;            ///< 背景页面（可调模糊度）
    QGraphicsOpacityEffect *cl_opacityEffect_ = nullptr; ///< 调节透明度
    QGraphicsBlurEffect *cl_blurEffect_ = nullptr;       ///< 调节模糊度

    QWidget *backgroundWidget = nullptr;

private:
    // ========== 静态成员 ==========
    static std::atomic<double> s_g_Opacity_;                     // 全局透明度 默认1.0 (0 到 1)
    static std::atomic<qreal> s_g_BlurRadius_;                   // 全局模糊度 默认0.0 (0 到 25)
    static QList<CumtomQWidgetGlobalBase *> s_g_instances_list_; // 所有实例列表
    static QMutex s_g_instance_mutex_;                           // 实例列表互斥锁

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // CUMTOM_QWIDGET_GLOBAL_BASE_H
