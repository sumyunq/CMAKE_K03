#include "SpeakerEq.h"
#include "ui_SpeakerEq.h"
#include "UndoRedo/UndoSliderVal.h"
#include "UndoRedo/UndoSpaceSize.h"
#include "UndoRedo/EqBandUndoCommands.h"
#include <QMainWindow>
#include <QGraphicsOpacityEffect>
#include "LoadApoDLL.h"
#include "APOThread/ApoManager.h"
#include "LoadLib.h"

#include "modules/HomePage/home_page_main_page.h"






struct LastValues {

    double m_GainLastVal[10];//eq增益
    double m_FreqLastVals[10];//eq频点
    double m_QlastVals[10];//eq Q值
    int m_FilterLastVals[10];//eq滤波器值

    int bass = 0;//低音
    int drc = 0;//灵犀算法（DRC）
    int totalGain = 0;//扬声器增益
    int footsteps = 0;//脚步增强
    int gunshot = 0;//枪声弱化
    int sfc = 0;//声场控制
    int clarity = 0;//清晰度
    int lingeringSound = 0;//余音消除
    int spatialReverb = 0;//空间混响
    int wind = 0;//风声弱化
    int space = 0;//空间强度
    int spaceReverb = 0;//空间混响强度
    int spaceSize = 0;//环境大小
};
LastValues lVals = {};


bool Temp_EqSwitchEn = true;

bool m_isUndoRedoActive = false;


bool isValidFormat(const QString& text) {
    bool hasDigit = false;
    bool hasDot = false;
    bool hasK = false;

    for (const QChar& c : text) {
        ushort unicode = c.unicode();

        // 检测汉字（基本汉字范围）
        if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
            return false; // 汉字直接判为无效
        }

        // 检测数字
        if (c.isDigit()) {
            hasDigit = true;
            continue;
        }

        // 检测小数点
        if (c == '.') {
            if (hasDot || !hasDigit) return false; // 多个小数点或小数点前无数字
            hasDot = true;
            continue;
        }

        // 检测k/K
        if (c == 'k' || c == 'K') {
            if (hasK || !hasDigit) return false; // 多个k或k前无数字
            hasK = true;
            continue;
        }

        // 其他字符均无效
        return false;
    }
    return hasDigit; // 必须至少有一个数字
}


