#ifndef UPDATEERROR_H
#define UPDATEERROR_H

/********************* 更新错误弹窗 *********************/

#include <QDialog>

namespace Ui {
class UpdateError;
}

class UpdateError : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateError(QWidget *parent = nullptr);
    ~UpdateError();

    void setTheme_UpdateError(int idx);

    void setTitle(int type);

private:
    Ui::UpdateError *ui;
};

#endif // UPDATEERROR_H
