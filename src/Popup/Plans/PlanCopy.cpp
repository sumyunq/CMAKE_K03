#include "Popup/Plans/PlanCopy.h"
#include "ui_PlanCopy.h"
#include <QLineEdit>

QString MyPlanName = "";
QString MyPlanDesc = "";
QStringList MyPlanLab1 = {};//方案标签1
QString MyPlanLab2 = "";//方案标签2
int m_maxBytes = 40;
QString rName;//重命名前的名称
QString tName;//标题

PlanCopy::PlanCopy(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PlanCopy)
{
    ui->setupUi(this);
    // 隐藏标题栏
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->lab_beUsed->hide();

    ui->pBt_ok->setDefault(true);//将该按钮设置为对话框的默认按钮（按回车触发）,QDialog 或基于 QDialog 的窗口才可以此实现


    //不区分机型
    ui->widget_Dev2->hide();
    ui->widget_Dev3->hide();
    ui->pBt_addDev->hide();


    M_SetCBoxShadow(ui->cBox_Scene);
    M_SetCBoxShadow(ui->cBox_PlanType_Idx);
    ui->cBox_Scene->setPopupOffsetXY(-8,2);
    ui->cBox_PlanType_Idx->setPopupOffsetXY(-8,2);

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
                                        margin-right:10px;
                                        height:14px;
                                        width:11px;
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
    ui->cBox_Scene->setStyleSheet(styleSheet);
    ui->cBox_PlanType_Idx->setStyleSheet(styleSheet);

    //创建自定义的 NoSelectListView 并设置下拉列表样式 -----
    QListView* listView = new QListView();
    QListView* listView_planType_Idx_ = new QListView();  ///< 分类选择下拉框

    listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
    listView->setAutoScroll(false);  // 禁用边缘自动滚动
    // 同上 listView
    listView_planType_Idx_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
    listView_planType_Idx_->setAutoScroll(false);  // 禁用边缘自动滚动

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
        width: 148px;
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
    ui->cBox_Scene->setView(listView);          // 替换下拉视图

    listView_planType_Idx_->setStyleSheet(listStyle);       // 样式只给 listView
    ui->cBox_PlanType_Idx->setView(listView_planType_Idx_); // 替换下拉视图

    //让下拉高度随项数自动增加（取消最大可见项限制）
    ui->cBox_Scene->setMaxVisibleItems(INT_MAX);   // 或一个足够大的数，例如 1000
    ui->cBox_PlanType_Idx->setMaxVisibleItems(INT_MAX);   // 或一个足够大的数，例如 1000

    // 防止机型的弹窗
    // AddDevBox = new QWidget(this);
    // ui->widget_AddDevBox->setWindowFlags(Qt::FramelessWindowHint);
    // ui->widget_AddDevBox->setModal(false);
    // 设置宽度为128，高度自适应（不设置固定高度）
    // ui->widget_AddDevBox->setFixedWidth(128);

    // 设置样式表
    ui->widget_AddDevBox->setStyleSheet(

        "    background: #283243;"
        "    border-radius: 6px;"
        );

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->widget_AddDevBox);
    shadow->setBlurRadius(8);
    shadow->setXOffset(0);
    shadow->setYOffset(4);
    shadow->setColor(QColor(0, 0, 0, 128));
    ui->widget_AddDevBox->setGraphicsEffect(shadow);


    // 创建垂直布局
    QVBoxLayout *layout = new QVBoxLayout(ui->widget_AddDevBox);

    // 设置边距，上16，左14，下16，右0
    layout->setContentsMargins(14, 16, 0, 16); //left, top, right, bottom

    // 设置控件间距为6
    layout->setSpacing(6);

    group_dev = new QButtonGroup(this);
    group_dev->setExclusive(false);  // 允许多选
    // 添加按钮
    auto createStyledButton = [&](const QString &text, QWidget *parent) -> QPushButton* {
        QPushButton *btn = new QPushButton(text, parent);
        btn->setFixedHeight(24);
        btn->setStyleSheet("QPushButton{"
                           "background: rgba(81, 96, 122, 0.2);"
                           "border-radius: 12px;"
                           "color: #FFFFFF;"
                           "font-family: \"Noto Sans S Chinese\";"
                           "font-weight: 500;"
                           "font-size: 14px;"
                           "padding: 0 12px; "
                           "}"
                           "QPushButton::checked{"
                           "background: #009FEF;"

                           "}"
                           );
        btn->setCheckable(true);
        btn->setChecked(false);
        btn->setEnabled(true);
        group_dev->addButton(btn);  // 加入组
        return btn;
    };

    QPushButton *btn1 =  createStyledButton("T10有线", ui->widget_AddDevBox);
    QPushButton *btn2 =  createStyledButton("T10无线", ui->widget_AddDevBox);
    QPushButton *btn3 =  createStyledButton("K03S", ui->widget_AddDevBox);
    QPushButton *btn4 =  createStyledButton("K03S超竞版", ui->widget_AddDevBox);
    QPushButton *btn5 =  createStyledButton("K06S", ui->widget_AddDevBox);
    QPushButton *btn6 =  createStyledButton("T7", ui->widget_AddDevBox);
    QPushButton *btn7 =  createStyledButton("T7 GT", ui->widget_AddDevBox);
    QPushButton *btn8 =  createStyledButton("S21无线智充版", ui->widget_AddDevBox);


    layout->addWidget(btn1, 0, Qt::AlignLeft);
    layout->addWidget(btn2, 1, Qt::AlignLeft);
    layout->addWidget(btn3, 2, Qt::AlignLeft);
    layout->addWidget(btn4, 3, Qt::AlignLeft);
    layout->addWidget(btn5, 4, Qt::AlignLeft);
    layout->addWidget(btn6, 5, Qt::AlignLeft);
    layout->addWidget(btn7, 6, Qt::AlignLeft);
    layout->addWidget(btn8, 7, Qt::AlignLeft);

    installEventFilter(this);   // 安装自身过滤器

    ui->widget_AddDevBox->hide();
    ui->widget_Dev2->hide();
    ui->widget_Dev3->hide();
    ui->lab_warning->hide();
    ui->lab_warning_des->hide();
    ui->lab_Dev2->setText("");
    ui->lab_Dev3->setText("");

    connect(group_dev,
            static_cast<void(QButtonGroup::*)(QAbstractButton*, bool)>(&QButtonGroup::buttonToggled),
            this,
            [this](QAbstractButton *button, bool checked)
            {
                if(checked)
                {
                    checkedCount++;
                    if(checkedCount == 2)
                    {
                        ui->lab_Dev2->setText(button->text());
                        ui->widget_Dev2->show();
                    }else if(checkedCount == 3)
                    {
                        ui->lab_Dev3->setText(button->text());
                        ui->widget_Dev3->show();
                    }
                }else
                {
                    checkedCount--;
                }
                ui->lab_warning->setVisible(checkedCount >= 3);
                // 根据数量决定 pBt_addDev 的可见性
                // ui->pBt_addDev->setVisible(checkedCount < 3);  // 少于3个时显示，≥3时隐藏
                ui->pBt_addDev->hide();
                if(ui->widget_AddDevBox->isVisible())
                {
                    ui->widget_AddDevBox->setVisible(checkedCount < 3);
                }
            });


}

