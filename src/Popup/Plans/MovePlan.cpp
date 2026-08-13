#include "Popup/Plans/MovePlan.h"
#include "ui_MovePlan.h"
#include "LoadLib.h"
QString MyPlanType = "";//方案分类名称
int MyPlanTypeIdx = 0;//方案分类ID
#include "QStandardItemModel"

MovePlan::MovePlan(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MovePlan)
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

    M_SetCBoxShadow(ui->cBox_Type);
    ui->cBox_Type->setPopupOffsetXY(-8,2);
    QString styleSheet = (R"(QComboBox{
                                        font-family: "Noto Sans S Chinese";
                                        font-weight: 500;
                                        font-size: 14px;
                                        border-radius: 4px;
                                        combobox-popup: 0;
                                        background-color:rgba(81, 96, 122, 0.2);
                                        padding-left: 10px;
                                        color: #616975;
                            }
                            QComboBox::drop-down{
                                        border-image: url(:/Skin/Images/cBox/selfDroptriangle_no.png);
                                        margin-top:0px;
                                        subcontrol-origin: padding;
                                        subcontrol-position: center right;
                                        margin-right:10px;height:14px;width:11px;
                            }
                            QComboBox::drop-down:checked{
                                        border-image: url(:/Skin/Images/cBox/selfDroptriangle_se.png);
                                        margin-top:0px;
                                        margin-right:10px;
                                        subcontrol-origin: padding;
                                        subcontrol-position: center right;
                                        height:14px;
                                        width:11px;
                            }
                            )"
                          );
    ui->cBox_Type->setStyleSheet(styleSheet);

    //创建自定义的 NoSelectListView 并设置下拉列表样式 -----
    QListView* listView = new QListView();


    listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
    listView->setAutoScroll(false);  // 禁用边缘自动滚动

    //一个 QListView 不能同时被两个 QComboBox 使用
    // 样式表
    QString listStyle = R"(
    QListView {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 14px;
        background: #0D0F14;
        border-radius: 6px;
        padding-left: 6px;
        padding-right: 6px;
        padding-top: 6px;
        padding-bottom: 6px;
        outline: 0;/*移除焦点轮廓*/

    }
    QListView::item {
        width: 243px;
        height: 25px;
        margin-top: 4px;
        margin-bottom: 4px;
        margin-left: 6px;          /* 添加左间距 */
        margin-right: 6px;         /* 添加右间距 */
        color: #A1A8B3;
        background-color: transparent;
        outline: 0;/*移除焦点轮廓*/
    }
    QListView::item:hover {
        background-color: rgba(223, 243, 255, 0.2);
        border-radius: 4px;
        /* 无需再设置 margin-left/right，会继承普通 item 的 */
    }
    QListView::item:selected {
        background-color: #0091DA;
        border-radius: 4px;
        color: #FFFFFF;
        /* 同理，删除 margin-left/right */
    }
)";


    listView->setStyleSheet(listStyle);            // 样式只给 listView
    ui->cBox_Type->setView(listView);          // 替换下拉视图

    //让下拉高度随项数自动增加（取消最大可见项限制）
    ui->cBox_Type->setMaxVisibleItems(INT_MAX);   // 一个足够大的数

    connect(ui->pBt_close,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_cancle,&QPushButton::clicked,this,&QDialog::reject);
}

