#include "EightMyPlan.h"
#include "ui_EightMyPlan.h"
#include "LoadLib.h"
#include "APOThread/ApoManager.h"
#include<QMouseEvent>
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots

EightMyPlan::EightMyPlan(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EightMyPlan)
{
    ui->setupUi(this);

    m_lastEmittedOrder = {0,1,2,3,4,5,6,7}; // 初始顺序

    buttonList.clear();
    buttonList.append(ui->pBt_plan1);
    buttonList.append(ui->pBt_plan2);
    buttonList.append(ui->pBt_plan3);
    buttonList.append(ui->pBt_plan4);
    buttonList.append(ui->pBt_plan5);
    buttonList.append(ui->pBt_plan6);
    buttonList.append(ui->pBt_plan7);
    buttonList.append(ui->pBt_plan8);

    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->pBt_plan1,0);
    buttonGroup->addButton(ui->pBt_plan2,1);
    buttonGroup->addButton(ui->pBt_plan3,2);
    buttonGroup->addButton(ui->pBt_plan4,3);
    buttonGroup->addButton(ui->pBt_plan5,4);
    buttonGroup->addButton(ui->pBt_plan6,5);
    buttonGroup->addButton(ui->pBt_plan7,6);
    buttonGroup->addButton(ui->pBt_plan8,7);

    // 启用拖拽接受
    setAcceptDrops(true);
    gridLayout = qobject_cast<QGridLayout*>(this->layout());


    //注册监听对象（事件过滤器）
    ui->pBt_plan1->installEventFilter(this);
    ui->pBt_plan2->installEventFilter(this);
    ui->pBt_plan3->installEventFilter(this);
    ui->pBt_plan4->installEventFilter(this);
    ui->pBt_plan5->installEventFilter(this);
    ui->pBt_plan6->installEventFilter(this);
    ui->pBt_plan7->installEventFilter(this);
    ui->pBt_plan8->installEventFilter(this);

}

EightMyPlan::~EightMyPlan()
{
    delete ui;
}

