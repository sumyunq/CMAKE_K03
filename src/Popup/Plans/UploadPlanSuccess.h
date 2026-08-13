#ifndef UPLOADPLANSUCCESS_H
#define UPLOADPLANSUCCESS_H

/********************* 成功上传我的方案到社区的弹窗 *********************/

#include <QDialog>

namespace Ui {
class UploadPlanSuccess;
}

class UploadPlanSuccess : public QDialog
{
    Q_OBJECT

public:
    explicit UploadPlanSuccess(QWidget *parent = nullptr);
    ~UploadPlanSuccess();
    //显示今天已经上传的方案数量
    void ShowUploadPlanCnt(int cnt);

    void setTheme_UploadPlanSuccess(int idx);//根据主题设置样式

private:
    Ui::UploadPlanSuccess *ui;
};

#endif // UPLOADPLANSUCCESS_H
