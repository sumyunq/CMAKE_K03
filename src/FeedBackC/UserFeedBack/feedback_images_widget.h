#ifndef FEEDBACK_IMAGES_WIDGET_H
#define FEEDBACK_IMAGES_WIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <algorithm>


#include "FeedBackC/UserFeedBack/feedback_images_label.h"  ///单个反馈图片

namespace Ui {
class FeedbackImagesWidget;
}

class FeedbackImagesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FeedbackImagesWidget(QWidget *parent = nullptr);
    ~FeedbackImagesWidget();

    void InitMember(); ///< 初始化内部成员

    void updateView();  ///更新视图

    QList<QString> getFeedbackImages_files() const;

    void clearOldImages();

protected slots:
    void dealwithAddFeedbackimagesSucceed(); ///处理反馈反馈图片添加成功
    void dealwithDelFeedbackimagesSucceed(); ///处理反馈反馈图片删除成功

private:
    Ui::FeedbackImagesWidget *ui;

    int maxSize;    ///最多反馈图片数
    QHBoxLayout *cl_hbox_layout_ = nullptr; ///水平布局
    QList<FeedbackImagesLabel *> cl_FeedbackImagesLabel_list_;  ///反馈图片部件集合

};

#endif // FEEDBACK_IMAGES_WIDGET_H