SpeakerEq::SpeakerEq(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SpeakerEq)
{
    ui->setupUi(this);
    InitUIInformation();



    {
        cl_need_checked_.clear();
        cl_need_checked_ = {ui->pBt_Footsteps_sub,
                            ui->hSlider_Footsteps,
                            ui->pBt_Footsteps_add,
                            ui->pBt_Gunshot_sub,
                            ui->hSlider_Gunshot,
                            ui->pBt_Gunshot_add,
                            ui->pBt_sfc_sub,
                            ui->hSlider_Sfc,
                            ui->pBt_sfc_add,
                            ui->pBt_Clarity_sub,
                            ui->hSlider_Clarity,
                            ui->pBt_Clarity_add,
                            ui->pBt_LSound_sub,
                            ui->hSlider_LingeringSound,
                            ui->pBt_LSound_add,
                            ui->pBt_SReverb_sub,
                            ui->hSlider_SpatialReverb,
                            ui->pBt_SReverb_add,
                            ui->pBt_wind_sub,
                            ui->hSlider_Wind,
                            ui->pBt_wind_add,
                            ui->pBt_low_sub,
                            ui->hSlider_Bass,
                            ui->pBt_low_add,
                            ui->pBt_totalGain_sub,
                            ui->hSlider_GainVal,
                            ui->pBt_totalGain_add,
                            ui->pBt_Environment_Large,
                            ui->pBt_Environment_Medium,
                            ui->pBt_Environment_Small,
                            ui->hSlider_Space,
                            ui->widget_eq,
                            ui->widget_drc};
    }
    for (QWidget *target_widget : cl_need_checked_) {
        target_widget->installEventFilter(this);    // 过滤禁用状态下的点击事件
    }

    tip_undo = new NewCustomToolTip(this);
    tip_undo->setLabelStyle(0);
    tip_undo->AddToolTip(ui->pBt_undoEq,tr("撤销"),Qt::AlignHCenter);
    tip_redo = new NewCustomToolTip(this);
    tip_redo->setLabelStyle(0);
    tip_redo->AddToolTip(ui->pBt_redoEq,tr("回退"),Qt::AlignHCenter);
    tip_reset = new NewCustomToolTip(this);
    tip_reset->setLabelStyle(0);
    tip_reset->AddToolTip(ui->pBt_EqReset,tr("重置"),Qt::AlignHCenter);

    tip_Function = new NewCustomToolTip(this);
    tip_Function->setLabelStyle(0);
    tip_Function->AddToolTip(ui->lab_tooptip,tr("调节各频段音量，自定义音效曲线。"),Qt::AlignHCenter);


    ui->hSlider_Space->setRange(0, 100);
    ui->hSlider_Space->setType(1,14,6,true,false);
    ui->hSlider_Space->setMargin(4);
    ui->hSlider_Space->setStyleSheet(
        QString(
            "QSlider {"
            "background: none;"
            "border: none;"
            "}"
            "QSlider::groove:horizontal {"
            "height: 10px;"
            " border: none;"
            "background: transparent;"
            "}"
            "QSlider::handle:horizontal {"
            "    width: 12px;"
            "    height: 0px;"
            "    background: transparent;"
            "    border: none;"
            "    margin: 0px;"
            "    padding: 0px;"
            "}"
            "QSlider::handle:horizontal:hover {"
            "    width: 12px;"
            "    height: 0px;"
            "    background: transparent;"
            "    border: none;"
            "    margin: 0px;"
            "    padding: 0px;"
            "}"
            ));
    ui->hSlider_Reverb->setRange(0, 100);
    ui->hSlider_Reverb->setType(1,14,6,true,false);
    ui->hSlider_Reverb->setMargin(4);
    ui->hSlider_Reverb->setStyleSheet(
        QString(
            "QSlider {"
            "background: none;"
            "border: none;"
            "}"
            "QSlider::groove:horizontal {"
            "height: 10px;"
            " border: none;"
            "background: transparent;"
            "}"
            "QSlider::handle:horizontal {"
            "    width: 12px;"
            "    height: 0px;"
            "    background: transparent;"
            "    border: none;"
            "    margin: 0px;"
            "    padding: 0px;"
            "}"
            "QSlider::handle:horizontal:hover {"
            "    width: 12px;"
            "    height: 0px;"
            "    background: transparent;"
            "    border: none;"
            "    margin: 0px;"
            "    padding: 0px;"
            "}"
            ));



    // 显示高亮阴影
    ui->widget_Horn->setButtonEnabled(true);
    ui->widget_Horn->setCenterShadowEnabled(true);



    group_space = new QButtonGroup(this);
    group_space->addButton(ui->pBt_Environment_Small,0);
    group_space->addButton(ui->pBt_Environment_Medium,1);
    group_space->addButton(ui->pBt_Environment_Large,2);
    group_space->setExclusive(true);


    group_page = new QButtonGroup(this);
    group_page->addButton(ui->pBt_Equalizer,0);
    group_page->addButton(ui->pBt_Algorithm,1);
    group_page->addButton(ui->pBt_Space,2);
    group_page->addButton(ui->pBt_Drc,3);
    group_page->setExclusive(true);
    connect(group_page, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),this, [this](int id,bool checked){
        if(checked)
        {
            m_undoStack_Eq->clear(); // 主动清空，提前释放命令对象
            switch(id)
            {
            case 0://均衡器（根据当前方案是否可看）
                ui->pBt_EQSwitch->blockSignals(true);
                ui->pBt_EQSwitch->setChecked(currentPlanVal.eqOpenEn);
                ui->pBt_EQSwitch->blockSignals(false);
                ui->lab_switchDes->setText(tr("均衡器"));
                tip_Function->AddToolTip(ui->lab_tooptip,tr("调节各频段音量，自定义音效曲线。"),Qt::AlignHCenter);
                ShowEqVal(EqShow);
                SetEQSwitchShadow(currentPlanVal.eqOpenEn,0);
                break;
            case 1://算法
                ui->pBt_EQSwitch->blockSignals(true);
                ui->pBt_EQSwitch->setChecked(currentPlanVal.AlgoOpenEn);
                ui->pBt_EQSwitch->blockSignals(false);
                ui->lab_switchDes->setText(tr("算法"));
                tip_Function->AddToolTip(ui->lab_tooptip,tr("一键启用游戏音效增强算法，包括脚步增强、枪声弱化等。拖动滑块调节强度，自由组合适配不同游戏。"),Qt::AlignHCenter);
                ui->stackedWidget->setCurrentWidget(ui->page_Algorithm);
                SetEQSwitchShadow(currentPlanVal.AlgoOpenEn,1);
                break;
            case 2://空间音频
                ui->pBt_EQSwitch->blockSignals(true);
                ui->pBt_EQSwitch->setChecked(currentPlanVal.spaceOpenEn);
                ui->pBt_EQSwitch->blockSignals(false);
                ui->lab_switchDes->setText(tr("空间音频"));
                tip_Function->AddToolTip(ui->lab_tooptip,tr("开启虚拟环绕声，模拟不同空间环境的声场效果。选择环境大小并调节各项参数，增强沉浸感。"),Qt::AlignHCenter);
                ui->stackedWidget->setCurrentWidget(ui->page_Space);
                SetEQSwitchShadow(currentPlanVal.spaceOpenEn,2);
                break;
            case 3://DRC
                ui->pBt_EQSwitch->blockSignals(true);
                ui->pBt_EQSwitch->setChecked(currentPlanVal.drcOpenEn);
                ui->pBt_EQSwitch->blockSignals(false);
                ui->lab_switchDes->setText(tr("灵晰算法"));
                tip_Function->AddToolTip(ui->lab_tooptip,tr("以AI声纹识别模型与实时动态降噪算法为基础,构建FPS高频毫秒级响应机制,当枪声、爆炸等噪音达到刺耳阈值时,快速消解多余高频音域,同时让脚步声、技能释放声等关键信息更清晰。"),Qt::AlignHCenter);
                ui->stackedWidget->setCurrentWidget(ui->page_drc);
                SetEQSwitchShadow(currentPlanVal.drcOpenEn,3);


            default:
                break;
            }

        }
    });


    // ui->hSlider_Bass->setType(1,15,5,false);
    // ui->hSlider_Space->setType(1,15,5,false);
    // ui->hSlider_GainVal->setType(1,15,5,false);

    currentPlan_e = ui->rBt_currentPlan;
    eightPlan_e = ui->widget_eight;


    // lVals.m_FreqLastVals = QVector<QString>({
    //     m_FlastVal1, m_FlastVal12, m_FlastVal13, m_FlastVal14,
    //     m_FlastVal15, m_FlastVal16, m_FlastVal17, m_FlastVal18,
    //     m_FlastVal19, m_FlastVal110
    // });

    //ui->widget_eight->ShowFirstEight();

    m_undoStack_Eq = new QUndoStack(this);

    ui->pBt_undoEq->setEnabled(false);
    // connect(ui->pBt_undoEq, &QPushButton::clicked,m_undoStack_Eq,&QUndoStack::undo);
    connect(ui->pBt_undoEq, &QPushButton::clicked, this, [this]() {
        m_isUndoRedoActive = true;
        m_undoStack_Eq->undo();
        m_isUndoRedoActive = false;
    });
    connect(m_undoStack_Eq,&QUndoStack::canUndoChanged,[=](){
        //canUndo()：这是一个查询函数，用于检查当前撤销栈中是否有可以撤销的操作。
        //undo()：这是一个执行函数，用于执行撤销操作（即回退到上一个状态）。
        //qDebug("进入 connect(m_undoStack_Eq,&QUndoStack::canUndoChanged, \n");
        if (!m) {
            return;
        }
        ui->pBt_undoEq->setEnabled(m_undoStack_Eq->canUndo());//没有撤回内容时，点击也没事，不会有效果

    });

    ui->pBt_redoEq->setEnabled(false);
    // connect(ui->pBt_redoEq, &QPushButton::clicked, m_undoStack_Eq, &QUndoStack::redo);
    connect(ui->pBt_redoEq, &QPushButton::clicked, this, [this]() {
        m_isUndoRedoActive = true;
        m_undoStack_Eq->redo();
        m_isUndoRedoActive = false;
    });
    connect(m_undoStack_Eq, &QUndoStack::canRedoChanged, [=]() {

        //qDebug("进入 connect(m_undoStack_Eq, &QUndoStack::canRedoChanged, \n");

        if (!m) {
            return;
        }
        ui->pBt_redoEq->setEnabled(m_undoStack_Eq->canRedo());//没有恢复时，点击也没事，不会有效果
    });

    // SetLastVal();

    connect(ui->hSlider_GainVal, &QSlider::valueChanged, this, [this](int value) {
        ui->lab_GainVal->setText(QString::number(value));
        currentPlanVal.GainVal = value;

        emit ApoManager::instance()->requestSetGlobalInputGainDb(value);



        //int currentVal = ui->hSlider_Bass->value();
        if (value != lVals.totalGain) {
            UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_GainVal,ui->lab_GainVal,lVals.totalGain,value,0);
            m_undoStack_Eq->push(cmd);
            lVals.totalGain = value;
        }
        emit PlanSave_E();
    });

    // 连接频点增益改变信号，更新对应的 SpinBox
    connect(ui->widget_eq, &EQCurveWidget::bandGainChanged, this,
            [this](int index, double newGain) {

                //qDebug("进入 connect(ui->widget_eq, &EQCurveWidget::bandGainChanged \n");
                if(currentPlanVal.ParentPlanName.isEmpty())
                {
                    currentPlanVal.eqVal[index] = newGain;
                    emit ApoManager::instance()->requestSetExtendEqualizerGain(0,index,newGain);
                }else
                {
                    currentPlanVal.eqVal_deriv[index] = newGain;
                    emit ApoManager::instance()->requestSetExtendEqualizerGain(1,index,newGain);
                }



                emit PlanSave_E();

                if (m_isUndoRedoActive)
                    return;

                double oldGain = lVals.m_GainLastVal[index];
                if (newGain != oldGain) {
                    lVals.m_GainLastVal[index] = newGain;            // 先更新
                    m_undoStack_Eq->push(new EQBandGainCommand(ui->widget_eq, index, oldGain, newGain));
                }


    });
    // 频点的频率改变
    connect(ui->widget_eq, &EQCurveWidget::bandFrequencyChanged, this,
            [this](int index, double newFreq) {

                //qDebug("进入 connect(ui->widget_eq, &EQCurveWidget::bandFrequencyChanged \n");
                int i = 0;
                // QVector<double> FVal;

                if(currentPlanVal.ParentPlanName.isEmpty())
                {
                    currentPlanVal.freqVal[index] = newFreq;
                    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequency(0,index,newFreq);
                }else
                {
                    currentPlanVal.freqVal_deriv[index] = newFreq;
                    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequency(1,index,newFreq);
                }
                // ui->widget_eq->AllFreq(FVal);
                // for(i = 0; i < 10; i++)
                // {
                //     FVal.append(currentPlanVal.freqVal[i]);
                // }
                // for(;i < 20; i++)
                // {
                //     FVal.append(currentPlanVal.freqVal_deriv[i-10]); //参数不能为0，会破音卡顿
                // }
                // qDebug("currentPlanVal存为：%lf\n",newFreq);
                // emit ApoManager::instance()->requestSetEqualizerCenterFrequencyEx(FVal);

                emit PlanSave_E();

                if (m_isUndoRedoActive)
                    return;

                double oldFreq = lVals.m_FreqLastVals[index];
                if (newFreq != oldFreq) {
                    lVals.m_FreqLastVals[index] = newFreq;            // 先更新
                    m_undoStack_Eq->push(new EQBandFrequencyCommand(ui->widget_eq, index, oldFreq, newFreq));
                }
            });
    //频点的Q值改变
    connect(ui->widget_eq, &EQCurveWidget::bandQChanged, this,
            [this](int index, double newQ) {
                QVector<double> QVal;
                // ui->widget_eq->AllQVal(QVal);
                if(currentPlanVal.ParentPlanName.isEmpty())
                {
                    currentPlanVal.qVal[index] = newQ;
                    emit ApoManager::instance()->requestSetExtendEqualizerBandQuality(0,index,newQ);
                }else
                {
                    currentPlanVal.qVal_deriv[index] = newQ;
                    emit ApoManager::instance()->requestSetExtendEqualizerBandQuality(1,index,newQ);
                }

                // int i = 0;

                // for(i = 0; i < 10; i++)
                // {
                //     QVal.append(currentPlanVal.qVal[i]);
                // }
                // for(;i < 20; i++)
                // {
                //     QVal.append(currentPlanVal.qVal_deriv[i-10]); //参数不能为0，会破音卡顿
                // }

                // emit ApoManager::instance()->requestSetEqualizerBandQualityEx(QVal);//设置Q值

                emit PlanSave_E();

                if (m_isUndoRedoActive)
                    return;

                double oldQ = lVals.m_QlastVals[index];
                if (newQ != oldQ) {
                    lVals.m_QlastVals[index] = newQ;               // 先更新
                    m_undoStack_Eq->push(new EQBandQCommand(ui->widget_eq, index, oldQ, newQ));
                }

            });

    //频点的滤波器值改变
    connect(ui->widget_eq, &EQCurveWidget::bandFilterTypeChanged, this,
            [this](int index, int NewfilterType) {
                QVector<int> filterTypeVal;
                // ui->widget_eq->AllQVal(QVal);
                if(currentPlanVal.ParentPlanName.isEmpty())
                {
                    currentPlanVal.filterVal[index] = NewfilterType;
                    emit ApoManager::instance()->requestSetExtendEqualizerBandFilter(0,index,static_cast<EqualizerFilter>(NewfilterType));
                }else
                {
                    currentPlanVal.filterVal_deriv[index] = NewfilterType;
                    emit ApoManager::instance()->requestSetExtendEqualizerBandFilter(1,index,static_cast<EqualizerFilter>(NewfilterType));
                }

                emit PlanSave_E();

                if (m_isUndoRedoActive)
                    return;

                double oldFilter = lVals.m_FilterLastVals[index];
                if (NewfilterType != oldFilter) {
                    lVals.m_FilterLastVals[index] = NewfilterType;               // 先更新
                    m_undoStack_Eq->push(new EQBandFliterCommand(ui->widget_eq, index, oldFilter, NewfilterType));
                }

            });
    //灵晰算法开关
    connect(ui->widget_drc,&AudioWaveWidget::setOpenDrcEn,this,
            [this](bool en){
                ui->pBt_EQSwitch->setChecked(en);
                // currentPlanVal.drcOpenEn = en;
                // emit PlanSave_E();
            });

    //灵晰算法（DRC）值改变
    connect(ui->widget_drc,&AudioWaveWidget::DrcLevelChanged,this,
            [this](int level){
                switch(level)
                {
                    case 0:
                        emit ApoManager::instance()->requestSetDrcState(false);
                        break;
                    case 1:
                        emit ApoManager::instance()->requestSetDrcState(true);
                        emit ApoManager::instance()->requestSetDrcRatio(15);
                        emit ApoManager::instance()->requestSetDrcThreshold(-20);
                        emit ApoManager::instance()->requestSetDrcMakeupEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterThreshold(-4);
                        emit ApoManager::instance()->requestSetDrcInputGain(0);
                        emit ApoManager::instance()->requestSetDrcOutputGain(0);
                        emit ApoManager::instance()->requestSetDrcAttackTime(0.3);
                        emit ApoManager::instance()->requestSetDrcReleaseTime(0.3);
                        break;
                    case 2:
                        emit ApoManager::instance()->requestSetDrcState(true);
                        emit ApoManager::instance()->requestSetDrcRatio(15);
                        emit ApoManager::instance()->requestSetDrcThreshold(-25);
                        emit ApoManager::instance()->requestSetDrcMakeupEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterThreshold(-6);
                        emit ApoManager::instance()->requestSetDrcInputGain(0);
                        emit ApoManager::instance()->requestSetDrcOutputGain(0);
                        emit ApoManager::instance()->requestSetDrcAttackTime(0.3);
                        emit ApoManager::instance()->requestSetDrcReleaseTime(0.3);
                        break;
                    case 3:
                        emit ApoManager::instance()->requestSetDrcState(true);
                        emit ApoManager::instance()->requestSetDrcRatio(20);
                        emit ApoManager::instance()->requestSetDrcThreshold(-30);
                        emit ApoManager::instance()->requestSetDrcMakeupEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterThreshold(-7);
                        emit ApoManager::instance()->requestSetDrcInputGain(0);
                        emit ApoManager::instance()->requestSetDrcOutputGain(0);
                        emit ApoManager::instance()->requestSetDrcAttackTime(0.3);
                        emit ApoManager::instance()->requestSetDrcReleaseTime(0.3);
                        break;
                    case 4:
                        emit ApoManager::instance()->requestSetDrcState(true);
                        emit ApoManager::instance()->requestSetDrcRatio(25);
                        emit ApoManager::instance()->requestSetDrcThreshold(-35);
                        emit ApoManager::instance()->requestSetDrcMakeupEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterThreshold(-8);
                        emit ApoManager::instance()->requestSetDrcInputGain(0);
                        emit ApoManager::instance()->requestSetDrcOutputGain(0);
                        emit ApoManager::instance()->requestSetDrcAttackTime(0.3);
                        emit ApoManager::instance()->requestSetDrcReleaseTime(0.3);
                        break;
                    case 5:
                        emit ApoManager::instance()->requestSetDrcState(true);
                        emit ApoManager::instance()->requestSetDrcRatio(30);
                        emit ApoManager::instance()->requestSetDrcThreshold(-50);
                        emit ApoManager::instance()->requestSetDrcMakeupEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterEnable(true);
                        emit ApoManager::instance()->requestSetDrcLimiterThreshold(-10);
                        emit ApoManager::instance()->requestSetDrcInputGain(0);
                        emit ApoManager::instance()->requestSetDrcOutputGain(0);
                        emit ApoManager::instance()->requestSetDrcAttackTime(0.3);
                        emit ApoManager::instance()->requestSetDrcReleaseTime(0.3);
                        break;
                    default:
                        break;

                }
                currentPlanVal.drcVal = level;
                emit PlanSave_E();
            });


    //加减号
    struct SliderAdjustPair {
        QPushButton* subBtn;
        QPushButton* addBtn;
        QSlider*    slider;
    };
    const std::vector<SliderAdjustPair> pairs = {
        {ui->pBt_Footsteps_sub, ui->pBt_Footsteps_add, ui->hSlider_Footsteps},
        {ui->pBt_Gunshot_sub,   ui->pBt_Gunshot_add,   ui->hSlider_Gunshot},
        {ui->pBt_sfc_sub,ui->pBt_sfc_add,ui->hSlider_Sfc},
        {ui->pBt_Clarity_sub,ui->pBt_Clarity_add,ui->hSlider_Clarity},
        {ui->pBt_LSound_sub,ui->pBt_LSound_add,ui->hSlider_LingeringSound},
        {ui->pBt_SReverb_sub,ui->pBt_SReverb_add,ui->hSlider_SpatialReverb},
        {ui->pBt_wind_sub,ui->pBt_wind_add,ui->hSlider_Wind},
        {ui->pBt_low_sub,ui->pBt_low_add,ui->hSlider_Bass},
        {ui->pBt_totalGain_sub,ui->pBt_totalGain_add,ui->hSlider_GainVal},
    };
    for (const auto& pair : pairs) {
        connect(pair.subBtn, &QPushButton::clicked, this, [this, slider = pair.slider]() {
            if (!slider) return;
            int newVal = slider->value() - 1;
            newVal = qBound(slider->minimum(), newVal, slider->maximum());
            slider->setValue(newVal);
        });
        connect(pair.addBtn, &QPushButton::clicked, this, [this, slider = pair.slider]() {
            if (!slider) return;
            int newVal = slider->value() + 1;
            newVal = qBound(slider->minimum(), newVal, slider->maximum());
            slider->setValue(newVal);
        });
    }

}

