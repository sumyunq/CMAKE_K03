#ifndef FILTERPOPUPWIDGET_H
#define FILTERPOPUPWIDGET_H

#include "qbuttongroup.h"
#include <QDialog>

namespace Ui {
class FilterPopupWidget;
}

class FilterPopupWidget : public QDialog
{
    Q_OBJECT

public:
    explicit FilterPopupWidget(QWidget *parent = nullptr);
    ~FilterPopupWidget();

    void SetCheckedBtn(QString FilterName);

private:
    Ui::FilterPopupWidget *ui;
    QButtonGroup *group;

signals:
    void SwitchFilter(int id);//切换滤波器
};

#endif // FILTERPOPUPWIDGET_H
