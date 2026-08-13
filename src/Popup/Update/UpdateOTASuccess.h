#ifndef UPDATEOTASUCCESS_H
#define UPDATEOTASUCCESS_H

/********************* OTA升级回退成功弹窗 *********************/

#include <QDialog>

namespace Ui {
class UpdateOTASuccess;
}

class UpdateOTASuccess : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateOTASuccess(QWidget *parent = nullptr);
    ~UpdateOTASuccess();

    void UpdateTitle(int type);
    void setTheme_UpdateOTASuccess(int idx);

private:
    Ui::UpdateOTASuccess *ui;
};

#endif // UPDATEOTASUCCESS_H