SpeakerEq::~SpeakerEq()
{
    m_undoStack_Eq->clear(); // 主动清空，提前释放命令对象
    delete ui;
}

void SpeakerEq::LanguageSet()
{
    //刷新文本
    ui->retranslateUi(this);


    // if(LanguageIdx == 0)//简体
    // {

    // }else if(LanguageIdx == 1)//繁體
    // {

    // }else if(LanguageIdx == 2)//英文
    // {

    // }




    // ui->widget_eq->LanguageSet();
}

void SpeakerEq::set_pBt_EQSwitch_hideData_checked(bool checked)
{
    // 纯显示同步：截断 toggled，避免连锁触发 on_pBt_EQSwitch_hideData_toggled 的保存/APO 业务
    // （原实现无截断：方案开关 → EQ 页开关 toggled → PlanSave_E → updateAllPlanValue 拷贝损坏的 PlanVal 崩溃）
    QSignalBlocker blocker(ui->pBt_EQSwitch_hideData);
    ui->pBt_EQSwitch_hideData->setChecked(checked);
}

void SpeakerEq::SetLastVal()
{
    lVals.totalGain = ui->hSlider_GainVal->value();
    lVals.bass = ui->hSlider_Bass->value();
    lVals.space = ui->hSlider_Space->value();
    lVals.spaceReverb = ui->hSlider_Reverb->value();

    lVals.m_GainLastVal[0] = ui->widget_eq->GetIndexGain(0);
    lVals.m_GainLastVal[1] = ui->widget_eq->GetIndexGain(1);
    lVals.m_GainLastVal[2] = ui->widget_eq->GetIndexGain(2);
    lVals.m_GainLastVal[3] = ui->widget_eq->GetIndexGain(3);
    lVals.m_GainLastVal[4] = ui->widget_eq->GetIndexGain(4);
    lVals.m_GainLastVal[5] = ui->widget_eq->GetIndexGain(5);
    lVals.m_GainLastVal[6] = ui->widget_eq->GetIndexGain(6);
    lVals.m_GainLastVal[7] = ui->widget_eq->GetIndexGain(7);
    lVals.m_GainLastVal[8] = ui->widget_eq->GetIndexGain(8);
    lVals.m_GainLastVal[9] = ui->widget_eq->GetIndexGain(9);

    lVals.m_FreqLastVals[0] = ui->widget_eq->GetIndexFreq(0);
    lVals.m_FreqLastVals[1] = ui->widget_eq->GetIndexFreq(1);
    lVals.m_FreqLastVals[2] = ui->widget_eq->GetIndexFreq(2);
    lVals.m_FreqLastVals[3] = ui->widget_eq->GetIndexFreq(3);
    lVals.m_FreqLastVals[4] = ui->widget_eq->GetIndexFreq(4);
    lVals.m_FreqLastVals[5] = ui->widget_eq->GetIndexFreq(5);
    lVals.m_FreqLastVals[6] = ui->widget_eq->GetIndexFreq(6);
    lVals.m_FreqLastVals[7] = ui->widget_eq->GetIndexFreq(7);
    lVals.m_FreqLastVals[8] = ui->widget_eq->GetIndexFreq(8);
    lVals.m_FreqLastVals[9] = ui->widget_eq->GetIndexFreq(9);

    lVals.m_QlastVals[0] = ui->widget_eq->GetIndexQ(0);
    lVals.m_QlastVals[1] = ui->widget_eq->GetIndexQ(1);
    lVals.m_QlastVals[2] = ui->widget_eq->GetIndexQ(2);
    lVals.m_QlastVals[3] = ui->widget_eq->GetIndexQ(3);
    lVals.m_QlastVals[4] = ui->widget_eq->GetIndexQ(4);
    lVals.m_QlastVals[5] = ui->widget_eq->GetIndexQ(5);
    lVals.m_QlastVals[6] = ui->widget_eq->GetIndexQ(6);
    lVals.m_QlastVals[7] = ui->widget_eq->GetIndexQ(7);
    lVals.m_QlastVals[8] = ui->widget_eq->GetIndexQ(8);
    lVals.m_QlastVals[9] = ui->widget_eq->GetIndexQ(9);

    lVals.m_FilterLastVals[0] = ui->widget_eq->GetIndexFilter(0);
    lVals.m_FilterLastVals[1] = ui->widget_eq->GetIndexFilter(1);
    lVals.m_FilterLastVals[2] = ui->widget_eq->GetIndexFilter(2);
    lVals.m_FilterLastVals[3] = ui->widget_eq->GetIndexFilter(3);
    lVals.m_FilterLastVals[4] = ui->widget_eq->GetIndexFilter(4);
    lVals.m_FilterLastVals[5] = ui->widget_eq->GetIndexFilter(5);
    lVals.m_FilterLastVals[6] = ui->widget_eq->GetIndexFilter(6);
    lVals.m_FilterLastVals[7] = ui->widget_eq->GetIndexFilter(7);
    lVals.m_FilterLastVals[8] = ui->widget_eq->GetIndexFilter(8);
    lVals.m_FilterLastVals[9] = ui->widget_eq->GetIndexFilter(9);


}

void SpeakerEq::updateCPVal()
{
    currentPlanVal.GainVal = ui->hSlider_GainVal->value();
    currentPlanVal.lowVal = ui->hSlider_Bass->value();
    currentPlanVal.spaceVal = ui->hSlider_Space->value();
    currentPlanVal.spaceReverb = ui->hSlider_Reverb->value();

    currentPlanVal.eqVal[0] = ui->widget_eq->GetIndexGain(0);// /10.0;
    currentPlanVal.eqVal[1] = ui->widget_eq->GetIndexGain(1);
    currentPlanVal.eqVal[2] = ui->widget_eq->GetIndexGain(2);
    currentPlanVal.eqVal[3] = ui->widget_eq->GetIndexGain(3);
    currentPlanVal.eqVal[4] = ui->widget_eq->GetIndexGain(4);
    currentPlanVal.eqVal[5] = ui->widget_eq->GetIndexGain(5);
    currentPlanVal.eqVal[6] = ui->widget_eq->GetIndexGain(6);
    currentPlanVal.eqVal[7] = ui->widget_eq->GetIndexGain(7);
    currentPlanVal.eqVal[8] = ui->widget_eq->GetIndexGain(8);
    currentPlanVal.eqVal[9] = ui->widget_eq->GetIndexGain(9);


    currentPlanVal.freqVal[0] = ui->widget_eq->GetIndexFreq(0);// /10.0;
    currentPlanVal.freqVal[1] = ui->widget_eq->GetIndexFreq(1);
    currentPlanVal.freqVal[2] = ui->widget_eq->GetIndexFreq(2);
    currentPlanVal.freqVal[3] = ui->widget_eq->GetIndexFreq(3);
    currentPlanVal.freqVal[4] = ui->widget_eq->GetIndexFreq(4);
    currentPlanVal.freqVal[5] = ui->widget_eq->GetIndexFreq(5);
    currentPlanVal.freqVal[6] = ui->widget_eq->GetIndexFreq(6);
    currentPlanVal.freqVal[7] = ui->widget_eq->GetIndexFreq(7);
    currentPlanVal.freqVal[8] = ui->widget_eq->GetIndexFreq(8);
    currentPlanVal.freqVal[9] = ui->widget_eq->GetIndexFreq(9);



    currentPlanVal.qVal[0] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[1] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[2] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[3] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[4] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[5] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[6] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[7] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[8] = ui->widget_eq->GetIndexQ(0);
    currentPlanVal.qVal[9] = ui->widget_eq->GetIndexQ(0);

    currentPlanVal.filterVal[0] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[1] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[2] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[3] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[4] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[5] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[6] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[7] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[8] = ui->widget_eq->GetIndexFilter(0);
    currentPlanVal.filterVal[9] = ui->widget_eq->GetIndexFilter(0);
}

void SpeakerEq::updateCPVal_Freq(QString text,int i)
{
    // 尝试去除单位后解析数字
    QString cleaned = text;
    bool hasKHz = false;
    QVector<double> FVal;

    if (cleaned.endsWith("K", Qt::CaseInsensitive)) {
        cleaned.chop(1); // 去掉 "K"
        hasKHz = true;
    }

    bool ok;
    double value = cleaned.toDouble(&ok);
    if (!ok) {
        currentPlanVal.freqVal[0] = 20.0;
        return;
    }
    double freqValue;
    if (hasKHz) {
        freqValue = static_cast<double>(value * 1000); // KHz -> Hz
    } else {
        freqValue = static_cast<double>(value);
    }
    // 限制范围
    freqValue = qBound(20.0, freqValue, 20000.0);
    currentPlanVal.freqVal[i] = freqValue;
}

// //点击当前预设，打开主界面中的page_Sperker页面，并显示为其中的page_plan页面
// void SpeakerEq::on_rBt_currentPlan_clicked()
// {
//     //emit PlanSave_E();
//     emit CurrentpageChange();
// }
//跳转到方案库界面
void SpeakerEq::on_pBt_Plans_clicked()
{
    emit CurrentpageChange();
}

//点击试听，若是游戏模式则打开主界面中的page_listen页面
void SpeakerEq::on_pBt_GameListen_clicked()
{
    emit ListenpageChange();
}

// //返回主页面
// void SpeakerEq::on_pBt_backMain_clicked()
// {
//     //emit PlanSave_E();
//     emit CurrentpageChange();
// }



void SpeakerEq::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

}

