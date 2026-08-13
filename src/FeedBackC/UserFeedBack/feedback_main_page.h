#ifndef FEEDBACK_MAIN_PAGE_H
#define FEEDBACK_MAIN_PAGE_H

#include <QGraphicsDropShadowEffect>
#include <QHttpMultiPart>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

#include "FeedBackC/UserFeedBack/feedback_scrollarea_widget.h"
#include "data/api_global.h" ///< BaseClient + ApiConfig + g_api_server_switch

namespace Ui {
class FeedbackMainPage;
}

/// \brief 用户反馈页面
/// 子控件：FeedbackScrollareaWidget: 可滑动表单区域
class FeedbackMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit FeedbackMainPage(QWidget *parent = nullptr);
    ~FeedbackMainPage();

private:
    void InitUIInformation();
    void InitMember();
    void InitConnect();

public:
    void sendFeedback(const DeSheng::UserFeedBacksRequest &req);
    void setCl_feedback_path(const QString &path);
    QString cl_access_token() const;
    void setCl_access_token(const QString &newCl_access_token);
    void addFileUploadsResponseToList(DeSheng::FileUploadsResponse res);
    void resetFileUploadsResponseList();
    int getFileUploadsResponseListSize();
    void onAllImagesUploaded();
    void resetForNewFeedback(); ///< 重置反馈表单，准备再次提交
    void LanguageSet();         ///< 语言切换：刷新 .ui 文本与一次性 setText 内容

public slots:
    void showOutInfo();

signals:
    void success(const QString &response);
    void error(const QString &errorMessage);
    void FeedBackSubmitting();
    void FeedBackSubmitSucceed();
    void FeedBackSubmitFail();

private:
    Ui::FeedbackMainPage *ui;

    FeedbackScrollareaWidget *clp_Scrollarea_widget = nullptr;
    QString cl_feedback_path_ = "/feedbacks";
    QString cl_access_token_;

    QMutex cl_upload_images_mutex_;
    QList<DeSheng::FileUploadsResponse> cl_uploadImagesResponse_;

    QGraphicsDropShadowEffect *clp_shadow_ = nullptr;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};

#endif // FEEDBACK_MAIN_PAGE_H
