#ifndef MICSET_H
#define MICSET_H

#include <QWidget>
#include <QLabel>
#include "Popup/CustomTipPopup/NewCustomToolTip.h"

#include "MicListening/WASAPIPipeline.h"

#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>

namespace Ui {
class MicSet;
}

class WASAPIPipeline;

class MicSet : public QWidget
{
    Q_OBJECT

public:
    explicit MicSet(QWidget *parent = nullptr);
    ~MicSet();
    void saveIniValue(bool &ClearVocalsEn,bool &RichVocalsEn,bool &listeningEn,bool &NoiseEn,int &NoiseVal);
    void readIniValue(bool ClearVocalsEn,bool RichVocalsEn,bool listeningEn,bool NoiseEn,int NoiseVal);

    void change_pBt_ClearVocals(bool targetStatus); ///< 改变 人声清晰 按键状态
    void change_pBt_RichVocals(bool targetStatus); ///< 改变 人声浑厚 按键状态

signals:
    void pBt_ClearVocals_changed(bool current_status);///< 改变 人声清晰 按键状态 后
    void pBt_RichVocals_changed(bool current_status);///< 改变 人声浑厚 按键状态 后

private slots:

    void on_pBt_listening_toggled(bool checked);

    void on_pBt_NR_toggled(bool checked);

    void on_hSlider_AI_valueChanged(int value);

    void on_pBt_ClearVocals_toggled(bool checked);

    void on_pBt_RichVocals_toggled(bool checked);

    void on_pBt_AI_sub_clicked();

    void on_pBt_AI_add_clicked();

public:
    void LanguageSet();
    void GetDevListen(int En);
    void GetDevNoise();
    void Test();//del

    void setUpEn(bool en);//设置上行功能使能

    void setTheme_MicSet(int idx);
    void setPanelTransparency_MicSet(int idx,int PValue);
    void setPanelBlur_MicSet(int PValue);

private:
    Ui::MicSet *ui;

    NewCustomToolTip *tip_ai = nullptr;
    NewCustomToolTip *tip_listening = nullptr;
    NewCustomToolTip *tip_ClearVocals = nullptr;
    NewCustomToolTip *tip_RichVocals = nullptr;

    void UpdateShadowLabelSize(QLabel*& labelOut);
    void createShadowLabel(QWidget* parent, QLabel*& labelOut);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void startListening();
    void stopListening();


private:
    std::unique_ptr<WASAPIPipeline> m_pipeline;

};

#endif // MICSET_H

extern bool MicOpenEn;