bool EightMyPlan::eventFilter(QObject *obj, QEvent *event)
{
    int idx = 0;
    for (int i = 0; i < buttonList.size(); ++i)
    {
        if (obj == buttonList[i])
        {
            // 添加 enabled 状态检查
            if (!buttonList[i]->isEnabled())
            {
                return false;
            }

            if (event->type() == QEvent::MouseButtonPress)
            {

                bool quitFav = false;
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                // 定义目标区域（以控件自身的坐标系）,若点击在目标区域（左上角）则取消收藏
                const QRectF targetArea(7.6, 7.6, 12.8, 12.8);

                // 获取点击位置（转换为控件本地坐标系）
                //QPointF localPos = buttonList[i]->mapFromParent(mouseEvent->pos());  // 如果是父组件的事件过滤
                QPointF localPos = mouseEvent->localPos();  // 如果不是在自定义组件中
                qDebug() << "点击位置在目标区域! (" << localPos.x() << "," << localPos.y() << ")";

                // 判断是否在目标区域内
                if (targetArea.contains(localPos)) {
                    qDebug() << "点击位置在目标区域! (" << localPos.x() << "," << localPos.y() << ")";
                    // 在这里处理特定区域点击逻辑
                    quitFav = true;

                }

                if(!quitFav)
                {
                    //buttonList[i]->setChecked(!buttonList[i]->isChecked());
                    buttonList[i]->setChecked(true);
                }

                /*if (buttonList[i]->isChecked()) {
                    // 已经选中了，再次点击则取消选中
                    // 临时关闭互斥模式，才能取消选中
                    buttonGroup->setExclusive(false);
                    buttonList[i]->setChecked(false);
                    buttonGroup->setExclusive(true);  // 立即恢复互斥模式
                }else
                {
                    buttonList[i]->setChecked(true);
                }*/

                qDebug() << "八个按钮点中了"<< i <<buttonList[i]->property("fullText");
                //点击八个收藏应进入对应的EQ界面

                for(int j = 0; j < 8; j++)
                {
                    if((EightFavPlan[j].PName == buttonList[i]->property("fullText")) && (EightFavPlan[j].PlanMode == buttonList[i]->property("PlanMode")))
                    {

                        idx = j;
                        break;
                    }
                }

                QString key = EightFavPlan[idx].PName;
                {
                    //分机型时
                    // const QStringList& devs = EightFavPlan[idx].label_Devs;
                    // for (const QString& dev : devs)
                }
                //不分机型时
                QString dev = QString();
                {

                    NewRadioBtn* btn = MovieVal.AllPlanRadioHash.value(qMakePair(key, dev), nullptr);
                    if (btn) {
                        if (!quitFav) {
                            // 对所有匹配的按钮都设为选中
                            btn->setChecked(true);
                        } else {
                            // 只要有一个匹配，就取消选中它的收藏状态并立刻返回
                            btn->AllpBt_fav->setChecked(false);
                            return true;
                        }
                    }
                }


                // PlanCheckedUpdate(idx);//收藏点亮状态同步
                QTimer::singleShot(0, this, [this, i]() {
                    emit FavToEq();
                    PlanCheckedUpdate(i);//收藏点亮状态同步
                });

                // // //QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                // if (mouseEvent->button() == Qt::LeftButton)
                // {
                //     // 记录拖动开始状态
                //     m_dragStartPos = mouseEvent->globalPos();
                //     m_dragIndex = i;
                //     m_dragging = true;

                //     // 计算鼠标在按钮内部的偏移量
                //     m_dragOffset = mouseEvent->pos();

                //     // 获取被拖动的按钮
                //     m_draggedButton = buttonList[i];

                //     // 创建占位符并添加到布局
                //     createPlaceholder(i);

                //     // 提升按钮层级使其显示在最前面
                //     m_draggedButton->raise();

                //     return true;
                // }
                if (mouseEvent->button() == Qt::LeftButton) {
                    // 只需记录起始信息，不开始拖动
                    m_potentialDrag = true;
                    m_pressGlobalPos = mouseEvent->globalPos();
                    m_dragOffset = mouseEvent->pos();
                    m_dragIndex = i;                     // 记录当前按钮索引
                    m_draggedButton = buttonList[i];     // 记录被点击的按钮
                    return true;
                }
            }
            else if (event->type() == QEvent::MouseMove) {
                // 只有当候选状态存在，且左键处于按下状态时才可能开始拖动
                if (m_potentialDrag) {
                    // 如果没有获得拖动按钮或按钮被禁用，重置状态
                    if (!m_draggedButton || !m_draggedButton->isEnabled()) {
                        m_potentialDrag = false;
                        return false;
                    }

                    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                    QPoint delta = mouseEvent->globalPos() - m_pressGlobalPos;

                    // 判断移动距离是否达到系统定义的拖动阈值
                    if (!m_dragging && delta.manhattanLength() >= QApplication::startDragDistance()) {
                        // 真正开始拖动
                        m_dragging = true;

                        // 创建占位符
                        createPlaceholder(m_dragIndex);
                        // 提升按钮层级使其显示在最前面
                        m_draggedButton->raise();
                    }

                    if (m_dragging) {
                        // 已经在拖动状态，处理按钮跟随与占位符切换
                        QPoint newPos = mouseEvent->globalPos() - m_dragOffset;
                        m_draggedButton->move(mapFromGlobal(newPos));

                        QPoint containerPos = this->mapFromGlobal(mouseEvent->globalPos());
                        int buttonWidth = this->width() / buttonList.size();
                        int toIndex = qBound(0, containerPos.x() / buttonWidth, buttonList.size() - 1);

                        if (toIndex != m_dragIndex && buttonList[toIndex]->isEnabled()) {
                            movePlaceholder(m_dragIndex, toIndex);
                            m_dragIndex = toIndex;
                        }
                        return true;
                    }
                }
            }
            else if (event->type() == QEvent::MouseButtonRelease) {
                if (m_potentialDrag) {
                    // 如果之前已经处于拖动状态，完成拖动
                    if (m_dragging) {
                        if (m_draggedButton && m_draggedButton->isEnabled()) {
                            completeDrag();
                            updateAllFavIndices();
                        }
                    }
                    // 重置所有拖动相关状态
                    m_potentialDrag = false;
                    m_dragging = false;
                    m_draggedButton = nullptr;
                    return true;
                }
            }
            // else if (event->type() == QEvent::MouseMove && m_dragging)
            // {
            //     // 确保当前拖动的按钮是 enabled 状态
            //     if (!m_draggedButton || !m_draggedButton->isEnabled()) {
            //         m_dragging = false;
            //         return false;
            //     }
            //     QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

            //     // 移动被拖动的按钮
            //     QPoint newPos = mouseEvent->globalPos() - m_dragOffset;
            //     m_draggedButton->move(mapFromGlobal(newPos));

            //     // 计算目标位置索引
            //     QPoint globalPos = mouseEvent->globalPos();
            //     QPoint containerPos = this->mapFromGlobal(globalPos);
            //     int buttonWidth = this->width() / buttonList.size();
            //     int toIndex = qBound(0, containerPos.x() / buttonWidth, buttonList.size() - 1);

            //     // 如果位置发生变化，更新占位符位置
            //     if (toIndex != m_dragIndex) {
            //         // 确保目标位置的按钮也是 enabled 状态
            //         if (buttonList[toIndex]->isEnabled()) {
            //             movePlaceholder(m_dragIndex, toIndex);
            //             m_dragIndex = toIndex;
            //         }
            //     }

            //     return true;
            // }
            // else if (event->type() == QEvent::MouseButtonRelease)
            // {
            //     if (m_dragging) {
            //         // 确保当前拖动的按钮是 enabled 状态
            //         if (!m_draggedButton || !m_draggedButton->isEnabled()) {
            //             m_dragging = false;
            //             return false;
            //         }
            //         // QString key = EightFavPlan[m_dragIndex].PName;//idx
            //         // NewRadioBtn* btn = AllPlanRadioHash.value(key, nullptr);
            //         // if (btn) {

            //         //     btn->favIdx = m_dragIndex;
            //         // }


            //         // 完成拖动，将按钮放回布局
            //         completeDrag();

            //         // 更新所有按钮的 favIdx 为新的顺序索引
            //         updateAllFavIndices();

            //         return true;
            //     }
            // }
        }
    }

    return QWidget::eventFilter(obj, event);
}

