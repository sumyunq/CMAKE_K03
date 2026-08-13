#include "MicSet.h"
#include "APOThread/ApoManager.h"
#include "LoadApoDLL.h"
#include "LoadLib.h"
#include "ui_MicSet.h"

MicSet::MicSet(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MicSet)
{
    ui->setupUi(this);

    ui->pBt_listening->setEnabled(true);
    //ui->pBt_NR->setChecked(false);
    ui->pBt_NR->setEnabled(true);

    tip_ai = new NewCustomToolTip(this);
    tip_ai->setLabelStyle(0);
    tip_ai->AddToolTip(ui->lab_tooptip_ai,
                       tr("智能识别并过滤环境噪音，提升语音清晰度。拖动滑块调节降噪强度，数值越高过滤越彻底。"),
                       Qt::AlignCenter);
    tip_listening = new NewCustomToolTip(this);
    tip_listening->setLabelStyle(0);
    tip_listening->AddToolTip(ui->lab_tooptip_listening,
                       tr("开启后可在耳机中实时听到自己的麦克风声音，便于控制说话音量和语调。"),
                       Qt::AlignCenter);
    tip_ClearVocals = new NewCustomToolTip(this);
    tip_ClearVocals->setLabelStyle(0);
    tip_ClearVocals->AddToolTip(ui->lab_tooptip_ClearVocals,
                       tr("增强麦克风人声的中高频，让语音更锐利透亮。"),
                       Qt::AlignCenter);
    tip_RichVocals = new NewCustomToolTip(this);
    tip_RichVocals->setLabelStyle(0);
    tip_RichVocals->AddToolTip(ui->lab_tooptip_RichVocals,
                       tr("增强麦克风人声的低频厚度，让声音更有力度和磁性。"),
                       Qt::AlignCenter);

    // ui->hSlider_AI->setType(1, 15, 5, true);

    ui->pBt_listening->setChecked(false);
    //ui->pBt_listening->setEnabled(false);//内测版灰掉

    ui->widget_top->hide();//不存在变声器时，隐藏
}

MicSet::~MicSet()
{
    delete ui;
}

void MicSet::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // UpdateShadowLabelSize(lab_AIshadow_Top);
    // UpdateShadowLabelSize(lab_AIshadow_Buttom);
}

void MicSet::Test()
{
    on_pBt_NR_toggled(false);
}
//若上行驱动未正常识别到，则不可使用上行功能
void MicSet::setUpEn(bool en)
{
    ui->pBt_NR->setEnabled(en);
    ui->pBt_ClearVocals->setEnabled(en);
    ui->pBt_RichVocals->setEnabled(en);
}


void MicSet::startListening()
{
    if (m_pipeline) {
        return;
    }

    m_pipeline = std::make_unique<WASAPIPipeline>();
    if (!m_pipeline->initialize()) {
        // 初始化失败，显示错误提示
        ui->pBt_listening->setChecked(false);
        ui->pBt_listening->setText(tr("开"));
        m_pipeline.reset();
        // TODO: 添加错误提示对话框
        return;
    }

    m_pipeline->start();
}
void MicSet::stopListening()
{
    if (m_pipeline) {
        m_pipeline->stop();
        m_pipeline.reset();
    }
}



