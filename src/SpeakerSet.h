#ifndef SPEAKERSET_H
#define SPEAKERSET_H

#include <QWidget>
#include <QUndoStack>
#include <QScrollArea>
#include <QGridLayout>
#include "GlobalDefinition.h"
//#include "NewRadioBtn.h"
#include "EightMyPlan.h"
#include "CustomControl/CustomRadioButton/NewRadioBtnText.h"
#include <QLabel>
#include <QGraphicsOpacityEffect>


#include <QButtonGroup>
#include <QHttpMultiPart>
#include <QClipboard>

#include "GlobalVariable.h"

#include "Popup/CustomTipPopup/NewCustomToolTip.h"
#include "Popup/Plans/EditPlansType.h"
#include "Popup/Plans/MovePlan.h"

#include "ShareImportPlan/QDialog_custom.h"   // 弹窗: 导入方案
#include "Popup/Plans/EqualizerHidden.h"

namespace Ui {
class SpeakerSet;
}

class SpeakerSet : public QWidget
{
    Q_OBJECT

public:
    explicit SpeakerSet(QWidget *parent = nullptr);
    ~SpeakerSet();

    void ShowcurrentPlanVal();
    void SwitchMode();
    int readExportPlanIni(QString filePath, bool TakeEffect); ///< 导入导出的方案文件到方案库

    // // 设置按钮列表（用于与UI设计中的按钮关联）
    // QList<QPushButton*> buttonList;
    // void ShowFirstEight();
    // void TruncateText(QString text,QPushButton *pbt);

    void setFavPbtEn(bool en);

    void addAllPlan(QString name,QString desc,bool IsLoad, PlanVal val, bool IsAdded, int favIdx, bool TakeEffect, bool saveAsInit,QStringList Lab1,QString Lab2,int PlanTypeIdx, bool sysEn, bool ifMerge, QString ShareCodeId,QString ShareCode);

    void RearrangeAllPlanAfterDel(int idx);
    void RearrangeAllPlanAfterSearch(int idx);


    void LanguageSet();


    NewRadioBtnText *currentPlan_s;
    EightMyPlan *eightPlan_s;

    bool m_layoutUpdatePending = false; // 防抖标志
    // 空态页图片等比缩放规格：img 为图片、text 为提示文字（高度钳制用）、
    // origSize 为图片原始尺寸（.ui 设计值）、page 为图片所在页面（缩放基准）、
    // baseSize 为页面首次正常布局尺寸（启动即全屏时不记录，避免基准失真）
    struct EmptyImageSpec {
        QWidget *img = nullptr;
        QWidget *text = nullptr;
        QSize origSize;
        QWidget *page = nullptr;
        QSize baseSize;
    };
    EmptyImageSpec m_searchEmptySpec; // "未搜索到"空态页
    EmptyImageSpec m_planEmptySpec;   // "方案为空"空态页
    void scheduleLayoutUpdate();
    void performLayoutUpdate();
    void setupRadioButtons(int isFullScreen);//全屏重新布局方案库
    // void updateButtonLayout();
    // void updateButtonLayoutAll();
    // void SpeakerSet::updateRadioButtonLayout(QList<NewRadioBtn*>& radioList,
    //                                          QGridLayout* layout,
    //                                          QScrollArea* scrollArea,
    //                                          QWidget* contentWidget,
    //                                          int& rowVar, int& colVar);
    void updateRadioButtonLayout(QList<NewRadioBtn*>& radioList,
                                             QGridLayout* layout,
                                             QScrollArea* scrollArea,
                                             QWidget* contentWidget,
                                             int& rowVar, int& colVar,
                                             int fullScreenMode);
    void appendRadioToLayout(NewRadioBtn* radio,
                             QGridLayout* layout,
                             int& currentRow,
                             int& currentCol,
                             QSpacerItem*& spacerItem,
                             int columnCount,
                             int buttonWidth,
                             int buttonHeight);
    void incrementalRearrangeAfterDelete(QGridLayout* layout,
                                                     QList<NewRadioBtn*>& radioList,
                                                     int deletedRow, int deletedCol,
                                                     int& currentRow, int& currentCol,
                                                     int columnCount,
                                                     int buttonWidth, int buttonHeight,
                                                     QSpacerItem*& spacerItem);
    void calculateOptimalButtonSize(int availableWidth, int spacing, int minWidth, int maxWidth);

    int FullScreenEn = 0;//-1;//0;

    void ResetValue(int type);//0:重置均衡器界面，1：重置算法界面，2：重置空间界面
    void on_pBt_save_clicked();

    void PlanSave();

    void FavShowEQpageChange();


    void ShowSysPlan();
    void ShowPlansType();