// 创建占位符
void EightMyPlan::createPlaceholder(int index)
{
    // 移除占位符（如果已存在）
    if (m_placeholder) {
        gridLayout->removeWidget(m_placeholder);
        delete m_placeholder;
    }

    // 创建新的占位符
    m_placeholder = new QWidget(this);
    m_placeholder->setFixedSize(buttonList[index]->size());
    m_placeholder->setStyleSheet("background-color: transparent;");

    // 将占位符添加到布局
    int row, col, rowSpan, colSpan;
    gridLayout->getItemPosition(gridLayout->indexOf(buttonList[index]),
                                &row, &col, &rowSpan, &colSpan);

    gridLayout->addWidget(m_placeholder, row, col, rowSpan, colSpan);
}

// 移动占位符
void EightMyPlan::movePlaceholder(int fromIndex, int toIndex)
{
    if (!m_placeholder) return;

    // 从布局中移除占位符
    gridLayout->removeWidget(m_placeholder);

    // 获取目标位置信息
    int row, col, rowSpan, colSpan;
    gridLayout->getItemPosition(gridLayout->indexOf(buttonList[toIndex]),
                                &row, &col, &rowSpan, &colSpan);

    // 将占位符添加到新位置
    gridLayout->addWidget(m_placeholder, row, col, rowSpan, colSpan);

    // 更新按钮列表顺序
    if (fromIndex != toIndex) {
        QPushButton* button = buttonList.takeAt(fromIndex);
        buttonList.insert(toIndex, button);

        // 同时交换EightFavPlan中对应位置的数据
        // 注意：只交换已经收藏的项，即索引小于EightFavPlanIndex的项
        if (fromIndex < EightFavPlanIndex && toIndex < EightFavPlanIndex) {
            std::swap(EightFavPlan[fromIndex], EightFavPlan[toIndex]);
        }
        // 更新按钮的显示文本
        TruncateText(EightFavPlan[fromIndex].PName, buttonList[fromIndex],EightFavPlan[fromIndex].PlanMode);
        TruncateText(EightFavPlan[toIndex].PName, buttonList[toIndex],EightFavPlan[toIndex].PlanMode);

        // 更新布局中实际按钮的位置
        updateAllButtonsLayout();
    }
}

// 完成拖动
void EightMyPlan::completeDrag()
{
    if (!m_dragging) return;

    // 将按钮放回布局、更新布局
    updateAllButtonsLayout();

    // 清理占位符
    delete m_placeholder;
    m_placeholder = nullptr;


    // === 检查顺序是否真的改变了 ===
    QList<int> currentOrder;
    for (auto btn : buttonList) {
        currentOrder << buttonGroup->id(btn);
    }

    // 避免发送未变化的信号（减少无效同步）
    if (m_lastEmittedOrder != currentOrder) {
        // updateLayoutFromOrder(currentOrder);
        emit layoutChanged(currentOrder);  // 通知外部按钮顺序已变
        m_lastEmittedOrder = currentOrder;
    }

    // 重置状态
    m_dragging = false;
    m_dragIndex = -1;
    m_draggedButton = nullptr;
}

