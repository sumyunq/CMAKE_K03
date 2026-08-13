#include "./CustomControl/CustomRadioButton/NewRadioBtn.h"
#include <QListView>
#include <QHelpEvent>

NewRadioBtn::NewRadioBtn(const PlanVal& val, QWidget *parent)
    : QRadioButton(parent), m_planValue(val)
{
    //setAutoExclusive(false);//关闭自动互斥
    setStyleSheet("QRadioButton{border-radius: 4px;background-color: rgba(0, 0, 0, 20%);color: rgb(255, 255, 255);}"
                  "QRadioButton:checked {border: 1.5px solid #009FEF;}"
                  "QRadioButton:checked {border-image: url(:/Skin/Images/Headphones/plan-sel.png);}"
                  "QRadioButton::indicator{height:71px;width:71px;image: url(:/Skin/Images/Headphones/edit/icon.png);background-color: rgb(70, 77, 86);margin-right:12px;margin-left:12px;margin-top:12px;margin-bottom:43px;}"
                  "QRadioButton::indicator:checked{height:71px;width:71px;image: url(:/Skin/Images/Headphones/edit/icon.png);background-color: rgb(0, 145, 198);margin-right:12px;margin-left:12px;margin-top:12px;margin-bottom:43px;}"
                  "QRadioButton:focus { outline: none; }"
                  );
    ShareCode = "";
    ShareCodeId = "";
    IsAdded = 0;
    IsLoad = 0;

    // 创建并设置内部按钮
    AllpBt_fav = new QPushButton(this);
    AllpBt_fav->setFixedSize(20, 20);  // 固定小尺寸
    AllpBt_fav->setStyleSheet(
        "QPushButton {background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/fav-no.png);}"
        "QPushButton:hover {background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/fav-no-ho.png);}"
        "QPushButton:checked{background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/fav-se.png);}"
        "QPushButton:checked:hover{background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/fav-se-ho.png);}"
        "QPushButton:disabled{background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/fav-dis.png);}"
        );
    AllpBt_fav->setCursor(QCursor(Qt::PointingHandCursor));//鼠标变成手型
    AllpBt_fav->setCheckable(true);
    AllpBt_fav->setChecked(false);

    //勾选框
    AllpBt_check = new QPushButton(this);
    AllpBt_check->setFixedSize(20, 20);  // 固定小尺寸
    AllpBt_check->setStyleSheet(
        "QPushButton {background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/Checkbox-no.png);}"
        "QPushButton:hover {background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/Checkbox-ho.png);}"
        "QPushButton:checked{background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/Checkbox-ch-no.png);}"
        "QPushButton:checked:hover{background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/Checkbox-ch-ho.png);}"
        );
    AllpBt_check->setCursor(QCursor(Qt::PointingHandCursor));//鼠标变成手型
    AllpBt_check->setCheckable(true);
    AllpBt_check->setChecked(false);
    AllpBt_check->hide();



    AllpBt_edit = new QPushButton(this);
    AllpBt_edit->setFixedSize(20, 20);  // 固定小尺寸
    AllpBt_edit->setStyleSheet(
        "QPushButton {background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/edit.png);}"
        "QPushButton::hover {background-color:transparent;border-image: url(:/Skin/Images/Headphones/AllEdit/edit-ho.png);}"
        "QPushButton::menu-indicator {image: none;}"
        );
    AllpBt_edit->setCursor(QCursor(Qt::PointingHandCursor));//鼠标变成手型
    //AllpBt_edit->hide();


    eMenu = new QMenu();
    A_rename = eMenu->addAction(tr("修改"));
    A_copy = eMenu->addAction(tr("复制"));
    A_del = eMenu->addAction(tr("删除"));
    A_move = eMenu->addAction(tr("移动到"));
    AllpBt_edit->setMenu(eMenu);

    //去掉投影(四周阴影)
    eMenu->setWindowFlag(Qt::NoDropShadowWindowHint);


    eMenu->setStyleSheet(R"(
    QMenu {
        background-color: #222934;
        padding-left: 12px;
        padding-right: 8px;
        width:83px;
        height:124px;
    }
    QMenu::item {
        padding: 0px 0px;
        color: #CCCCCC;
        border-bottom: 1px solid rgba(216, 216, 216, 0.1);
        min-width: 80px; /* 匹配 menu 宽度 */
        min-height: 29px; /* 匹配 menu 宽度 */
    }
    QMenu::item:selected {
        color: #8F8F8F;
    }
)");

    //方案名称
    lab_name = new QLabel(this);
    lab_name->setFixedSize(126, 20);          // 固定宽高
    lab_name->setStyleSheet(
        "QLabel {"
        "  color: #A1A8B3;"
        "  font-family: \"Noto Sans S Chinese\";"
        "  font-weight: 500;"
        "  font-size: 14px;"
        "}"
        );

    PlanDescription = new QWidget(this);;//放置方案描述的控件
    PlanDescription->setMaximumHeight(14);
    PlanDescription->setMinimumHeight(14);
    PlanDescription->setMinimumWidth(131);
    HLayout_description = new QHBoxLayout(PlanDescription);//水平布局
    //方案描述
    lab_description = new QLabel(this);
    lab_description->setStyleSheet(
        "QLabel {"
        "  color: #454D57;"
        "  font-family: \"Noto Sans S Chinese\";"
        "  font-weight: 500;"
        "  font-size: 10px;"
        "}"
        );
    HLayout_description->addWidget(lab_description);
    HLayout_description->setContentsMargins(0,0,0,0);
    // PlanDescription->show();


    //标签
    container = new QWidget(this);
    container->setFixedWidth(168);
    container->setMaximumHeight(20);
    container->setMinimumHeight(20);
    // container->setStyleSheet("background-color:red;");
    HLayout_label = new QHBoxLayout(container);//放置两个标签的水平布局

    lab1 = new AutoResizeLabel(container);//标签1
    lab1->setText("T10");
    lab1->setMinimumWidth(0);
    // lab1->setFixedHeight(20);//多机型时的高度
    lab1->setMaximumHeight(16);
    lab1->setMinimumHeight(16);
    // lab1->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    lab1->setAlignment(Qt::AlignCenter);


    lab2 = new AutoResizeLabel(container);//标签2
    lab2->setText(tr("三角洲行动"));
    lab2->setMinimumWidth(0);
    lab2->setMaximumHeight(16);
    lab2->setMinimumHeight(16);
    lab2->setAlignment(Qt::AlignCenter);
    //多机型时，字体宽度设置为300
    lab2->setStyleSheet("background-color:rgba(161, 168, 179, 0.2);"
                        "border-radius:8px;"
                        "color:#A1A8B3;"
                        "font-family: \"Noto Sans S Chinese\";"
                        "font-weight: 500;"
                        "font-size: 10px;");

    // 将标签添加到布局中（由布局负责摆位，但不改变父对象）
    // HLayout_label->addWidget(lab1);
    // HLayout_label->addWidget(lab2);
    HLayout_label->addWidget(lab1, 0, Qt::AlignBottom);
    HLayout_label->addWidget(lab2, 0, Qt::AlignBottom);
    HLayout_label->addStretch();
    HLayout_label->setContentsMargins(0,0,0,0);
    HLayout_label->setSpacing(6);
    container->show();


    // ---------- 创建多机型悬浮框 ----------
    m_tooltipWidget = new QWidget(nullptr);  // 独立顶层窗口
    m_tooltipWidget->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_tooltipWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_tooltipWidget->setStyleSheet(
        "QWidget {"
        "   border-image:none;"
        "   border: none;"
        "   background: transparent;"
        "   border-radius: 6px;"
        "}"
        );
    // 创建内部容器（用于显示背景和圆角）
    QFrame* contentFrame = new QFrame(m_tooltipWidget);
    contentFrame->setObjectName("TooltipContent");
    contentFrame->setStyleSheet(
        "#TooltipContent {"
        "   border-image:none;"
        "   background-color: #0D0F14;" // 深色背景
        "   border-radius: 6px;"        // 圆角
        "}"
        );


    // 内部布局：边距 8（上、左、右、下），间距 8
    m_tooltipLayout = new QVBoxLayout(contentFrame);
    m_tooltipLayout->setContentsMargins(8, 8, 8, 8);
    m_tooltipLayout->setSpacing(8);

    //创建外层布局，用于管理 contentFrame
    QVBoxLayout *outerLayout = new QVBoxLayout(m_tooltipWidget);
    outerLayout->setContentsMargins(0, 4, 12, 12); // 留出阴影空间
    outerLayout->addWidget(contentFrame);

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(contentFrame);
    shadow->setBlurRadius(8);// 模糊半径
    shadow->setXOffset(0);// X轴偏移
    shadow->setYOffset(4);// Y轴偏移（阴影向下）
    shadow->setColor(QColor(0, 0, 0, 128));
    // shadow->setColor(Qt::red);
    contentFrame->setGraphicsEffect(shadow);

    tip_des = new NewCustomToolTip(this);
    tip_des->setLabelStyle(2);

    m_tooltipWidget->hide();

    // 安装事件过滤器到 lab1
    lab1->installEventFilter(this);

}