void SpeakerEq::on_pBt_EQSwitch_toggled(bool checked)
{
    //IsSwitch = false;
    SetEQSwitchShadow(checked,group_page->checkedId());
    emit PlanSave_E();

    // // 检查当前预设是否为首页预设（ 如果为 首页预设 更新，否则，不做额外处理）
    emit UpdateHomePageUIInfo(checked);

}
//0:均衡器，1：算法，2空间,3:DRC
void SpeakerEq::SetEQSwitchShadow(bool checked,int type)
{
    ui->pBt_undoEq->setEnabled(checked);
    ui->pBt_redoEq->setEnabled(checked);
    ui->pBt_EqReset->setEnabled(checked);
    switch(type)
    {
    case 0:
    {
        currentPlanVal.eqOpenEn = checked;
        // emit ApoManager::instance()->requestSetEqualizerState(checked);
        emit ApoManager::instance()->requestSetExtendEqState(0,checked);
        emit ApoManager::instance()->requestSetExtendEqState(1,checked);
        QVector<bool> enables(10, checked);
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(0,enables);//
        emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(1,enables);
        //波形图遮盖
        ui->widget_eq->setDisableOverlay(!checked);
        break;
    }
    case 1:
    {
        //设置算法
        currentPlanVal.AlgoOpenEn = checked;
        //低音增强
        if(currentPlanVal.lowVal>0)
        {
            emit ApoManager::instance()->requestSetBassBoostState(checked);
        }else
        {
            emit ApoManager::instance()->requestSetBassBoostState(false);
        }
        //增益
        if(currentPlanVal.GainVal > 0)
        {
            if(checked)
            {
                emit ApoManager::instance()->requestSetGlobalInputGainDb(currentPlanVal.GainVal);
            }else
            {
                emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
            }
        }else
        {
            emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
        }

        //额外eq
        if(currentPlanVal.ExtraEq[0] == 0)
        {
            emit ApoManager::instance()->requestSetExtendEqState(2,false);
        }else
        {
           emit ApoManager::instance()->requestSetExtendEqState(2,checked);
        }
        if(currentPlanVal.ExtraEq[1] == 0)
        {
            emit ApoManager::instance()->requestSetExtendEqState(3,false);
        }else
        {
            emit ApoManager::instance()->requestSetExtendEqState(3,checked);
        }
        if(currentPlanVal.ExtraEq[2] == 0)
        {
            qDebug("requestSetExtendEqState 4 false\n");
            emit ApoManager::instance()->requestSetExtendEqState(4,false);
        }else
        {
            qDebug("requestSetExtendEqState 4 %d\n",checked);
            emit ApoManager::instance()->requestSetExtendEqState(4,checked);
        }
        if(currentPlanVal.ExtraEq[3] == 0)
        {
            emit ApoManager::instance()->requestSetExtendEqState(5,false);
        }else
        {
            emit ApoManager::instance()->requestSetExtendEqState(5,checked);
        }
        if(currentPlanVal.ExtraEq[4] == 0)
        {
            emit ApoManager::instance()->requestSetExtendEqState(6,false);
        }else
        {
            emit ApoManager::instance()->requestSetExtendEqState(6,checked);
        }
        if(currentPlanVal.ExtraEq[5] == 0)
        {
            emit ApoManager::instance()->requestSetExtendEqState(7,false);
        }else
        {
            emit ApoManager::instance()->requestSetExtendEqState(7,checked);
        }
        if(currentPlanVal.ExtraEq[6] == 0)
        {
            emit ApoManager::instance()->requestSetExtendEqState(8,false);
        }else
        {
            emit ApoManager::instance()->requestSetExtendEqState(8,checked);
        }



        // emit ApoManager::instance()->requestSetExtendEqState(3,checked);
        // emit ApoManager::instance()->requestSetExtendEqState(4,checked);
        // emit ApoManager::instance()->requestSetExtendEqState(5,checked);
        // emit ApoManager::instance()->requestSetExtendEqState(6,checked);
        // emit ApoManager::instance()->requestSetExtendEqState(7,checked);
        // emit ApoManager::instance()->requestSetExtendEqState(8,checked);
        // // emit ApoManager::instance()->requestSetExtendEqState(9,false);//每一组的总开关

        const QList<QWidget*> algoControls = {
            ui->lab_algoIcon_1,
            ui->lab_algoIcon_2,
            ui->lab_algoIcon_3,
            ui->lab_algoIcon_4,
            ui->lab_algoIcon_5,
            ui->lab_algoIcon_6,
            ui->lab_algoIcon_7,
            ui->pBt_Footsteps_sub,
            ui->hSlider_Footsteps,
            ui->pBt_Footsteps_add,
            ui->pBt_Gunshot_sub,
            ui->hSlider_Gunshot,
            ui->pBt_Gunshot_add,
            ui->pBt_sfc_sub,
            ui->hSlider_Sfc,
            ui->pBt_sfc_add,
            ui->pBt_Clarity_sub,
            ui->hSlider_Clarity,
            ui->pBt_Clarity_add,
            ui->pBt_LSound_sub,
            ui->hSlider_LingeringSound,
            ui->pBt_LSound_add,
            ui->pBt_SReverb_sub,
            ui->hSlider_SpatialReverb,
            ui->pBt_SReverb_add,
            ui->pBt_wind_sub,
            ui->hSlider_Wind,
            ui->pBt_wind_add,
            ui->pBt_low_sub,
            ui->hSlider_Bass,
            ui->pBt_low_add,
            ui->pBt_totalGain_sub,
            ui->hSlider_GainVal,
            ui->pBt_totalGain_add,
        };

        for (QWidget* w : algoControls) {
            w->setEnabled(checked);
        }

        break;
    }
    case 2:
    {
        currentPlanVal.spaceOpenEn = checked;

        if(currentPlanVal.spaceVal == 0)
        {
            qDebug("requestSetSurroundState false\n");
            emit ApoManager::instance()->requestSetSurroundState(false);
        }else
        {
            qDebug("requestSetSurroundState  %d\n",checked);
            emit ApoManager::instance()->requestSetSurroundState(checked);
        }
        if(currentPlanVal.spaceReverb == 0)
        {
            qDebug("requestSetReverbState false\n");
            emit ApoManager::instance()->requestSetReverbState(false);
        }else
        {
            qDebug("requestSetReverbState  %d\n",checked);
            emit ApoManager::instance()->requestSetReverbState(checked);
        }
        ui->widget_Horn->setEnabled(checked);

        ui->widget_Horn->setButtonEnabled(checked);
        ui->widget_Horn->setCenterShadowEnabled(checked);

        ui->hSlider_Space->setEnabled(checked);
        ui->hSlider_Reverb->setEnabled(checked);
        if (checked) {

            // 水平条 渐变
            ui->hSlider_Space->animateHandleColor(QColor("#ACACAC"), QColor("#FFFFFF"), 100);
            ui->hSlider_Space->animateFillColor(QColor("#0F6796"), QColor("#009FEF"), 100);
            ui->hSlider_Reverb->animateHandleColor(QColor("#ACACAC"), QColor("#FFFFFF"), 100);
            ui->hSlider_Reverb->animateFillColor(QColor("#0F6796"), QColor("#009FEF"), 100);
        } else {

            // 水平条 渐变
            ui->hSlider_Space->animateHandleColor(QColor("#FFFFFF"), QColor("#ACACAC"), 100);
            ui->hSlider_Space->animateFillColor(QColor("#009FEF"), QColor("#0F6796"), 100);
            ui->hSlider_Reverb->animateHandleColor(QColor("#FFFFFF"), QColor("#ACACAC"), 100);
            ui->hSlider_Reverb->animateFillColor(QColor("#009FEF"), QColor("#0F6796"), 100);
        }
        ui->pBt_Environment_Small->setEnabled(checked);
        ui->pBt_Environment_Medium->setEnabled(checked);
        ui->pBt_Environment_Large->setEnabled(checked);
        break;
    }
    case 3:
    {
        currentPlanVal.drcOpenEn = checked;
        ui->widget_drc->setEnabled(currentPlanVal.drcOpenEn);
        //灵晰算法（DRC）
        if(currentPlanVal.drcVal == 0)
        {
            emit ApoManager::instance()->requestSetDrcState(false);
        }else
        {
            emit ApoManager::instance()->requestSetDrcState(checked);
            if(checked)
            {
                emit ui->widget_drc->DrcLevelChanged(currentPlanVal.drcVal);
            }
        }


    }
    default:
        break;
    }
}

void SpeakerEq::InitUIInformation()
{
    {
        // ui->pBt_Plans 完整设置
        ui->pBt_Plans->setCl_min_size(QSize(88, 88));
        ui->pBt_Plans->setCl_icon_size(QSize(30, 29), QSize(30, 29));
        ui->pBt_Plans->setCl_icon_text_spacing(5);
        ui->pBt_Plans->setCl_icon_point(QPoint(29, 29), QPoint(29, 23));
        ui->pBt_Plans->setCl_bg_default_color(QColor(81, 96, 122, 51));
        ui->pBt_Plans->setCl_bg_hover_color(QColor(255, 255, 255, 25));
        ui->pBt_Plans->setCl_border_radius(10);
        // 显示信息 后手设置
        ui->pBt_Plans->setCl_pixmap(
            QPixmap(":/Skin/Images/soundTest/plansLib_icon_2x.png"));
        ui->pBt_Plans->setCl_classification_name(tr("预设库"));
        ui->pBt_Plans->repaint(); // 立即重绘
    }
    {
        // ui->pBt_GameListen 完整设置
        ui->pBt_GameListen->setCl_min_size(QSize(88, 88));
        ui->pBt_GameListen->setCl_icon_size(QSize(34, 25), QSize(34, 25));
        ui->pBt_GameListen->setCl_icon_text_spacing(7);
        ui->pBt_GameListen->setCl_icon_point(QPoint(27, 32), QPoint(27, 25));
        ui->pBt_GameListen->setCl_bg_default_color(QColor(81, 96, 122, 51));
        ui->pBt_GameListen->setCl_bg_hover_color(QColor(255, 255, 255, 25));
        ui->pBt_GameListen->setCl_border_radius(10);
        // 显示信息 后手设置
        ui->pBt_GameListen->setCl_pixmap(
            QPixmap(":/Skin/Images/soundTest/soundTest_icon_2x.png"));
        ui->pBt_GameListen->setCl_classification_name(tr("试听"));
        ui->pBt_GameListen->repaint(); // 立即重绘
    }
    {
        ui->widget_13->setCornerRadius(10);
        ui->widget_14->setCornerRadius(10);
        ui->widget_18->setCornerRadius(10);
        ui->widget_19->setCornerRadius(10);
        ui->widget_all->setCornerRadius(10);
    }
}

void SpeakerEq::InitMember() {}

void SpeakerEq::InitConnect() {}
void SpeakerEq::ShowEqVal(bool en)
{
    EqShow = en;
    if(en)
    {
        ui->stackedWidget_setEq->setCurrentWidget(ui->page_dataShow);
        ui->stackedWidget->setCurrentWidget(ui->page_eqSet);
    }else
    {
        ui->lab_SysName->setText(tr("当前预设：%1").arg(currentPlanRadio->lab_name->text()));
        ui->stackedWidget_setEq->setCurrentWidget(ui->page_dataHide);
        if(currentPlanVal.ParentPlanName.isEmpty())
        {
            ui->stackedWidget_edit->setCurrentWidget(ui->page_create_edit);
            ui->widget_deriv_dataHide->hide();
        }else
        {
            ui->stackedWidget_edit->setCurrentWidget(ui->page_create_dis);
            ui->lab_parentName_dataHide->setText(currentPlanVal.ParentPlanName);
            ui->widget_deriv_dataHide->show();
        }
        // ui->stackedWidget->setCurrentWidget(ui->page_eqHid);
    }

}
//显示当前方案值
void SpeakerEq::ShowcurrentPlanVal()
{
    ui->widget_eq->hideEditPanelTip();
    emit ApoManager::instance()->requestlogWithTime((QString("SpeakerEq::ShowcurrentPlanVal")));

    if(currentPlanVal.ParentPlanName.isEmpty())
    {
        ui->widget_deriv->hide();
    }else
    {
        ui->widget_deriv->show();
        ui->lab_parentName->setText(tr("源于 %1 方案").arg(currentPlanVal.ParentPlanName));
    }

    emit ApoManager::instance()->requestSetArEffectState(true);//不开启，则无音效
    ShowCurrentPlanGain();
    ShowCurrentPlanEqVal(currentPlanVal);
    m_undoStack_Eq->clear(); // 主动清空，提前释放命令对象

    resizeEvent(NULL);
}
//上传数据，显示曲线
void SpeakerEq::refreshCurve(QVector<double> freq,QVector<double> QVal,QVector<double> GainVal, QVector<int> FilterType,bool en)
{
    int idx = 0;
    int fdx = 10;
    QVector<EQBand> bands;
    for (int i = idx;i< fdx;i++) {
        EQBand band;
        band.filterType = FilterType[i];
        band.enabled = true;
        band.frequency = freq[i];
        band.gain = GainVal[i];
        band.q = QVal[i];
        bands.append(band);
    }
    ui->widget_eq->setBands(bands);
    SetLastVal();
}
//
void SpeakerEq::SwitchEqPage()
{
    ui->pBt_Equalizer->setChecked(true);
}

