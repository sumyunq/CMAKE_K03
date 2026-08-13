#ifndef FEEDBACK_SCROLLAREA_WIDGET_H
#define FEEDBACK_SCROLLAREA_WIDGET_H

#include <QClipboard>
#include <QEvent>
#include <QPlainTextEdit>
#include <QScroller>
#include <QSysInfo>
#include <QWidget>


#include "data/api_global.h" ///网络请求结构体
#include "FeedBackC/UserFeedBack/feedback_images_widget.h"      ///反馈图片部件

#include "FeedBackC/FirmwareTool/firmware_tool.h" ///固件信息查询等

namespace Ui {
class FeedbackScrollareaWidget;
}

///
/// \brief The FeedbackScrollareaWidget class
/// 用户反馈滑动区域
class FeedbackScrollareaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FeedbackScrollareaWidget(QWidget *parent = nullptr);
    ~FeedbackScrollareaWidget();

    void InitUIInformation();
    void InitMember();
    void InitConnect();

    int getFeedBackImagesSize(); ///获取反馈图片数量，用于 FeedbackMainPage 轮询定时器判断
    QString getFeedBackImagesName(int index); ///获取反馈图片名称

    DeSheng::UserFeedBacksRequest getReq() const;

    bool requiredFieldVerify(); ///必填字段校验

    void updateFWInfo();     ///更新一下固件信息
    void clearLineOldInfo(); ///清空一下旧的信息
    void resetForNewFeedback(); ///重置为一次新的反馈
    void ShowLineInfo();     //显示自动获取文件信息
    void LanguageSet();      ///语言切换：刷新 .ui 文本与代码内一次性 setText

    void setLineEditError(QLineEdit *lineEdit, bool hasError);
    void setQPlainTextEditError(QPlainTextEdit *plainTextEdit, bool hasError);

private:
    Ui::FeedbackScrollareaWidget *ui;
    FeedbackImagesWidget *clp_FeedbackImages_; ///   反馈图片部件

    FirmwareTool *clp_FirmwareTool_; ///< 固件信息相关处理

    // QWidget interface
protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override; ///< 事件过滤器
};
#endif // FEEDBACK_SCROLLAREA_WIDGET_H
