#include "Popup/FactoryReset.h"
#include "ui_FactoryReset.h"
#include <QGraphicsDropShadowEffect>
#include <QTimer>

FactoryReset::FactoryReset(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FactoryReset)
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
            ui->pBt_reset->setText(QString("%1s").arg(m_countdownValue));
        } else {
            m_countdownTimer->stop();
            ui->pBt_reset->setText(tr("重置"));
            ui->pBt_reset->setEnabled(true);  // 倒计时结束后可点击
        }
    });


    connect(ui->pBt_exit,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_Cancel,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_reset,&QPushButton::clicked,this,&QDialog::accept);
}

FactoryReset::~FactoryReset()
{
    delete ui;
}
//开启4s倒计时，倒计时结束，用户才可点击重置按钮
void FactoryReset::startTimer()
{
    // 初始化倒计时
    m_countdownValue = 4;
    ui->pBt_reset->setText("4s");
    ui->pBt_reset->setEnabled(false);  // 倒计时期间不可点击
    m_countdownTimer->start(1000);  // 每秒触发一次
}

//根据主题设置样式
void FactoryReset::setTheme_FactoryReset(int idx)
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
    ui->pBt_Cancel->setStyleSheet(
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
    ui->pBt_reset->setStyleSheet(
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
    case 0: {textColor = "#616975"; break;}   // 深蓝色
    case 1: {textColor = "#616975"; break;}   // 白色
    case 2: {textColor = "#616975"; break;}   // 黑色
    default: {textColor = "#616975"; break;}
    }
    ui->lab_content->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight:500;"
                "font-size: 14px;")
            .arg(textColor)
        );

}
