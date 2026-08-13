#ifndef USERGUIDE_H
#define USERGUIDE_H

#include <QDialog>

namespace Ui {
class UserGuide;
}

class UserGuide : public QDialog
{
    Q_OBJECT

public:
    explicit UserGuide(QWidget *parent = nullptr);
    ~UserGuide();

private slots:
    void on_pBt_exit_clicked();

private:
    Ui::UserGuide *ui;
};

#endif // USERGUIDE_H
