#include "Popup/bExitDirectly.h"
#include "ui_bExitDirectly.h"
#include "LoadLib.h"

bExitDirectly::bExitDirectly(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    ui->pBt_ok->setDefault(true);//将该按钮设置为对话框的默认按钮（按回车触发）,QDialog 或基于 QDialog 的窗口才可以此实现

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(10);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 26));    // 黑色半透明 rgba(0,0,0,0.1)
    ui->frame->setGraphicsEffect(shadow);

    connect(ui->pBt_close,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_exit,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::accept);
}

bExitDirectly::~bExitDirectly()
{
    delete ui;
}
void bExitDirectly::ShowExitMode()
{
    if(g_user_system_settings_config_info.is_exit_directly.load())
    {
        ui->rButton_exit->setChecked(true);
    }else
    {
        ui->rButton_Minimize->setChecked(true);
    }
}


void bExitDirectly::SetExitMode()
{
    if(ui->rButton_Minimize->isChecked())
    {
        g_user_system_settings_config_info.is_exit_directly.store(false);
    }else
    {
        g_user_system_settings_config_info.is_exit_directly.store(true);
    }
    g_user_system_settings_config_info.is_remember_choice.store(
        ui->cBox_remember->isChecked());
    globalSettings->setValue("bExitDirectly",
                             g_user_system_settings_config_info.is_exit_directly.load());
    globalSettings->setValue("bRemember",
                             g_user_system_settings_config_info.is_remember_choice.load());
}


//根据主题设置样式
void bExitDirectly::setTheme_bExitDirectly(int idx)
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
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }
    ui->lab_title->setStyleSheet(
        QString("border:null;"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-size: 16px;")
            .arg(textColor)
        );

    switch (idx) {
    case 0: {textColor = "#616975"; break;}   // 深蓝色
    case 1: {textColor = "#616975"; break;}   // 白色
    case 2: {textColor = "#616975"; break;}   // 黑色
    default: {textColor = "#616975"; break;}
    }

    ui->rButton_Minimize->setStyleSheet(
        QString("QRadioButton{"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-size: 12px;"
                "}"
                /*指示器样式*/
                "QRadioButton::indicator{"
                "height:13px;"
                "width:13px;"
                "border-image: url(:/Skin/Images/close/radio-no%2.png);"
                "margin-right: 9px;"
                "}"
                /*指示器被选中样式*/
                "QRadioButton::indicator:checked {"
                "border-image: url(:/Skin/Images/close/radio-se%2.png);"
                "}"
                /*指示器未选中悬浮样式*/
                "QRadioButton::indicator:unchecked:hover{"
                "border-image: url(:/Skin/Images/close/radio-ho%2.png);}")
            .arg(textColor).arg(suffix)
        );

    ui->rButton_exit->setStyleSheet(
        QString("QRadioButton{"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-size: 12px;"
                "}"
                /*指示器样式*/
                "QRadioButton::indicator{"
                "height:13px;"
                "width:13px;"
                "border-image: url(:/Skin/Images/close/radio-no%2.png);"
                "margin-right: 9px;"
                "}"
                /*指示器被选中样式*/
                "QRadioButton::indicator:checked {"
                "border-image: url(:/Skin/Images/close/radio-se%2.png);"
                "}"
                /*指示器未选中悬浮样式*/
                "QRadioButton::indicator:unchecked:hover{"
                "border-image: url(:/Skin/Images/close/radio-ho%2.png);}")
            .arg(textColor).arg(suffix)
        );



    ui->cBox_remember->setStyleSheet(
        QString("QCheckBox {"
                "color: %1;"
                "font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 10px;"
                "}"

                "QCheckBox::indicator {"
                "height: 12px;"
                "width: 12px;"
                "border-image: url(:/Skin/Images/close/remember-no%2.png);"
                "margin-right: 9px;"
                "}"

                /* 未选中 + 悬停 */
                "QCheckBox::indicator:unchecked:hover {"
                "border-image: url(:/Skin/Images/close/remember-no-ho%2.png);"
                "}"

                /* 选中状态 */
                "QCheckBox::indicator:checked {"
                "border-image: url(:/Skin/Images/close/remember-se%2.png);"
                "}"

                /* 选中 + 悬停 */
                "QCheckBox::indicator:checked:hover {"
                "border-image: url(:/Skin/Images/close/remember-se-ho%2.png);"
                "}")
        .arg(textColor).arg(suffix)
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
                "border-image: url(:/Skin/Images/Popup/confirm-ho%2.png);}")
            .arg(textColor).arg(suffix)
        );
}
