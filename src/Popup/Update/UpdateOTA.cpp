#include "Popup/Update/UpdateOTA.h"
#include "ui_UpdateOTA.h"

#include "cchineseconvertor.h"
#include "mainwindow.h"
#include <tchar.h>
#include <QThread>
#include "LoadLib.h"
QString m_csUpdateFWPath = NULL;;
QString lastVer = NULL;
bool updateFW = 0;
int rupdateCnt = 0;//耳机升级失败，重新尝试次数

UpdateOTA::UpdateOTA(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateOTA)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    ui->frame->setGraphicsEffect(shadow);

    ui->lab_ver->hide();

    timer_update = new QTimer(this);
    connect(timer_update,SIGNAL(timeout()),this,SLOT(Timer_UpdateShow()));

    // 连接信号到槽
    connect(this, &UpdateOTA::sigUpdateLabelText, this, &UpdateOTA::SetLabelTxt);
    // 连接关闭请求信号到 QWidget 自带的 close 槽
    // connect(this, &UpdateOTA::sigCloseWindow, this, &QWidget::close);
    connect(this, &UpdateOTA::sigCloseWindow, this,[this]{
        emit SetSuccess_U();
        this->close();
    });
    //isHidRun

}

UpdateOTA::~UpdateOTA()
{
    delete ui;
}

