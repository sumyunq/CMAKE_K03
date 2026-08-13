#include "Popup/Update/UpdateOTASuccess.h"
#include "ui_UpdateOTASuccess.h"
#include <QGraphicsDropShadowEffect>

UpdateOTASuccess::UpdateOTASuccess(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateOTASuccess)
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

UpdateOTASuccess::~UpdateOTASuccess()
{
    delete ui;
}
void UpdateOTASuccess::UpdateTitle(int type)
{
    if(type == 0)
    {
        ui->lab_title->setText(tr("固件升级成功"));
    }else if(type == 1)
    {
        ui->lab_title->setText(tr("固件回退成功"));
    }
}
//根据主题设置样式
void UpdateOTASuccess::setTheme_UpdateOTASuccess(int idx)
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
}