PlanCopy::~PlanCopy()
{
    delete ui;
}

bool PlanCopy::eventFilter(QObject *watched, QEvent *event)
{
    if ((event->type() == QEvent::MouseButtonPress ||
         event->type() == QEvent::MouseButtonDblClick) &&
        ui->widget_AddDevBox && ui->widget_AddDevBox->isVisible())
    {
        //点击到非widget_AddDevBox区域，则隐藏
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        // 将全局坐标映射到 widget_AddDevBox 的局部坐标
        QPoint localPos = ui->widget_AddDevBox->mapFromGlobal(me->globalPos());
        // 判断是否在控件的 rect() 内
        if (!ui->widget_AddDevBox->rect().contains(localPos)) {
            ui->widget_AddDevBox->hide();
        }
    }
    return false;   // 始终传递事件
}


void PlanCopy::M_SetCBoxShadow(NewComboBox *cBox)
{
    QWidget* container = cBox->view()->parentWidget();
    if (!container) return;

    container->setWindowFlags(container->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    container->setAttribute(Qt::WA_TranslucentBackground);

    container->setFixedWidth(160 + 8);
    if (container->layout()) {
        container->layout()->setContentsMargins(8, 8, 8, 8);  // 四周留出阴影空间
    }

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 128));
    shadow->setOffset(0, 4);
    container->setGraphicsEffect(shadow);

    //把容器告诉 NewComboBox
    cBox->setPopupContainer(container);
}