//麦克风侦听设置
void MicSet::on_pBt_listening_toggled(bool checked)
{
    // if(isHidRun)
    // {
    //     int res = lolib->SetMicListening(checked);
    //     if (res < 0) {
    //         //qDebug("Unable to write()\n");
    //         msgBox.critical(NULL,tr("错误"),tr("麦克风侦听设定失败"));
    //     }
    // }
    if (checked) {
        startListening();
    } else {
        stopListening();
    }
    globalSettings->setValue(QString("MIC/listeningEn"), checked);
}
//麦克风降噪开关设置
void MicSet::on_pBt_NR_toggled(bool checked)
{
    // if (checked) {
    //     ui->hSlider_AI->animateHandleColor(QColor("#C7C7C7"), QColor("#FFFFFF"), 100);
    //     ui->hSlider_AI->animateFillColor(QColor("#006184"), QColor("#0091C6"), 100);
    // } else {
    //     ui->hSlider_AI->animateHandleColor(QColor("#FFFFFF"), QColor("#C7C7C7"), 100);
    //     ui->hSlider_AI->animateFillColor(QColor("#0091C6"), QColor("#006184"), 100);
    // }
    ui->hSlider_AI->setEnabled(checked);

    emit ApoManager::instance()->requestSetAINSEnable(checked);
    globalSettings->setValue(QString("MIC/Noise Reduction En"), checked);
}
//麦克风AI降噪值改变
void MicSet::on_hSlider_AI_valueChanged(int value)
{
    int val = ui->hSlider_AI->sliderPosition();
    ui->lab_AiVal->setText(QString::number(val));
    if(val == ui->hSlider_AI->maximum())
    {
        ui->pBt_AI_add->setEnabled(false);
    }else
    {
        ui->pBt_AI_add->setEnabled(true);
    }

    if(val == ui->hSlider_AI->minimum())
    {
        ui->pBt_AI_sub->setEnabled(false);
    }else
    {
        ui->pBt_AI_sub->setEnabled(true);
    }

    emit ApoManager::instance()->requestSetAINSLevel(val*10);
    globalSettings->setValue(QString("MIC/Noise Reduction Val"), val);
}

void MicSet::LanguageSet()
{
    //刷新文本
    ui->retranslateUi(this);

    // if (LanguageIdx == 0) //简体
    // {
    //     tip_ai->AddToolTip(ui->lab_ai,
    //                        tr("通过AI算法，过滤麦克风收到的环境杂音让人声更加清晰"),
    //                        Qt::AlignCenter);
    // } else if (LanguageIdx == 1) //繁體
    // {
    //     tip_ai->AddToolTip(ui->lab_ai,
    //                        tr("通過AI算法，過濾麥克風收到的環境雜音讓人聲更加清晰"),
    //                        Qt::AlignCenter);
    // } else if (LanguageIdx == 2) //英文
    // {
    //     tip_ai->AddToolTip(ui->lab_ai,
    //                        tr("Using AI algorithms to filter out ambient noise from the microphone "
    //                           "input, making the voice clearer"),
    //                        Qt::AlignCenter);
    // }
}
//保存文件
void MicSet::saveIniValue(
    bool &ClearVocalsEn, bool &RichVocalsEn, bool &listeningEn, bool &NoiseEn, int &NoiseVal)
{
    ClearVocalsEn = ui->pBt_ClearVocals->isChecked(); //人声清晰
    RichVocalsEn = ui->pBt_RichVocals->isChecked();   //人声浑厚
    listeningEn = ui->pBt_listening->isChecked();     //侦听
    NoiseEn = ui->pBt_NR->isChecked();                //AI降噪
    NoiseVal = ui->hSlider_AI->value();
}
//读取文件
void MicSet::readIniValue(
    bool ClearVocalsEn, bool RichVocalsEn, bool listeningEn, bool NoiseEn, int NoiseVal)
{
    ui->pBt_ClearVocals->setChecked(ClearVocalsEn);
    ui->pBt_RichVocals->setChecked(RichVocalsEn);
    // ui->pBt_listening->setOpen(listeningEn);
    ui->pBt_listening->setChecked(listeningEn);

    // ui->pBt_NR->setOpen(NoiseEn);
    if (ui->pBt_NR->isChecked() == NoiseEn) {
        on_pBt_NR_toggled(NoiseEn);
    } else {
        ui->pBt_NR->setChecked(NoiseEn);
    }

    ui->hSlider_AI->setValue(NoiseVal);
}

void MicSet::change_pBt_ClearVocals(bool targetStatus)
{
    ui->pBt_ClearVocals->setChecked(targetStatus);
}

void MicSet::change_pBt_RichVocals(bool targetStatus)
{
    ui->pBt_RichVocals->setChecked(targetStatus);
}

