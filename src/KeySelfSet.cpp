#include "KeySelfSet.h"
#include "ui_KeySelfSet.h"
#include "LoadLib.h"
#include <QListView>
#include <array>

#include <QStandardItemModel>

QLabel *lab_BeepShadow_Top;//提示音关闭时，覆盖ui->widget_top 的阴影
QLabel *lab_BeepShadow_Buttom;//
int KeyIdx = 3;

KeySelfSet::KeySelfSet(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KeySelfSet)
{
    ui->setupUi(this);

    ui->hSlider_level->setType(1,8,4,false,false);


    // 封装创建自定义 ListView 的函数
    auto createListView = []() {
        QListView* view = new QListView();
        view->setStyleSheet(R"(
                                    QListView{font-family: "Noto Sans S Chinese";
                font-weight: 500;font-size: 14px;margin-top:6px;border-radius: 2px;padding-left: 17px;padding-right: 17px;color: #FFFFFF;outline: 0;selection-background-color: transparent !important;;show-decoration-selected: 0 !important;;}
                                    QListView::item {height: 31px;padding-right: 20px;border-bottom: 1px solid rgba(216, 216, 216, 0.1);}
                                    /*下方下拉列表项选中项的样式*/
                                    QListView::item:selected{background-color: transparent;background-image: url(:/Skin/Images/cBox/item_se.png);background-repeat: no-repeat;background-position:right center;}
                                   /*下方下拉列表项鼠标悬停的样式*/
                                    QListView::item:hover
                                    {
                                        background-color: transparent;
                                    }
                                    QListView::item:focus {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                                    QListView::item:disabled {
                                        color: gray;
                                    }
                                )");
        return view;
    };
    QString styleSheet = (R"(QComboBox{border-radius: 2px;combobox-popup: 0;border-image: url(:/Skin/Images/cBox/dropdownCollapse_bk.png);padding-left: 10px; color: rgb(255, 255, 255);icon: url(:/image/Headphones/AllEdit/add.png);}
                                        QComboBox::drop-down{border-image: url(:/Skin/Images/cBox/droptriangle_no.png);margin-top:0px;subcontrol-origin: padding;subcontrol-position: center right; margin-right:10px;height:8px;width:5px;}
                                        QComboBox::drop-down:checked{border-image: url(:/Skin/Images/cBox/droptriangle_se.png);margin-top:0px;margin-right:10px;subcontrol-origin: padding;subcontrol-position: center right;height:5px;width:8px;}
                                        )"
                          );

    // 所有需要设置自定义视图的 ComboBox 指针列表
    std::array<QComboBox*, 7> comboBoxes = {
        ui->cBox_muteKey,
        ui->cBox_muteAct,
        ui->cBox_playKey,
        ui->cBox_playAct,
        ui->cBox_EQKey,
        ui->cBox_EQAct,
        ui->cBox_beepLanguage
    };

    // 批量设置独立的 ListView 视图
    for (QComboBox* cb : comboBoxes) {
        cb->setStyleSheet(styleSheet);
        cb->setView(createListView());
        K_SetCBoxShadow(cb);
    }


    //按键1：麦克风键(idx:0)    按键2：电源键(idx:1)    按键3：音量＋键(idx:2)

    //K03S只存在按键1与按键2
    ui->cBox_muteKey->removeItem(2);
    ui->cBox_playKey->removeItem(2);
    ui->cBox_EQKey->removeItem(2);
    KeyIdx = 2;

    //1.播放暂停默认操作：单击电源键(0 1)  2.麦克风开关默认操作： 单击麦克风键(0 0)  3.EQ 切换：默认三击电源键(2 1)
    ui->cBox_playKey->blockSignals(true);
    ui->cBox_playKey->setCurrentIndex(1);
    ui->cBox_playKey->blockSignals(false);

    ui->cBox_playAct->blockSignals(true);
    ui->cBox_playAct->setCurrentIndex(0);
    ui->cBox_playAct->blockSignals(false);

    ui->cBox_muteKey->blockSignals(true);
    ui->cBox_muteKey->setCurrentIndex(0);
    ui->cBox_muteKey->blockSignals(false);

    ui->cBox_muteAct->blockSignals(true);
    ui->cBox_muteAct->setCurrentIndex(0);
    ui->cBox_muteAct->blockSignals(false);

    ui->cBox_EQKey->blockSignals(true);
    ui->cBox_EQKey->setCurrentIndex(1);
    ui->cBox_EQKey->blockSignals(false);

    ui->cBox_EQAct->blockSignals(true);
    ui->cBox_EQAct->setCurrentIndex(2);
    ui->cBox_EQAct->blockSignals(false);

    //禁用个别项，三种按键自定义不能一致，自定义角色存储 -1 表示可用，0 表示禁用
    EnAllCurrentActIdx();
    ui->cBox_playAct->setItemData(2, QVariant(0), Qt::UserRole - 1);
    ui->cBox_EQAct->setItemData(0, QVariant(0), Qt::UserRole - 1);

    updateAllStates();
    // updateItemsEnable(ui->cBox_playAct);
    // updateItemsEnable(ui->cBox_EQAct);

}

KeySelfSet::~KeySelfSet()
{
    delete ui;
}
void KeySelfSet::K_SetCBoxShadow(QComboBox *cBox)
{
    QWidget* containerObj = cBox->view()->parentWidget();
    if (containerObj) {
        containerObj->setWindowFlags(containerObj->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        //containerObj->setAttribute(Qt::WA_TranslucentBackground);
        containerObj->setStyleSheet("background-color: #313A48;border: 1px solid #1A1A1A;");
        // 创建阴影效果
        QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(containerObj);
        shadowEffect->setBlurRadius(8);  // 阴影模糊半径
        shadowEffect->setColor(QColor(0, 0, 0, 179));  // 颜色
        shadowEffect->setOffset(0, 0);  // 阴影偏移

        containerObj->setGraphicsEffect(shadowEffect);

        //设置QAbstractItemView样式
        cBox->view()->setStyleSheet(R"(
                                    QListView{border-radius: 2px;padding-left: 17px;padding-right: 17px;color: rgb(206, 207, 211);outline: 0;selection-background-color: transparent !important;;show-decoration-selected: 0 !important;;}
                                    QListView::item {height: 31px;padding-right: 20px;border-bottom: 1px solid rgba(216, 216, 216, 0.1);}

                                    QListView::item:selected{background-color: transparent;background-image: url(:/Skin/Images/cBox/item_se.png);background-repeat: no-repeat;background-position:right center;}
                                    QListView::item:hover {
                                        background-color: transparent;
                                    }
                                    QListView::item:focus {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                                    QListView::item:selected:hover {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                                    QListView::item:selected:focus {
                                        background-color: transparent;
                                        outline: 0;
                                    }
                                    QListView::item:disabled {
                                        color: gray;
                                    }
                )");
    }
}
void KeySelfSet::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    UpdateShadowLabelSize(lab_BeepShadow_Top);
    UpdateShadowLabelSize(lab_BeepShadow_Buttom);

}

void KeySelfSet::LanguageSet()
{
    //刷新文本
    ui->retranslateUi(this);
}

//自定义角色存储 -1 表示可用，0 表示禁用
//使能所有操作项
void KeySelfSet::EnAllCurrentActIdx()
{
    for(int i = 0; i < 8; ++i) {
        ui->cBox_muteAct->setItemData(i, QVariant(-1), Qt::UserRole - 1);
        ui->cBox_playAct->setItemData(i, QVariant(-1), Qt::UserRole - 1);
        ui->cBox_EQAct->setItemData(i, QVariant(-1), Qt::UserRole - 1);
    }
    updateItemsEnable(ui->cBox_muteAct);
    updateItemsEnable(ui->cBox_playAct);
    updateItemsEnable(ui->cBox_EQAct);
}
//使能所有按键项
void KeySelfSet::EnAllCurrentKeyIdx()
{
    for(int i = 0; i < KeyIdx; ++i) {
        ui->cBox_muteKey->setItemData(i, QVariant(-1), Qt::UserRole - 1);
        ui->cBox_playKey->setItemData(i, QVariant(-1), Qt::UserRole - 1);
        ui->cBox_EQKey->setItemData(i, QVariant(-1), Qt::UserRole - 1);
    }
    updateItemsEnable(ui->cBox_muteKey);
    updateItemsEnable(ui->cBox_playKey);
    updateItemsEnable(ui->cBox_EQKey);
}
//更新项是否可用/禁用
void KeySelfSet::updateItemsEnable(QComboBox* combo)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(combo->model());
    if (!model) return;

    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        if (!item) continue;


        int data = item->data(Qt::UserRole - 1).toInt();
        item->setEnabled(data == -1);
    }
}
//禁用项
void KeySelfSet::disCurrentIdx(QComboBox* cBox1, QComboBox* cBox2,QComboBox* cBox3, QComboBox* cBox4)
{
    if(cBox1->currentIndex() ==  cBox2->currentIndex())
    {

        cBox4->setItemData(cBox3->currentIndex(), QVariant(0), Qt::UserRole - 1);
        updateItemsEnable(cBox4);
    }
}
// 统一更新所有状态
void KeySelfSet::updateAllStates()
{
    // 1. 先全部启用
    EnAllCurrentActIdx();
    EnAllCurrentKeyIdx();

    // 2. 获取当前选择的索引
    int muteKey = ui->cBox_muteKey->currentIndex();
    int playKey = ui->cBox_playKey->currentIndex();
    int eqKey   = ui->cBox_EQKey->currentIndex();
    int muteAct = ui->cBox_muteAct->currentIndex();
    int playAct = ui->cBox_playAct->currentIndex();
    int eqAct   = ui->cBox_EQAct->currentIndex();

    // 将组合框和索引放入数组便于遍历
    QComboBox* keyBoxes[3] = { ui->cBox_muteKey, ui->cBox_playKey, ui->cBox_EQKey };
    QComboBox* actBoxes[3] = { ui->cBox_muteAct, ui->cBox_playAct, ui->cBox_EQAct };
    int keyIdx[3] = { muteKey, playKey, eqKey };
    int actIdx[3] = { muteAct, playAct, eqAct };

    // 3. 遍历所有组对，设置禁用项
    for (int i = 0; i < 3; ++i) {
        for (int j = i + 1; j < 3; ++j) {
            // 如果Key相同，则两个组的Act不能相同
            if (keyIdx[i] == keyIdx[j]) {
                // 组i的Act不能等于组j的Act
                actBoxes[i]->setItemData(actIdx[j], QVariant(0), Qt::UserRole - 1);
                // 组j的Act不能等于组i的Act
                actBoxes[j]->setItemData(actIdx[i], QVariant(0), Qt::UserRole - 1);
            }
            // 如果Act相同，则两个组的Key不能相同
            if (actIdx[i] == actIdx[j]) {
                // 组i的Key不能等于组j的Key
                keyBoxes[i]->setItemData(keyIdx[j], QVariant(0), Qt::UserRole - 1);
                // 组j的Key不能等于组i的Key
                keyBoxes[j]->setItemData(keyIdx[i], QVariant(0), Qt::UserRole - 1);
            }
        }
    }

    // 4. 更新所有组合框的实际启用状态
    updateItemsEnable(ui->cBox_muteKey);
    updateItemsEnable(ui->cBox_playKey);
    updateItemsEnable(ui->cBox_EQKey);
    updateItemsEnable(ui->cBox_muteAct);
    updateItemsEnable(ui->cBox_playAct);
    updateItemsEnable(ui->cBox_EQAct);
}

void KeySelfSet::on_cBox_muteKey_currentIndexChanged(int index)
{
    updateAllStates();
    // EnAllCurrentActIdx();
    // disCurrentIdx(ui->cBox_muteKey,ui->cBox_playKey,ui->cBox_muteAct,ui->cBox_playAct);
    // disCurrentIdx(ui->cBox_muteKey,ui->cBox_EQKey,ui->cBox_muteAct,ui->cBox_EQAct);
    // disCurrentIdx(ui->cBox_EQKey,ui->cBox_playKey,ui->cBox_EQAct,ui->cBox_playAct);


    DevSetKey(ui->cBox_muteKey->currentIndex()+1, ui->cBox_muteAct->currentIndex()+1
                 ,ui->cBox_playKey->currentIndex()+1, ui->cBox_playAct->currentIndex()+1
                 ,ui->cBox_EQKey->currentIndex()+1, ui->cBox_EQAct->currentIndex()+1
                 );
}

void KeySelfSet::on_cBox_playKey_currentIndexChanged(int index)
{
    updateAllStates();
    // EnAllCurrentActIdx();
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_muteKey,ui->cBox_playAct,ui->cBox_muteAct);
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_EQKey,ui->cBox_playAct,ui->cBox_EQAct);
    // disCurrentIdx(ui->cBox_EQKey,ui->cBox_muteKey,ui->cBox_EQAct,ui->cBox_muteAct);

    DevSetKey(ui->cBox_muteKey->currentIndex()+1, ui->cBox_muteAct->currentIndex()+1
                 ,ui->cBox_playKey->currentIndex()+1, ui->cBox_playAct->currentIndex()+1
                 ,ui->cBox_EQKey->currentIndex()+1, ui->cBox_EQAct->currentIndex()+1
                 );
}