//设置标题
void PlanCopy::EditTitle(QString titleName)
{
    // ui->lab_title->setText(titleName);
    tName = titleName;
    ui->pBt_ok->setText(titleName);
}
//二创方案：显示出系统方案名称
void PlanCopy::showSysName(QString name)
{
    ui->lEdit_Name->setText(name);
}
//修改：显示方案当前名称,当前描述，当前标签(设备+场景)
void PlanCopy::showCurName(QString name,QString desc,QStringList lab_devs,QString lab2)
{
    ui->lEdit_Name->setText(name);
    ui->pTextEdit_Description->setPlainText(desc);
    // ui->lab_Dev->setText(lab1);
    if(lab_devs.size() <=1)
    {
        ui->lab_Dev->setText(lab_devs[0]);
    }else
    {
        QString lab1 = ui->lab_Dev->text();
        // 收集所有不等于 lab1 的设备名
        QStringList otherDevs;
        for (const QString &dev : lab_devs) {
            if (dev != lab1) {
                otherDevs << dev;

                QList<QAbstractButton*> allBtns = group_dev->buttons();
                for (QAbstractButton *btn : allBtns) {
                    if (btn->text() == dev) {
                        btn->setChecked(true);
                        break;  // 找到后退出循环
                    }
                }
            }
        }
        // 获取前两个不重复的设备名（最多两个）
        QString dev2 = otherDevs.value(0, "");
        QString dev3 = otherDevs.value(1, "");

        ui->lab_Dev2->setText(dev2);
        ui->lab_Dev3->setText(dev3);

        // 控制可见性
        ui->widget_Dev2->setVisible(!dev2.isEmpty());
        ui->widget_Dev3->setVisible(!dev3.isEmpty());
    }
    ui->cBox_Scene->setCurrentText(lab2);
    rName = name;
}
void PlanCopy::on_pBt_ok_clicked()
{
    bool beUsed = false;
    if(ui->lEdit_Name->text().isEmpty())
    {
        ui->lab_beUsed->setText(tr("名称为空"));
        ui->lab_beUsed->show();
    }else
    {
        MyPlanName = ui->lEdit_Name->text();
        MyPlanDesc = ui->pTextEdit_Description->toPlainText();
        MyPlanLab1 = QStringList{ui->lab_Dev->text()};

        MyPlanLab2 = ui->cBox_Scene->currentText();

        {
            //分机型显示时
            // MyPlanLab1.clear();
            // for (QLabel *lab : {ui->lab_Dev, ui->lab_Dev2, ui->lab_Dev3}) {
            //     if (!lab->text().isEmpty())
            //         MyPlanLab1 << lab->text();
            // }
            // for (const QString& dev : MyPlanLab1)
        }
        //不分机型显示时
        QString dev = QString();
        {
            NewRadioBtn* existingBtn = MovieVal.AllPlanRadioHash.value(qMakePair(MyPlanName,dev), nullptr);
            if (existingBtn) {
                // 找到同名按钮，需要判断是否排除自身（修改且名称未变）
                if (!(tName == "修改方案" && rName == MyPlanName)) {
                    beUsed = true;
                    ui->lab_beUsed->setText(tr("此名称已被使用"));
                    ui->lab_beUsed->show();
                }
            }
        }
        if(!beUsed)
        {
            ui->lab_beUsed->hide();
            accept();
        }
    }

}

//点击关闭按钮
void PlanCopy::on_pBt_close_clicked()
{
    reject();
}
//点击取消按钮
void PlanCopy::on_pBt_cancle_clicked()
{
    reject();
}


void PlanCopy::on_pushButton_clicked()
{
    reject();
}

void PlanCopy::enforceByteLimit() {
    const QString currentText = ui->lEdit_Name->text();
    const QByteArray utf8Bytes = currentText.toUtf8();

    // 检查字节长度
    if (utf8Bytes.size() > m_maxBytes) {
        // 查找不超过最大字节数的有效截断点
        int validLength = 0;
        for (int i = m_maxBytes; i > 0; --i) {
            if ((utf8Bytes[i] & 0xC0) != 0x80) { // 检查UTF-8序列起始字节
                validLength = i;
                break;
            }
        }

        // 截断文本并设置回控件
        const QString truncated = QString::fromUtf8(utf8Bytes.constData(), validLength);
        if (truncated != currentText) {
            blockSignals(true); // 阻止递归调用
            ui->lEdit_Name->setText(truncated);
            blockSignals(false);

            // 发出自定义信号
            //emit byteLimitExceeded(currentText, truncated);
        }
    }
}

void PlanCopy::on_lEdit_Name_textChanged(const QString &arg1)
{
    // 检查字节长度限制
    //enforceByteLimit();
}

