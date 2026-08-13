#ifndef EQUALIZERHIDDEN_H
#define EQUALIZERHIDDEN_H

#include "Popup/CustomTipPopup/NewCustomToolTip.h"
#include <QDialog>

namespace Ui {
class EqualizerHidden;
}

class EqualizerHidden : public QDialog
{
    Q_OBJECT

public:
    explicit EqualizerHidden(QWidget *parent = nullptr);
    ~EqualizerHidden();
    bool GetEqShowEn();

private:
    Ui::EqualizerHidden *ui;

    NewCustomToolTip *tip_HideEq = nullptr;
};

#endif // EQUALIZERHIDDEN_H