void KeySelfSet::on_cBox_EQKey_currentIndexChanged(int index)
{
    updateAllStates();
    // EnAllCurrentActIdx();
    // disCurrentIdx(ui->cBox_EQKey,ui->cBox_playKey,ui->cBox_EQAct,ui->cBox_playAct);
    // disCurrentIdx(ui->cBox_EQKey,ui->cBox_muteKey,ui->cBox_EQAct,ui->cBox_muteAct);
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_muteKey,ui->cBox_playAct,ui->cBox_muteAct);

    DevSetKey(ui->cBox_muteKey->currentIndex()+1, ui->cBox_muteAct->currentIndex()+1
                 ,ui->cBox_playKey->currentIndex()+1, ui->cBox_playAct->currentIndex()+1
                 ,ui->cBox_EQKey->currentIndex()+1, ui->cBox_EQAct->currentIndex()+1
                 );
}

void KeySelfSet::on_cBox_muteAct_currentIndexChanged(int index)
{
    updateAllStates();
    // //恢复所有选项为可选
    // EnAllCurrentActIdx();
    // disCurrentIdx(ui->cBox_muteKey,ui->cBox_playKey,ui->cBox_muteAct,ui->cBox_playAct);
    // disCurrentIdx(ui->cBox_muteKey,ui->cBox_EQKey,ui->cBox_muteAct,ui->cBox_EQAct);
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_EQKey,ui->cBox_playAct,ui->cBox_EQAct);

    // EnAllCurrentKeyIdx();
    // disCurrentIdx(ui->cBox_muteAct,ui->cBox_playAct,ui->cBox_muteKey,ui->cBox_playKey);
    // disCurrentIdx(ui->cBox_muteAct,ui->cBox_EQAct,ui->cBox_muteKey,ui->cBox_EQKey);
    // disCurrentIdx(ui->cBox_playAct,ui->cBox_EQAct,ui->cBox_playKey,ui->cBox_EQKey);

    DevSetKey(ui->cBox_muteKey->currentIndex()+1, ui->cBox_muteAct->currentIndex()+1
                 ,ui->cBox_playKey->currentIndex()+1, ui->cBox_playAct->currentIndex()+1
                 ,ui->cBox_EQKey->currentIndex()+1, ui->cBox_EQAct->currentIndex()+1
                 );
}

