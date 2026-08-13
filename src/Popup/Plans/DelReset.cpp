#include "Popup/Plans/DelReset.h"
#include "ui_DelReset.h"
#include <QGraphicsDropShadowEffect>

DelReset::DelReset(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DelReset)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    ui->pBt_ok->setDefault(true);//将该按钮设置为对话框的默认按钮（按回车触发）,QDialog 或基于 QDialog 的窗口才可以此实现

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 20px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    ui->frame->setGraphicsEffect(shadow);

    connect(ui->pBt_close,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_exit,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::accept);
}

DelReset::~DelReset()
{
    delete ui;
}

void DelReset::editText(int idx)
{
    switch(idx)
    {
    case 0:
        //删除方案
        ui->lab_1->setText(tr("确认删除此方案吗？"));
        ui->lab_2->setText(tr("方案删除后无法恢复，请谨慎操作"));
        ui->pBt_ok->setText(tr("删除"));
        ui->lab_del_warn->hide();
        break;
    case 1:
        //重置均衡器
        ui->lab_1->setText(tr("确认重置均衡器吗？"));
        ui->lab_2->setText(tr("重置操作无法撤销，请谨慎操作"));
        ui->pBt_ok->setText(tr("重置"));
        ui->lab_del_warn->hide();
        break;
    case 2:
        //重置算法
        ui->lab_1->setText(tr("确认重置算法吗？"));
        ui->lab_2->setText(tr("重置操作无法撤销，请谨慎操作"));
        ui->pBt_ok->setText(tr("重置"));
        ui->lab_del_warn->hide();
        break;
    case 3:
        //重置空间音频
        ui->lab_1->setText(tr("确认重置空间音频吗？"));
        ui->lab_2->setText(tr("重置操作无法撤销，请谨慎操作"));
        ui->pBt_ok->setText(tr("重置"));
        ui->lab_del_warn->hide();
        break;
    case 4:
        //删除分类
        ui->lab_1->setText(tr("确认删除此分类吗？"));
        ui->lab_2->setText(tr("分类删除后无法恢复，请谨慎操作"));
        ui->pBt_ok->setText(tr("删除"));
        ui->lab_del_warn->hide();
        break;
    case 5:
        //删除所有选中方案
        ui->lab_1->setText(tr("确认删除选中方案吗？"));
        ui->lab_2->setText(tr("方案删除后无法恢复，请谨慎操作。"));
        ui->pBt_ok->setText(tr("删除"));
        ui->lab_del_warn->show();
    case 6:
        //删除已上传方案
        ui->lab_1->setText(tr("确认删除此方案吗？"));
        ui->lab_2->setText(tr("删除仅下架社区，预设库预设保留不变"));
        ui->pBt_ok->setText(tr("删除"));
        ui->lab_del_warn->show();

    default:
        break;
    }
}


//根据主题设置样式
void DelReset::setTheme_DelReset(int idx)
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
    //取消按钮
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

    switch (idx)
    {
    case 0: {textColor = "#FFFFFF"; break;}   // 深蓝色
    case 1: {textColor = "#FFFFFF"; break;}   // 白色
    case 2: {textColor = "#FFFFFF"; break;}   // 黑色
    default: {textColor = "#FFFFFF"; break;}
    }
    //删除、重置按钮
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

    //字体
    switch (idx)
    {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }
    ui->lab_1->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 16px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );

    switch (idx)
    {
    case 0: {textColor = "#616975"; break;}   // 深蓝色
    case 1: {textColor = "#616975"; break;}   // 白色
    case 2: {textColor = "#616975"; break;}   // 黑色
    default: {textColor = "#616975"; break;}
    }
    ui->lab_2->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->lab_del_warn->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );


}
