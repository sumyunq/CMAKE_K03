#ifndef GLOBALDEFINITION_H
#define GLOBALDEFINITION_H

#include <QMediaPlayer>
#include "Popup/CustomTipPopup/NewCustomToolTip.h"

struct PlanVal
{
    bool DataVisibleEn;
    QString ParentPlanName;//二创方案源于的官方方案的名称

    bool AlgoOpenEn;//算法
    bool spaceOpenEn;//空间音频
    bool eqOpenEn;//均衡器
    bool drcOpenEn;//灵晰算法

    int ExtraEq[7];//额外eq（脚步增强，枪声弱化，声场控制，清晰度，余音消除，空间混响，风声弱化），相当与APO十组eq的第1-7组，每一组存在10个频点
    int lowVal;//低音增强
    int drcVal;//灵晰算法
    int GainVal;//增益

    int spaceVal;//空间强度
    int spaceSize;//空间环境大小
    int spaceReverb;//混响

    //正常（相当于APO的十组eq的第0组中的0-9频点）
    double freqVal[10];
    double eqVal[10];
    double qVal[10];
    int filterVal[10];//滤波器

    //二创（相当于APO的十组eq的第1组中的0-9频点）
    double freqVal_deriv[10];//不能为0，否则会破音卡顿
    double eqVal_deriv[10];
    double qVal_deriv[10];
    int filterVal_deriv[10];//滤波器
};
struct  PlansType
{
    QString Name;//分类名称
    bool en;//分类是否使用
};
#endif // GLOBALDEFINITION_H
extern PlanVal currentPlanVal;
extern PlansType PlansTypes[8];//分类1-分类8
extern int PlansTypeIdx;
// extern PlanVal TempCurrentPlanVal;


