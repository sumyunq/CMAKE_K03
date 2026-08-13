#include "Popup/Plans/UploadPlanSuccess.h"
#include "ui_UploadPlanSuccess.h"
#include <QGraphicsDropShadowEffect>

UploadPlanSuccess::UploadPlanSuccess(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UploadPlanSuccess)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 20px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    ui->frame->setGraphicsEffect(shadow);


    ui->pBt_ok->setDefault(true);//将该按钮设置为对话框的默认按钮（按回车触发）,QDialog 或基于 QDialog 的窗口才可以此实现
    connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::reject);
}

UploadPlanSuccess::~UploadPlanSuccess()
{
    delete ui;
}
//显示今天已经上传的方案数量
void UploadPlanSuccess::ShowUploadPlanCnt(int cnt)
{
    ui->lab_cnt->setText(QString("（%1/10）").arg(cnt));
}
//根据主题设置样式
 void UploadPlanSuccess::setTheme_UploadPlanSuccess(int idx)
{
     QString textColor,colorStr,bkcolor,textColor2,bkcolor2;

    //背景颜色
    switch (idx) {
    case 0: {colorStr = "#10151D"; break;}   // 深蓝色
    case 1: {colorStr = "#10151D"; break;}   // 白色
    case 2: {colorStr = "#10151D"; break;}   // 黑色
    default: {colorStr = "#10151D"; break;}
    }
    ui->frame->setStyleSheet(QString("border-radius: 16px;background-color: %1;").arg(colorStr));

     //按钮图片
     QString suffix;
     switch (idx)
     {
     case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
     case 1: suffix = "_white";  break;//白色
     case 2: suffix = "_black";  break;//黑色
     default: suffix = "";      break;
     }
     //成功图标
     ui->lab_ico->setStyleSheet(QString("border-image: url(:/Skin/Images/Popup/UploadSuccess%1.png);").arg(suffix));
     //"我知道了"按钮
     switch (idx)
     {
     case 0: {textColor = "#FFFFFF"; colorStr = "#009FEF"; break;}   // 深蓝色
     case 1: {textColor = "#FFFFFF"; colorStr = "#009FEF"; break;}   // 白色
     case 2: {textColor = "#FFFFFF"; colorStr = "#009FEF"; break;}   // 黑色
     default: {textColor = "#FFFFFF"; colorStr = "#009FEF"; break;}
     }
     ui->pBt_ok->setStyleSheet(
         QString("QPushButton{"
                 "color: %1;"
                 "border:null;"
                 "background: %2;"
                 "border-radius: 15px;"
                 "}")
             .arg(textColor).arg(colorStr)
         );
     //字体
     //标题
     switch (idx)
     {
     case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
     case 1: {textColor = "#A1A8B3"; break;}   // 白色
     case 2: {textColor = "#A1A8B3"; break;}   // 黑色
     default: {textColor = "#A1A8B3"; break;}
     }
     ui->lab_title->setStyleSheet(
         QString("font-family: \"Noto Sans S Chinese\"; "
                 "font-weight: 500;"
                 "font-size: 16px;"
                 "color: %1;"
                 "background:transparent;")
             .arg(textColor)
         );
     //数量
     switch (idx)
     {
     case 0: {textColor = "rgba(161, 168, 179, 0.5)"; break;}   // 深蓝色
     case 1: {textColor = "rgba(161, 168, 179, 0.5)"; break;}   // 白色
     case 2: {textColor = "rgba(161, 168, 179, 0.5)"; break;}   // 黑色
     default: {textColor = "rgba(161, 168, 179, 0.5)"; break;}
     }
     ui->lab_cnt->setStyleSheet(
         QString("font-family: \"Noto Sans S Chinese\"; "
                 "font-weight: 500;"
                 "font-size: 14px;"
                 "color: %1;"
                 "background:transparent;")
             .arg(textColor)
         );
}
