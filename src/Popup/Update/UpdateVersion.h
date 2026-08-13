#ifndef UPDATEVERSION_H
#define UPDATEVERSION_H

/********************* 上位机（驱动）、固件当前版本已是最新版，无可回退固件弹窗 *********************/

#include <QDialog>

namespace Ui {
class UpdateVersion;
}

class UpdateVersion : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateVersion(QWidget *parent = nullptr);
    ~UpdateVersion();

    void UpdateVer(QString ver);
    void UpdateTitle(int type);

    void setTheme_UpdateVersion(int idx);

private:
    Ui::UpdateVersion *ui;
};

#endif // UPDATEVERSION_H