void EightMyPlan::updateAllFavIndices()
{

    // QString key = EightFavPlan[m_dragIndex].PName;//idx
    // NewRadioBtn* btn = AllPlanRadioHash.value(key, nullptr);
    // if (btn) {

    //     btn->favIdx = m_dragIndex;
    // }
    for(int i = 0;i < 8; i++)
    {

        QString key = EightFavPlan[i].PName;
        {
           //分机型时
            // const auto& devs = EightFavPlan[i].label_Devs;  // 假设是 QStringList
            // for (const QString& dev : devs)
        }
        //不分机型时
        QString dev = QString();
        {
            if (dev.isEmpty())
                continue;

            NewRadioBtn* btn = MovieVal.AllPlanRadioHash.value(qMakePair(key, dev), nullptr);
            if (btn) {

                btn->favIdx = i;
            }
        }
    }
}


// 更新布局的函数（不破坏其他部件）
//void EightMyPlan::updateAllButtonsLayout(QGridLayout *gLayout,QList<QPushButton*> btnList)
void EightMyPlan::updateAllButtonsLayout()
{
    QGridLayout *gridLayout = qobject_cast<QGridLayout*>(this->layout());
    if (!gridLayout) return;

    // 1. 找出布局中按钮的起始列（跳过前面的QLabel等部件）
    int startCol = -1,rowEnd = -1;
    int rowSpan1 = -1, colSpan1 = -1;
    for (int i = 0; i < gridLayout->count(); i++) {
        QWidget *widget = gridLayout->itemAt(i)->widget();
        if (buttonList.contains(static_cast<QPushButton*>(widget))) {
            int row, col, rowSpan, colSpan;
            gridLayout->getItemPosition(i, &row, &col, &rowSpan, &colSpan);
            startCol = col;
            rowEnd = row;
            rowSpan1 = rowSpan;
            colSpan1 = colSpan;
            break;
        }
    }
    if (startCol == -1) return; // 没有找到按钮

    // 2. 暂时移除所有按钮（不删除）
    QList<QPushButton*> tempList = buttonList; // 保存按钮指针
    for (QPushButton *button : tempList) {
        gridLayout->removeWidget(button);
    }

    // 3. 按新顺序重新添加按钮（连续布局）
    for (int i = 0; i < tempList.size(); i++) {
        gridLayout->addWidget(tempList[i], rowEnd, startCol + i,rowSpan1,colSpan1);
    }
}
// 接收同步指令：根据新的顺序重排按钮
void EightMyPlan::updateLayoutFromOrder(const QList<int>& newOrder)
{
    if (newOrder.size() != buttonList.size()) return;

    QList<QPushButton*> reordered;
    for (int id : newOrder) {
        QPushButton *btn = qobject_cast<QPushButton*>(buttonGroup->button(id));
        if (btn && buttonList.contains(btn)) {
            reordered.append(btn);
        }
    }

    if (reordered.size() == buttonList.size()) {
        buttonList = reordered;
        updateAllButtonsLayout();
    }
}

void EightMyPlan::updateChecked(int id)
{
    QPushButton *btn = buttonList[id];//qobject_cast<QPushButton*>(buttonGroup->button(id));
    if(btn)
    {
        //emit PlanSave_F();
        btn->setChecked(true);
    }
}
//方案库中，若被选中方案在八个被收藏中，则同步被选中
void EightMyPlan::PlanCheckedUpdate(int id)
{
    QPushButton *btn = buttonList[id];//qobject_cast<QPushButton*>(buttonGroup->button(id));
    if(btn)
    {
        btn->setChecked(true);
        emit btnCheckedChanged(id);//使方案库页面，均衡器页面，试听页面，八个收藏一致
    }
}
//取消所有收藏按钮的选中状态，不通知其他页面
void EightMyPlan::AllBtnDisChecked()
{
    // 临时关闭互斥模式，才能取消选中
    QAbstractButton *checkedBtn = buttonGroup->checkedButton();
    if (checkedBtn) {
        buttonGroup->setExclusive(false);  // 暂时允许非互斥操作
        checkedBtn->setChecked(false);     // 取消选中
        buttonGroup->setExclusive(true);   // 恢复互斥
    }
}
//取消所有收藏按钮的选中状态，且通知其他页面
void EightMyPlan::AllDisChecked()
{
    // 临时关闭互斥模式，才能取消选中
    QAbstractButton *checkedBtn = buttonGroup->checkedButton();
    if (checkedBtn) {
        buttonGroup->setExclusive(false);  // 暂时允许非互斥操作
        checkedBtn->setChecked(false);     // 取消选中
        buttonGroup->setExclusive(true);   // 恢复互斥

        emit btnAllDisChecked();
    }

}

