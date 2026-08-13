#ifndef CUSTOM_QPUSHBUTTON_HOVER_LEFTWARD_H
#define CUSTOM_QPUSHBUTTON_HOVER_LEFTWARD_H

#include <QObject>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QWidget>
#include <QParallelAnimationGroup>


///
/// \brief The btnStatus enum
/// 按键状态
enum class Custom_QPushButton_Status {
    DefaultStatus = 0, ///默认状态
    ExpandStatus = 1,   ///悬停时展开状态
    ReboundStatus = 2   ///离开时回缩状态
};

///
/// \brief The CustomQPushButtonHoverLeftward class
///
/// 构造时 需指定 setCl_text setCl_pixmap setCl_default_pixmap setCl_text_rect
class CustomQPushButtonHoverLeftward : public QPushButton
{
    Q_OBJECT
public:
    CustomQPushButtonHoverLeftward(QWidget *parent = nullptr);
    ~CustomQPushButtonHoverLeftward();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember(); ///< 初始化内部成员
    void InitConnect(); ///< 连接默认的信号槽

public:
    void setCl_reference_point(QPoint newCl_reference_point);
    void setCl_text(const QString &newCl_text);

    void setCl_default_size(const QSize &newCl_default_size); ///< 修改默认尺寸
    QSize cl_default_size() const;

    void setCl_expand_size(const QSize &newCl_expand_size); ///< 修改展开尺寸

    void setCl_text_rect(const QRect &newCl_text_rect); ///< 确定内部矩阵文字位置

    void setCl_pixmap(const QPixmap &newCl_pixmap);

    void setCl_default_pixmap(const QPixmap &newCl_default_pixmap);

private:
    Custom_QPushButton_Status cl_pbt_status_ = Custom_QPushButton_Status::DefaultStatus; ///< 按键状态
    QSize cl_default_size_ = QSize(32, 32); ///< 默认状态：32*32 图标 可修改
    QSize cl_expand_size_ = QSize(99, 32); ///< 展开状态：99*32 图标 可修改

    QPropertyAnimation *cl_size_anim_ = nullptr; ///< 大小动画
    QPropertyAnimation *cl_pos_anim_ = nullptr; ///< pos->x位置动画
    QPoint cl_reference_point_; /// 动画参考位置点坐标（相对于父坐标）
    QParallelAnimationGroup *cl_anim_group_ = nullptr;

    QString cl_text_ = ""; ///< 内部文字
    QRect cl_text_rect_;   ///< 内部文字矩阵位置
    QPixmap cl_pixmap_;    ///< 对应的悬停图标
    QRect cl_pixmap_rect_ = QRect(14, 8, 17, 16); ///< 图标 矩阵位置
    QPixmap cl_default_pixmap_;    ///< 对应的默认图标

    // QWidget interface
protected:
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // CUSTOM_QPUSHBUTTON_HOVER_LEFTWARD_H
