#include "Popup/Plans/EditPlansType.h"
#include "ui_EditPlansType.h"
#include "GlobalDefinition.h"

EditPlansType::EditPlansType(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditPlansType)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(10);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 26));    // 黑色半透明 rgba(0,0,0,0.1)
    ui->frame->setGraphicsEffect(shadow);

    connect(ui->pBt_close,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_cancle,&QPushButton::clicked,this,&QDialog::reject);
    // connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::accept);
    ui->widget->hide();
}

EditPlansType::~EditPlansType()
{
    delete ui;
}
//根据内容，显示字数
void EditPlansType::on_lineEdit_textChanged(const QString &arg1)
{
    // 获取字符数（中英文都算1个字）
    int charCount = arg1.length();
    QString txt = QString::number(charCount) + "/6";
    ui->lab_count->setText(txt);
}

//新建方案，判断分类名称是否存在
void EditPlansType::on_pBt_ok_clicked()
{
    QString name = ui->lineEdit->text();
    for (auto& plan : PlansTypes) {
        // 不区分大小写匹配
        if (QString::compare(name, plan.Name, Qt::CaseInsensitive) == 0)
        {
            ui->widget->show();
            return;
        }
    }
    if(updateId!=-1)
    {
        PlansTypes[updateId].Name = name;
    }else
    {
        PlansTypes[PlansTypeIdx].Name = name;
        PlansTypes[PlansTypeIdx].en = true;
    }


    accept();
}
//隐藏警告
void EditPlansType::hidePrompt()
{
    ui->widget->hide();
    ui->lineEdit->clear();
}

void EditPlansType::EditTitle(int idx)
{
    switch(idx)
    {
    case 0:
        //新建分类
        ui->lab_title->setText(tr("新建分类"));
        ui->pBt_ok->setText(tr("+ 新建方案"));
        break;
    case 1:
        //重命名分类
        ui->lab_title->setText(tr("重命名分类"));
        ui->pBt_ok->setText(tr("重命名"));
        break;
        break;
    }
}

void EditPlansType::ShowEditName(int Id,QString txt)
{
    updateId = Id;
    ui->lineEdit->setText(txt);
}

//根据主题设置样式
void EditPlansType::setTheme_EditPlansType(int idx)
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
    ui->pBt_close->setStyleSheet(
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
    switch (idx)
    {
    case 0: {textColor = "#009FEF"; break;}   // 深蓝色
    case 1: {textColor = "#009FEF"; break;}   // 白色
    case 2: {textColor = "#009FEF"; break;}   // 黑色
    default: {textColor = "#009FEF"; break;}
    }
    //取消按钮
    ui->pBt_cancle->setStyleSheet(
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
    //新建分类按钮
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
    ui->lab_img->setStyleSheet(QString("border-image: url(:/Skin/Images/Popup/prompt%1.png);").arg(suffix));


    //字体
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

    switch (idx)
    {
    case 0: {textColor = "#D44040"; break;}   // 深蓝色
    case 1: {textColor = "#D44040"; break;}   // 白色
    case 2: {textColor = "#D44040"; break;}   // 黑色
    default: {textColor = "#D44040"; break;}
    }
    ui->lab_error->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 10px;"
                "color: %1;")
            .arg(textColor)
        );



    switch (idx)
    {
    case 0:// 深蓝色
    {
        textColor = "#616975";
        colorStr = "rgba(81, 96, 122, 0.2)";
        break;
    }
    case 1:// 白色
    {
        textColor = "#616975";
        colorStr = "rgba(81, 96, 122, 0.2)";
        break;
    }
    case 2: // 黑色
    {
        textColor = "#616975";
        colorStr = "rgba(81, 96, 122, 0.2)";
        break;
    }
    default:
    {
        textColor = "#616975";
        colorStr = "rgba(81, 96, 122, 0.2)";
        break;
    }
    }
    ui->lineEdit->setStyleSheet(
        QString("border-radius: 4px;"
                /* 20%面板 */
                "background: %2;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight: 500;"
                "font-weight: 400;"
                "font-size: 14px;"
                /* 50%字体灰色 */
                "color: %1;")
        .arg(textColor).arg(colorStr)
        );

    ui->lab_count->setStyleSheet(
        QString("background: transparent;"
                "font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-weight: 400;"
                "font-size: 14px;"
                /* 50%字体灰色 */
                "color: %1;")
        .arg(textColor)
        );


}