//仅显示当前方案值，不设置APO
void SpeakerEq::PageShowPlanVal()
{
    emit ApoManager::instance()->requestlogWithTime((QString("SpeakerEq::PageShowPlanVal")));
    // ui->->setFocus();
    if(!EqShow)
    {
        ui->pBt_EQSwitch_hideData->blockSignals(true);
        ui->pBt_EQSwitch_hideData->setChecked(currentPlanVal.eqOpenEn);
        on_pBt_EQSwitch_hideData_toggled(currentPlanVal.eqOpenEn);
        ui->pBt_EQSwitch_hideData->blockSignals(false);
    }else
    {
        if(currentPlanVal.ParentPlanName.isEmpty())
        {
            ui->widget_deriv->hide();

        }else
        {
            ui->widget_deriv->show();
            ui->lab_parentName->setText(tr("源于 %1 方案").arg(currentPlanVal.ParentPlanName));

        }
        ui->pBt_EQSwitch->blockSignals(true);
        ui->pBt_EQSwitch->setChecked(currentPlanVal.eqOpenEn);
        SetEQSwitchShadow(currentPlanVal.eqOpenEn,0);
        ui->pBt_EQSwitch->blockSignals(false);
    }


    ui->hSlider_GainVal->blockSignals(true);
    ui->hSlider_GainVal->setValue(currentPlanVal.GainVal);
    ui->lab_GainVal->setText(QString::number(currentPlanVal.GainVal));
    ui->hSlider_GainVal->blockSignals(false);

    ui->hSlider_Bass->blockSignals(true);
    ui->hSlider_Bass->setValue(currentPlanVal.lowVal);
    ui->lab_BassVal->setText(QString::number(currentPlanVal.lowVal));
    ui->hSlider_Bass->blockSignals(false);

    ui->widget_drc->blockSignals(true);
    ui->widget_drc->setDrcLevel(currentPlanVal.drcVal);
    ui->widget_drc->blockSignals(false);


    ui->hSlider_Space->blockSignals(true);
    ui->hSlider_Space->setValue(currentPlanVal.spaceVal);
    ui->lab_SpacsVal->setText(QString::number(currentPlanVal.spaceVal));
    ui->widget_Horn->setButtonEnabled(currentPlanVal.spaceVal!=0);
    ui->hSlider_Space->blockSignals(false);

    ui->hSlider_Reverb->blockSignals(true);
    ui->hSlider_Reverb->setValue(currentPlanVal.spaceReverb);
    ui->lab_Reverb->setText(QString::number(currentPlanVal.spaceReverb));
    ui->hSlider_Reverb->blockSignals(false);

    QVector<double> freq;
    QVector<double> QVal;
    QVector<double> GainVal;
    QVector<int> FilterVal;
    QVector<double> freq2;
    QVector<double> QVal2;
    QVector<double> GainVal2;
    QVector<int> FilterVal2;

    //频点值

    // 使用智能指针自动管理验证器内存
    for (size_t i = 0; i < 10; ++i)
    {
        double value = currentPlanVal.freqVal[i];
        double freqValue;
        // if (value >= 1000) {
        //     freqValue = static_cast<int>(value * 1000); // KHz -> Hz
        // } else {
        // freqValue = static_cast<int>(value);
        //}
        // 限制范围
        freqValue = qBound(20.0, value, 20000.0);//20Hz~20kHz

        freq.append(static_cast<double>(freqValue));
    }
    for (size_t i = 0; i < 10; ++i)
    {
        double value = currentPlanVal.freqVal_deriv[i];
        double freqValue;
        // if (value >= 1000) {
        //     freqValue = static_cast<int>(value * 1000); // KHz -> Hz
        // } else {
        // freqValue = static_cast<int>(value);
        //}
        // 限制范围
        freqValue = qBound(20.0, value, 20000.0);//20Hz~20kHz

        freq2.append(static_cast<double>(freqValue));
    }

    //Q值
    // 使用智能指针自动管理验证器内存
    for (size_t i = 0; i < 10; ++i)
    {
        QVal.append(currentPlanVal.qVal[i]);
    }
    for (size_t i = 0; i < 10; ++i)
    {
        QVal2.append(currentPlanVal.qVal_deriv[i]);
    }


    //增值
    for (size_t i = 0; i < 10; ++i)
    {
        GainVal.append(currentPlanVal.eqVal[i]);
    }
    for (size_t i = 0; i < 10; ++i)
    {
        GainVal2.append(currentPlanVal.eqVal_deriv[i]);
    }

    //滤波器
    for (size_t i = 0; i < 10; ++i)
    {
        FilterVal.append(currentPlanVal.filterVal[i]);
    }
    for (size_t i = 0; i < 10; ++i)
    {
        FilterVal2.append(currentPlanVal.filterVal_deriv[i]);
    }

    m_undoStack_Eq->clear(); // 主动清空，提前释放命令对象
    bool ifDeriv = currentPlanVal.ParentPlanName.isEmpty();
    emit ApoManager::instance()->requestlogWithTime((QString("refreshCurve前")));
    //画EQ曲线
    if(ifDeriv)//
    {
        refreshCurve(freq,QVal,GainVal,FilterVal,0);
    }else
    {
        refreshCurve(freq2,QVal2,GainVal2,FilterVal2,0);
    }
    emit ApoManager::instance()->requestlogWithTime((QString("refreshCurve后")));

    resizeEvent(NULL);
}


//显示当前方案的均衡器值，并设置eq,设置APO
void SpeakerEq::ShowCurrentPlanEqVal(PlanVal PlanVal)
{
    QVector<double> freq;
    QVector<double> QVal;
    QVector<double> GainVal;
    QVector<int> FilterVal;

    QVector<double> freq2;
    QVector<double> QVal2;
    QVector<double> GainVal2;
    QVector<int> FilterVal2;



    //频点值
    // 使用智能指针自动管理验证器内存
    for (size_t i = 0; i < 10; ++i)
    {
        double value = PlanVal.freqVal[i];
        double freqValue;
        // if (value >= 1000) {
        //     freqValue = static_cast<int>(value * 1000); // KHz -> Hz
        // } else {
        // freqValue = static_cast<int>(value);
        //}
        // 限制范围
        freqValue = qBound(20.0, value, 20000.0);//20Hz~20kHz

        freq.append(static_cast<double>(freqValue));
    }
    for (size_t i = 0; i < 10; ++i)
    {
        double value = PlanVal.freqVal_deriv[i];
        double freqValue;
        // if (value >= 1000) {
        //     freqValue = static_cast<int>(value * 1000); // KHz -> Hz
        // } else {
        // freqValue = static_cast<int>(value);
        //}
        // 限制范围
        freqValue = qBound(20.0, value, 20000.0);//20Hz~20kHz

        freq2.append(static_cast<double>(freqValue));
    }

    //Q值
    // 使用智能指针自动管理验证器内存
    for (size_t i = 0; i < 10; ++i)
    {
        QVal.append(PlanVal.qVal[i]);
    }
    for (size_t i = 0; i < 10; ++i)
    {
        QVal2.append(PlanVal.qVal_deriv[i]);
    }


    //增值
    for (size_t i = 0; i < 10; ++i)
    {
        GainVal.append(PlanVal.eqVal[i]);
    }
    for (size_t i = 0; i < 10; ++i)
    {
        GainVal2.append(PlanVal.eqVal_deriv[i]);
    }

    //滤波器
    for (size_t i = 0; i < 10; ++i)
    {
        FilterVal.append(PlanVal.filterVal[i]);
    }
    for (size_t i = 0; i < 10; ++i)
    {
        FilterVal2.append(PlanVal.filterVal_deriv[i]);
    }


    bool ifDeriv = PlanVal.ParentPlanName.isEmpty();
    //画EQ曲线
    if(ifDeriv)//
    {
        refreshCurve(freq,QVal,GainVal,FilterVal,0);
    }else
    {
        refreshCurve(freq2,QVal2,GainVal2,FilterVal2,0);
    }


    {
        SetEQSwitchShadow(PlanVal.eqOpenEn,0);


        emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(0,freq);//设置频点
        emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(1,freq2);//设置频点
        //emit ApoManager::instance()->requestGetEqualizerCenterFrequencyEx();
        emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(0,QVal);//设置Q值
        emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(1,QVal2);//设置Q值
        emit ApoManager::instance()->requestSetExtendEqualizerGainEx(0,GainVal);//设置增益值
        emit ApoManager::instance()->requestSetExtendEqualizerGainEx(1,GainVal2);//设置增益值
        //emit ApoManager::instance()->requestGetEqualizerGainEx();
    }


    if(!EqShow)
    {
        ui->pBt_EQSwitch_hideData->blockSignals(true);
        ui->pBt_EQSwitch_hideData->setChecked(PlanVal.eqOpenEn);
        on_pBt_EQSwitch_hideData_toggled(PlanVal.eqOpenEn);
        ui->pBt_EQSwitch_hideData->blockSignals(false);
    }else
    {
        if(PlanVal.ParentPlanName.isEmpty())
        {
            ui->widget_deriv->hide();

        }else
        {
            ui->widget_deriv->show();
            ui->lab_parentName->setText(tr("源于 %1 方案").arg(PlanVal.ParentPlanName));

        }
        ui->pBt_EQSwitch->blockSignals(true);
        ui->pBt_EQSwitch->setChecked(PlanVal.eqOpenEn);
        // SetEQSwitchShadow(currentPlanVal.eqOpenEn,0);
        ui->pBt_EQSwitch->blockSignals(false);
    }




}
//显示当前方案的算法、空间音频、drc，设置APO
void SpeakerEq::ShowCurrentPlanGain()
{
    //设置算法开关
    SetEQSwitchShadow(currentPlanVal.AlgoOpenEn,1);

    //DRC
    SetEQSwitchShadow(currentPlanVal.drcOpenEn,3);
    ui->widget_drc->blockSignals(true);
    ui->widget_drc->setDrcLevel(currentPlanVal.drcVal);
    ui->widget_drc->blockSignals(false);
    //增益
    ui->hSlider_GainVal->blockSignals(true);
    ui->hSlider_GainVal->setValue(currentPlanVal.GainVal);
    ui->lab_GainVal->setText(QString::number(currentPlanVal.GainVal));
    ui->hSlider_GainVal->blockSignals(false);
    // emit ApoManager::instance()->requestSetGlobalInputGainDb(currentPlanVal.GainVal);//该函数即是开关又是数值
    //低音
    ui->hSlider_Bass->blockSignals(true);
    ui->hSlider_Bass->setValue(currentPlanVal.lowVal);
    ui->lab_BassVal->setText(QString::number(currentPlanVal.lowVal));
    ui->hSlider_Bass->blockSignals(false);
    emit ApoManager::instance()->requestSetCompBassCenterFrequency(60.0);
    emit ApoManager::instance()->requestSetBassBoostGain(currentPlanVal.lowVal);//apo设置低音数值

    //算法（脚步增强....）
    ui->hSlider_Footsteps->setValue(currentPlanVal.ExtraEq[0]);//脚步增强
    setSliderValueForceSignal(ui->hSlider_Footsteps, currentPlanVal.ExtraEq[0]);
    ui->hSlider_Gunshot->setValue(currentPlanVal.ExtraEq[1]);//枪声弱化
    setSliderValueForceSignal(ui->hSlider_Gunshot, currentPlanVal.ExtraEq[1]);
    ui->hSlider_Sfc->setValue(currentPlanVal.ExtraEq[2]);//声场控制
    setSliderValueForceSignal(ui->hSlider_Sfc, currentPlanVal.ExtraEq[2]);
    ui->hSlider_Clarity->setValue(currentPlanVal.ExtraEq[3]);//清晰度
    setSliderValueForceSignal(ui->hSlider_Clarity, currentPlanVal.ExtraEq[3]);
    ui->hSlider_LingeringSound->setValue(currentPlanVal.ExtraEq[4]);//余音消除
    setSliderValueForceSignal(ui->hSlider_LingeringSound, currentPlanVal.ExtraEq[4]);
    ui->hSlider_SpatialReverb->setValue(currentPlanVal.ExtraEq[5]);//空间混响
    setSliderValueForceSignal(ui->hSlider_SpatialReverb, currentPlanVal.ExtraEq[5]);
    ui->hSlider_Wind->setValue(currentPlanVal.ExtraEq[6]);//风声弱化
    setSliderValueForceSignal(ui->hSlider_Wind, currentPlanVal.ExtraEq[6]);

    //空间音频
    SetEQSwitchShadow(currentPlanVal.spaceOpenEn,2);
    ui->hSlider_Space->blockSignals(true);
    ui->hSlider_Space->setValue(currentPlanVal.spaceVal);
    ui->lab_SpacsVal->setText(QString::number(currentPlanVal.spaceVal));
    ui->widget_Horn->setButtonEnabled(currentPlanVal.spaceVal!=0);
    ui->hSlider_Space->blockSignals(false);

    ui->hSlider_Reverb->blockSignals(true);
    ui->hSlider_Reverb->setValue(currentPlanVal.spaceReverb);
    ui->lab_Reverb->setText(QString::number(currentPlanVal.spaceReverb));
    ui->hSlider_Reverb->blockSignals(false);


    switch(currentPlanVal.spaceSize)
    {
    case 0:
        ui->pBt_Environment_Small->blockSignals(true);
        ui->pBt_Environment_Small->setChecked(true);
        ui->pBt_Environment_Small->blockSignals(false);
        on_pBt_Environment_Small_toggled(true);
        break;
    case 1:
        ui->pBt_Environment_Medium->blockSignals(true);
        ui->pBt_Environment_Medium->setChecked(true);
        ui->pBt_Environment_Medium->blockSignals(false);
        on_pBt_Environment_Medium_toggled(true);
        break;
    case 2:
        ui->pBt_Environment_Large->blockSignals(true);
        ui->pBt_Environment_Large->setChecked(true);
        ui->pBt_Environment_Large->blockSignals(false);
        on_pBt_Environment_Large_toggled(true);
        break;
    default:
        break;
    }



    // emit ApoManager::instance()->requestSetSurroundState(currentPlanVal.spaceOpenEn);//空间环绕关闭
    // emit ApoManager::instance()->requestSetReverbFilter(E_REVERB_ROOM_CHURCH);//房间类型
    emit ApoManager::instance()->requestSetDistance(currentPlanVal.spaceVal);//设置空间环绕数值
    emit ApoManager::instance()->requestSetArReverbRatio(currentPlanVal.spaceReverb);//设置空间混响数值
    // emit ApoManager::instance()->requestSetReverbState(currentPlanVal.spaceOpenEn);//设置空间使能


}


