#include "ActivationCode.h"
#include "ui_ActivationCode.h"
#include <QGraphicsDropShadowEffect>
#include "APOThread/ApoManager.h"

ActivationCode::ActivationCode(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ActivationCode)
{
    ui->setupUi(this);

    // 设置无边框
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0, 0); // 设置阴影偏移
    shadow->setColor(QColor("#000000")); // 设置阴影颜色
    shadow->setBlurRadius(30); // 设置模糊半径

    ui->widget->setGraphicsEffect(shadow);
    showError(false);

}

ActivationCode::~ActivationCode()
{
    delete ui;
}

void ActivationCode::on_pBt_Activate_clicked()
{
    emit ApoManager::instance()->requestlogWithTime("click active");
    QString code = ui->lEdit_Code->text();
    // emit ApoManager::instance()->requestActivateAsync(code);
}

void ActivationCode::showError(bool en)
{
    if(en)
    {
        ui->pBt_error->show();
        ui->lab_error->show();
    }else
    {
        ui->pBt_error->hide();
        ui->lab_error->hide();
    }
}
