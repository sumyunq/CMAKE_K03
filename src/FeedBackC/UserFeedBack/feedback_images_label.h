#ifndef FEEDBACK_IMAGES_LABEL_H
#define FEEDBACK_IMAGES_LABEL_H

#include <QLabel>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>

#include <QFileDialog>
#include <QApplication>
#include <QScreen>

#include <QPushButton>
#include <QPainterPath>

#include "modules/GeneralCustomUI/custom_QDialog_general_tips.h"




class FeedbackImagesLabel : public QLabel
{
    Q_OBJECT
public:
    explicit FeedbackImagesLabel(QWidget *parent = nullptr);
    ~FeedbackImagesLabel();

    void InitMember();  ///< 初始化内部成员
    void InitUIInformation();  ///< 设置默认Ui样式
    void InitConnect(); ///初始化必要的信号槽

    bool isImageSizeValid(QString imagefileName);  ///< 图片格式校验
    void showImageLimitDialog(); ///< 显示图片大小超限提示

    // QWidget interface
    static int cl_minrect_w();
    static int cl_minrect_h();
    static int cl_maxrect_w();
    static int cl_maxrect_h();

    QString cl_feedback_file_name() const;
    void setCl_feedback_file_name(const QString &newCl_feedback_file_name);
    QPixmap cl_feedback_pixmap() const;
    void setCl_feedback_pixmap(const QPixmap &newCl_feedback_pixmap);

    void resetLabel();

signals:
    // void FeedbackImagesLabelClicked();  ///反馈图片点击
    void AddFeedbackImagesLabelSucceed();  ///反馈图片添加成功
    void DelFeedbackImagesLabelSucceed();  ///反馈图片删除成功

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
private:
    QString cl_feedback_file_name_;   ///< 反馈背景图片文件名
    QPixmap cl_feedback_pixmap_; ///< 缓存背景图
    bool cl_is_selected_;   ///< 是否选中,用于高亮显示，并可设置对应的背景透明度
    bool cl_is_hover_;   ///< 是否悬停显示,鼠标悬停时

    inline static int side = 13;         ///< 加号大小,默认 min_width/6
    inline static int cl_minrect_w_ = 80; ///< 最小宽
    inline static int cl_minrect_h_ = 80; ///< 最小高
    inline static int cl_maxrect_w_ = 80; ///< 最大宽
    inline static int cl_maxrect_h_ = 80; ///< 最大高

    QPushButton *clp_pushButotn_del_;   ///删除按钮


};

#endif // FEEDBACK_IMAGES_LABEL_H
