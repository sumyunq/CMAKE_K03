#ifndef UPDATESOFTWAREFIND_H
#define UPDATESOFTWAREFIND_H

/********************* 上位机（驱动）可升级版本弹窗 *********************/

#include <QDialog>

namespace Ui {
class UpdateSoftWareFind;
}

class UpdateSoftWareFind : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateSoftWareFind(QWidget *parent = nullptr);
    ~UpdateSoftWareFind();
    void UpdateExplanation(QString txt);

    void setVersion(QString version);

    void setTheme_UpdateSoftWareFind(int idx);

    void setShowPage(int idx);
    void ShowProgress(int val);

private:
    Ui::UpdateSoftWareFind *ui;

public: signals:
    void startUpdate();

};

#endif // UPDATESOFTWAREFIND_H
