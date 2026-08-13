#include "Popup/Update/UpdateVersion.h"
#include "ui_UpdateVersion.h"
#include <QGraphicsDropShadowEffect>

UpdateVersion::UpdateVersion(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateVersion)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    ui->frame->setGraphicsEffect(shadow);


    ui->pBt_ok->setDefault(true);//将该按钮设置为对话框的默认按钮（按回车触发）,QDialog 或基于 QDialog 的窗口才可以此实现
    connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::reject);
}

UpdateVersion::~UpdateVersion()
{
    delete ui;
}
void UpdateVersion::UpdateVer(QString ver)
{
    ver = "V"+ ver;
    ui->lab_ver->setText(ver);
}
void UpdateVersion::UpdateTitle(int type)
{
    if(type == 0)
    {
        ui->lab_ver->show();
        //上位机驱动升级
        ui->lab_title->setText(tr("当前已是最新版本"));
    }else if(type == 1)
    {
        ui->lab_ver->show();
        //OTA升级
        ui->lab_title->setText(tr("当前固件已是最新版本"));
    }else if(type == 2)
    {
        ui->lab_ver->hide();
        //OTA升级
        ui->lab_title->setText(tr("不存在"));
    }else if(type == 3)
    {
        ui->lab_ver->show();
        //OTA升级
        ui->lab_title->setText(tr("当前固件已是回退版本"));
    }else if(type == 4)
    {
        ui->lab_ver->hide();
        //OTA升级
        ui->lab_title->setText(tr("无可回退固件"));
    }
}
//根据主题设置样式
void UpdateVersion::setTheme_UpdateVersion(int idx)
{

    QString textColor,colorStr;
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
    case 0: {textColor = "#616871"; break;}   // 深蓝色
    case 1: {textColor = "#616871"; break;}   // 白色
    case 2: {textColor = "#616871"; break;}   // 黑色
    default: {textColor = "#616871"; break;}
    }
    ui->lab_ver->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-size: 12px;")
            .arg(textColor)
        );
}







