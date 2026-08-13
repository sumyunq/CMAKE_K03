#ifndef UPDATEOTA_H
#define UPDATEOTA_H

/********************* OTA更新烧录弹窗 *********************/

#include <QDialog>
#include <QTimer>

enum UpdateState {
    State_Idle,   //无
    State_WaitDevice,//等待2.4G重新连接
    State_BurnFirmware,//烧录固件
    State_TP1_UpdateDone,//TP1烧录完成
    State_WaitAfterBurn,//烧录完成后，等待设备重连，该功能没用
    State_Done,
    State_Failed
};



namespace Ui {
class UpdateOTA;
}

class UpdateOTA : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateOTA(QWidget *parent = nullptr);
    ~UpdateOTA();

    void runOTAFile(const QString &filePath,int type,QString ver);

    void StartDownload();


    void ShowProgress(int val);
    void StartUpdateBin();


    void Ceshi();

    void updateSuccess();

    void setTheme_UpdateOTA(int idx);

    void SetTitleAndVer(bool titleType,QString ver);

    void setProgressTxt(QString txt);//设置进度前的文本类型（0：升级中，1：回退中，3：下载中）

private:
    Ui::UpdateOTA *ui;

    QString ProgressTxt;//进度前的文本类型

    QTimer *timer_update;

    UpdateState m_state = State_Idle;
    int m_retryCount = 0;          //耳机升级失败，重新尝试次数

    void setState(UpdateState newState);
    void waitForDeviceReady();
    void decideNextBurn();
    void startBurn();
    void onWaitAfterBurn();
    void finishUpdate();
    void handleFailure();


private slots:
    void Timer_UpdateShow();
    void SetLabelTxt(int type);


signals:
    void GetOTAFile_U(int type);
    void UpdateNewOTAFile_U();//更新3.0版本
    void sigUpdateLabelText(int type);  // 请求更新标签文字
    void sigCloseWindow();  // 请求关闭窗口
    void SetFailed_U();
    void SetSuccess_U();
};

#endif // UPDATEOTA_H
