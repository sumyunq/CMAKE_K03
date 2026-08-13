#ifndef EDITPANELTIP_H
#define EDITPANELTIP_H

/********************* 鼠标悬浮在EQ波形图频点上时显示的数据弹窗 *********************/

#include <QWidget>
#include <QDoubleSpinBox>
#include "EQCurve/FilterPopupWidget.h"

namespace Ui {
class EditPanelTip;
}

class EditPanelTip : public QWidget
{
    Q_OBJECT

public:
    explicit EditPanelTip(QWidget *parent = nullptr);
    ~EditPanelTip();

    void setBandData(int index, double freq, double gain, double q,int FilterType);
    void setBandIndex(int index) { m_bandIndex = index; }
    int bandIndex() const { return m_bandIndex; }

    void setTheme_EditPanelTip(int idx);

    FilterPopupWidget *filterPopup;

private:
    Ui::EditPanelTip *ui;
    int m_bandIndex = -1;

signals:
    void frequencyChanged(int bandIndex, double newFreq);
    void gainChanged(int bandIndex, double newGain);
    void qChanged(int bandIndex, double newQ);
    void filterChanged(int bandIndex, int newFilter);

private slots:
    // void on_lEdit_freq_textEdited(const QString &arg1);
    // void on_lEdit_QVal_textEdited(const QString &arg1);
    // void on_lEdit_GVal_textEdited(const QString &arg1);
    void on_lEdit_freq_editingFinished();
    void on_lEdit_QVal_editingFinished();
    void on_lEdit_GVal_editingFinished();
    void on_pBt_filter_clicked();

protected:
    void hideEvent(QHideEvent *event) override;//重写隐藏事件
};

#endif // EDITPANELTIP_H
