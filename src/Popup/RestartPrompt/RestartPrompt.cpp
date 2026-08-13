#include "Popup/RestartPrompt/RestartPrompt.h"
#include "qgraphicseffect.h"
#include "ui_RestartPrompt.h"

RestartPrompt::RestartPrompt(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RestartPrompt)
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

    connect(ui->pBt_Cancel,&QPushButton::clicked,this,&QDialog::reject);
    connect(ui->pBt_Restart,&QPushButton::clicked,this,&QDialog::accept);
}

RestartPrompt::~RestartPrompt()
{
    delete ui;
}