void PlanCopy::ShowDev()
{
    if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
    {
        if(SelDev_PID == 0xF016 || SelDev_PID == 0xF017)
        {
            ui->lab_Dev->setText(tr("K03S超竞版"));

        }else
        {
            ui->lab_Dev->setText("K03S");
        }

    }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText("K06S");
    }else if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive))
    {
        if(SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
        {
            ui->lab_Dev->setText(tr("T10无线"));
        }else
        {
            ui->lab_Dev->setText(tr("T10有线"));
        }

    }else if(SelDev_DeviceName.contains("T7 GT",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText("T7 GT");
    }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText("T7");
    }else if(SelDev_DeviceName.contains("S21",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText(tr("S21无线智充版"));
    }else
    {
        ui->lab_Dev->setText("");
    }

    QList<QAbstractButton*> allBtns = group_dev->buttons();
    for (QAbstractButton *btn : allBtns) {
        if (btn->text() == ui->lab_Dev->text()) {
            btn->setChecked(true);
            btn->setEnabled(false);//不可取消掉
            break;  // 找到后退出循环
        }
    }
}

void PlanCopy::updateUI_cBox_PlanType_Idx(QStringList planTypes)
{
    ui->cBox_PlanType_Idx->clear();

    if (planTypes.size() == 0)
        return;

    ui->cBox_PlanType_Idx->addItems(planTypes);

    ui->cBox_PlanType_Idx->setCurrentIndex (0);

}

int PlanCopy::get_cBox_PlanType_Idx_currentIndex()
{
    return ui->cBox_PlanType_Idx->currentIndex();
}

//描述最多为50个字
void PlanCopy::on_pTextEdit_Description_textChanged()
{
    QString text = ui->pTextEdit_Description->toPlainText();
    if (text.length() > 50) {
        ui->lab_warning_des->show();
        // 阻止信号递归，避免再次触发 textChanged
        ui->pTextEdit_Description->blockSignals(true);
        // 截断到最大长度
        text = text.left(50);
        ui->pTextEdit_Description->setPlainText(text);
        // 将光标移动到文本末尾
        QTextCursor cursor = ui->pTextEdit_Description->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->pTextEdit_Description->setTextCursor(cursor);
        ui->pTextEdit_Description->blockSignals(false);
    }else
    {
        ui->lab_warning_des->hide();
    }
}

//添加机型
void PlanCopy::on_pBt_addDev_clicked()
{
    qDebug("显示\n");
    ui->widget_AddDevBox->show();

}


void PlanCopy::on_pBt_Dev2_clicked()
{

    QList<QAbstractButton*> allBtns = group_dev->buttons();
    for (QAbstractButton *btn : allBtns) {
        if (btn->text() == ui->lab_Dev2->text()) {
            btn->setChecked(false);
            break;  // 找到后退出循环
        }
    }
    ui->lab_Dev2->setText("");
    ui->widget_Dev2->hide();
}


void PlanCopy::on_pBt_Dev3_clicked()
{
    QList<QAbstractButton*> allBtns = group_dev->buttons();
    for (QAbstractButton *btn : allBtns) {
        if (btn->text() == ui->lab_Dev3->text()) {
            btn->setChecked(false);
            break;  // 找到后退出循环
        }
    }
    ui->lab_Dev3->setText("");
    ui->widget_Dev3->hide();
}


//根据主题设置样式
void PlanCopy::setTheme_PlanCopy(int idx)
{
    QString textColor,colorStr;

    //背景
    switch (idx) {
    case 0: {colorStr = "#10151D"; break;}   // 深蓝色
    case 1: {colorStr = "#10151D"; break;}   // 白色
    case 2: {colorStr = "#10151D"; break;}   // 黑色
    default: {colorStr = "#10151D"; break;}
    }
    ui->frame->setStyleSheet(QString("border-radius: 16px;background-color: %1;").arg(colorStr));

    //图片
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


    //字体
    switch (idx)
    {
    case 0: {textColor = "#A1A8B3"; break;}   // 深蓝色
    case 1: {textColor = "#A1A8B3"; break;}   // 白色
    case 2: {textColor = "#A1A8B3"; break;}   // 黑色
    default: {textColor = "#A1A8B3"; break;}
    }

    ui->label_3->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->label_4->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->label_5->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->label_6->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );
    ui->label_7->setStyleSheet(
        QString("font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 14px;"
                "color: %1;"
                "background:transparent;")
            .arg(textColor)
        );

}
//场景改变，右上角的图标跟着改变
void PlanCopy::on_cBox_Scene_currentIndexChanged(int index)
{
    QString suffix = "game";
    switch(index)
    {
    case 0:
        //游戏
        suffix = "game";
        break;
    case 1:
        //电影
        suffix = "movie";
        break;
    case 2:
        //音乐
        suffix = "music";
        break;
    case 3:
        //三角洲
        suffix = "delta";
        break;
    case 4:
        //PUBG
        suffix = "pubg";
        break;
    case 5:
        //CSGO
        suffix = "csgo";
        break;
    case 6:
        //无畏契约
        suffix = "valorant";
        break;
    case 7:
        //暗区突围
        suffix = "AB";
        break;
    case 8:
        //APEX
        suffix = "apex";
        break;
    case 9:
        //穿越火线
        suffix = "CF";
        break;

    }
    ui->lab_ico->setStyleSheet(QString("border-radius:0px;border-image: url(:/Skin/Images/Headphones/edit/%1-ch.png);").arg(suffix));

}



