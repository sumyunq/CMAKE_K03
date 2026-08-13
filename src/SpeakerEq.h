#ifndef SPEAKEREQ_H
#define SPEAKEREQ_H

#include <QWidget>
#include <QUndoStack>
#include <QLabel>
#include "CustomControl/CustomRadioButton/NewRadioBtnText.h"
#include "EightMyPlan.h"
#include <QMediaPlayer>

#include "GlobalDefinition.h"
#include <QLineEdit>
#include <QParallelAnimationGroup>

class QDoubleSpinBox;
class QCheckBox;

namespace Ui {
class SpeakerEq;
}

class SpeakerEq : public QWidget
{
    Q_OBJECT

public:
    explicit SpeakerEq(QWidget *parent = nullptr);
    ~SpeakerEq();

    void SetLastVal();

    void ShowcurrentPlanVal();
    void ShowEqVal(bool en);

    void PageShowPlanVal();

    void SwitchEqPage();


    void LanguageSet();

    NewRadioBtnText * currentPlan_e;
    EightMyPlan *eightPlan_e;


    ///
    /// \brief set_pBt_EQSwitch_hideData_checked 设置方案开关
    /// \param checked 目标状态
    ///
    void set_pBt_EQSwitch_hideData_checked(bool checked);

    void SetEQSwitchShadow(bool checked,int type);//0:均衡器，1：算法，2空间

    void setThemeAndPanelTransparency_SpeakerEq(int idx,int PValue);//切换主题
    void setPanelTransparency_SpeakerEq(int idx,int PValue);//设置面板透明度
    void setPanelBlur_SpeakerEq(int PValue);//设置面板模糊度
    void ShowDRC();//显示DRC功能

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽


private:
    Ui::SpeakerEq *ui;

    NewCustomToolTip *tip_undo = nullptr;
    NewCustomToolTip *tip_redo = nullptr;
    NewCustomToolTip *tip_reset = nullptr;
    NewCustomToolTip *tip_Function = nullptr;

    QButtonGroup *group_page;
    QButtonGroup *group_space;
    bool EqShow = true;

    //上传数据，显示曲线,en:是否设置apo
    void refreshCurve(QVector<double> freq,QVector<double> QVal,QVector<double> GainVal, QVector<int> FilterType,bool en);


    QUndoStack* m_undoStack_Eq;

    QList<QWidget *> cl_need_checked_;  ///< 需要在禁用状态下响应点击事件的控件


    void ShowCurrentPlanEqVal(PlanVal PlanVal);
    void ShowCurrentPlanGain();

    // void ApoSetGainVal(int idx,int val);
    //void ApoSetFreqVal();



    void updateCPVal_Freq(QString text,int i);
    void updateCPVal();



    void setSliderValueForceSignal(QSlider* slider, int value);

protected:
    void resizeEvent(QResizeEvent* event) override;
    // virtual void paintEvent(QPaintEvent *event) override;

signals:
    void CurrentpageChange();
    void ListenpageChange();

    void PlanSave_E();
    void PlanReset_E(int type);//0:重置均衡器界面，1：重置算法界面，2：重置空间界面

    void CreateDerivPlan();//创建二创方案

    void UpdateHomePageUIInfo(bool targetStatus);    ///< 更新首页预设UI显示（信号截断）


private slots:
    // void on_rBt_currentPlan_clicked();
    void on_pBt_GameListen_clicked();

    void on_pBt_EQSwitch_toggled(bool checked);

    void on_pBt_EqReset_clicked();


    void on_hSlider_Bass_valueChanged(int value);

    void on_hSlider_Space_valueChanged(int value);

    void on_pBt_Environment_Small_toggled(bool checked);
    void on_pBt_Environment_Medium_toggled(bool checked);
    void on_pBt_Environment_Large_toggled(bool checked);


    void on_pBt_Plans_clicked();

    void on_pBt_deriv_clicked();

    void on_pBt_EQSwitch_hideData_toggled(bool checked);

    void on_hSlider_Footsteps_valueChanged(int value);

    void on_hSlider_Gunshot_valueChanged(int value);

    void on_hSlider_Sfc_valueChanged(int value);

    void on_hSlider_SpatialReverb_valueChanged(int value);

    void on_hSlider_Wind_valueChanged(int value);

    void on_hSlider_Clarity_valueChanged(int value);

    void on_hSlider_LingeringSound_valueChanged(int value);

    void on_pBt_ClosePlanPage_clicked();

    void on_hSlider_Reverb_valueChanged(int value);

    void on_pBt_SysClose_clicked();

  public slots:
    // void on_pBt_backMain_clicked();
    void resetVal();

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SPEAKEREQ_H