DWORD WINAPI DownFWThread(LPVOID lpvThreadParam)
{
    UpdateOTA* pdlg = (UpdateOTA*)lpvThreadParam;
    pdlg->StartUpdateBin();

    return 1;
}
/*void UpdateOTA::StartUpdateBin()
{

    UpdateEn = TRUE;

    CChineseConvertor con;
    int ret = lolib->pfn_ActHID_DownFW(con.ToMultiByte(reinterpret_cast<LPCTSTR>(m_csUpdateFWPath.utf16()) , CP_ACP) , iDevIndex);//设备升级

    UpdateEn = FALSE;
    if (ret == 1) {
        qDebug("HUB Update finish success");
        if (QFile::remove(m_csUpdateFWPath)) {
            qDebug() << "文件删除成功:" << m_csUpdateFWPath;
        } else {
            qDebug() << "文件删除失败:" << m_csUpdateFWPath;
        }
        //若版本比TP1低，则先升级为TP1(先升dongle(TX 0)再升耳机(RX 1)，再升级为3.0(先升耳机，再升升dongle)
        if(iDevIndex == 0 && UpdateTP1)
        {
            timer_update->stop();
            emit sigUpdateLabelText(0);
            //判断是否链接上，并一直判断，直到提升链接上
            memset(DongleVer,0,sizeof(DongleVer));
            memset(EarVer,0,sizeof(EarVer));

        agin:       int res = lolib->GetVersion(DongleVer,EarVer);
            if(res < 0)
            {
                qDebug("错误，获取设备版本信息失败");
                // 设备还没链接上
                QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                goto agin;
            }else{
                qDebug() << "Dongle版本" << QString::fromLocal8Bit(DongleVer, 30);
                qDebug() << "耳机版本" << QString::fromLocal8Bit(EarVer, 30);
                // if (EarVer[0] == '\0' || DongleVer[0] == '\0') {
                if (DongleVer[0] == '\0') {
                    // 字符串为空
                    QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                    goto agin;
                }else
                {
                    if(EarVer[0] == '\0')
                    {
                        int res = -1;
                        DevStatus sta;
                        res = lolib->GetDevStatus(sta);
                        if (res < 0 || res == 0)
                        {
                            //若获得dongle版本后，没有获得耳机状态信息，存在两种情况，一种是2.4G没有链接，一种是耳机旧版本协议不接收HID，所以要直接更新，更新失败则重新更新
                            qDebug("获取耳机状态信息失败");
                            QThread::msleep(40000);  // 当前线程休眠 40 秒，一般情况下30s就能链接上了
                            // goto agin;
                        }else if(res > 0)
                        {
                            //耳机连接状态
                            if(sta.ConnectSta == 0)
                            {
                                qDebug("耳机未连接");
                                QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                                goto agin;

                            }

                        }
                    }

                }
            }
            qDebug("获取设备版本信息成功");
            // 更新耳机
            QMetaObject::invokeMethod(this, [this]() {
                // 这段代码在主线程中执行，但立即返回，不会阻塞
                QTimer::singleShot(8000, this, [this]() {
                    UpdateTxDone = true;
                    iDevIndex = 1;
                    timer_update->start();
                    emit GetOTAFile_U(1);
                });
            }, Qt::QueuedConnection);
        }else if(iDevIndex == 1 && UpdateTP1)
        {
            //TP1更新完成，更新3.0最新版(先升级rx，在升级tx)
            UpdateTP1 = false;
            UpdateTP1En = false;
            UpdateRxDone = false;
            UpdateTxDone = false;
            //更新3.0及以上
            timer_update->stop();
            emit sigUpdateLabelText(0);
            //判断是否链接上，并一直判断，直到提升链接上
            memset(DongleVer,0,sizeof(DongleVer));
            memset(EarVer,0,sizeof(EarVer));

        agin3:       int res = lolib->GetVersion(DongleVer,EarVer);
            if(res < 0)
            {
                qDebug("错误，获取设备版本信息失败");
                // 设备还没链接上
                QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                goto agin3;
            }else{
                qDebug() << "Dongle版本" << QString::fromLocal8Bit(DongleVer, 30);
                qDebug() << "耳机版本" << QString::fromLocal8Bit(EarVer, 30);
                // if (EarVer[0] == '\0' || DongleVer[0] == '\0') {
                if (DongleVer[0] == '\0') {
                    // 字符串为空
                    QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                    goto agin3;
                }else
                {
                    if(EarVer[0] == '\0')
                    {
                        int res = -1;
                        DevStatus sta;
                        res = lolib->GetDevStatus(sta);
                        if (res < 0 || res == 0)
                        {
                            //若获得dongle版本后，没有获得耳机状态信息，存在两种情况，一种是2.4G没有链接，一种是耳机旧版本协议不接收HID，所以要直接更新，更新失败则重新更新
                            qDebug("获取耳机状态信息失败");
                            QThread::msleep(40000);  // 当前线程休眠 40 秒，一般情况下30s就能链接上了
                            // goto agin3;
                        }else if(res > 0)
                        {
                            //耳机连接状态
                            if(sta.ConnectSta == 0)
                            {
                                qDebug("耳机未连接");
                                QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                                goto agin3;

                            }

                        }
                    }

                }
            }
            qDebug("获取设备版本信息成功");
            // 更新dongle
            QMetaObject::invokeMethod(this, [this]() {
                // 这段代码在主线程中执行，但立即返回，不会阻塞
                QTimer::singleShot(8000, this, [this]() {
                    iDevIndex = 1;
                    timer_update->start();
                    // emit GetOTAFile_U(1);
                    emit UpdateNewOTAFile_U();
                });
            }, Qt::QueuedConnection);


        }else if(iDevIndex == 1 && !UpdateTP1)
        {
            //更新3.0及以上的耳机后，更新dongle
            timer_update->stop();
            emit sigUpdateLabelText(0);
            //判断是否链接上，并一直判断，直到提升链接上
            memset(DongleVer,0,sizeof(DongleVer));
            memset(EarVer,0,sizeof(EarVer));

        agin2:       int res = lolib->GetVersion(DongleVer,EarVer);
            if(res < 0)
            {
                qDebug("错误，获取设备版本信息失败");
                // 设备还没链接上
                QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                goto agin2;
            }else{
                qDebug() << "Dongle版本" << QString::fromLocal8Bit(DongleVer, 30);
                qDebug() << "耳机版本" << QString::fromLocal8Bit(EarVer, 30);
                // if (EarVer[0] == '\0' || DongleVer[0] == '\0') {
                if (DongleVer[0] == '\0') {
                    // 字符串为空
                    QThread::msleep(10000);  // 当前线程休眠 10 秒，不阻塞其他线程
                    goto agin2;
                }
            }
            qDebug("获取设备版本信息成功");
            // 更新dongle
            QMetaObject::invokeMethod(this, [this]() {
                // 这段代码在主线程中执行，但立即返回，不会阻塞
                QTimer::singleShot(8000, this, [this]() {
                    UpdateRxDone = true;
                    iDevIndex = 0;
                    timer_update->start();
                    emit GetOTAFile_U(0);
                });
            }, Qt::QueuedConnection);
        }else
        {
            updateFW = 1;
            emit sigUpdateLabelText(1);
            emit sigCloseWindow();  // ✅ 发射信号，由主线程关闭
        }

    }else
    {
        //更新耳机失败，可能是2.4G未链接，重新执行一遍
        if(rupdateCnt < 20)
        {
            if(iDevIndex == 1)
            {
                rupdateCnt++;
                goto agin;
            }
        }
        qDebug("HUB Update finish failed");
        if (QFile::remove(m_csUpdateFWPath)) {
            qDebug() << "文件删除成功:" << m_csUpdateFWPath;
        } else {
            qDebug() << "文件删除失败:" << m_csUpdateFWPath;
        }
        emit SetFailed_U();

    }
    // ui->pButton_update->setEnabled(true);

}*/