void KeySelfSet::on_cBox_playAct_currentIndexChanged(int index)
{
    updateAllStates();
    // //恢复所有选项为可选
    // EnAllCurrentActIdx();
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_muteKey,ui->cBox_playAct,ui->cBox_muteAct);
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_EQKey,ui->cBox_playAct,ui->cBox_EQAct);
    // disCurrentIdx(ui->cBox_muteKey,ui->cBox_EQKey,ui->cBox_muteAct,ui->cBox_EQAct);

    // EnAllCurrentKeyIdx();
    // disCurrentIdx(ui->cBox_playAct,ui->cBox_muteAct,ui->cBox_playKey,ui->cBox_muteKey);
    // disCurrentIdx(ui->cBox_playAct,ui->cBox_EQAct,ui->cBox_playKey,ui->cBox_EQKey);
    // disCurrentIdx(ui->cBox_muteAct,ui->cBox_EQAct,ui->cBox_muteKey,ui->cBox_EQKey);

    DevSetKey(ui->cBox_muteKey->currentIndex()+1, ui->cBox_muteAct->currentIndex()+1
                 ,ui->cBox_playKey->currentIndex()+1, ui->cBox_playAct->currentIndex()+1
                 ,ui->cBox_EQKey->currentIndex()+1, ui->cBox_EQAct->currentIndex()+1
                 );
}

