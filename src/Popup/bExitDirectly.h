#ifndef BEXITDIRECTLY_H
#define BEXITDIRECTLY_H

/********************* 点击应该右上角关闭按钮后，询问是最小化到系统托盘还是退出应用 *********************/

#include <QDialog>

namespace Ui {
class Dialog;
}

class bExitDirectly : public QDialog
{
    Q_OBJECT

public:
    explicit bExitDirectly(QWidget *parent = nullptr);
    ~bExitDirectly();

    void ShowExitMode();
    void SetExitMode();

    void setTheme_bExitDirectly(int idx);


private:
    Ui::Dialog *ui;
};

#endif // BEXITDIRECTLY_H