void UpdateOTA::StartUpdateBin()
{
    UpdateEn = TRUE;
    m_retryCount = 0;                 // 重连计数清零

    CChineseConvertor con;
    int ret = lolib->pfn_ActHID_DownFW(
        con.ToMultiByte(reinterpret_cast<LPCTSTR>(m_csUpdateFWPath.utf16()), CP_ACP),
        iDevIndex
        );

    if (ret == 1) {
        qDebug("HUB Update finish success");
        QFile::remove(m_csUpdateFWPath);   // 升级成功，删除文件

        // 等待设备重连，并根据状态判断是否接着升级
        setState(State_WaitDevice);
    } else {
        // 升级失败处理
        if(m_retryCount < 20)
        {
            if(iDevIndex == 1)
            {
                m_retryCount++;
                iDevIndex = 0;
                setState(State_WaitDevice);
            }
        }else
        {
            m_retryCount = 0;
            qDebug("HUB Update finish failed");
            QFile::remove(m_csUpdateFWPath);    //升级失败，删除文件
            emit SetFailed_U();
            UpdateEn = FALSE;
        }


    }
}

void UpdateOTA::setState(UpdateState newState)
{
    m_state = newState;
    switch (m_state) {
    case State_WaitDevice:
        waitForDeviceReady();
        break;
    case State_BurnFirmware:
        startBurn();
        break;
    case State_WaitAfterBurn:
        QTimer::singleShot(8000, this, &UpdateOTA::onWaitAfterBurn);
        break;
    case State_Done:
        m_state = State_Idle;
        finishUpdate();
        break;
    case State_Failed:
        m_state = State_Idle;
        handleFailure();
        break;
    case State_TP1_UpdateDone:
    {
        timer_update->start();
        emit UpdateNewOTAFile_U();
        break;
    }
    default:
        break;
    }
}
//等待设备2.4G链接上
void UpdateOTA::waitForDeviceReady()
{
    qDebug("进入waitForDeviceReady\n");
    if (iDevIndex == 0 && !UpdateTP1)
    {
        updateFW = 1;
        emit sigUpdateLabelText(1);
        // 全部完成
        setState(State_Done);
        return;
    }

    timer_update->stop();
    emit sigUpdateLabelText(0);
    memset(DongleVer, 0, sizeof(DongleVer));
    memset(EarVer, 0, sizeof(EarVer));

    int res = lolib->GetVersion(DongleVer, EarVer);
    if (res < 0 || DongleVer[0] == '\0') {
        // 设备未就绪，重试
        if (++m_retryCount > 100) {   // 最多重试100次,100*10s，16分钟多
            emit SetFailed_U();
            UpdateEn = FALSE;
            setState(State_Failed);
            return;
        }
        QThread::msleep(10000);
        setState(State_WaitDevice);
        //QTimer::singleShot(10000, this, &UpdateOTA::waitForDeviceReady);// 当前线程休眠 10 秒，不阻塞其他线程
        return;
    }

    if(DongleVer[0] != '\0' )
    {
        if((iDevIndex == 1 && !UpdateTP1))
        {

        }else if (EarVer[0] == '\0') {// 检查耳机版本是否为空（需要额外等待）？
            // 尝试获取设备状态
            DevStatus sta;
            int statusRes = lolib->GetDevStatus(sta);


            if (statusRes <= 0)
            {
                //若获得dongle版本后，没有获得耳机状态信息，存在两种情况，一种是2.4G没有链接，一种是耳机旧版本协议不接收HID，所以要直接更新，更新失败则重新更新
                qDebug("获取耳机状态信息失败");
                QThread::msleep(40000);  // 当前线程休眠 40 秒，一般情况下30s就能链接上了
            }else if(res > 0)
            {
                //耳机连接状态
                if(sta.ConnectSta == 0)
                {
                    qDebug("耳机未连接");
                    QThread::msleep(10000);
                    setState(State_WaitDevice);
                    // QTimer::singleShot(10000, this, &UpdateOTA::waitForDeviceReady);// 当前线程休眠 10 秒，不阻塞其他线程
                    return;
                }

            }
        }

    }





    // 设备已完全就绪，决定下一步升级动作
    decideNextBurn();
}
//进行下一步烧录
void UpdateOTA::decideNextBurn()
{
    //若版本比TP1低，则先升级为TP1(先升dongle(TX 0)再升耳机(RX 1)，再升级为3.0(先升耳机，再升升dongle)
    // UpdateTP1，true代表上一个程序的烧录的TP1版本
    if (UpdateTP1) {
        // TP1 模式
        if (iDevIndex == 0) {
            // 刚升级完 Dongle（TX=0），接下来升 Ear（RX=1）
            iDevIndex = 1;
            setState(State_BurnFirmware);
        } else if (iDevIndex == 1) {
            // 刚升级完 Ear，升级TP1 完成，转 3.0 升级（先 Ear 后 Dongle）
            UpdateTP1 = false;
            UpdateTP1En = false;
            UpdateTxDone = false;
            UpdateRxDone = false;
            iDevIndex = 1;   // 先升耳机
            setState(State_TP1_UpdateDone);
        }
    } else {
        // 非 TP1 模式（3.0 及以上）
        if (iDevIndex == 1) {
            // 刚升级完 Ear，接下来升 Dongle
            iDevIndex = 0;
            setState(State_BurnFirmware);
        } else if (iDevIndex == 0) {
            // 全部完成()
            setState(State_Done);
        }
    }
}
//开始烧录
void UpdateOTA::startBurn()
{
    // 根据 iDevIndex 决定发送哪个信号
    // 启动定时器
    timer_update->start();
    if (UpdateTP1) {
        if (iDevIndex == 1) {
            // 需要升级耳机（RX=1）
            UpdateTxDone = true;
            emit GetOTAFile_U(1);
        } else {
            //目前逻辑，不会执行
            UpdateRxDone = true;
            emit GetOTAFile_U(0);
        }
    } else {
        if (iDevIndex == 1) {
            //目前逻辑，不会执行
            UpdateTxDone = true;
            emit GetOTAFile_U(1);
        } else {
            UpdateRxDone = true;
            emit GetOTAFile_U(0);
        }
    }

}
void UpdateOTA::onWaitAfterBurn()
{
    // 烧录完成后，等待设备重连，再次进入等待状态
    setState(State_WaitDevice);
}

