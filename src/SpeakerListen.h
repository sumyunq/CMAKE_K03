#ifndef SPEAKERLISTEN_H
#define SPEAKERLISTEN_H

#include <QWidget>
#include <QButtonGroup>
#include <QLayoutItem>
#include <QMediaPlayer>
#include <QLabel>
#include <QMutex>
#include "CustomControl/CustomRadioButton/NewRadioBtnText.h"
#include "EightMyPlan.h"
#include "GlobalVariable.h"
#include <vlc/vlc.h>

#include "FeedBackC/SoundTest/sound_test_main_page.h"

namespace Ui {
class SpeakerListen;
}

class SpeakerListen : public QWidget
{
    Q_OBJECT

public:
    explicit SpeakerListen(QWidget *parent = nullptr);
    ~SpeakerListen();

    void LanguageSet();

    void setThemeAndPanelTransparency_SpeakerListen(int idx,int PValue);//切换主题
    void setPanelTransparency_SpeakerListen(int idx,int PValue);//设置面板透明度
    void setPanelBlur_SpeakerListen(int PValue);//设置面板模糊度

    void updateVideoHoverPosition();

signals:
    void ChangeToPlanPage();    ///跳转到方案库页面


private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

private slots:
    void on_rBt_currentPlan_clicked();


public:
    NewRadioBtnText *currentPlan_l;
    EightMyPlan *eightPlan_l;

    // void closeListen();

    std::unique_ptr<SoundTestMainPage> cl_sound_test_main_page_ = nullptr;

private:
    Ui::SpeakerListen *ui;

    QColor Painter_Background = QColor(81, 96, 122, 51);

signals:
    void ChangeToSpeakerPage(int index, bool ShowVH, int VHIdx, float currentPos);

    // QWidget interface
protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SPEAKERLISTEN_H

