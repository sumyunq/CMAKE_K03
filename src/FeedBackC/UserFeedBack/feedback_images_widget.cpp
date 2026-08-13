#include "FeedBackC/UserFeedBack/feedback_images_widget.h"
#include "ui_feedback_images_widget.h"

FeedbackImagesWidget::FeedbackImagesWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FeedbackImagesWidget)
{
    ui->setupUi(this);
    InitMember();

    // this->setMinimumSize(QSize(3 * FeedbackImagesLabel::cl_minrect_w() + 10,FeedbackImagesLabel::cl_minrect_h() + 10 ));
    // this->setMaximumSize(QSize(3 * FeedbackImagesLabel::cl_maxrect_w() + 10,FeedbackImagesLabel::cl_maxrect_h() + 10));

    /// 创建默认的显示
    for (int i = 0; i < 3; ++i) {
        FeedbackImagesLabel *p_b = new FeedbackImagesLabel(ui->widget_images);

        /// 连接到统一槽函数,进行单独处理
        QObject::connect(p_b,&FeedbackImagesLabel::AddFeedbackImagesLabelSucceed,this,&FeedbackImagesWidget::dealwithAddFeedbackimagesSucceed);
        QObject::connect(p_b,&FeedbackImagesLabel::DelFeedbackImagesLabelSucceed,this,&FeedbackImagesWidget::dealwithDelFeedbackimagesSucceed);

        cl_FeedbackImagesLabel_list_.append(p_b);
    }

    updateView();
}

FeedbackImagesWidget::~FeedbackImagesWidget()
{
    // 清空 layout
    if (cl_hbox_layout_) {
        while (QLayoutItem* item = cl_hbox_layout_->takeAt(0)) {
            delete item; // 只删 item，不删 widget
        }
        delete cl_hbox_layout_;
        cl_hbox_layout_ = nullptr;
    }

    // 清理 list
    cl_FeedbackImagesLabel_list_.clear();

    // UI
    delete ui;
    ui = nullptr;
}

void FeedbackImagesWidget::InitMember()
{
    maxSize = 3; ///最多三张
    cl_hbox_layout_ = new QHBoxLayout(ui->widget_images);  ///对应的水平布局
    if (cl_hbox_layout_) {
        cl_hbox_layout_->setSpacing(10);
        cl_hbox_layout_->setContentsMargins(0, 0, 0, 0);
    }

}

void FeedbackImagesWidget::updateView() {

    if (!cl_hbox_layout_) {
        return;
    }

    this->setUpdatesEnabled(false);

    QList<FeedbackImagesLabel*> imageList;
    QList<FeedbackImagesLabel*> emptyList;

    for (auto* label : cl_FeedbackImagesLabel_list_) {
        if (!label) {
            continue;
        }

        if (label->cl_feedback_file_name().isEmpty()) {
            emptyList.append(label);
        } else {
            imageList.append(label);
        }
    }

    while (QLayoutItem* item = cl_hbox_layout_->takeAt(0)) {

        if (QWidget* w = item->widget()) {
            cl_hbox_layout_->removeWidget(w);
            w->hide();
        }

        delete item;
    }

    for (auto* label : imageList) {
        cl_hbox_layout_->addWidget(label);
        label->show();
    }

    if (imageList.size() < maxSize && !emptyList.isEmpty()) {
        FeedbackImagesLabel* addLabel = emptyList.first();
        cl_hbox_layout_->addWidget(addLabel);
        addLabel->show();
    }

    cl_hbox_layout_->addStretch();

    this->setUpdatesEnabled(true);
    this->update();
}

void FeedbackImagesWidget::dealwithAddFeedbackimagesSucceed()
{
    updateView();
}

void FeedbackImagesWidget::dealwithDelFeedbackimagesSucceed()
{

    updateView();
}

QList<QString> FeedbackImagesWidget::getFeedbackImages_files() const
{
    QList<QString> FeedbackImages_files; ///反馈图片文件列表
    for (auto t : cl_FeedbackImagesLabel_list_) {
        if (!t->cl_feedback_file_name().isEmpty())
            FeedbackImages_files << t->cl_feedback_file_name();
    }

    return FeedbackImages_files;
}

void FeedbackImagesWidget::clearOldImages()
{
    for (auto t : cl_FeedbackImagesLabel_list_) {
        t->resetLabel();
    }
}