MovePlan::~MovePlan()
{
    delete ui;
}
//给QComBobox设置阴影
void MovePlan::M_SetCBoxShadow(NewComboBox *cBox)
{
    QWidget* container = cBox->view()->parentWidget();
    if (!container) return;

    container->setWindowFlags(container->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    container->setAttribute(Qt::WA_TranslucentBackground);
    container->setFixedWidth(255 + 8);
    if (container->layout())
         container->layout()->setContentsMargins(8, 8, 8, 8);  // 四周留出阴影空间

    // 阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 128));
    shadow->setOffset(0, 4);
    container->setGraphicsEffect(shadow);

    //把容器告诉 NewComboBox
    cBox->setPopupContainer(container);
}

void MovePlan::addType(QString name)
{
    ui->cBox_Type->addItem(name);
}
void MovePlan::delType(QString name)
{
    int idx = ui->cBox_Type->findText(name);
    if (idx != -1) {
        ui->cBox_Type->removeItem(idx);
    }
}
void MovePlan::delAllType() {
    while (ui->cBox_Type->count() > 0) {
        ui->cBox_Type->removeItem(0);   // 一直删第0项，或从最后删
    }
}
void MovePlan::rnameType(QString oldName,QString newName)
{
    int idx = ui->cBox_Type->findText(oldName);
    if (idx != -1) {
        ui->cBox_Type->setItemText(idx, newName);
    }
}
void MovePlan::showType(QString name)
{
    ui->cBox_Type->setCurrentText(name);
}

void MovePlan::on_pBt_ok_clicked()
{
    MyPlanTypeIdx = ui->cBox_Type->currentIndex();
    MyPlanType = ui->cBox_Type->currentText();

    accept();
}

//根据主题设置样式
void MovePlan::setTheme_MovePlan(int idx)
{
    QString textColor,colorStr,bkcolor,textColor2,bkcolor2;

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


    //文本
    switch (idx)
    {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }
    ui->label->setStyleSheet(
        QString("color: %1;"
                "font-family: \"Noto Sans S Chinese\";"
                "font-weight:500;"
                "font-size: 16px;"
                "background:transparent;")
        .arg(textColor)
        );

    //QComboBox
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


    QString styleSheet = QString((R"(QComboBox{
                                        font-family: "Noto Sans S Chinese";
                                        font-weight: 500;
                                        font-size: 14px;
                                        border-radius: 4px;
                                        combobox-popup: 0;
                                        background-color:%1;
                                        padding-left: 10px;
                                        color: %2;
                            }
                            QComboBox::drop-down{
                                        border-image: url(:/Skin/Images/cBox/selfDroptriangle_no%3.png);
                                        margin-top:0px;
                                        subcontrol-origin: padding;
                                        subcontrol-position: center right;
                                        margin-right:10px;height:14px;width:11px;
                            }
                            QComboBox::drop-down:checked{
                                        border-image: url(:/Skin/Images/cBox/selfDroptriangle_se%3.png);
                                        margin-top:0px;
                                        margin-right:10px;
                                        subcontrol-origin: padding;
                                        subcontrol-position: center right;
                                        height:14px;
                                        width:11px;
                            }
                            )"
                                  )).arg(colorStr).arg(textColor).arg(suffix);
    ui->cBox_Type->setStyleSheet(styleSheet);

    //创建自定义的 QListView 并设置下拉列表样式 -----
    QListView* listView = new QListView();


    listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
    listView->setAutoScroll(false);  // 禁用边缘自动滚动

    //一个 QListView 不能同时被两个 QComboBox 使用
    // 样式表
    switch (idx)
    {
    case 0:// 深蓝色
    {
        bkcolor = "#0D0F14";
        textColor = "#A1A8B3";
        colorStr = "rgba(223, 243, 255, 0.2)";
        textColor2 = "#FFFFFF";
        bkcolor2 = "#0091DA";
        break;
    }
    case 1:// 白色
    {
        bkcolor = "#0D0F14";
        textColor = "#A1A8B3";
        colorStr = "rgba(223, 243, 255, 0.2)";
        textColor2 = "#FFFFFF";
        bkcolor2 = "#0091DA";
        break;
    }
    case 2:// 黑色
    {
        bkcolor = "#0D0F14";
        textColor = "#A1A8B3";
        colorStr = "rgba(223, 243, 255, 0.2)";
        textColor2 = "#FFFFFF";
        bkcolor2 = "#0091DA";
        break;
    }
    default:
    {
        bkcolor = "#0D0F14";
        textColor = "#A1A8B3";
        colorStr = "rgba(223, 243, 255, 0.2)";
        textColor2 = "#FFFFFF";
        bkcolor2 = "#0091DA";
        break;
    }
    }

    QString listStyle = QString(R"(
    QListView {
        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 14px;
        background: %1;
        border-radius: 6px;
        padding-left: 6px;
        padding-right: 6px;
        padding-top: 6px;
        padding-bottom: 6px;
        outline: 0;/*移除焦点轮廓*/

    }
    QListView::item {
        width: 243px;
        height: 25px;
        margin-top: 4px;
        margin-bottom: 4px;
        margin-left: 6px;          /* 添加左间距 */
        margin-right: 6px;         /* 添加右间距 */
        color: %2;
        background-color: transparent;
        outline: 0;/*移除焦点轮廓*/
    }
    QListView::item:hover {
        background-color: %3;
        border-radius: 4px;
        /* 无需再设置 margin-left/right，会继承普通 item 的 */
    }
    QListView::item:selected {
        background-color: %4;
        border-radius: 4px;
        color: %5;
        /* 同理，删除 margin-left/right */
    }
)").arg(bkcolor).arg(textColor).arg(colorStr).arg(bkcolor2).arg(textColor2);


    listView->setStyleSheet(listStyle);            // 样式只给 listView
}