void UpdateOTA::finishUpdate()
{
    updateFW = 1;
    emit sigUpdateLabelText(1);
    emit sigCloseWindow();
    UpdateEn = FALSE;
}

void UpdateOTA::handleFailure()
{
    UpdateEn = FALSE;
    emit SetFailed_U();
    // 可在此处添加额外清理
}



void UpdateOTA::Ceshi()
{
    emit GetOTAFile_U(0);
}
//ota更新
void UpdateOTA::runOTAFile(const QString &filePath,int type,QString ver)
{
    // iDevIndex = type;
    m_csUpdateFWPath = filePath;
    updateFW = 0;
    // lastVer = ver;

    qDebug("OTA升级 iDevIndex:%d\n",iDevIndex);
    qDebug()<<"OTA升级 路径"<<m_csUpdateFWPath;

    timer_update->start(1000); // // 1秒触发一次,将内存中的ANSI编码(CP_ACP)字符串转为Unicode‌并通过进度条控件界面显示

    DWORD dwThreadId;
    HANDLE hThread = CreateThread(NULL, 0, DownFWThread, (LPVOID)this, 0, &dwThreadId);
}


//更新进度条
void UpdateOTA::Timer_UpdateShow()
{
    CChineseConvertor con;
    TCHAR* pszLog = con.ToUnicode((char*)m_pMem , CP_ACP);

    int curProgress = _tstof(pszLog);
    qDebug("qt %d 百分\n",curProgress);
    ShowProgress(curProgress);
    if(updateFW)
    {
        timer_update->stop();
    }
}

