#ifndef ACTIVATIONCODE_H
#define ACTIVATIONCODE_H

#include <QDialog>

namespace Ui {
class ActivationCode;
}

class ActivationCode : public QDialog
{
    Q_OBJECT

public:
    explicit ActivationCode(QWidget *parent = nullptr);
    ~ActivationCode();

    void showError(bool en);

private slots:
    void on_pBt_Activate_clicked();

private:
    Ui::ActivationCode *ui;
};

#endif // ACTIVATIONCODE_H