void KeySelfSet::on_cBox_EQAct_currentIndexChanged(int index)
{
    updateAllStates();
    // //恢复所有选项为可选
    // EnAllCurrentActIdx();
    // disCurrentIdx(ui->cBox_EQKey,ui->cBox_playKey,ui->cBox_EQAct,ui->cBox_playAct);
    // disCurrentIdx(ui->cBox_EQKey,ui->cBox_muteKey,ui->cBox_EQAct,ui->cBox_muteAct);
    // disCurrentIdx(ui->cBox_playKey,ui->cBox_muteKey,ui->cBox_playAct,ui->cBox_muteAct);

    // EnAllCurrentKeyIdx();
    // disCurrentIdx(ui->cBox_EQAct,ui->cBox_playAct,ui->cBox_EQKey,ui->cBox_playKey);
    // disCurrentIdx(ui->cBox_EQAct,ui->cBox_muteAct,ui->cBox_EQKey,ui->cBox_muteKey);
    // disCurrentIdx(ui->cBox_playAct,ui->cBox_muteAct,ui->cBox_playKey,ui->cBox_muteKey);

    DevSetKey(ui->cBox_muteKey->currentIndex()+1, ui->cBox_muteAct->currentIndex()+1
                 ,ui->cBox_playKey->currentIndex()+1, ui->cBox_playAct->currentIndex()+1
                 ,ui->cBox_EQKey->currentIndex()+1, ui->cBox_EQAct->currentIndex()+1
                 );
}
//提示音开关
void KeySelfSet::on_pBt_BeepSwitch_toggled(bool checked)
{
    // if(isHidRun)
    // {
    //     int res = lolib->SetBeepEn(checked);
    //     if (res < 0) {
    //         //qDebug("Unable to write()\n");
    //         msgBox.critical(NULL,tr("错误"),tr("提示音开关设置失败"));
    //     }
    // }
    if(checked)
    {
        if(lab_BeepShadow_Top)
        {
            delete lab_BeepShadow_Top;
            lab_BeepShadow_Top = nullptr;  // 必须置空防止野指针
        }
        if(lab_BeepShadow_Buttom)
        {
            delete lab_BeepShadow_Buttom;
            lab_BeepShadow_Buttom = nullptr;  // 必须置空防止野指针
        }
        ////qDebug("....sx:%d,sy:%d,lx:%d,lw:%d\n",ui->pBt_EQSwitch->x(),ui->pBt_EQSwitch->y(),ui->label->x(),ui->label->width());
    }else
    {
        createShadowLabel(ui->widget_top,lab_BeepShadow_Top);
        createShadowLabel(ui->widget_Beep,lab_BeepShadow_Buttom);

        // 最后显示阴影标签（保持底层）
        lab_BeepShadow_Top->raise();
        lab_BeepShadow_Top->show();

        lab_BeepShadow_Buttom->raise();
        lab_BeepShadow_Buttom->show();

        ui->pBt_BeepSwitch->raise();
        ui->pBt_BeepSwitch->show();
    }
    globalSettings->setValue("Beep",checked);
}
//提示音音量
void KeySelfSet::on_hSlider_level_valueChanged(int value)
{
    // if(isHidRun)
    // {
    //     int res = lolib->SetBeepVolume(value);
    //     if (res < 0) {
    //         //qDebug("Unable to write()\n");
    //         msgBox.critical(NULL,tr("错误"),tr("提示音音量设置失败"));
    //     }
    // }
    globalSettings->setValue("BeepVal",value);
}
//恢复默认设置
void KeySelfSet::on_pBt_reset_toggled(bool checked)
{
    if(isHidRun)
    {
        int res = lolib->KeyReset();
        if (res < 0) {
            //qDebug("Unable to write()\n");
            msgBox.critical(NULL,tr("错误"),tr("按键恢复默认值失败"));
        }else
        {
            //按键与操作也恢复默认值
            //1.播放暂停默认操作：单击电源键(0 1)  2.麦克风开关默认操作： 单击麦克风键(0 0)  3.EQ 切换：默认三击电源键(2 1)
            ui->cBox_playKey->setCurrentIndex(1);
            ui->cBox_playAct->setCurrentIndex(0);
            ui->cBox_muteKey->setCurrentIndex(0);
            ui->cBox_muteAct->setCurrentIndex(0);
            ui->cBox_EQKey->setCurrentIndex(1);
            ui->cBox_EQAct->setCurrentIndex(2);

            //禁用个别项，三种按键自定义不能一致，自定义角色存储 -1 表示可用，0 表示禁用
            EnAllCurrentActIdx();
            ui->cBox_playAct->setItemData(2, QVariant(0), Qt::UserRole - 1);
            ui->cBox_EQAct->setItemData(0, QVariant(0), Qt::UserRole - 1);

            updateItemsEnable(ui->cBox_playAct);
            updateItemsEnable(ui->cBox_EQAct);
        }
    }
}