void UpdateOTA::ShowProgress(int val)
{

    ui->label_val->setText(tr("%1：%2%").arg(ProgressTxt).arg(val));
    ui->progressBar->setValue(val);
}
//设置固件升级
void UpdateOTA::SetLabelTxt(int type)
{
    switch(type)
    {
    case 0:
        ui->label_val->setText(tr("等待，更新完成后，需等待设备连接，还未更新完毕"));
        break;
    case 1:
        ui->label_val->setText(tr("更新成功"));
        break;
    case 2:
        ui->label_val->setText(tr("更新失败"));
        break;
    default:
        break;

    }
}
//设置进度前的文本类型（升级中，回退中，下载中）
void UpdateOTA::setProgressTxt(QString txt)
{
    ProgressTxt = txt;
}
void UpdateOTA::updateSuccess()
{
    updateFW = 1;
    emit sigUpdateLabelText(1);
    emit sigCloseWindow();  // ✅ 发射信号，由主线程关闭
}
//设置标题，版本号
void UpdateOTA::SetTitleAndVer(bool titleType,QString ver)
{
    if(titleType)
    {
        ui->lab_title->setText("固件回退");
    }else
    {
        ui->lab_title->setText("固件升级");
    }
    // ui->lab_ver->setText("V"+ver);
}


//根据主题设置样式
void UpdateOTA::setTheme_UpdateOTA(int idx)
{
    QString textColor,colorStr,colorStr2;
    switch (idx) {
    case 0: {colorStr = "#10151D"; break;}   // 深蓝色
    case 1: {colorStr = "#10151D"; break;}   // 白色
    case 2: {colorStr = "#10151D"; break;}   // 黑色
    default: {colorStr = "#10151D"; break;}
    }
    ui->frame->setStyleSheet(QString("border-radius: 16px;background-color: %1;").arg(colorStr));

    switch (idx) {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }
    ui->lab_title->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight:500;"
                "font-size: 16px;")
            .arg(textColor)
        );

    switch (idx) {
    case 0: {textColor = "#454D57"; break;}   // 深蓝色
    case 1: {textColor = "#454D57"; break;}   // 白色
    case 2: {textColor = "#454D57"; break;}   // 黑色
    default: {textColor = "#454D57"; break;}
    }
    ui->lab_ver->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-size: 12px;")
            .arg(textColor)
        );

    switch (idx) {
    case 0: {textColor = "#616871"; break;}   // 深蓝色
    case 1: {textColor = "#616871"; break;}   // 白色
    case 2: {textColor = "#616871"; break;}   // 黑色
    default: {textColor = "#616871"; break;}
    }
    ui->label_val->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-size: 10px;")
            .arg(textColor)
        );

    //进度条
    switch (idx) {
    case 0:// 深蓝色
    {
        colorStr = "rgba(0, 0, 0, 0.4)";
        colorStr2 = "#009FEF";
        break;
    }
    case 1:// 白色
    {
        colorStr = "rgba(0, 0, 0, 0.4)";
        colorStr2 = "#009FEF";
        break;
    }
    case 2:// 黑色
    {
        colorStr = "rgba(0, 0, 0, 0.4)";
        colorStr2 = "#009FEF";
        break;
    }
    default:
    {
        colorStr = "rgba(0, 0, 0, 0.4)";
        colorStr2 = "#009FEF";
        break;
    }
    }
    ui->progressBar->setStyleSheet(
        QString("QSlider{"
                "background: none;"
                "border: none;"
                "}"
                /*horizontal ：水平QSlider*/
                "QSlider::groove:horizontal {"
                "height:8px;"
                "border-bottom:4px;"
                "background-color: %1;"
                "}"
                /* 滑块左侧（已填充范围） */
                "QSlider::sub-page:horizontal {"
                "background-color: %2;"
                "border-radius: 4px;   "
                "}"
                /*3.平时滑动的滑块设计参数*/
                "QSlider::handle:horizontal {"
                /*滑块的宽度*/
                "width: 0px;"
                "}"

                /*4.手动拉动时显示的滑块设计参数*/
                "QSlider::handle:horizontal:hover {"
                /*滑块的宽度*/
                "width:0px;"
                "}")
        .arg(colorStr).arg(colorStr2)
        );
}
