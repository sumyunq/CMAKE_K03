#ifndef CUSTOM_QPUSHBUTTON_SINGLE_PLAN_H
#define CUSTOM_QPUSHBUTTON_SINGLE_PLAN_H

#include <QObject>
#include <QPainter>
#include <QPushButton>
#include <QWidget>

/// \brief 单个方案预设按键
/// 支持 默认/悬停/选中 三态图标切换
class CustomQPushButtonSinglePlan : public QPushButton
{
    Q_OBJECT
public:
    CustomQPushButtonSinglePlan(QWidget *parent = nullptr);
    ~CustomQPushButtonSinglePlan();

    void setImages(const QString &normal, const QString &hover, const QString &checked);

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember(); ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

    void updateStyle(); ///<更新样式


public:
    /******************** UI ********************/
    QPair<QString, QString> cl_plan_key_;  ///< 记录 方案键值
    QString cl_plans_name_ = "NULL"; ///< 该方案名称
    QPixmap cl_plans_pixmap_; ///< 背景图片
    QSize cl_default_size_ = QSize(58,58);  ///默认大小
    QSize cl_max_size_ = QSize(116,116);  ///max大小


    QRect cl_text_rect_; //文本框位置

    QString cl_normalImg_ = ""; //正常背景
    QString cl_hover_img_ = ""; //悬浮背景
    QString cl_checkedImg_ = ""; //选中背景

    // QWidget interface
    void setPlan_key(const QPair<QString, QString> &newPlan_key);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // CUSTOM_QPUSHBUTTON_SINGLE_PLAN_H
