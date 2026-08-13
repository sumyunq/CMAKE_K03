#include "Popup/Plans/EqualizerHidden.h"
#include "qgraphicseffect.h"
#include "ui_EqualizerHidden.h"

EqualizerHidden::EqualizerHidden(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EqualizerHidden)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);


    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 20px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    ui->frame->setGraphicsEffect(shadow);

    tip_HideEq = new NewCustomToolTip(this);
    tip_HideEq->setLabelStyle(0);
    tip_HideEq->AddToolTip(ui->lab_hideEqTip,tr("勾选后，上传该方案时将均衡器（EQ）配置隐藏不可见。"),Qt::AlignHCenter);

    connect(ui->pBt_exit,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_ok,&QPushButton::clicked,this,&QDialog::accept);
}

EqualizerHidden::~EqualizerHidden()
{
    delete ui;
}

bool EqualizerHidden::GetEqShowEn()
{
    if(ui->rButton_hide->isChecked())
    {
        return (false);
    }else
    {
        return (true);
    }
}
