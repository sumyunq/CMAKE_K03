#include "Popup/Update/UpdateOTAFind.h"
#include "ui_UpdateOTAFind.h"
#include <QGraphicsDropShadowEffect>
#include <QTimer>

UpdateOTAFind::UpdateOTAFind(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateOTAFind)
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


    m_countdownTimer = new QTimer(this);
    connect(m_countdownTimer, &QTimer::timeout, this, [this]() {
        m_countdownValue--;
        if (m_countdownValue > 0) {
            ui->pBt_ok->setText(QString("%1s").arg(m_countdownValue));
        } else {
            m_countdownTimer->stop();
            ui->pBt_ok->setText(BtnTxt);
            ui->pBt_ok->setEnabled(true);  // 倒计时结束后可点击
        }
    });


    connect(ui->pBt_close,&QPushButton::clicked,this,&QDialog::reject);//取消
    connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::accept);//升级
}

UpdateOTAFind::~UpdateOTAFind()
{
    delete ui;
}

void UpdateOTAFind::updateTitle(int themeidx,int type, QString version)
{
    QString textColor,textColor2;
    switch (themeidx) {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }
    switch (themeidx) {
    case 0: {textColor2 = "#009FEF"; break;}   // 深蓝色
    case 1: {textColor2 = "#009FEF"; break;}   // 白色
    case 2: {textColor2 = "#009FEF"; break;}   // 黑色
    default: {textColor2 = "#009FEF"; break;}
    }
    ui->lab_title->setTextFormat(Qt::RichText);

    //根据主题变化，获得主题全局变量
    if(type == 0)
    {
        BtnTxt = "升级";
        //ota升级
        ui->lab_title->setText(
            tr(
                "<span style='color:%1;'>是否将耳机固件升级到</span>"
                "<span style='color:%2;'>%3</span>"
                "<span style='color:%1;'>版本</span>")
                .arg(textColor).arg(textColor2).arg(version)
            );
    }else if(type == 1)
    {
        BtnTxt = "回退";
        //ota回退
        ui->lab_title->setText(
            tr(
                "<span style='color:#A1A8B3;'>是否将耳机固件回退到</span>"
                "<span style='color:#009FEF;'>%1</span>"
                "<span style='color:#A1A8B3;'>版本</span>")
                .arg(version)
            );
    }
}
//根据主题设置样式
void UpdateOTAFind::setTheme_UpdateOTAFind(int idx)
{
    QString textColor,colorStr;

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
    ui->pBt_close->setStyleSheet(
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
    ui->pBt_ok->setStyleSheet(
        QString("QPushButton{"
                "font-family: \"Noto Sans S Chinese\";"
                " font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;"
                "border-image: url(:/Skin/Images/Popup/confirm-no%2.png);"
                "}"
                "QPushButton:hover{"
                "border-image: url(:/Skin/Images/Popup/confirm-ho%2.png);"
                "}"
                "QPushButton:disabled{"
                "border-image: url(:/Skin/Images/Popup/confirm-ho%2.png);"
                "}")
            .arg(textColor).arg(suffix)
        );

    // switch (idx) {
    // case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    // case 1: {textColor = "#A1A8B3"; break;}   // 白色
    // case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    // default: {textColor = "#A1A8B3"; break;}
    // }
    // ui->lab_title->setStyleSheet(
    //     QString("color: %1;"
    //             "font-family: \"Noto Sans S Chinese\";"
    //             "font-weight:500;"
    //             "font-size: 14px;"
    //             "border-image:none;")
    //         .arg(textColor)
    //     );

    ui->lab_title->setStyleSheet(
        QString(
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight:500;"
                "font-size: 14px;"
                "border-image:none;")
        );
}

void UpdateOTAFind::startTimer()
{
    // 初始化倒计时
    m_countdownValue = 4;
    ui->pBt_ok->setText("4s");
    ui->pBt_ok->setEnabled(false);  // 倒计时期间不可点击
    m_countdownTimer->start(1000);  // 每秒触发一次
}