void SpeakerEq::resetVal()
{
    emit ApoManager::instance()->requestlogWithTime((QString("SpeakerEq::resetVal")));
    PageShowPlanVal();
}

//非系统方案可重置整个方案的值（EQ+算法+空间），系统方案只可重置（算法+空间），只重置当前页面的数据
void SpeakerEq::on_pBt_EqReset_clicked()
{
    ui->widget_eq->hideEditPanel();
    emit PlanReset_E(group_page->checkedId());
    qDebug("group_page->checkedId():%d\n",group_page->checkedId());

}


//低音
void SpeakerEq::on_hSlider_Bass_valueChanged(int value)
{

    // qDebug("低音值修改%d,lVals.bass:%d\n",value,lVals.bass);
    ui->lab_BassVal->setText(QString::number(value));

    currentPlanVal.lowVal = value;

    emit ApoManager::instance()->requestlogWithTime("SpeakerEq:on_hSlider_Bass_valueChanged");
    emit ApoManager::instance()->requestSetBassBoostGain(value);


    if(value == 0)
    {
        ui->pBt_low_sub->setEnabled(false);
        emit ApoManager::instance()->requestSetBassBoostState(false);
    }else
    {
        ui->pBt_low_sub->setEnabled(true);
        if(value == ui->hSlider_Bass->maximum())
        {
            ui->pBt_low_add->setEnabled(false);
        }else
        {
            ui->pBt_low_add->setEnabled(true);
        }
        emit ApoManager::instance()->requestSetBassBoostState(true);
    }



    //int currentVal = ui->hSlider_Bass->value();
    if (value != lVals.bass) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Bass,ui->lab_BassVal,lVals.bass,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.bass = value;
        // qDebug("低音值添加lVals.bass:%d\n",lVals.bass);
    }

    emit PlanSave_E();
}


//空间
void SpeakerEq::on_hSlider_Space_valueChanged(int value)
{
    // 更新图标尺寸并调整位置（以当前环境为中心缩放）
    ui->widget_Horn->setGlobalScale(value);

    ui->lab_SpacsVal->setText(QString::number(value));
    currentPlanVal.spaceVal = value;
    // currentPlanVal.spaceOpenEn = true;

    if(value > 0)
    {
        emit ApoManager::instance()->requestSetSurroundState(true);
    }else
    {
        emit ApoManager::instance()->requestSetSurroundState(false);
    }

    emit ApoManager::instance()->requestSetDistance(value);



    // 撤销记录（原有逻辑）
    if (value != lVals.space) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Space, ui->lab_SpacsVal, lVals.space, value, 0);
        m_undoStack_Eq->push(cmd);
        lVals.space = value;
    }
    emit PlanSave_E();
}
//空间环境类型小
void SpeakerEq::on_pBt_Environment_Small_toggled(bool checked)
{
    if(checked) {
        ui->widget_Horn->setMode(0);

        emit ApoManager::instance()->requestSetReverbFilter(E_REVERB_ROOM_STUDIO);
        currentPlanVal.spaceSize = 0;
        emit PlanSave_E();

        // 撤销处理（保持原有逻辑）
        if (0 != lVals.spaceSize) {
            UndoSpaceSize* cmd = new UndoSpaceSize(group_space, lVals.spaceSize, 0);
            m_undoStack_Eq->push(cmd);
            lVals.spaceSize = 0;
        }
    }
}
//空间环境类型中
void SpeakerEq::on_pBt_Environment_Medium_toggled(bool checked)
{
    if(checked)
    {
        qDebug("on_pBt_Environment_Medium_toggled\n");
        ui->widget_Horn->setMode(1);


        emit ApoManager::instance()->requestSetReverbFilter(E_REVERB_ROOM_THEATER);//房间类型
        currentPlanVal.spaceSize = 1;
        emit PlanSave_E();

        if (1 != lVals.spaceSize) {
            // 创建并执行撤销命令
            UndoSpaceSize* cmd = new UndoSpaceSize(group_space, lVals.spaceSize, 1);
            m_undoStack_Eq->push(cmd);  // push 会自动调用 redo()

            // 更新业务数据
            lVals.spaceSize = 1;
        }
    }

}

//空间环境类型大
void SpeakerEq::on_pBt_Environment_Large_toggled(bool checked)
{
    if(checked)
    {
        qDebug("on_pBt_Environment_Large_toggled\n");
        ui->widget_Horn->setMode(2);


        emit ApoManager::instance()->requestSetReverbFilter(E_REVERB_ROOM_CONCERT);//房间类型
        currentPlanVal.spaceSize = 2;
        emit PlanSave_E();

        if (2 != lVals.spaceSize) {
            // 创建并执行撤销命令
            UndoSpaceSize* cmd = new UndoSpaceSize(group_space, lVals.spaceSize, 2);
            m_undoStack_Eq->push(cmd);  // push 会自动调用 redo()

            // 更新业务数据
            lVals.spaceSize = 2;
        }
    }

}
//混响(混响关闭时，房间变化，没效果)
void SpeakerEq::on_hSlider_Reverb_valueChanged(int value)
{
    ui->lab_Reverb->setText(QString::number(value));
    currentPlanVal.spaceReverb = value;

    if(value > 0)
    {
        emit ApoManager::instance()->requestSetReverbState(true);
    }else
    {
        emit ApoManager::instance()->requestSetReverbState(false);
    }


    emit ApoManager::instance()->requestSetArReverbRatio(value);

    // 撤销记录（原有逻辑）
    if (value != lVals.spaceReverb) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Reverb, ui->lab_Reverb, lVals.spaceReverb, value, 0);
        m_undoStack_Eq->push(cmd);
        lVals.spaceReverb = value;
    }
    emit PlanSave_E();
}



//方案二创（创建新方案）
void SpeakerEq::on_pBt_deriv_clicked()
{
    emit CreateDerivPlan();
}

//隐藏方案的开关
void SpeakerEq::on_pBt_EQSwitch_hideData_toggled(bool checked)
{
    currentPlanVal.eqOpenEn = checked;

    ui->pBt_deriv->setEnabled(checked);

    //额外EQ(总开关)
    emit ApoManager::instance()->requestSetExtendEqState(0,checked);//主eq
    emit ApoManager::instance()->requestSetExtendEqState(1,checked);//二次修改方案的eq
    emit ApoManager::instance()->requestSetExtendEqState(2,checked);//脚步增强
    emit ApoManager::instance()->requestSetExtendEqState(3,checked);//枪声优化
    qDebug("requestSetExtendEqState 4 %d\n",checked);
    emit ApoManager::instance()->requestSetExtendEqState(4,checked);//声场控制
    emit ApoManager::instance()->requestSetExtendEqState(5,checked);//清晰度
    emit ApoManager::instance()->requestSetExtendEqState(6,checked);//余音消除
    emit ApoManager::instance()->requestSetExtendEqState(7,checked);//空间混响
    emit ApoManager::instance()->requestSetExtendEqState(8,checked);//风声弱化

    //设置算法
    currentPlanVal.AlgoOpenEn = checked;
    //低音增强
    if(currentPlanVal.lowVal>0)
    {
        emit ApoManager::instance()->requestSetBassBoostState(checked);
    }
    //增益
    if(currentPlanVal.GainVal > 0)
    {
        if(checked)
        {
            emit ApoManager::instance()->requestSetGlobalInputGainDb(currentPlanVal.GainVal);
        }else
        {
            emit ApoManager::instance()->requestSetGlobalInputGainDb(0);
        }
    }

    //灵晰算法（DRC）
    currentPlanVal.drcOpenEn = checked;
    emit ApoManager::instance()->requestSetDrcState(checked);
    if(checked)
    {
      emit ui->widget_drc->DrcLevelChanged(currentPlanVal.drcVal);
    }


    currentPlanVal.spaceOpenEn = checked;

    if(currentPlanVal.spaceVal == 0)
    {
        qDebug("requestSetSurroundState false\n");
        emit ApoManager::instance()->requestSetSurroundState(false);
    }else
    {
        qDebug("requestSetSurroundState  %d\n",checked);
        emit ApoManager::instance()->requestSetSurroundState(checked);
    }
    if(currentPlanVal.spaceReverb == 0)
    {
        qDebug("requestSetReverbState false\n");
        emit ApoManager::instance()->requestSetReverbState(false);
    }else
    {
        qDebug("requestSetReverbState  %d\n",checked);
        emit ApoManager::instance()->requestSetReverbState(checked);
    }


    emit PlanSave_E();

}
//值相等也要触发信号
void SpeakerEq::setSliderValueForceSignal(QSlider* slider, int value)
{
    if (!slider) return;
    if (slider->value() != value) {
        slider->setValue(value);
    } else {
        emit slider->valueChanged(value);
    }
}

