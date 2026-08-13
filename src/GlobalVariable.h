#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H
#include <QString>
#include "CustomControl/CustomRadioButton/NewRadioBtn.h"

using PlanKey = QPair<QString, QString>;//方案名称，机型（分机型时，传递正确机型。不分机型时，传递空）
struct ModeVal
{
    QString ModeName;//模式名称：电影模式、音乐模式、游戏模式(已无用，保留是为了兼容1.9及以前的版本)
    QList<NewRadioBtn*> AllPlanRadioList;//所有预设(不保存到本地)
    QHash<PlanKey, NewRadioBtn*> AllPlanRadioHash;//预设哈希表
    QList<NewRadioBtn*> AllPlanRadioList_Dev;//当前设备的所有预设(不保存到本地)
    QList<NewRadioBtn*> AllPlanRadioList_Check;//勾选的方案(不保存到本地)

    QList<NewRadioBtn*> MyPlanRadioList;//我的预设（自建+导入，保存到本地）
    QHash<PlanKey, NewRadioBtn*> MyPlanRadioHash;//预设哈希表


    int C_PlanPageSel;//预设分类(0:无（所有），1：分类1,2：分类2....)
    QString C_PlanName;//当前预设方案的名称
    QStringList C_PlanDev;//当前预设方案的设备类型
};
struct FavPlan
{
    QString PName;//当前预设方案的名称
    int PlanMode;//预设(0:所有预设，1：我的预设)
    bool IsloadEn;//导入方案
    QStringList label_Devs;//标签1（设备）,最多可设三个设备
    QString label_Scene;//标签2（使用场景）
};
struct SysVal
{
    QList<NewRadioBtn*> SysPlanRadioList_Mode;//官方预设（保存到本地）
    QHash<PlanKey, NewRadioBtn*> SysPlanRadioHash_Mode;//预设哈希表
};
#endif // GLOBALVARIABLE_H


extern NewRadioBtn *currentPlanRadio;
extern ModeVal MovieVal;//电影
// extern ModeVal MusicVal;//音乐
// extern ModeVal GameVal;//游戏

extern FavPlan EightFavPlan[8];//八个我的收藏
extern int EightFavPlanIndex;//EightFavPlan数组中第几项
extern int EightFavPlanCnt;//收藏的总数
extern bool IsSwitch;//是否切换时（切换时模式开关为关时，仍生效之前的音效）

extern SysVal SysPlanVal;//官方预设