//显示八个我的收藏，并显示对应图标
void EightMyPlan::ShowEightFavorite(bool signalEn)
{
    for(int i = 0;i < 8; i++)
    {
        if(i < EightFavPlanIndex)
        {
            emit ApoManager::instance()->requestlogWithTime(QString("显示收藏按钮名称：%1,%2").arg(i).arg(EightFavPlan[i].PName));
            TruncateText(EightFavPlan[i].PName,buttonList[i],EightFavPlan[i].PlanMode);
            QString txt = EightFavPlan[i].label_Scene;


            // 建立大小写不敏感映射（key 全部用小写）
            static const QHash<QString, QString> imageMap = {
                { "游戏",      "game" },
                { "电影",      "movie" },
                { "音乐",      "music" },
                { "三角洲行动", "delta" },
                { "pubg",     "pubg" },
                { "csgo",     "csgo" },
                { "无畏契约",   "valorant" },
                { "暗区突围",   "AB" },
                { "apex",     "apex" },
                { "穿越火线",   "CF" }
            };

            QString baseName = imageMap.value(txt.toLower(), "default"); // 找不到时用 "default"

            // 公共样式模板，使用 %1 替换图片基础名
            QString style
                = QString("QPushButton {"
                          "  border: none;"
                          "  color: #A1A8B3;"
                          "  text-align: left;"
                          "  padding-bottom: -35px;" /* 根据按钮高度调整此值 */
                          "  padding-left: 6px;"
                          "  border-image: url(:/Skin/Images/Headphones/Fav/%1-no.png);"
                          "  font-family: \"Noto Sans S Chinese\";"
                          "  font-weight: 500;"
                          "  font-size: 10px;"
                          "}"
                          "QPushButton:checked {"
                          "  border-image: url(:/Skin/Images/Headphones/Fav/%1-ch.png);"
                          "}"
                          "QPushButton:hover:!checked {"
                          "  border-image: url(:/Skin/Images/Headphones/Fav/%1-ho.png);"
                          "}"
                          "QPushButton:disabled {"
                          "  border-image: url(:/Skin/Images/Headphones/current-dis.png);"
                          "}")
                      .arg(baseName);

            buttonList[i]->setStyleSheet(style);
            buttonList[i]->setEnabled(true);

        }else
        {
            buttonList[i]->setText("");
            buttonList[i]->setEnabled(false);
        }
    }
    if(signalEn)
    {
        updateAllFavIndices();
        emit ShowBtn(false);
    }

}

void EightMyPlan::Rename(int idx)
{
    QPushButton *btn = buttonList[idx];//qobject_cast<QPushButton*>(buttonGroup->button(idx));
    TruncateText(EightFavPlan[idx].PName,btn,EightFavPlan[idx].PlanMode);

    emit RenameBtn(idx);
}

void EightMyPlan::TruncateText(QString text,QPushButton *pbt,int PlanMode)
{
    QString originalText = text;
    QString fullText = text;
    // 获取按钮的字体度量
    QFontMetrics fm(pbt->font());
    // 获取按钮的可用宽度（减去1是为了避免边界问题
    const int availableWidth = 51;//pbt->width()/2 + 10;
    // 计算省略号宽度
    // int ellipsisWidth = fm.horizontalAdvance("…");

    // 省略号处理（ASCII "..."，基线对齐）
    QString elidedText = DeSheng::elideTextWithDots(text, pbt->font(), availableWidth);


    emit ApoManager::instance()->requestlogWithTime(QString("收藏按钮名称：%1,%2").arg(elidedText).arg(text));

    // 设置按钮文本和属性
    pbt->setText(elidedText);
    pbt->setProperty("fullText", text);

    pbt->setProperty("PlanMode",PlanMode);//0:所有预设   1：我的预设


    // 仅在被截断时显示tooltip
    pbt->setToolTip(elidedText == text ? "" : text);
}