void KeySelfSet::saveIniValue(int &MuteKey,int &MuteAct,int &PlayKey,int &PlayAct,int &EqKey,int &EqAct,bool &BeepEn,int &BeepVal,int &BeepLanguage)
{
    MuteKey = ui->cBox_muteKey->currentIndex();
    MuteAct = ui->cBox_muteAct->currentIndex();

    PlayKey = ui->cBox_playKey->currentIndex();
    PlayAct = ui->cBox_playAct->currentIndex();

    EqKey = ui->cBox_EQKey->currentIndex();
    EqAct = ui->cBox_EQAct->currentIndex();

    BeepEn = ui->pBt_BeepSwitch->isChecked();

    BeepVal = ui->hSlider_level->value();
    BeepLanguage = ui->cBox_beepLanguage->currentIndex();
}
void KeySelfSet::readIniValue(int MuteKey,int MuteAct,int PlayKey,int PlayAct,int EqKey,int EqAct,bool BeepEn,int BeepVal,int BeepLanguage)
{
    ui->cBox_muteKey->setCurrentIndex(MuteKey);
    ui->cBox_muteAct->setCurrentIndex(MuteAct);

    ui->cBox_playKey->setCurrentIndex(PlayKey);
    ui->cBox_playAct->setCurrentIndex(PlayAct);

    ui->cBox_EQKey->setCurrentIndex(EqKey);
    ui->cBox_EQAct->setCurrentIndex(EqAct);

    ui->pBt_BeepSwitch->setChecked(BeepEn);

    ui->hSlider_level->setValue(BeepVal);
    ui->cBox_beepLanguage->setCurrentIndex(BeepLanguage);
}

