#ifndef KEYSELFSET_H
#define KEYSELFSET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>

namespace Ui {
class KeySelfSet;
}

class KeySelfSet : public QWidget
{
    Q_OBJECT

public:
    explicit KeySelfSet(QWidget *parent = nullptr);
    ~KeySelfSet();

private:
    Ui::KeySelfSet *ui;
    void K_SetCBoxShadow(QComboBox *cBox);
    void EnAllCurrentActIdx();
    void EnAllCurrentKeyIdx();
    void disCurrentIdx(QComboBox* cBoxKey1, QComboBox* cBoxKey2,QComboBox* cBoxAct1, QComboBox* cBoxAct2);
    void updateItemsEnable(QComboBox* combo);
    void updateAllStates();

    void DevSetKey(int MuteKey,int MuteAct,int PlayKey,int PlayAct,int EqKey,int EqAct);

    void UpdateShadowLabelSize(QLabel*& labelOut);
    void createShadowLabel(QWidget* parent, QLabel*& labelOut);

public:
    void LanguageSet();
    void saveIniValue(int &MuteKey,int &MuteAct,int &PlayKey,int &PlayAct,int &EqKey,int &EqAct,bool &BeepEn,int &BeepVal,int &BeepLanguage);
    void readIniValue(int MuteKey,int MuteAct,int PlayKey,int PlayAct,int EqKey,int EqAct,bool BeepEn,int BeepVal,int BeepLanguage);

    void GetDevBeep(int En,int level);

private slots:
    void on_cBox_muteKey_currentIndexChanged(int index);
    void on_cBox_playKey_currentIndexChanged(int index);
    void on_cBox_EQKey_currentIndexChanged(int index);
    void on_cBox_muteAct_currentIndexChanged(int index);
    void on_cBox_playAct_currentIndexChanged(int index);
    void on_cBox_EQAct_currentIndexChanged(int index);
    void on_pBt_BeepSwitch_toggled(bool checked);
    void on_pBt_reset_toggled(bool checked);
    void on_hSlider_level_valueChanged(int value);

    void on_pBt_reset_clicked();

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // KEYSELFSET_H
