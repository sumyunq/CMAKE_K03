#include "Popup/UserGuide/UserGuide.h"
#include "ui_UserGuide.h"

UserGuide::UserGuide(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserGuide)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

}

UserGuide::~UserGuide()
{
    delete ui;
}

void UserGuide::on_pBt_exit_clicked()
{
    accept();
}