//设置侦听
void MicSet::GetDevListen(int En)
{
    if (En == 1) {
        ui->pBt_listening->setChecked(true);
    } else if (En == 2) {
        ui->pBt_listening->setChecked(false);
    }
}
//设置AI降噪
void MicSet::GetDevNoise()
{
    int En = apo->GetAINSEnable();
    if (En == 1) {
        ui->pBt_NR->setChecked(true);
    } else if (En == 0) {
        ui->pBt_NR->setChecked(false);
    }
}

void MicSet::UpdateShadowLabelSize(QLabel *&labelOut)
{
    // if(labelOut)
    // {
    //     if(labelOut->parentWidget() == ui->widget_AI)
    //     {
    //         // 设置几何区域为父控件大小
    //         labelOut->setGeometry(0, ui->widget_AITop->height(), labelOut->parentWidget()->width(), labelOut->parentWidget()->height() - ui->widget_AITop->height());
    //     }else
    //     {
    //         // 设置几何区域为父控件大小
    //         labelOut->setGeometry(0, 0, labelOut->parentWidget()->width(), labelOut->parentWidget()->height());
    //     }
    // }
}

void MicSet::createShadowLabel(QWidget *parent, QLabel *&labelOut)
{
    // 创建 QLabel 并设置父控件
    labelOut = new QLabel(parent);

    // 设置透明度效果
    // QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect;
    // opacityEffect->setOpacity(0.5); // 50% 透明度
    // labelOut->setGraphicsEffect(opacityEffect);

    // 提升到顶层并显示
    labelOut->raise();
    labelOut->show();
}

//设置人声清晰
void MicSet::on_pBt_ClearVocals_toggled(bool checked)
{
    if (checked)
    {
        ui->pBt_RichVocals->setChecked(false);
    }
    emit pBt_ClearVocals_changed(checked);///< 改变 人声清晰 按键状态 后
    emit ApoManager::instance()->requestSetVocalEffectsEnable(checked);
    globalSettings->setValue(QString("MIC/ClearVocalsEn"), checked);
}

//设置人声浑厚
void MicSet::on_pBt_RichVocals_toggled(bool checked)
{
    if (checked)
    {
        ui->pBt_ClearVocals->setChecked(false);
    }
    emit pBt_RichVocals_changed(checked);///< 改变 人声浑厚 按键状态 后
    emit ApoManager::instance()->requestSetRichVocalsEnable(checked);
    globalSettings->setValue(QString("MIC/RichVocalsEn"), checked);
}
//AI降噪减
void MicSet::on_pBt_AI_sub_clicked()
{
    int val = ui->hSlider_AI->value() - 1;
    ui->hSlider_AI->setValue(val);
}

//AI降噪加
void MicSet::on_pBt_AI_add_clicked()
{
    int val = ui->hSlider_AI->value() + 1;
    ui->hSlider_AI->setValue(val);
}

//根据主题设置样式
void MicSet::setTheme_MicSet(int idx)
{
    QString textColor,colorStr;

    switch (idx) {
    case 0: {colorStr = "rgba(0, 0, 0, 20%)"; break;}   // 深蓝色
    case 1: {colorStr = "rgba(0, 0, 0, 20%)"; break;}   // 白色
    case 2: {colorStr = "rgba(0, 0, 0, 20%)"; break;}   // 黑色
    default: {colorStr = "rgba(0, 0, 0, 20%)"; break;}
    }
    ui->frame->setStyleSheet(QString("border-radius:8px;background-color: %1;").arg(colorStr));
}
//设置面板透明度和颜色
void MicSet::setPanelTransparency_MicSet(int idx,int PValue)
{
    double PanelTransparency = PValue / 100.0;   //面板透明度(默认值是0.2)
    int r, g, b;
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

    ui->widget->setStyleSheet(QString("background-color: %1;border-radius:14px;").arg(colorStr));
    ui->widget_AI->setStyleSheet(QString("background-color: %1;border-radius:14px;").arg(colorStr));


}
//设置面板模糊度
void MicSet::setPanelBlur_MicSet(int PValue)
{

}