    bool isDeviceMatchingPlanDev(const QString& planDev);//判断标签对应的设备
    QString GetCurrentDeviceIdentifier();//获得设备对应的标签
    void selectFirstPresetForDevice(const QString& dev);//寻找改标签对应的第一个方案

    void CreateDerivPlanSlot();//创建二创方案

    void IntegratePlansAndCompatible();//整合和兼容新旧版本方案，仅当前设备的方案显示


    // cl_network_manager_ 已移除（2026-08-04）：分享码请求统一走 ApiClient


    void setThemeAndPanelTransparency_SpeakerSet(int idx,int PValue);//切换主题
    void setPanelTransparency_SpeakerSet(int idx,int PValue);//设置面板透明度
    void setPanelBlur_SpeakerSet(int PValue);//设置面板模糊度

protected:
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

private slots:

    void on_pBt_GameListen_clicked(bool checked);

    //void on_rBt_currentPlan_clicked();

    // void on_pBt_Type1_clicked();

    void on_pBt_All_clicked();

    void on_pBt_LoadPlan_clicked();

    void on_pBt_NewPlan_clicked();

    void on_lEdit_search_textChanged(const QString &arg1);



    void on_pBt_ExportPlan_clicked();

    void on_pBt_CreateType_clicked();

    void on_pBt_EditType_clicked();

    void on_pBt_CancelEditType_clicked();

    void on_pBt_DelPlans_clicked();

    void on_pBt_MoveToType_clicked();


private:
    Ui::SpeakerSet *ui;

    EqualizerHidden *eqHiddenDlg = nullptr;

    NewCustomToolTip *tip_edit = nullptr;
    NewCustomToolTip *tip_export = nullptr;
    NewCustomToolTip *tip_load = nullptr;
    NewCustomToolTip *tip_add = nullptr;

    NewCustomToolTip *tip_editing = nullptr;
    NewCustomToolTip *tip_PlansDel = nullptr;
    NewCustomToolTip *tip_PlansMove = nullptr;

    QDialogCustom *cl_dialog_loadPlan_ = nullptr; ///< 分享码 导入方案弹窗

    MovePlan *Mplan = nullptr;

    QButtonGroup *group_AlltypeBtn;
    QButtonGroup *group_type_del;
    QPushButton* typeButtons[16] = {nullptr};
    QWidget* typeWidgets[8] = {nullptr};

    EditPlansType *CPlansType = nullptr;

    QScrollArea *scrollArea_All;
    QWidget *content_All;
    QGridLayout *layout_All;

    // QScrollArea *scrollArea_My;
    // QWidget *content_My;
    // QGridLayout *layout_My;
    // QSpacerItem* spacer_Temp_My;QFrame *spacer_Temp_My2;

    QSpacerItem* spacer_Temp_All = nullptr;
    QSpacerItem* spacer_Temp_Search = nullptr;

    int m_stretchRow_All = -1; // 底部弹性行索引（吸收多余垂直空间，保证按钮行间距固定），-1表示暂无


    bool m_pageAnimating = false;//是否在执行on_rBt_currentPlan_clicked的动画

    bool m_dragging = false; // 标记是否正在拖动
    NewRadioBtn* m_draggedButton = nullptr; // 记录拖拽过程中第一个被拖动的按钮
    NewRadioBtn* m_lastToggledButton = nullptr;  // 记录拖拽过程中结束时的按钮


    void Initialize();
    void CreateScrollArea();


    void UpdateSearchPlanPosition();//搜索后

    QButtonGroup *buttonGroup;

    void updateSize();
    void updateEmptyImageScale(EmptyImageSpec &t_spec);

    void AddSysPlan(const QString& name,
                    const QString& description,
                    const QStringList& Lab1,
                    const QString& Lab2,
                    bool IsAdded,
                    int favIdx,
                    bool DataVisibleEn,bool AlgoOpenEn, bool spaceOpenEn, bool eqOpenEn,
                    int lowVal, int spaceVal, int GainVal,
                    double freqVal[10], double eqVal[10], double qVal[10]);



    int writeExportPlanIni(QString filePath,bool ShowEqEn);

    void CreateNewMyPlan(QString txt);

    void ShowPlansCheckBox(bool en);//显示方案的勾选框
    void setTypeBtnStyle(bool checked);//设置分类的样式

signals:
    void pageChange();
    void EQpageChange(bool ShowEqEn);
    void PlanReset_S();
    void SetApoVal();//设置APO数值

    void RealTimeSaveModeVal_S();
    void RealTimeSaveSysPlan_S();


    void FavEQpageChange(bool ShowEqEn);

    void RealTimeSaveSysPlanValInit_S();
};

#endif // SPEAKERSET_H


