#ifndef UPLOADMYPLANS_H
#define UPLOADMYPLANS_H

/********************* 上传我的方案到社区的弹窗 *********************/

#include <QDialog>
#include <QButtonGroup>
#include "CustomControl/NewComboBox.h"
#include <QScrollArea>
#include "GlobalVariable.h"
#include "modules/GeneralCustomUI/custom_QScrollArea_general_layout.h"
#include "APOThread/ApoManager.h"

namespace Ui {
class UploadMyPlans;
}

class UploadMyPlans : public QDialog
{
    Q_OBJECT

public:
    explicit UploadMyPlans(QWidget *parent = nullptr);
    ~UploadMyPlans();

    void showMyPlans();//显示我的方案

signals:
    /// \brief 方案创建成功（configId 为服务端返回 ID；-1 表示响应未携带 ID，调用方按全量刷新处理）
    void planUploaded(int configId);

private:
    Ui::UploadMyPlans *ui;

    QHash<QString, NewRadioBtn*> MyPlanRadioHash_Upload;//预设哈希表

    QButtonGroup *group_dev;
    int checkedCount = 0;

    NewCustomToolTip *tip_HideEq = nullptr;

    CustomQScrollAreaGeneralLayout *ScrollArea_MyPlans = nullptr; //我的方案窗体
    DeSheng::UserConfigsCreateRequest *UploadPlansRequest = {};//上传方案 请求结构体
    DeSheng::UserConfigsCreateResponse *UploadPlansResponse = {};//上传方案 回应结构体
    PlanVal UploadPlanVal = {};//上传方案的数据
    QNetworkAccessManager *cl_network_manager_;
    QString UploadPlanUrl = "";//上传方案的服务器路径网址
    QString user_token = "";//用户token
    void CreateConfigs();//使用创建配置接口


    void M_SetCBoxShadow(NewComboBox *cBox);//QComboBox控件的下拉框阴影
    void ShowDev();//显示所有机型

    int writeExportPlanIni(QString filePath);//把方案写入ini文件


protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void on_pTextEdit_Description_textChanged();
    void on_pBt_addDev_clicked();
    void on_pBt_Dev2_clicked();
    void on_pBt_Dev3_clicked();
    void on_pBt_upload_clicked();
    void on_pBt_exit_clicked();

    void on_lEdit_search_textChanged(const QString &arg1);
};

#endif // UPLOADMYPLANS_H
