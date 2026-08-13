#include "modules/GlobalCustomUI/cumtom_QWidget_global_base.h"

// ========== 静态成员初始化 ==========
std::atomic<double> CumtomQWidgetGlobalBase::s_g_Opacity_{1.0};   // 默认透明度 1.0
std::atomic<qreal> CumtomQWidgetGlobalBase::s_g_BlurRadius_{0.0}; // 默认模糊度 0
QList<CumtomQWidgetGlobalBase *> CumtomQWidgetGlobalBase::s_g_instances_list_;
QMutex CumtomQWidgetGlobalBase::s_g_instance_mutex_;

CumtomQWidgetGlobalBase::CumtomQWidgetGlobalBase(QWidget *parent)
    : QWidget{parent}
{
    InitUIInformation(); // 初始化UI的默认信息
    InitMember();        // 初始化内部成员
    InitConnect();       // 连接默认的信号槽

    // 注册到全局实例列表
    QMutexLocker locker(&s_g_instance_mutex_);
    s_g_instances_list_.append(this);
}

CumtomQWidgetGlobalBase::~CumtomQWidgetGlobalBase()
{
    // 从全局实例列表中移除
    QMutexLocker locker(&s_g_instance_mutex_);
    s_g_instances_list_.removeAll(this);
}

void CumtomQWidgetGlobalBase::InitUIInformation()
{
    setAttribute(Qt::WA_TranslucentBackground);

    {
        cl_background_widget_ = new QWidget(this); ///< 背景页面（可调模糊度）
        cl_background_widget_->setGeometry(rect());
        // 设置背景样式（半透明黑色）
        cl_background_widget_->setStyleSheet("QWidget {"
                                             "background: rgba(0, 0, 0, 0);"
                                             "}");
    }

    cl_content_widget_ = new QWidget(this); ///< 内容页面（可调透明度）
    cl_content_widget_->raise();

    cl_opacityEffect_ = new QGraphicsOpacityEffect(this);
    cl_blurEffect_ = new QGraphicsBlurEffect(this);

    cl_blurEffect_->setBlurHints(QGraphicsBlurEffect::QualityHint);
}

void CumtomQWidgetGlobalBase::InitMember() {}

void CumtomQWidgetGlobalBase::InitConnect() {}

void CumtomQWidgetGlobalBase::applyEffects()
{
    // 读取全局静态值并应用到当前实例
    qreal blurRadius = s_g_BlurRadius_.load();
    double opacity = s_g_Opacity_.load();

    // 更新
    if (cl_blurEffect_) {
        cl_blurEffect_->setBlurRadius(blurRadius);
    }
    //if (cl_opacityEffect_) {
    //    cl_opacityEffect_->setOpacity(opacity);
    //}

    // // 内容层设置透明度
    // if (cl_content_widget_) {
    //     // 动态调整背景层的透明度
    //     int alpha = static_cast<int>(opacity * 100);
    //     cl_content_widget_->setGraphicsEffect(cl_opacityEffect_);
    // }

    // 背景层设置模糊度
    if (cl_background_widget_) {
        cl_background_widget_->setGraphicsEffect(cl_blurEffect_);
    }

    if (cl_content_widget_) {
        cl_content_widget_->raise();
        // 动态调整背景层的透明度
        // int alpha = static_cast<int>(opacity * 100);
        // cl_content_widget_->setGraphicsEffect(cl_opacityEffect_);
    }

    // 触发重绘
    update();
}

double CumtomQWidgetGlobalBase::s_g_Opacity()
{
    return s_g_Opacity_.load();
}

void CumtomQWidgetGlobalBase::setS_g_Opacity(double newS_g_Opacity)
{
    s_g_Opacity_.store(qBound(0.0, newS_g_Opacity, 1.0));
    updateAllInstances(); // 通知所有实例更新
}

qreal CumtomQWidgetGlobalBase::s_g_BlurRadius()
{
    return s_g_BlurRadius_.load();
}

void CumtomQWidgetGlobalBase::setS_g_BlurRadius(qreal newS_g_BlurRadius)
{
    s_g_BlurRadius_.store(qBound(0.0, newS_g_BlurRadius, 25.0));
    updateAllInstances();
}

void CumtomQWidgetGlobalBase::updateAllInstances()
{
    QMutexLocker locker(&s_g_instance_mutex_);
    for (CumtomQWidgetGlobalBase *instance : s_g_instances_list_) {
        if (instance) {
            instance->applyEffects(); // 更新每个实例的效果
        }
    }
}



void CumtomQWidgetGlobalBase::resizeEvent(QResizeEvent *event)
{
    // 同步子控件大小
    if (cl_background_widget_) {
        cl_background_widget_->setGeometry(rect());
    }
    if (cl_content_widget_) {
        cl_content_widget_->setGeometry(rect());
    }
    QWidget::resizeEvent(event);
}

void CumtomQWidgetGlobalBase::paintEvent(QPaintEvent *event)
{
    // 每次绘制时确保效果是最新的
    applyEffects();
    QWidget::paintEvent(event);
}