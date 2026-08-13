#include "Popup/Update/UpdateSoftWareFind.h"
#include "ui_UpdateSoftWareFind.h"
#include <QGraphicsDropShadowEffect>
UpdateSoftWareFind::UpdateSoftWareFind(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateSoftWareFind)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    ui->frame->setGraphicsEffect(shadow);

    connect(ui->pBt_exit,&QPushButton::clicked,this,&QDialog::reject);//关闭
    connect(ui->pBt_Ignore,&QPushButton::clicked,this,&QDialog::reject);//忽略
    // connect(ui->pBt_update,&QPushButton::clicked,this,&QDialog::accept);//更新
    connect(ui->pBt_update,&QPushButton::clicked,this,&UpdateSoftWareFind::startUpdate);//更新
}

UpdateSoftWareFind::~UpdateSoftWareFind()
{
    delete ui;
}
//更新 新版本说明
void UpdateSoftWareFind::UpdateExplanation(QString txt)
{
    ui->tEdit_explanation->setText(txt);
}
//更新版本号
void UpdateSoftWareFind::setVersion(QString version)
{
    ui->lab_ver->setText("V"+version);
}
//设置进度
void UpdateSoftWareFind::ShowProgress(int val)
{
    ui->progressBar->setValue(val);
    //创建带动态值的文本
    QString text = QString("正在下载：%1%").arg(val);
    //更新标签文本
    ui->label_val->setText(text);
}
//显示的页面（按钮、进度条）
void UpdateSoftWareFind::setShowPage(int idx)
{
    ui->stackedWidget->setCurrentIndex(idx);
}

//根据主题设置样式
void UpdateSoftWareFind::setTheme_UpdateSoftWareFind(int idx)
{
    QString textColor,colorStr,colorStr2;;
    //按钮图片
    QString suffix;
    switch (idx)
    {
    case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
    case 1: suffix = "_white";  break;//白色
    case 2: suffix = "_black";  break;//黑色
    default: suffix = "";      break;
    }
    ui->frame->setStyleSheet(
        QString("border-radius: 16px;"
                "background:transparent;"
                "border-image: url(:/Skin/Images/Popup/update-bk%1.png);")
            .arg(suffix)
        );

    //右上角关闭按钮
    ui->pBt_exit->setStyleSheet(
        QString("QPushButton{"
                "	border-radius:0px;"
                "	border-image: url(:/Skin/Images/Popup/close-no%1.png);"
                "	background:transparent;"
                "}"
                "QPushButton:hover{"
                "border-image: url(:/Skin/Images/Popup/close-ho%1.png);"
                "}")
            .arg(suffix)
        );

    switch (idx) {
    case 0: {textColor = "#009FEF"; break;}   // 深蓝色
    case 1: {textColor = "#009FEF"; break;}   // 白色
    case 2: {textColor = "#009FEF"; break;}   // 黑色
    default: {textColor = "#009FEF"; break;}
    }
    ui->pBt_Ignore->setStyleSheet(
        QString("QPushButton{"
                "font-family: \"Noto Sans S Chinese\";"
                " font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;"
                "border-image: url(:/Skin/Images/Popup/cancel-no%2.png);"
                "}"
                "QPushButton:hover{"
                "border-image: url(:/Skin/Images/Popup/cancel-ho%2.png);}")
            .arg(textColor).arg(suffix)
        );

    switch (idx) {
    case 0: {textColor = "#FFFFFF"; break;}   // 深蓝色
    case 1: {textColor = "#FFFFFF"; break;}   // 白色
    case 2: {textColor = "#FFFFFF"; break;}   // 黑色
    default: {textColor = "#FFFFFF"; break;}
    }
    ui->pBt_update->setStyleSheet(
        QString("QPushButton{"
                "font-family: \"Noto Sans S Chinese\";"
                " font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;"
                "border-image: url(:/Skin/Images/Popup/confirm-no%2.png);"
                "}"
                "QPushButton:hover{"
                "border-image: url(:/Skin/Images/Popup/confirm-ho%2.png);}")
            .arg(textColor).arg(suffix)
        );

    //字体
    switch (idx) {
    case 0: {textColor = "#009FEF"; break;}   // 深蓝色
    case 1: {textColor = "#009FEF"; break;}   // 白色
    case 2: {textColor = "#009FEF"; break;}   // 黑色
    default: {textColor = "#009FEF"; break;}
    }
    ui->lab_title->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight:500;"
                "font-size: 16px;"
                "border-image:none;")
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
                "font-weight:500;"
                "font-size: 12px;"
                "border-image:none;")
            .arg(textColor)
        );

    switch (idx) {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }
    ui->lab_ver->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight:500;"
                "font-size: 14px;"
                "border-image:none;")
            .arg(textColor)
        );




    switch (idx) {
    case 0: {textColor = "#616975"; break;}   // 深蓝色
    case 1: {textColor = "#616975"; break;}   // 白色
    case 2: {textColor = "#616975"; break;}   // 黑色
    default: {textColor = "#616975"; break;}
    }
    ui->lab_ver->setStyleSheet(
        QString("QTextEdit {"
                "	border:none;"
                "border-image:none;"
                "	background:transparent;"
                "	font-family: \"Noto Sans S Chinese\";"
                "	font-weight: 500;"
                "	font-size: 12px;"
                "	color: %1;"
                "}"

                "QScrollBar:vertical {"
                "    background: transparent;"
                "    border: none;     "
                "border-image:none;"
                "    width: 6px;"
                "    margin: 0px;"
                "    padding: 0px;"
                "}"

                "QScrollBar::handle:vertical {"
                "    background-color: rgba(255, 255, 255, 0.1); "
                "    border: none;            "
                "border-image:none;      "
                "    border-radius: 3px;"
                "    min-height: 30px;"
                "}"

                "QScrollBar::add-line:vertical,"
                "QScrollBar::add-page:vertical,"
                "QScrollBar::sub-page:vertical {"
                "    background: none;"
                "    border: none;"
                "border-image:none;"
                "}")
            .arg(textColor)
        );

    //进度数值
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