//按键功能自定义
void KeySelfSet::DevSetKey(int MuteKey,int MuteAct,int PlayKey,int PlayAct,int EqKey,int EqAct)
{
    if(isHidRun)
    {
        int res = lolib->SetKey(MuteKey,MuteAct,PlayKey,PlayAct,EqKey,EqAct);
        if (res < 0) {
            //qDebug("Unable to write()\n");
            msgBox.critical(NULL,tr("错误"),tr("按键自定义失败"));
        }
    }

    globalSettings->setValue("Customize keys/Mute/key",MuteKey);
    globalSettings->setValue("Customize keys/Mute/Action",MuteAct);
    globalSettings->setValue("Customize keys/Play/key",PlayKey);
    globalSettings->setValue("Customize keys/Play/Action",PlayAct);
    globalSettings->setValue("Customize keys/Eq/key",EqKey);
    globalSettings->setValue("Customize keys/Eq/Action",EqAct);
}


//设置提示音开关、音量
void KeySelfSet::GetDevBeep(int En,int level)
{
    if(En)
    {
        ui->pBt_BeepSwitch->setChecked(true);
    }else
    {
        ui->pBt_BeepSwitch->setChecked(false);
    }
    ui->hSlider_level->setValue(level);
}

//更新阴影尺寸
void KeySelfSet::UpdateShadowLabelSize(QLabel*& labelOut)
{
    if(labelOut)
    {
        if(labelOut->parentWidget() == ui->widget_Beep)
        {
            // 设置几何区域为父控件大小
            labelOut->setGeometry(0, ui->widget_top->height(), labelOut->parentWidget()->width(), labelOut->parentWidget()->height() - ui->widget_top->height());
        }else
        {
            // 设置几何区域为父控件大小
            labelOut->setGeometry(0, 0, labelOut->parentWidget()->width(), labelOut->parentWidget()->height());
        }
    }
}
//创建阴影
void KeySelfSet::createShadowLabel(QWidget* parent, QLabel*& labelOut)
{
    // 创建 QLabel 并设置父控件
    labelOut = new QLabel(parent);


    if(parent == ui->widget_Beep)
    {
        // 设置几何区域为父控件大小
        labelOut->setGeometry(0, ui->widget_top->height(), labelOut->parentWidget()->width(), labelOut->parentWidget()->height() - ui->widget_top->height());
    }else
    {
        // 设置几何区域为父控件大小
        labelOut->setGeometry(0, 0, labelOut->parentWidget()->width(), labelOut->parentWidget()->height());
    }

    // 设置背景图片
    labelOut->setStyleSheet("border-image: url(:/Skin/Images/Headphones/close-bk.png);");

    // 设置透明度效果
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect;
    opacityEffect->setOpacity(0.5); // 50% 透明度
    labelOut->setGraphicsEffect(opacityEffect);

    // 提升到顶层并显示
    labelOut->raise();
    labelOut->show();
}

//恢复为默认按键样式
void KeySelfSet::on_pBt_reset_clicked()
{

}