//脚步增强
void SpeakerEq::on_hSlider_Footsteps_valueChanged(int value)
{
    if(value == 0)
    {
        ui->pBt_Footsteps_sub->setEnabled(false);
        //额外eq
        emit ApoManager::instance()->requestSetExtendEqState(2,false);

    }else
    {
        // //测试
        // QVector<bool> enables(10, true);
        // emit ApoManager::instance()->requestSetExtendEqualizerBandEnableEx(2,enables);//脚步增强

        ui->pBt_Footsteps_sub->setEnabled(true);
        qDebug("requestSetExtendEqState2 true\n");
        emit ApoManager::instance()->requestSetExtendEqState(2,true);
        if(value == ui->hSlider_Footsteps->maximum())
        {
            ui->pBt_Footsteps_add->setEnabled(false);
        }else
        {
            ui->pBt_Footsteps_add->setEnabled(true);
        }
    }

    ui->lab_FootstepsVal->setText(QString::number(value));

    double freq[10] = {80,20000,20000,20000,20000,20000,20000,20000,20000,20000};
    // double freq[10] = {20,75,150,250,700,1500,2000,4000,8000,16000};//测试 {-12,-12,-12,-12,-12,-12,-12,-12,-12,-12}  {0,0,0,0,0,0,0,0,0,0} {-1.6,-2,0,0,0,0,0,0,0,0}
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(2,freqVec);//设置频点
    double QVal[10] = {1.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    QVector<double> QValVec(QVal, QVal + 10);

    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(2,QValVec);//设置Q

    double GVal[7][10] = {{0,0,0,0,0,0,0,0,0,0}
                          ,{1,0,0,0,0,0,0,0,0,0}
                          ,{2,0,0,0,0,0,0,0,0,0}
                          ,{3,0,0,0,0,0,0,0,0,0}
                          ,{4,0,0,0,0,0,0,0,0,0}
                          ,{5,0,0,0,0,0,0,0,0,0}
                          ,{6,0,0,0,0,0,0,0,0,0}};


    QVector<double> GValVec(GVal[value], GVal[value] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(2,GValVec);//设置Gain
    currentPlanVal.ExtraEq[0] = value;
    emit PlanSave_E();

    if (value != lVals.footsteps) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Footsteps,NULL,lVals.footsteps,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.footsteps = value;
    }
}
//枪声弱化
void SpeakerEq::on_hSlider_Gunshot_valueChanged(int value)
{
    if(value == 0)
    {
        ui->pBt_Gunshot_sub->setEnabled(false);
        emit ApoManager::instance()->requestSetExtendEqState(3,false);
    }else
    {
        ui->pBt_Gunshot_sub->setEnabled(true);
        emit ApoManager::instance()->requestSetExtendEqState(3,true);
        if(value == ui->hSlider_Gunshot->maximum())
        {
            ui->pBt_Gunshot_add->setEnabled(false);
        }else
        {
            ui->pBt_Gunshot_add->setEnabled(true);
        }
    }

    ui->lab_GunshotVal->setText(QString::number(value));

    double freq[10] = {5500,7500,20000,20000,20000,20000,20000,20000,20000,20000};
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(3,freqVec);//设置频点
    double QVal[10] = {1.0,0.5,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    QVector<double> QValVec(QVal, QVal + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(3,QValVec);//设置Q


    double GVal[7][10] = {{0,0,0,0,0,0,0,0,0,0}
                          ,{-1,-1,0,0,0,0,0,0,0,0}
                          ,{-2,-2,0,0,0,0,0,0,0,0}
                          ,{-2,-3,0,0,0,0,0,0,0,0}
                          ,{-3,-3,0,0,0,0,0,0,0,0}
                          ,{-3,-4,0,0,0,0,0,0,0}
                          ,{-4,-4,0,0,0,0,0,0,0,0}};

    QVector<double> GValVec(GVal[value], GVal[value] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(3,GValVec);//设置Gain
    currentPlanVal.ExtraEq[1] = value;
    emit PlanSave_E();

    if (value != lVals.gunshot) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Gunshot,NULL,lVals.gunshot,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.gunshot = value;
    }
}
//声场控制
void SpeakerEq::on_hSlider_Sfc_valueChanged(int value)
{
    if(value == 0)
    {
        qDebug("requestSetExtendEqState 4 false\n");
        emit ApoManager::instance()->requestSetExtendEqState(4,false);
    }else
    {
        qDebug("requestSetExtendEqState 4 true\n");
        emit ApoManager::instance()->requestSetExtendEqState(4,true);
    }

    ui->lab_SfcVal->setText(QString::number(value));

    if(value == ui->hSlider_Sfc->minimum())
    {
        ui->pBt_sfc_sub->setEnabled(false);

    }else
    {
        ui->pBt_sfc_sub->setEnabled(true);

        if(value == ui->hSlider_Sfc->maximum())
        {
            ui->pBt_sfc_add->setEnabled(false);
        }else
        {
            ui->pBt_sfc_add->setEnabled(true);
        }
    }
    double freq[10] = {1000,20000,20000,20000,20000,20000,20000,20000,20000,20000};
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(4,freqVec);//设置频点
    double QVal[10] = {0.5,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    QVector<double> QValVec(QVal, QVal + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(4,QValVec);//设置Q

    double GVal[11][10] = {{-6,0,0,0,0,0,0,0,0,0}
                           ,{-4.8,0,0,0,0,0,0,0,0,0}
                           ,{-3.6,0,0,0,0,0,0,0,0,0}
                           ,{-2.4,0,0,0,0,0,0,0,0,0}
                           ,{-1.2,0,0,0,0,0,0,0,0,0}
                           ,{0,0,0,0,0,0,0,0,0,0}
                           ,{1.2,0,0,0,0,0,0,0,0,0}
                           ,{2.4,0,0,0,0,0,0,0,0,0}
                           ,{3.6,0,0,0,0,0,0,0,0,0}
                           ,{4.8,0,0,0,0,0,0,0,0,0}
                           ,{6.0,0,0,0,0,0,0,0,0,0}};

    QVector<double> GValVec(GVal[value+5], GVal[value+5] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(4,GValVec);//设置Gain
    currentPlanVal.ExtraEq[2] = value;
    emit PlanSave_E();

    if (value != lVals.sfc) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Sfc,NULL,lVals.sfc,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.sfc = value;
    }
}
//清晰度控制
void SpeakerEq::on_hSlider_Clarity_valueChanged(int value)
{
    if(value == 0)
    {
        ui->pBt_Clarity_sub->setEnabled(false);
        emit ApoManager::instance()->requestSetExtendEqState(5,false);
    }else
    {
        ui->pBt_Clarity_sub->setEnabled(true);
        emit ApoManager::instance()->requestSetExtendEqState(5,true);
        if(value == ui->hSlider_Clarity->maximum())
        {
            ui->pBt_Clarity_add->setEnabled(false);
        }else
        {
            ui->pBt_Clarity_add->setEnabled(true);
        }
    }

    ui->lab_ClarityVal->setText(QString::number(value));

    double freq[10] = {270,20000,20000,20000,20000,20000,20000,20000,20000,20000};
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(5,freqVec);//设置频点
    double QVal[10] = {2,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    QVector<double> QValVec(QVal, QVal + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(5,QValVec);//设置Q


    double GVal[7][10] = {{0,0,0,0,0,0,0,0,0,0}
                          ,{-2,0,0,0,0,0,0,0,0,0}
                          ,{-4,0,0,0,0,0,0,0,0,0}
                          ,{-6,0,0,0,0,0,0,0,0,0}
                          ,{-8,0,0,0,0,0,0,0,0,0}
                          ,{-10,0,0,0,0,0,0,0,0,0}
                          ,{-12,0,0,0,0,0,0,0,0,0}};

    QVector<double> GValVec(GVal[value], GVal[value] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(5,GValVec);//设置Gain
    currentPlanVal.ExtraEq[3] = value;
    emit PlanSave_E();

    if (value != lVals.clarity) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Clarity,NULL,lVals.clarity,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.clarity = value;
    }

}
//余音消除（只开第一、二、三个频点即可）
void SpeakerEq::on_hSlider_LingeringSound_valueChanged(int value)
{
    if(value == 0)
    {
        ui->pBt_LSound_sub->setEnabled(false);
        emit ApoManager::instance()->requestSetExtendEqState(6,false);
    }else
    {
        ui->pBt_LSound_sub->setEnabled(true);
        emit ApoManager::instance()->requestSetExtendEqState(6,true);
        if(value == ui->hSlider_LingeringSound->maximum())
        {
            ui->pBt_LSound_add->setEnabled(false);
        }else
        {
            ui->pBt_LSound_add->setEnabled(true);
        }
    }

    ui->lab_LingeringSoundVal->setText(QString::number(value));

    double freq[10] = {20,150,450,20000,20000,20000,20000,20000,20000,20000};
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(6,freqVec);//设置频点

    double QVal[7][10] = {{0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7}
                          ,{0.8,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7}
                          ,{0.8,2.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7}
                          ,{0.8,2.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7}
                          ,{0.8,2.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7}
                          ,{0.8,0.7,1.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7}
                          ,{0.8,0.7,1.0,0.7,0.7,0.7,0.7,0.7,0.7,0.7}};
    QVector<double> QValVec(QVal[value], QVal[value] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(6,QValVec);//设置Q


    double GVal[7][10] = {{0,0,0,0,0,0,0,0,0,0}
                          ,{-1,0,0,0,0,0,0,0,0,0}
                          ,{-2,-1,0,0,0,0,0,0,0,0}
                          ,{-3,-2,0,0,0,0,0,0,0,0}
                          ,{-4,-3,-1,0,0,0,0,0,0,0}
                          ,{-5,0,-2,0,0,0,0,0,0,0}
                          ,{-6,0,-3,0,0,0,0,0,0,0}};

    QVector<double> GValVec(GVal[value], GVal[value] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(6,GValVec);//设置Gain
    currentPlanVal.ExtraEq[4] = value;
    emit PlanSave_E();

    if (value != lVals.lingeringSound) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_LingeringSound,NULL,lVals.lingeringSound,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.lingeringSound = value;
    }

}
//空间混响控制
void SpeakerEq::on_hSlider_SpatialReverb_valueChanged(int value)
{
    if(value == 0)
    {
        qDebug("requestSetExtendEqState 4 false\n");
        emit ApoManager::instance()->requestSetExtendEqState(7,false);
    }else
    {
        qDebug("requestSetExtendEqState 4 true\n");
        emit ApoManager::instance()->requestSetExtendEqState(7,true);
    }

    ui->lab_SpatialReverbVal->setText(QString::number(value));

    if(value == ui->hSlider_SpatialReverb->minimum())
    {
        ui->pBt_SReverb_sub->setEnabled(false);
    }else
    {
        ui->pBt_SReverb_sub->setEnabled(true);
        if(value == ui->hSlider_SpatialReverb->maximum())
        {
            ui->pBt_SReverb_add->setEnabled(false);
        }else
        {
            ui->pBt_SReverb_add->setEnabled(true);
        }
    }
    double freq[10] = {450,20000,20000,20000,20000,20000,20000,20000,20000,20000};
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(7,freqVec);//设置频点
    double QVal[10] = {2,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    QVector<double> QValVec(QVal, QVal + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(7,QValVec);//设置Q

    double GVal[11][10] = {{-5,0,0,0,0,0,0,0,0,0}
                           ,{-4,0,0,0,0,0,0,0,0,0}
                           ,{-3,0,0,0,0,0,0,0,0,0}
                           ,{-2,0,0,0,0,0,0,0,0,0}
                           ,{-1,0,0,0,0,0,0,0,0,0}
                           ,{0,0,0,0,0,0,0,0,0,0}
                           ,{1,0,0,0,0,0,0,0,0,0}
                           ,{2,0,0,0,0,0,0,0,0,0}
                           ,{3,0,0,0,0,0,0,0,0,0}
                           ,{4,0,0,0,0,0,0,0,0,0}
                           ,{5,0,0,0,0,0,0,0,0,0}};

    QVector<double> GValVec(GVal[value+5], GVal[value+5] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(7,GValVec);//设置Gain
    currentPlanVal.ExtraEq[5] = value;
    emit PlanSave_E();

    if (value != lVals.spatialReverb) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_SpatialReverb,NULL,lVals.spatialReverb,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.spatialReverb = value;
    }
}

//风声弱化
void SpeakerEq::on_hSlider_Wind_valueChanged(int value)
{
    if(value == 0)
    {
        ui->pBt_wind_sub->setEnabled(false);
        emit ApoManager::instance()->requestSetExtendEqState(8,false);
    }else
    {
        ui->pBt_wind_sub->setEnabled(true);
        emit ApoManager::instance()->requestSetExtendEqState(8,true);
        if(value == ui->hSlider_Wind->maximum())
        {
            ui->pBt_wind_add->setEnabled(false);
        }else
        {
            ui->pBt_wind_add->setEnabled(true);
        }
    }

    ui->lab_WindVal->setText(QString::number(value));

    double freq[10] = {500,1000,16000,20000,20000,20000,20000,20000,20000,20000};
    QVector<double> freqVec(freq, freq + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerCenterFrequencyEx(8,freqVec);//设置频点
    double QVal[10] = {1,1,0.5,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    QVector<double> QValVec(QVal, QVal + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerBandQualityEx(8,QValVec);//设置Q

    double GVal[7][10] = {{0,0,0,0,0,0,0,0,0,0}
                          ,{-0.8,-0.5,-1,0,0,0,0,0,0,0}
                          ,{-1.6,-1,-2,0,0,0,0,0,0,0}
                          ,{-2.4,-1.5,-3,0,0,0,0,0,0}
                          ,{-3.2,-2,-4,0,0,0,0,0,0,0}
                          ,{-4,-2.5,-5,0,0,0,0,0,0,0}
                          ,{-4.8,-3,-6,0,0,0,0,0,0,0}};

    QVector<double> GValVec(GVal[value], GVal[value] + 10);
    emit ApoManager::instance()->requestSetExtendEqualizerGainEx(8,GValVec);//设置Gain
    currentPlanVal.ExtraEq[6] = value;
    emit PlanSave_E();

    if (value != lVals.wind) {
        UndoSliderVal *cmd = new UndoSliderVal(ui->hSlider_Wind,NULL,lVals.wind,value,0);
        m_undoStack_Eq->push(cmd);
        lVals.wind = value;
    }
}



bool SpeakerEq::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *widget = qobject_cast<QWidget *>(watched);

    if (widget && cl_need_checked_.contains(widget)) {
        QString objName = widget->objectName();

        // 单独处理
        if (objName.contains("widget_eq")) {
            // switch (event->type()) {
            // case QEvent::MouseButtonPress: {
            //     QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            //     // 获取点击的内部子控件
            //     QWidget *clickedChild = widget->childAt(mouseEvent->pos());
            //     if (clickedChild) {
            //         if (!ui->pBt_EQSwitch->isChecked())
            //             emit ui->pBt_EQSwitch->setChecked(true);
            //         return false;
            //     }

            // }
            // case QEvent::MouseMove:
            // case QEvent::MouseButtonRelease:
            // default:
            //     return false;
            // }
            EQCurveWidget *t_eq_curve_widget = qobject_cast<EQCurveWidget *>(widget);
            if (t_eq_curve_widget) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
                // 左键 点击在 有效的 圆形标记上
                if ((t_eq_curve_widget->hitTestBand(mouseEvent->pos()) != -1)  && mouseEvent->button () == Qt::LeftButton) {
                    if (!ui->pBt_EQSwitch->isChecked())
                        emit ui->pBt_EQSwitch->setChecked(true);
                    return false;
                }
            }
        } else {
            // 其他控件 只需要判断 鼠标按下 事件
            if (event->type() == QEvent::MouseButtonPress && !widget->isEnabled()) {
                if (!ui->pBt_EQSwitch->isChecked())
                    emit ui->pBt_EQSwitch->setChecked(true);
            }
        }
    }
    // 不拦截事件，让控件正常处理
    return false;
}
//关闭当前页面，进入方案库页面
void SpeakerEq::on_pBt_ClosePlanPage_clicked()
{
    emit CurrentpageChange();
}
//关闭系统方案当前页面，进入方案库页面
void SpeakerEq::on_pBt_SysClose_clicked()
{
    emit CurrentpageChange();
}


// void SpeakerEq::paintEvent(QPaintEvent *event)
// {
//     NewRadioBtnText *child1 = findChild<NewRadioBtnText *>("rBt_currentPlan");
//     if (!child1)
//         return;

//     QPainter painter(this);
//     painter.setRenderHint(QPainter::Antialiasing);
//     painter.translate(21, 9);   // 统一偏移

//     QPainterPath redPath;
//     QRect btnRect = child1->geometry();
//     int bottomH = 16;       // 矩形高度，与扇形半径相同
//     int btnRadius = 10;    // 按钮底部圆角半径
//     int Radius = 16;     // 右下角扇形半径

//     // 1. 按钮正下方矩形（宽度向右扩展 Radius，与扇形重叠，消除缝隙）
//     redPath.addRect(btnRect.left(), btnRect.bottom()+1,
//                     btnRect.width() + Radius, bottomH);

//     // 2. 右下角扇形（圆心位于按钮右下角右侧+半径，半径16，角度180°~270°）
//     if (Radius > 0) {
//         qreal rad = Radius;
//         QPointF center(btnRect.right() + rad + 1, btnRect.bottom() + 1);
//         QRectF circleRect(center.x() - rad, center.y() - rad,2 * rad, 2 * rad);
//         QPainterPath sector;
//         sector.moveTo(center);
//         sector.arcTo(circleRect, 180, 90);
//         sector.closeSubpath();
//         redPath.addPath(sector);
//     }

//     // 3. 按钮左右下角的圆角区域（半径10，过渡矩形与按钮之间的缝隙）
//     if (btnRadius > 0) {
//         qreal rad = btnRadius;
//         QRectF r = btnRect;

//         // 左下角扇形
//         QPainterPath leftCorner;
//         leftCorner.moveTo(r.left(), r.bottom());
//         leftCorner.arcTo(r.left(), r.bottom() - 2*rad, 2*rad, 2*rad, 180, 90);
//         leftCorner.closeSubpath();
//         redPath.addPath(leftCorner);

//         // 右下角扇形
//         QPainterPath rightCorner;
//         rightCorner.moveTo(r.right(), r.bottom());
//         rightCorner.arcTo(r.right() - 2*rad, r.bottom() - 2*rad, 2*rad, 2*rad, 270, 90);
//         rightCorner.closeSubpath();
//         redPath.addPath(rightCorner);
//     }

//     painter.fillPath(redPath, QColor(81, 96, 122, 51));
// }

//切换主题
void SpeakerEq::setThemeAndPanelTransparency_SpeakerEq(int idx,int PValue)
{

}
//设置面板透明度（参数：主题，透明度）
void SpeakerEq::setPanelTransparency_SpeakerEq(int idx,int PValue)
{
    double PanelTransparency = PValue / 100.0;   //面板透明度(默认值是0.2)
    int r, g, b;
    QString suffix;
    switch (idx)
    {
    case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
    case 1: suffix = "_white";  break;//白色
    case 2: suffix = "_black";  break;//黑色
    default: suffix = "";      break;
    }
    //当前预设
    ui->rBt_currentPlan->setThemeAndPanelTransparency(idx,PValue);

    switch (idx) {
    case 0:
        r = 81; g = 96; b = 122; break;// 深蓝色
    case 1:
        r = 81; g = 96; b = 122; break;// 白色
    case 2:
        r = 81; g = 96; b = 122; break; // 黑色
    default:
        r = 81; g = 96; b = 122; break;
    }

    QString colorStr = QString("rgba(%1, %2, %3, %4)")
                           .arg(r).arg(g).arg(b).arg(PanelTransparency);
    QColor background = QColor(r, g, b, PanelTransparency);
    switch (idx) {
    case 0:
        r = 255; g = 255; b = 255; break;// 深蓝色
    case 1:
        r = 255; g = 255; b = 255; break;// 白色
    case 2:
        r = 255; g = 255; b = 255; break; // 黑色
    default:
        r = 255; g = 255; b = 255; break;
    }
    QString colorStr2 = QString("rgba(%1, %2, %3, %4)")
                            .arg(r).arg(g).arg(b).arg(PanelTransparency);
    //我的收藏
    ui->widget_all->setStyleSheet(QString("background-color: %1;border-radius: 10px;").arg(colorStr));
    //方案库
    ui->pBt_Plans->setBackground(background);
    //试听
    ui->pBt_GameListen->setBackground(background);

    //均衡器等功能页面 rgba(81, 96, 122, 0.2)
    ui->stackedWidget_setEq->setStyleSheet(
        QString("border-top-left-radius: 0px;"
                "border-top-right-radius: 10px;"
                "border-bottom-left-radius: 10px;"
                "border-bottom-right-radius: 10px;"
                "background-color: %1;"
                "border-image:none;"
                )
        .arg(colorStr)
        );


}
//设置面板模糊度
void SpeakerEq::setPanelBlur_SpeakerEq(int PValue)
{

}

//只有T7系列（T7,T7 GT）显示灵晰算法
void SpeakerEq::ShowDRC()
{
    if( (SelDev_DeviceName.contains("T7 GT",Qt::CaseInsensitive) && (SelDev_PID == 0xF009 || SelDev_PID == 0xF015))
        ||(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive) && (SelDev_PID == 0xF014 || SelDev_PID == 0xF008)) )
    {
        ui->pBt_Drc->setVisible(true);
    }else
    {
        ui->pBt_Drc->setVisible(false);
    }
}