//多机型悬浮显示
bool NewRadioBtn::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == lab1) && (lab_devs.size() > 1)) {
        if (event->type() == QEvent::Enter)
        {
            // 鼠标进入 lab1
            updateTooltip();  // 更新内容
            // 计算显示位置：lab1 下方左对齐
            QPoint pos = lab1->mapToGlobal(QPoint(0, lab1->height()));
            m_tooltipWidget->move(pos);
            m_tooltipWidget->show();
            return true;
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开 lab1
            m_tooltipWidget->hide();
            return true;
        }
    }
    return QRadioButton::eventFilter(watched, event);
}
// 更新机型内容
void NewRadioBtn::updateTooltip()
{
    // 清空旧控件
    QLayoutItem *item;
    while ((item = m_tooltipLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 根据 m_labDevs 创建 QLabel
    for (const QString &dev : lab_devs) {
        QLabel *label = new QLabel(dev, m_tooltipWidget);
        label->setFixedHeight(16);
        label->setStyleSheet(
            "QLabel {"
            "    background: rgba(161, 168, 179, 0.2);"
            "    border-radius: 8px;"
            "    padding: 2px 8px;"
            "    color: #A1A8B3;"
            "   font-family: \"Noto Sans S Chinese\";"
            "   font-weight: 300;"
            "   font-size: 10px;"
            "}"
            );
        label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        label->setWordWrap(false);    // 单行显示，或根据需求修改
        label->adjustSize();// 让label根据内容自适应宽度
        // m_tooltipLayout->addWidget(label);
        // 保持内容宽度
        m_tooltipLayout->addWidget(label, 0, Qt::AlignLeft);
    }
    // 布局自适应
    m_tooltipWidget->adjustSize();
}
// 获取关联的PlanVal
PlanVal NewRadioBtn::getAllPlanValue()
{
    return m_planValue;
}

// 更新PlanVal
void NewRadioBtn::updateAllPlanValue(const PlanVal& newVal)
{
    m_planValue = newVal;
}

void NewRadioBtn::resizeEvent(QResizeEvent *event)
{
    QRadioButton::resizeEvent(event);
    updateButtonPosition();  // 更新按钮位置
}
void NewRadioBtn::updateButtonPosition()
{
    // 放置在左下角（带2px边距）
    // int x = 2;
    // int y = height() - innerButton->height() - 2;
    // innerButton->move(x, y);
    /*//收藏 放置在右上角
    int x = width() - AllpBt_fav->width() - 15;  // 右边距
    //int y = height() - AllpBt_fav->height(); // 下边距
    int y = AllpBt_fav->height() -5; // 下边距2px
    AllpBt_fav->move(x, y);
    //更多 放置在右下角（带2px边距）
    x = width() - AllpBt_edit->width() - 13;  // 右边距
    y = height() - AllpBt_edit->height() - 19; // 下边距
    AllpBt_edit->move(x, y);*/

    //收藏 放置在右下角
    int x = width() - AllpBt_fav->width() - 45;  // 右边距
    int y = height() - AllpBt_fav->height() - 11; // 下边距
    AllpBt_fav->move(x, y);
    //更多 放置在右下角（带2px边距）
    x = width() - AllpBt_edit->width() - 17;  // 右边距
    y = height() - AllpBt_edit->height() - 11; // 下边距
    AllpBt_edit->move(x, y);

    //勾选框 放置在右下角（带2px边距）
    x = width() - AllpBt_check->width() - 17;  // 右边距
    y = height() - AllpBt_check->height() - 11; // 下边距
    AllpBt_check->move(x, y);



    //标签
    x = 12;//左边距
    y = height() - AllpBt_edit->height() - 11; // 下边距
    container->move(x,y);

    //方案名称，放置在文本上方
    x = 95; //左边距
    y = 18; // 上边距
    lab_name->move(x,y);
    //方案描述，放置在方案名称下方
    x = 95; //左边距
    y = 47; // 上边距
    PlanDescription->move(x,y);

}

/*void NewRadioBtn::updateElidedText(QString m_fullText) {
    QFontMetrics fm(font());
    // 计算可用宽度（按钮宽度 - 图标宽度 - 边距）
    const int availableWidth = width()/2 - iconSize().width() + 20;
    //const int availableWidth = width()-20;

    if (availableWidth > 0) {
        // 生成省略文本
        QString elidedText = fm.elidedText(m_fullText, Qt::ElideRight, availableWidth);
        QRadioButton::setText(elidedText);

        // 设置Tooltip（仅当文本被截断时显示）
        setToolTip(elidedText == m_fullText ? "" : m_fullText);
    } else {
        QRadioButton::setText(m_fullText);
    }
    setProperty("fullText",m_fullText);
}*/
//描述最多显示五个字符
void NewRadioBtn::updateElidedText(QString m_fullText, QString PlanName)
{
    // 1. 设置方案名称
    lab_name->setText(PlanName);

    // 2. 根据字符数截断
    QString displayText = m_fullText;
    if (m_fullText.length() > 5) {
        displayText = m_fullText.left(5) + QStringLiteral("...");
    }

    // 3. 显示截断后的文本
    lab_description->setText(displayText);
    lab_description->show();

    // 4. 设置提示与属性
    // setToolTip(displayText == m_fullText ? QString() : m_fullText);
    tip_des->AddToolTip(PlanDescription,
                        displayText == m_fullText ? QString() : m_fullText,
                        Qt::AlignCenter);

    setProperty("fullText", m_fullText);
}

/*//描述最多显示2行
void NewRadioBtn::updateElidedText(QString m_fullText,QString PlanName)
{
    lab_name->setText(PlanName);
    QFontMetrics fm(font());
    // const int availableWidth = width() / 2 - iconSize().width() + 20;
    const int availableWidth = width() - iconSize().width() - 70;

    if (availableWidth <= 0) {
        // QRadioButton::setText(m_fullText);
        lab_description->setText(m_fullText);
        setProperty("fullText", m_fullText);
        return;
    }

    QString result;
    QString remaining = m_fullText;
    const int maxLines = 2;          // 最多显示一行
    int lineCount = 0;

    while (lineCount < maxLines && !remaining.isEmpty()) {
        if (lineCount == maxLines - 1) { // 最后一行
            // 判断是否需要省略：文本宽超出可用宽度 或 文本本身需要换行（即还有后续内容）
            bool needElide = false;
            if (fm.size(Qt::TextSingleLine, remaining).width() > availableWidth)
                needElide = true;
            else {
                // 即使单行能放下，也要检查 remaining 是否真就是全部剩余内容
                // 若前面已经取走了一些文本，则 remaining 只是剩余部分，
                // 这里 remaining 就是第三行及之后的全部文本，若能完全放下则无省略
                // （不需要额外判断）
            }

            if (needElide) {
                QString elided = fm.elidedText(remaining, Qt::ElideRight, availableWidth);
                result += elided;
            } else {
                result += remaining;
            }
            break;
        } else {
            // 普通行（第一、二行）：按字符宽度直接换行
            int charCount = 0;
            int currentWidth = 0;
            while (charCount < remaining.length()) {
                QChar ch = remaining.at(charCount);
                if (ch == QLatin1Char('\n')) {
                    charCount++;  // 吞掉换行符，本行结束
                    break;
                }
                int charWidth = fm.horizontalAdvance(ch);
                if (currentWidth + charWidth > availableWidth)
                    break;
                currentWidth += charWidth;
                charCount++;
            }
            if (charCount == 0) {
                // 连一个字符都放不下，强制至少取一个字符，防止死循环
                charCount = 1;
            }
            result += remaining.left(charCount);
            remaining = remaining.mid(charCount);
            lineCount++;
            if (lineCount < maxLines && !remaining.isEmpty())
                result += QLatin1Char('\n');
        }
    }

    //QRadioButton::setText(result);
    lab_description->show();
    lab_description->setText(result);
    setToolTip(result == m_fullText ? QString() : m_fullText);
    setProperty("fullText", m_fullText);
}
*/
//收藏
void NewRadioBtn::setIsAddedEn( bool en, int idx)
{
    IsAdded = en;//是否已收藏
    favIdx = idx;
}
void NewRadioBtn::setIsLoad(bool en)
{
    IsLoad = en;

}
//设置标签2，根据标签2设置图标
void NewRadioBtn::setLabel2(const QString &label2)
{
    lab2->setText(label2);

    // 1. 定义标签到图片基础名的映射（不区分大小写，统一用小写）
    static const QHash<QString, QString> nameMap = {
        { "游戏",      "game" },
        { "电影",      "movie" },
        { "音乐",      "music" },
        { "三角洲行动", "delta" },
        { "pubg",     "pubg" },      // 英文原样
        { "csgo",     "csgo" },
        { "无畏契约",   "valorant" },
        { "暗区突围",   "AB" },
        { "apex",     "apex" },
        { "穿越火线",   "CF" }
    };

    QString baseName = nameMap.value(label2.toLower(), "default"); // 找不到时用 "default"
    if(baseName == "default")
    {
        return;
    }

    // 系统方案加 "sys-"，否则空
    QString prefix = IsSys ? "sys-" : "";

    // 公共样式
    static const QString commonStyle = QStringLiteral(
        "QRadioButton {"
        "  border-radius: 4px;"
        "  background-color: rgba(0, 0, 0, 20%);"
        "  color: rgb(255, 255, 255);"
        "}"
        "QRadioButton:checked {"
        "  border: 1.5px solid #009FEF;"
        "}"
        "QRadioButton:focus { outline: none; }"
        );

    static const QString indicatorStyle = QStringLiteral(
        "QRadioButton::indicator {"
        "  height: 71px; width: 71px;"
        "  image: url(:/Skin/Images/Headphones/edit/%1%2-no.png);"
        "  background-color: transparent;"
        "  margin: 12px 12px 43px 12px;"
        "}"
        "QRadioButton::indicator:checked {"
        "  height: 71px; width: 71px;"
        "  image: url(:/Skin/Images/Headphones/edit/%1%2-ch.png);"
        "  background-color: transparent;"
        "  margin: 12px 12px 43px 12px;"
        "}"
        );

    //合成样式并设置
    setStyleSheet(commonStyle + indicatorStyle.arg(prefix, baseName));
}

//复制
void NewRadioBtn::setStyle(bool IsAdded)
{
    if(IsAdded)
    {

    }else
    {

    }
}

int NewRadioBtn::GetPlanPageSel()
{
    return PlanPageSel;
}

bool NewRadioBtn::GetDataVisibleEn()
{
    return m_planValue.DataVisibleEn;
}


// 显示 tooltip
bool NewRadioBtn::event(QEvent *event) {
    // if (event->type() == QEvent::ToolTip) {
    //     QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
    //     QString tooltipText = toolTip(); // 获取要显示的文本
    //     if (!tooltipText.isEmpty()) {
    //         // 如果已有自定义tooltip，先关闭
    //         if (m_customTooltip) {
    //             m_customTooltip->close();
    //             m_customTooltip->deleteLater();
    //             m_customTooltip = nullptr;
    //         }
    //         // 创建自定义 tooltip 窗口
    //         QLabel *customTooltip = new QLabel(tooltipText);
    //         customTooltip->setFixedWidth(166);
    //         customTooltip->setMinimumHeight(12);
    //         customTooltip->setWordWrap(true);                 //自动换行
    //         customTooltip->setStyleSheet(
    //             "QLabel {"
    //             "   background-color: #0D0F14;"
    //             "   color: #454D57;"
    //             "   border: none;"
    //             "   padding: 6px 8px;"
    //             "   border-radius: 6px;"
    //             "  font-family: \"Noto Sans S Chinese\";"
    //             "  font-weight: 500;"
    //             "  font-size: 10px;"
    //             "}"
    //             );
    //         customTooltip->setWindowFlags(Qt::ToolTip);
    //         customTooltip->setAttribute(Qt::WA_StyledBackground, true);

    //         int idealHeight = customTooltip->heightForWidth(166);
    //         customTooltip->resize(166, idealHeight);

    //         QPoint pos = helpEvent->globalPos();
    //         customTooltip->move(pos.x(), pos.y() + 20); // 偏移显示
    //         customTooltip->show();
    //         m_customTooltip = customTooltip; // 保存指针

    //         // 可选：设置定时器作为后备（鼠标离开时也会关闭，所以可以不用）
    //     }
    //     return true; // 阻止默认 tooltip 显示
    // }
    return QRadioButton::event(event);
}

void NewRadioBtn::leaveEvent(QEvent *event)
{
    // 鼠标离开控件，关闭自定义tooltip
    if (m_customTooltip) {
        m_customTooltip->close();
        m_customTooltip->deleteLater();
        m_customTooltip = nullptr;
    }
    QRadioButton::leaveEvent(event);
}

//设置标签1的样式
void NewRadioBtn::setLab1Style(QString DeviceName)
{
    static const QHash<QString, QString> nameMap = {
        { "T10有线",      "T10" },
        { "T10无线",      "T10Wireless" },
        { "K03S",      "K03S" },
        { "K03S超竞版", "K03SSupper" },
        { "K06S",     "K06S" },
        { "T7",     "T7" },
        { "T7 GT",     "T7 GT" },
        { "S21无线智充版", "S21Wireless_7_1" }
    };

    QString baseName = nameMap.value(DeviceName, "default"); // 找不到时用 "default"
    if(baseName == "default")
    {
        return;
    }

    if(lab_devs.size() <=1)
    {

        lab1->setStyleSheet("background-color:rgba(161, 168, 179, 0.2);"
            "border-radius:8px;"
            "color:#A1A8B3;"
            "font-family: \"Noto Sans S Chinese\";"
            "font-weight: 500;"
            "font-size: 10px;");

        {
            //多机型时设置的样式表
            // lab1->setStyleSheet(
            //     QString("background-image: url(:/Skin/Images/Headphones/Label/one-%1.png);"
            //             "background-repeat: no-repeat;"
            //             "background-position: bottom center;"
            //             "background-color: transparent;"
            //             "color: transparent;")
            //         .arg(baseName)
            //     );
        }


    }else
    {

        // lab1->setFixedHeight(20);
        // lab1->setFixedSize(75,20);
        static const QString indicatorStyle = QStringLiteral("QLabel{"
                                                             "background:transparent;"
                                                             "image: url(:/Skin/Images/Headphones/Label/more-%1.png);"
                                                             "color:transparent;}"
                                                             "QLabel::hover{"
                                                             "image: url(:/Skin/Images/Headphones/Label/one-%1.png);"
                                                             "background-repeat: no-repeat;"
                                                             "background-position: center;}");
        lab1->setStyleSheet(indicatorStyle.arg(baseName));

    }
}
// 添加单个设备
void NewRadioBtn::setLabDevsOne(const QString& dev)
{
    lab_devs.clear();
    lab_devs << dev;
}
// 设置设备列表
void NewRadioBtn::setLabDevs(const QStringList& devs)
{
    lab_devs = devs;
}

// 获取设备列表
const QStringList& NewRadioBtn::getLabDevs() const
{
    return lab_devs;
}
