#include "SpeakerSet.h"
#include "ui_SpeakerSet.h"

#include "Popup/Plans/PlanCopy.h"
#include "Popup/Plans/DelReset.h"

// #include "UndoPbtChecked.h"
#include "UndoRedo/UndoSliderVal.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include <QMainWindow>

#include <QAction>

#include <QFileDialog>
#include<QMouseEvent>

#include <QtConcurrent>
#include "LoadLib.h"
#include "data/api_global.h"
#include "modules/CommunityModule/infrastructure/logger/logger.h"  ///< LOG_WARN/LOG_ERROR（spdlog 异步日志）
#include "network/http_client.h"
#include "network/request_options.h"
#include "network/server_router.h"

bool RetrievePlan = false;
int suffix = 2;
bool IsSwitch = false;

PlanVal currentPlanVal = {};//当前数据值
PlansType PlansTypes[8] = {};//当前分类
int PlansTypeIdx = 0;//已显示分类总数
// QList<PlanVal> SysPlanVal_Init = {};
QHash<QString, PlanVal> SysPlanVal_Init;

int SysPlanVal_Index = 0;
// PlanVal TempCurrentPlanVal = {};//保存修改前当前数据值，用于重置，现在重置直接赋0
NewRadioBtn *currentPlanRadio = NULL;//当前预设方案

int row_Allrb = 0, row_Myrb = 0, row_Allrb_Temp = 0, row_Myrb_Temp = 0;
int col_Allrb = 0, col_Myrb = 0, col_Allrb_Temp = 0, col_Myrb_Temp = 0;


int COLUMN_COUNT_rb = 4; // 设置每行4个按钮(当前列数)
const int btnWight_rb = 243, btnHight_rb = 126;//290 100
const int btnMaxWight_rb = 243, bthMaxHight_rb = 126;
int CurrentBtnWight_rb = 243, CurrentBtnHight_rb = 126;


//QList查找速度慢，所以使用QList+QHash
// QList<NewRadioBtn*> MyPlanRadioList;//我的预设（导入+自建），目的是方便系统和我的分开保存，不做其他使用
// QHash<QString, NewRadioBtn*> MyPlanRadioHash;
QList<NewRadioBtn*> AllPlanRadioList_Move = {};
// QList<NewRadioBtn*> AllPlanRadioList = {};//所有预设（系统+导入+自建）
QList<NewRadioBtn*> AllPlanRadioList_Temp = {};//搜索时
//哈希表
// QHash<QString, NewRadioBtn*> PlanHashTables[8]={};
QList<NewRadioBtn*> PlanRadioLists[8] = {};


SysVal SysPlanVal = {};//官方预设
//后续改成分类，分类有十个，先改侧边导航栏，只保留电影，且把用户在其他模式下的我的预设都保存到电影里面
ModeVal MovieVal = {};//电影
// ModeVal MusicVal = {};//音乐
// ModeVal GameVal = {};//游戏
FavPlan EightFavPlan[8] = {};//八个我的收藏
int EightFavPlanIndex = 0;//EightFavPlan的索引
int EightFavPlanCnt = 0;//收藏的总数

bool ifShowPlan = true;//切换模式时，会跳转到均衡器页面，所有不需要多给apo传数据一遍
bool editEn = false;

SpeakerSet::SpeakerSet(QWidget *parent)
    : QWidget(parent)
    // cl_network_manager_ 已移除（2026-08-04）：分享码请求统一走 ApiClient
    , ui(new Ui::SpeakerSet)
{
    ui->setupUi(this);
    InitUIInformation();

    // 空态页图片缩放规格初始化（origSize 与 .ui 设计值一致，baseSize 首次正常布局时记录）
    m_searchEmptySpec = {ui->label,   ui->label_2, QSize(243, 192), ui->page_searchEmpty, {}};
    m_planEmptySpec   = {ui->label_3, ui->label_4, QSize(249, 187), ui->page_planEmpty,   {}};

    cl_dialog_loadPlan_ = new QDialogCustom(this);  ///< 分享码 导入方案弹窗

    currentPlan_s = ui->rBt_currentPlan;
    eightPlan_s = ui->widget_eight;

    // 按钮数组初始化（16 个）
    for (int i = 1; i <= 8; ++i) {
        typeButtons[i - 1]      = findChild<QPushButton*>(QString("pBt_Type%1").arg(i));
        typeButtons[8 + i - 1]  = findChild<QPushButton*>(QString("pBt_del%1").arg(i));
    }

    // 容器数组初始化（8 个）
    for (int i = 1; i <= 8; ++i) {
        typeWidgets[i - 1] = findChild<QWidget*>(QString("widget_type%1").arg(i));
    }

    buttonGroup = new QButtonGroup(this);

    group_AlltypeBtn = new QButtonGroup(this);
    group_AlltypeBtn->addButton(ui->pBt_All,0);
    group_AlltypeBtn->addButton(ui->pBt_Type1,1);
    group_AlltypeBtn->addButton(ui->pBt_Type2,2);
    group_AlltypeBtn->addButton(ui->pBt_Type3,3);
    group_AlltypeBtn->addButton(ui->pBt_Type4,4);
    group_AlltypeBtn->addButton(ui->pBt_Type5,5);
    group_AlltypeBtn->addButton(ui->pBt_Type6,6);
    group_AlltypeBtn->addButton(ui->pBt_Type7,7);
    group_AlltypeBtn->addButton(ui->pBt_Type8,8);
    group_AlltypeBtn->setExclusive(true);

    // 遍历按钮组，安装事件过滤器
    for (QAbstractButton *btn : group_AlltypeBtn->buttons()) {
        if (btn != ui->pBt_All)
        {
            btn->installEventFilter(this);
        }
    }
    ui->rBt_currentPlan->installEventFilter(this); // 当前方案指示器：双击跳转EQ
    //点击分类
    connect(group_AlltypeBtn, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),this, [this](int id,bool checked){
        if(checked)
        {
            if(id != 0)
            {
               typeButtons[8+id-1]->raise();
            }

            MovieVal.C_PlanPageSel = id;

            if(ui->lEdit_search->text().isEmpty())
            {
                scheduleLayoutUpdate();
            }else
            {
                UpdateSearchPlanPosition();
            }


        }
    });


    //清除分类
    group_type_del = new QButtonGroup(this);
    group_type_del->addButton(ui->pBt_del1,0);
    group_type_del->addButton(ui->pBt_del2,1);
    group_type_del->addButton(ui->pBt_del3,2);
    group_type_del->addButton(ui->pBt_del4,3);
    group_type_del->addButton(ui->pBt_del5,4);
    group_type_del->addButton(ui->pBt_del6,5);
    group_type_del->addButton(ui->pBt_del7,6);
    group_type_del->addButton(ui->pBt_del8,7);
    //点击删除分类
    connect(group_type_del, QOverload<int>::of(&QButtonGroup::buttonClicked),this, [this](int id)
            {
                qDebug()<< "typeButtons[i+8] name:" << typeButtons[id+8]->objectName();
                //删除导入的方案
                DelReset *del = new DelReset(m);
                del->editText(4);

                del->setModal(true);
                int result = del->exec();
                if(result == QDialog::Accepted)
                {
                    int lastVisible = -1;
                    lastVisible = PlansTypeIdx-1;

                    if(group_AlltypeBtn->checkedId() == id+1)
                    {
                        for (NewRadioBtn* radio : PlanRadioLists[id]) {
                            if (radio) {  // 防御性检查，防止列表中有空指针
                                radio->PlanPageSel = 0;
                            }
                        }
                        PlanRadioLists[id].clear();
                        QAbstractButton *btn = group_AlltypeBtn->button(id);
                        if (btn) btn->setChecked(true);
                    }

                            // 将 id 之后的内容向前移动
                    for (int i = id; i < lastVisible; ++i) {
                        typeButtons[i]->setText(typeButtons[i + 1]->text());
                        PlansTypes[i] = PlansTypes[i + 1];
                    }


                            // if(!Mplan)
                            // {
                            //     Mplan = new MovePlan(m);
                            // }
                            // Mplan->delType(PlansTypes[lastVisible].Name);//删除分类

                            // 清空并隐藏最后一项
                    typeButtons[lastVisible]->setText("");
                    typeWidgets[lastVisible]->hide();
                    PlansTypes[lastVisible].Name = "";
                    PlansTypes[lastVisible].en = false;

                    PlansTypeIdx--;

                    if(PlansTypeIdx >= 8)
                    {
                        ui->widget_CreateType->setVisible(false);
                    }else
                    {
                        ui->widget_CreateType->setVisible(true);
                    }
                }
            });

    Initialize();

    CreateScrollArea();
    //ui->widget_eight->ShowFirstEight();
    ui->widget_eight->ShowEightFavorite(true);




}

SpeakerSet::~SpeakerSet()
{
    // currentPlanRadio->updateAllPlanValue(currentPlanVal);

    CPlansType->deleteLater();
    CPlansType = nullptr;

    Mplan->deleteLater();
    Mplan = nullptr;

    delete ui;
}

void SpeakerSet::Initialize()
{
    //电影
    MovieVal.ModeName = tr("电影模式");
    MovieVal.AllPlanRadioList = {};
    MovieVal.AllPlanRadioHash = {};
    // MovieVal.C_PlanPageSel = ui->stackedWidget->currentIndex();
    // MovieVal.C_PlanName = ui->rBt_currentPlan->getIndicatorText();

    // ui->rBt_currentPlan->setIndicatorText("","");


    //导出方案时是否隐藏均衡器弹窗
    eqHiddenDlg = new EqualizerHidden(m);
    eqHiddenDlg->setModal(true);

    //搜索框创建图标
    {
        QAction *searchAction = new QAction(ui->lEdit_search);
        searchAction->setIcon(QIcon(":/Skin/Images/search/icon.png")); // 资源文件中的图标
        // 添加到 LineEdit 左侧
        ui->lEdit_search->addAction(searchAction, QLineEdit::LeadingPosition);
    }
    tip_edit = new NewCustomToolTip(this);
    tip_edit->setLabelStyle(0);
    tip_edit->AddToolTip(ui->pBt_EditType,tr("编辑"),Qt::AlignHCenter);

    tip_export = new NewCustomToolTip(this);
    tip_export->setLabelStyle(0);
    tip_export->AddToolTip(ui->pBt_ExportPlan,tr("导出方案"),Qt::AlignHCenter);

    tip_load = new NewCustomToolTip(this);
    tip_load->setLabelStyle(0);
    tip_load->AddToolTip(ui->pBt_LoadPlan,tr("导入方案"),Qt::AlignHCenter);

    tip_add = new NewCustomToolTip(this);
    tip_add->setLabelStyle(0);
    tip_add->AddToolTip(ui->pBt_NewPlan,tr("新建方案"),Qt::AlignHCenter);

    tip_editing = new NewCustomToolTip(this);
    tip_editing->setLabelStyle(0);
    tip_editing->AddToolTip(ui->pBt_CancelEditType,tr("取消编辑"),Qt::AlignHCenter);

    tip_PlansDel = new NewCustomToolTip(this);
    tip_PlansDel->setLabelStyle(0);
    tip_PlansDel->AddToolTip(ui->pBt_DelPlans,tr("删除方案"),Qt::AlignHCenter);

    tip_PlansMove = new NewCustomToolTip(this);
    tip_PlansMove->setLabelStyle(0);
    tip_PlansMove->AddToolTip(ui->pBt_MoveToType,tr("移动到"),Qt::AlignHCenter);
}

void SpeakerSet::ShowSysPlan()
{
    bool IsAdded = false;
    int favIdx = -1;
    double freqVal[10] = {800,2300,5500,6500,8000,20000,20000,20000,20000,20000};//频点不能为0
    double eqVal[10] = {-3,2,-3,-3,-3,0,0,0,0,0};
    double qVal[10] = {0.5,1.5,4,0.5,4,0.7,0.7,0.7,0.7,0.7};
    AddSysPlan("无畏契约优化版","GAME模式，针对《无畏契约》游戏场景优化，增强脚步声方位识别精度，提升枪声辨识度，降低环境噪音干扰，适合竞技对抗场景长时间使用",QStringList{"T10有线"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal,eqVal,qVal);
    double freqVal2[10] = {20,450,1200,5500,8000,20000,20000,20000,20000,20000};
    double eqVal2[10] = {-4,-2,-4,-3,-5,0,0,0,0,0};
    double qVal2[10] = {0.7,2.5,0.5,4,4,0.7,0.7,0.7,0.7,0.7};
    AddSysPlan("CSGO优化版","GAME模式，针对《CSGO》游戏特性优化，强化近距离脚步声清晰度，平衡中远距离枪声层次感，提升音效定位准确性，支持玩家快速判断敌人位置",QStringList{"T10有线"},"CSGO",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal2,eqVal2,qVal2);
    double freqVal3[10] = {100,2500,5000,20000,20000,20000,20000,20000,20000,20000};
    double eqVal3[10] = {4,-5,-4,0,0,0,0,0,0,0};
    double qVal3[10] = {1.2,0.5,3,0.7,0.7,0.7,0.7,0.7,0.7,0.7};
    AddSysPlan("绝地求生优化版","GAME模式，针对《绝地求生》特性优化，增强远距离脚步声捕捉能力，提升枪声传播距离感知，优化环境音细节表现，帮助玩家提前发现敌人",QStringList{"T10有线"},"PUBG",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal3,eqVal3,qVal3);

    double freqVal5[10] = {6500,8000,13000,270,700,20,2500,180,8000,16000};
    double eqVal5[10] = {-2,-2,-3,-4,-2,-3,2,0,0,0};
    double qVal5[10] = {2,7,2,2,2,1.3,1,2.5,4,0.5};
    AddSysPlan("三角洲优化版","GAME模式，针对《三角洲行动》场景优化，平衡脚步声、枪声、环境音三者关系，提升复杂地形下的音效定位能力，增强游戏沉浸感和竞技优势",QStringList{"T10有线"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal5,eqVal5,qVal5);

    double freqVal4[10] = {80,20,20,250,1500,270,2200,5500,8000,16000};
    double eqVal4[10] = {3,-4,0,0,0,-4,0,-4,-6,-3};
    double qVal4[10] = {1.5,1.0,1.5,0.5,2.5,2.5,2.5,2.5,2.0,1.5};
    AddSysPlan("GAME-舒适版","通用FPS游戏模式，采用柔和音效，降低高频刺激声，提升长时间佩戴舒适度，同时保持脚步声基本辨识度，适合多游戏场景通用",QStringList{"T10有线"},"游戏",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal4,eqVal4,qVal4);

    double freqVal6[10] = {80,20,120,250,1500,270,2200,5500,8000,16000};
    double eqVal6[10] = {2,-4,0,0,3,-3,3,-2.5,-3,-2};
    double qVal6[10] = {2.5,1.0,0.5,0.5,2.5,2.5,2.5,2.0,2.0,1.5};
    AddSysPlan("GAME-清脆版","通用FPS游戏模式，采用柔和音效，降低高频刺激声，提升长时间佩戴舒适度，同时保持脚步声基本辨识度，适合多游戏场景通用",QStringList{"T10有线"},"游戏",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal6,eqVal6,qVal6);

    double freqVal7[10] = {20,80,120,250,500,1650,3000,5500,8000,16000};
    double eqVal7[10] = {-7,2,0,0,0,-3,5,-3,0,-4};
    double qVal7[10] = {1.0,1.5,0.5,0.5,0.5,1.5,3.5,2.5,0.5,2.5};
    AddSysPlan("FPS模式-舒适版","通用FPS游戏模式，采用柔和音效，降低高频刺激声，提升长时间佩戴舒适度，同时保持脚步声基本辨识度，适合多游戏场景通用",QStringList{"T10无线"},"游戏",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal7,eqVal7,qVal7);

    double freqVal8[10] = {20,80,120,250,500,1000,3000,5500,8000,16000};
    double eqVal8[10] = {-3,2,0,0,0,0,4,-4,0,-5};
    double qVal8[10] = {1.0,1.5,0.5,0.5,0.5,0.5,3.5,8.0,0.5,2.5};
    AddSysPlan("FPS增强-清脆版","通用FPS游戏模式，优化高频音效表现，提升脚步声清脆度，避免枪声沉闷感，平衡音效层次感，适合追求清晰音效的玩家",QStringList{"T10无线"},"游戏",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal8,eqVal8,qVal8);

    double freqVal9[10] = {90,450,270,1500,100,20,2000,4000,8000,16000};
    double eqVal9[10] = {3,2,-3,3,0,0,0,0,0,0};
    double qVal9[10] = {2.0,2.0,2.0,2.0,0.5,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("FPS增强-洲-优化","FPS增强模式，优化了脚步声、枪声、上下楼层，提升复杂地形下的音效定位能力",QStringList{"K06S"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal9,eqVal9,qVal9);

    double freqVal10[10] = {120,90,1500,20,500,1000,2000,4000,8000,16000};
    double eqVal10[10] = {-4,3.5,3,-2,0,0,0,0,0,0};
    double qVal10[10] = {0.8,2.0,2.0,1.0,0.5,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("FPS模式-瓦-优化","FPS模式，增强脚步声方位识别精度，提升枪声辨识度，降低环境噪音干扰",QStringList{"K06S"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal10,eqVal10,qVal10);




    double freqVal11[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal11[10] = {-2,2,-3,0,-2,3,2,0,0,-3};
    double qVal11[10] = {1.0,2.0,3.0,0.5,2.0,2.0,2.0,0.5,0.5,1.5};
    AddSysPlan("瓦-清风优化版","GAME模式，提升脚步量感，提升声音明亮，可根据自己听感微调",QStringList{"T10有线"},"无畏契约",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal11,eqVal11,qVal11);

    double freqVal12[10] = {30,85,120,250,500,1000,1500,5500,8000,16000};
    double eqVal12[10] = {0,0,0,0,0,0,0,0,-3,-3};
    double qVal12[10] = {0.5,2.0,0.5,0.5,0.5,0.5,0.5,0.5,1.5,2.0};
    AddSysPlan("GAME-枪声弱化","GAME模式，减少枪声刺激感，根据自己听感微调",QStringList{"T10有线"},"三角洲行动",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal12,eqVal12,qVal12);

    double freqVal13[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal13[10] = {0,2,0.1,0,-2,1,0,0,-2,-2};
    double qVal13[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("三角洲-清风优化版","GAME模式，优化声场通透度，脚步声细节更清晰，弱化了枪声，可根据听感微调",QStringList{"T10有线"},"三角洲行动",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal13,eqVal13,qVal13);

    double freqVal14[10] = {20,85,150,270,500,1500,2000,5500,8000,16000};
    double eqVal14[10] = {-4,3,-2,-2,0,2,2,0,-2.5,-2};
    double qVal14[10] = {1,2,3,2.5,0.5,2.5,2.0,0.5,1.0,2.5};
    AddSysPlan("三角洲-焚决V2.0","GAME模式，脚步声、枪声、细节表现进行深度优化，版本 V2.0",QStringList{"T10有线"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal14,eqVal14,qVal14);

    double freqVal15[10] = {20,85,150,250,500,1500,2000,5500,8000,16000};
    double eqVal15[10] = {-4,-2,-3,0,-3,2,0,-2,-2,-3};
    double qVal15[10] = {0.8,1.0,3.0,0.5,0.7,3.0,4.0,2.0,1.5,2.5};
    AddSysPlan("无畏契约-焚决V1.1","GAME模式，脚步声、枪声、定位进行优化，版本 V1.0",QStringList{"T10有线"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal15,eqVal15,qVal15);

    double freqVal16[10] = {20,85,120,270,500,1000,2000,5500,8000,16000};
    double eqVal16[10] = {-4,3,0,0,0,0,-2,0,-3,-3};
    double qVal16[10] = {1.0,2.5,0.5,0.1,0.5,0.5,1.5,2.5,1.5,2.5};
    AddSysPlan("FPS增强-舒适版","FPS 增强模式，听感柔和不刺耳，长时间佩戴舒适，适配多数射击游戏",QStringList{"T10无线"},"游戏",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal16,eqVal16,qVal16);

    double freqVal17[10] = {20,85,120,250,750,1000,2000,5500,8000,16000};
    double eqVal17[10] = {-4,3,0,0,0,0,0,0,0,-4};
    double qVal17[10] = {1.0,2.0,0.5,0.5,0.5,0.5,0.5,2.0,1.5,1.5};
    AddSysPlan("三角洲脚步增强","FPS 增强模式，脚步声细节，远距离脚步声放大，定位更准",QStringList{"T10无线"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal17,eqVal17,qVal17);

    double freqVal18[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal18[10] = {0,0,0,0,0,0,0,0,-3,-3};
    double qVal18[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,1.5,1.5};
    AddSysPlan("三角洲弱化枪声","FPS 增强模式，弱化枪声刺激感，突出脚步声，降低听觉疲劳",QStringList{"T10无线"},"三角洲行动",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal18,eqVal18,qVal18);

    double freqVal19[10] = {30,90,120,250,500,1000,2000,4000,8000,16000};
    double eqVal19[10] = {0,3,0,0,-1,-1,-3,0,-3,-3};
    double qVal19[10] = {0.5,2.0,0.5,0.5,2.0,1.5,1.5,0.5,1.5,1.5};
    AddSysPlan("洲-清风优化版","三角洲行动优化，声场更通透，脚步声更清晰",QStringList{"T10无线"},"三角洲行动",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal19,eqVal19,qVal19);


    double freqVal20[10] = {20,85,150,250,700,1000,2000,5500,8000,16000};
    double eqVal20[10] = {-3,0,0,0,-2,0,-1.5,-1.5,-2,-4};
    double qVal20[10] = {1.0,1.0,3.0,0.5,2.5,2.5,3.0,1.5,1.5,1.5};
    AddSysPlan("瓦-焚决V1.0","FPS 增强模式，针对《无畏契约》脚步声响应、定位进行优化，版本 V1.0",QStringList{"T10无线"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal20,eqVal20,qVal20);

    double freqVal21[10] = {30,60,120,250,500,1500,2000,4000,8000,16000};
    double eqVal21[10] = {0,4,0,0,0,3,0,0,-3,-3};
    double qVal21[10] = {0.5,1.5,0.5,0.5,0.5,2.0,0.5,0.5,1.6,1.5};
    AddSysPlan("CF校长-焚决版","GAME 模式，针对《穿越火线》脚步声、枪声、细节表现进行优化，版本 V1.0",QStringList{"T10有线"},"穿越火线",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal21,eqVal21,qVal21);

    double freqVal22[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal22[10] = {-8,5,2,-5,-2,2,2,0,4,3};
    double qVal22[10] = {0.7,1.5,1.5,1.0,1.0,1.0,1.0,0.5,1.0,0.5};
    AddSysPlan("FPS模式暗区V2.0","FPS 模式，针对《暗区突围》环境音、脚步声，版本 V2.0",QStringList{"T10无线"},"暗区突围",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal22,eqVal22,qVal22);

    double freqVal23[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal23[10] = {-4,2,2,-5,-3,0,0,0,-4,-5};
    double qVal23[10] = {1.0,1.0,1.0,1.5,1.5,0.5,0.5,0.5,1.0,0.5};
    AddSysPlan("GAME-暗区V2.0","GAME 模式，针对《暗区突围》环境音、脚步声，版本 V2.0",QStringList{"T10有线"},"暗区突围",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal23,eqVal23,qVal23);

    double freqVal24[10] = {90,1500,10000,450,270,20,7000,4000,8000,16000};
    double eqVal24[10] = {4,3,4,-3,-2,-8,3,-2,0,0};
    double qVal24[10] = {2.0,2.0,2.0,2.5,2.0,0.8,1.0,6.0,0.5,0.5};
    AddSysPlan("FPS模式暗区V1.0","FPS 模式，针对《暗区突围》脚步声、枪声、环境音优化，版本 V1.0",QStringList{"T10无线"},"暗区突围",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal24,eqVal24,qVal24);

    double freqVal25[10] = {20,7000,5500,450,180,800,10000,1500,270,90};
    double eqVal25[10] = {-5,-6,-4,-2,-3,-2,2,2,-3,3};
    double qVal25[10] = {0.9,2.0,3.0,2.0,3.0,0.7,2.0,2.0,2.0,2.0};
    AddSysPlan("GAME-暗区V1.0","GAME 模式，针对《暗区突围》脚步声、枪声优化，版本 V1.0",QStringList{"T10有线"},"暗区突围",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal25,eqVal25,qVal25);

    double freqVal26[10] = {20,85,120,250,450,750,1500,5500,8000,16000};
    double eqVal26[10] = {-3,2,0,0,-2,-2,2,1,2,0};
    double qVal26[10] = {1.0,1.5,0.5,0.5,3.0,3.0,1.5,2.0,1.5,0.5};
    AddSysPlan("瓦2-FPS模式","FPS 模式，针对《无畏契约》脚步声响应、定位优化，版本V 2 ",QStringList{"K06S"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal26,eqVal26,qVal26);

    double freqVal27[10] = {20,85,270,250,500,1500,2000,5500,8000,16000};
    double eqVal27[10] = {-3,2,-2,0,0,3,0,1,2,0};
    double qVal27[10] = {1.0,2.0,2.5,0.5,0.5,2.0,0.5,2.0,1.5,0.5};
    AddSysPlan("洲-焚决V1.0","FPS 模式，针对《三角洲行动》脚步声、枪声、细节表现优化，焚决 V1.0",QStringList{"K06S"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal27,eqVal27,qVal27);

    double freqVal28[10] = {20,270,90,1500,800,1000,2000,4000,7500,16000};
    double eqVal28[10] = {-6,-4,3.5,1.5,-1,0,0,0,2,0};
    double qVal28[10] = {0.8,1.7,1.5,2.0,0.7,1.0,0.5,1.0,2.0,0.5};
    AddSysPlan("CS学长特调V1","FPS 模式，脚步声清晰度优化",QStringList{"K06S"},"CSGO",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal28,eqVal28,qVal28);

    double freqVal29[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal29[10] = {-5,2,2,-3,0,0,0,-2,0,0};
    double qVal29[10] = {2.0,2.0,0.7,1.0,0.5,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("CS学长特调V2","FPS 模式，脚步声、枪声细节，可二次修改",QStringList{"K06S"},"CSGO",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal29,eqVal29,qVal29);

    double freqVal30[10] = {30,60,120,250,800,1000,2000,4000,8000,16000};
    double eqVal30[10] = {-3.5,2,2,-3,-2,0,3,0,0,2};
    double qVal30[10] = {1.0,1.5,1.5,1.0,1.0,0.5,0.7,0.5,0.5,0.5};
    AddSysPlan("吃鸡V2","FPS 模式，针对《绝地求生》远距离脚步声二次优化",QStringList{"K06S"},"PUBG",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal30,eqVal30,qVal30);

    double freqVal31[10] = {20,180,270,250,800,1000,1500,4000,7800,16000};
    double eqVal31[10] = {-6,2,-4.5,0,-2.1,-2.8,3,-1.3,2,0};
    double qVal31[10] = {4.0,2.0,2.0,0.5,0.5,0.5,2.0,3.0,2.0,0.1};
    AddSysPlan("吃鸡V1","FPS 模式，针对《绝地求生》脚步声、枪声表现优化",QStringList{"K06S"},"PUBG",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal31,eqVal31,qVal31);

    double freqVal32[10] = {20,90,450,270,800,1000,1500,4000,8000,16000};
    double eqVal32[10] = {-6,3,1.0,-4,-1,0,3,-1,0,0};
    double qVal32[10] = {4.0,2.0,2.0,3.0,1.2,0.5,2.0,2.0,0.5,0.5};
    AddSysPlan("CS学长优化V1","FPS 模式，脚步声清晰度优化",QStringList{"T10无线"},"CSGO",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal32,eqVal32,qVal32);

    double freqVal33[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal33[10] = {-3,2,2,-3,-2,0,0,-1,0,0};
    double qVal33[10] = {1.0,0.8,1.5,1.0,1.0,0.5,0.5,0.7,0.5,0.5};
    AddSysPlan("CS学长优化V2","FPS 模式，脚步声、枪声细节，可二次修改",QStringList{"T10无线"},"CSGO",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal33,eqVal33,qVal33);

    double freqVal34[10] = {20,90,1500,270,450,8000,2000,4000,8000,16000};
    double eqVal34[10] = {-3,3,3,-3,-2,3,0,0,0,0};
    double qVal34[10] = {0.9,2.0,2.0,2.0,2.5,2.0,0.5,0.5,2.0,0.5};
    AddSysPlan("暗区1-FPS模式","FPS 模式，针对《暗区突围》脚步声、枪声、细节声、分离度，版本 V1",QStringList{"K06S"},"暗区突围",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal34,eqVal34,qVal34);

    double freqVal35[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal35[10] = {-3,2,2,-3,-2,0,3,0,2,0};
    double qVal35[10] = {1.0,1.0,1.0,1.5,2.0,0.5,1.5,0.5,1.0,0.5};
    AddSysPlan("暗区2-FPS模式","FPS 模式，针对《暗区突围》脚步声、细节声、分离度，可二次修改，版本 V2",QStringList{"K06S"},"暗区突围",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal35,eqVal35,qVal35);

    double freqVal36[10] = {20,90,270,450,1500,1000,2000,4000,8000,16000};
    double eqVal36[10] = {-6,3,-3,-2,2,0,0,-1,1,1.3};
    double qVal36[10] = {2.0,2.0,2.0,2.0,2.0,0.5,0.5,3,2.0,1.0};
    AddSysPlan("FPS模式-今晚吃鸡","FPS 模式，针对《绝地求生》远距离脚步声优化",QStringList{"T10无线"},"PUBG",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal36,eqVal36,qVal36);



    double freqVal37[10] = {20,90,160,250,500,1000,2000,5000,7000,16000};
    double eqVal37[10] = {-6,-4,0,0,0,0,-2.5,0,-2,0};
    double qVal37[10] = {1.0,1.0,0.5,0.5,0.5,0.5,1.0,1.5,1.0,0.5};
    AddSysPlan("T7-焚决V1.0","FPS增强模式，优化声场，提升声音清晰度，弱化枪声刺耳，声音更通透",QStringList{"T7"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal37,eqVal37,qVal37);

    double freqVal38[10] = {20,75,150,250,700,1500,2000,4000,8000,16000};
    double eqVal38[10] = {-4,-2,-5,0,-3,0,0,0,0,0};
    double qVal38[10] = {1.0,1.5,1.5,0.5,2.0,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("无畏契约-焚决V1","FPS模式，优化混响感，声音更清脆明亮，提升清晰度，分离度，声音更通透",QStringList{"T7"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal38,eqVal38,qVal38);

    double freqVal39[10] = {30,60,160,250,500,1000,2000,4000,8000,16000};
    double eqVal39[10] = {0,-2,-3,0,0,0,-2,0,-2,0};
    double qVal39[10] = {0.5,0.5,1.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("T7-三角洲-清风","FPS增强模式， 减低混响，提升清晰度，弱化了枪声，版本V1.0",QStringList{"T7"},"三角洲行动",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal39,eqVal39,qVal39);

    double freqVal40[10] = {2000,4700,8000,20,82,8000,450,4700,8000,16000};
    double eqVal40[10] = {1,1,1,-3,2,-2,1,0,0,0};
    double qVal40[10] = {3.0,3.0,1.0,4.0,2.0,2.0,1.0,3.0,2.0,0.5};
    AddSysPlan("T7 PUBG优化","FPS增强模式，优化脚步距离表现，放大脚步声，版本V0.1",QStringList{"T7"},"PUBG",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal40,eqVal40,qVal40);

    double freqVal41[10] = {2500,100,1500,450,700,1000,2000,4000,8000,16000};
    double eqVal41[10] = {-5,3,3,-1.5,-2,0,0,0,0,0};
    double qVal41[10] = {1.5,2.5,2.0,2.0,1.5,0.5,0.5,0.5,0.1,0.5};
    AddSysPlan("T7 CS优化","FPS模式，优化脚步距离表现，脚步声影响范围，优化枪声，版本V0.1 ",QStringList{"T7"},"CSGO",IsAdded,favIdx,true,false,false,true,0,0,0,freqVal41,eqVal41,qVal41);

    double freqVal42[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
    double eqVal42[10] = {0,2,0.1,0,-2,1,0,0,-2,-2};
    double qVal42[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("三角洲-焚决V3.0","GAME模式，脚步声、枪声、细节表现进行深度优化，版本V3.0",QStringList{"T10有线"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal42,eqVal42,qVal42);

    //K03S超竞版
    double freqVal43[10] = {90,90,300,800,1500,8000,20000,20000,20000,20000};
    double eqVal43[10] = {-5.0, 3.0, 3.0, -3.0, 3.0, 3.0, 0, 0, 0, 0};
    double qVal43[10] = {2.5, 1.5, 1.5, 1.5, 1.5, 1.0, 0.5, 0.5, 0.5, 0.5};
    AddSysPlan("三角洲-焚决V1.0","提升整体清晰度，让声音细节更干净，脚步变得更清晰--焚决v1.0版本",QStringList{"K03S超竞版"},"三角洲行动",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal43,eqVal43,qVal43);

    double freqVal44[10] = {90,90,300,800,1500,8000,20000,20000,20000,20000};
    double eqVal44[10] = {-5.0,  3.0,  3.0,  -3.0,  3.0,  3.0,  0,  0,  0,  0};
    double qVal44[10] = {2.5, 1.5, 1.5, 1.5, 1.5, 1.0, 0.5, 0.5, 0.5, 0.5};
    AddSysPlan("无畏契约-焚决V1.0","优化了声音的分离度，脚步、技能、枪声分离使得更清晰，--焚决v1.0版本",QStringList{"K03S超竞版"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal44,eqVal44,qVal44);

    double freqVal45[10] = {90,90,300,800,8000,20000,20000,20000,20000,20000};
    double eqVal45[10] = {-5.0, 3.0, 4.0, -3.0, 3.0, 0, 0, 0, 0, 0};
    double qVal45[10] = {2.5, 1.5, 1.5, 1.5, 1.0, 0.5, 0.5, 0.5, 0.5, 0.5};
    AddSysPlan("GO学长专用","优化了声音的分离度，脚步、道具、枪声分离使得更清晰，快速定位敌人击败对手！！--焚决v1.0版本",QStringList{"K03S超竞版"},"CSGO",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal45,eqVal45,qVal45);

    double freqVal46[10] = {20,50,90,140,300,800,1500,3500,8000,20000};
    double eqVal46[10] = {4.0, 4.0, -4.0, 4.0, 3.0, -3.0, 3.0, 3.0, 3.0, 0};
    double qVal46[10] = {0.7, 1.5, 2.0, 1.5, 1.5, 1.5, 1.5, 2.0, 1.5, 0.5};
    AddSysPlan("大吉大利今晚吃鸡","增强远距离脚步声捕捉能力，提升枪声传播距离感知，优化环境音细节表现，帮助玩家提前发现敌人",QStringList{"K03S超竞版"},"PUBG",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal46,eqVal46,qVal46);

    double freqVal47[10] = {20,75,150,250,700,1500,2000,4000,8000,16000};
    double eqVal47[10] = {12,12,12,12,12,12,12,12,12,12};
    double qVal47[10] = {1.0,1.5,1.5,0.5,2.0,0.5,0.5,0.5,0.5,0.5};
    AddSysPlan("测试","测试",QStringList{"T7"},"无畏契约",IsAdded,favIdx,false,false,false,true,0,0,0,freqVal47,eqVal47,qVal47);

    //修改为读取文件之后，再执行所有预设
    //第一次加载这个，然后报存。第二次直接运行保存的，且若用户点击初始化，则调用此处的

    globalSettings->setValue("ShowSysPlanEn",false);
    globalSettings->setValue("SysPlanAdd",0);//下次为0则新增，旧版本用的这个，为了防止测试时，这块出现问题，所以直接用新的，旧的给写成1，这样安装回旧版本，则执行旧版本的系统方案
    globalSettings->setValue("SysPlanAdd_New",1);//下次为0,1则新增
}


void SpeakerSet::CreateScrollArea()
{
    //所有预设
    // 创建 QScrollArea
    scrollArea_All = new QScrollArea(ui->widget_CloudPlan);
    //scrollArea->setGeometry(0, 0, 400, 300); // 设置滚动区域大小
    scrollArea_All->setWidgetResizable(true);
    // 背景透明
    scrollArea_All->setStyleSheet("border:0px;background: transparent;");
    // 滚动条样式
    scrollArea_All->verticalScrollBar()->setStyleSheet(R"(
    QScrollBar:vertical {
        background-color: transparent;
        width: 10px;
        margin: 0px;
        padding: 0px;
        border-radius: 5px;
    }
    QScrollBar::handle:vertical {
        background: rgba(0, 0, 0,51);
        border-radius: 5px;
        min-height: 97px;
    }
    QScrollBar::sub-line:vertical,
    QScrollBar::add-line:vertical {
        height: 0px;
        background: none;
    }
    QScrollBar::add-page:vertical,
    QScrollBar::sub-page:vertical {
        background: none;
    }
)");
    scrollArea_All->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 隐藏水平滚动条
    // scrollArea_All->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);    // 隐藏垂直滚动条
    // 创建内容区域
    content_All = new QWidget();
    content_All->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    //栅格布局
    layout_All = new QGridLayout(content_All);
    layout_All->setSpacing(13); // 按钮之间的固定间距为13px
    layout_All->setContentsMargins(30, 0, 30, 0); // 按钮区域与左右边框的距离为30px

    // 关键修改：禁止所有列拉伸
    for (int c = 0; c < COLUMN_COUNT_rb; c++) {
        layout_All->setColumnMinimumWidth(c, CurrentBtnWight_rb);  // 设置最小列宽
        layout_All->setColumnStretch(c, 0);
    }

    // 可选：设置行间距和列间距
    // layout_All->setHorizontalSpacing(5);
    // layout_All->setVerticalSpacing(12);

    content_All->setLayout(layout_All);
    scrollArea_All->setWidget(content_All);

    //垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(ui->widget_CloudPlan);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 可选：设置边距
    mainLayout->addWidget(scrollArea_All);
}

void SpeakerSet::AddSysPlan(const QString& name,
                            const QString& description,
                            const QStringList& Lab1,
                            const QString& Lab2,
                            bool IsAdded,
                            int favIdx,
                            bool DataVisibleEn,bool AlgoOpenEn, bool spaceOpenEn, bool eqOpenEn,
                            int lowVal, int spaceVal, int GainVal,
                            double freqVal[10], double eqVal[10], double qVal[10])
{
    PlanVal aVal;
    aVal.DataVisibleEn = DataVisibleEn;
    aVal.ParentPlanName = "";
    aVal.AlgoOpenEn = AlgoOpenEn;
    aVal.spaceOpenEn = spaceOpenEn;
    aVal.eqOpenEn = eqOpenEn;
    memset(aVal.ExtraEq,0,sizeof(aVal.ExtraEq));
    aVal.lowVal = lowVal;
    aVal.drcVal = 0;
    aVal.GainVal = GainVal;
    aVal.spaceVal = spaceVal;
    aVal.spaceReverb = 0;
    aVal.spaceSize = 0;
    memcpy(aVal.freqVal,freqVal,sizeof(aVal.freqVal));
    memcpy(aVal.eqVal,eqVal,sizeof(aVal.eqVal));
    memcpy(aVal.qVal,qVal,sizeof(aVal.qVal));
    std::fill(std::begin(aVal.filterVal),    std::end(aVal.filterVal),    0);
    //二创
    std::fill(std::begin(aVal.freqVal_deriv), std::end(aVal.freqVal_deriv), 20000.0);
    std::fill(std::begin(aVal.eqVal_deriv),   std::end(aVal.eqVal_deriv),   0.0);
    std::fill(std::begin(aVal.qVal_deriv),    std::end(aVal.qVal_deriv),    0.5);
    std::fill(std::begin(aVal.filterVal_deriv),    std::end(aVal.filterVal_deriv),    0);

    addAllPlan(name,description,0,aVal,IsAdded,favIdx,false,true,Lab1,Lab2,0,true,false,"","");
}


//语言更新
void SpeakerSet::LanguageSet()
{
    //刷新文本
    ui->retranslateUi(this);
}

//窗体大小发生变化时
void SpeakerSet::resizeEvent(QResizeEvent* event)
{
    if (this->isMinimized()) {
        QWidget::resizeEvent(event);
        return;
    }
    QWidget::resizeEvent(event);

    updateSize();
    QTimer::singleShot(50, this, [this]() {
        setupRadioButtons(FullScreenEn);
    });
}
void SpeakerSet::updateSize()
{
    // 空态页图片随窗口成比例变大（保持宽高比不变形，border-image 等比拉伸不失真）
    updateEmptyImageScale(m_searchEmptySpec);
    updateEmptyImageScale(m_planEmptySpec);
}

// 空态页图片等比缩放：
// 以页面首次正常布局尺寸为设计基准，取当前宽/高缩放倍率中较小者（"最小值"）作为缩放系数，
// 保证图片等比放大后完整放入页面；下限为原始尺寸（窗口缩小时图片不回缩）。
// 放大后按文字上方可用空间钳制高度，防止压到提示文字。
void SpeakerSet::updateEmptyImageScale(EmptyImageSpec &t_spec)
{
    if (!t_spec.img || !t_spec.text || !t_spec.page) return;
    // 仅在窗口正常（非最大化/全屏）状态记录基准，避免启动即全屏时把全屏尺寸记为基准
    if (t_spec.baseSize.isEmpty() && !isMaximized() && !isFullScreen())
        t_spec.baseSize = t_spec.page->size();
    if (t_spec.baseSize.isEmpty())
        return; // 尚无正常状态基准（启动即全屏）：保持原始尺寸，窗口还原后再按比例缩放

    // 取宽、高缩放倍率中较小者：图片完整放入页面
    qreal t_scale = qMin(double(t_spec.page->width()) / t_spec.baseSize.width(),
                         double(t_spec.page->height()) / t_spec.baseSize.height());
    if (t_scale < 1.0) t_scale = 1.0; // 只放大不缩小

    // 高度上限：图片底部不能压到文字（父容器 VBox 内 Fixed spacer 与文字高度）
    QWidget *t_parent = t_spec.img->parentWidget();
    QLayout *t_layout = t_parent ? t_parent->layout() : nullptr;
    QLayoutItem *t_spacerItem = t_layout ? t_layout->itemAt(2) : nullptr;
    const int t_fixedSpacerH = (t_spacerItem && t_spacerItem->spacerItem())
            ? t_spacerItem->spacerItem()->sizeHint().height() : 0;
    const int t_maxImgH = qMax(t_spec.origSize.height(),
                               (t_parent ? t_parent->height() : t_spec.page->height()) - t_fixedSpacerH - t_spec.text->height());
    const int t_maxImgW = qMax(t_spec.origSize.width(), t_spec.page->width());
    t_scale = qMin(t_scale, double(t_maxImgH) / t_spec.origSize.height());
    t_scale = qMin(t_scale, double(t_maxImgW) / t_spec.origSize.width());

    const int t_newW = qRound(t_spec.origSize.width() * t_scale);
    const int t_newH = qRound(t_spec.origSize.height() * t_scale);
    t_spec.img->setFixedSize(t_newW, t_newH);
}


// ========== 请求延迟布局（防抖） ==========
void SpeakerSet::scheduleLayoutUpdate()
{
    if (m_layoutUpdatePending)
        return;
    m_layoutUpdatePending = true;
    QTimer::singleShot(0, this, &SpeakerSet::performLayoutUpdate);
}

// ========== 实际执行布局更新 ==========
void SpeakerSet::performLayoutUpdate()
{
    m_layoutUpdatePending = false;
    setupRadioButtons(FullScreenEn);
    scrollArea_All->ensureWidgetVisible(currentPlanRadio);
}

// ========== 全屏/正常布局入口 ==========
void SpeakerSet::setupRadioButtons(int isFullScreen)
{
    const int fullScreenMode = (isFullScreen == 1 || isFullScreen == 0)
    ? isFullScreen : FullScreenEn;

    if (ui->lEdit_search->text().isEmpty())
    {

        const int typeIndex = group_AlltypeBtn->checkedId();
        QList<NewRadioBtn*>* targetList = nullptr;
        //若存在根据机型显示方案时，只需要更新这些方案布局即可。否则更新所有方案布局
        if(MovieVal.AllPlanRadioList_Dev.isEmpty())
        {
            qDebug("MovieVal.AllPlanRadioList_Dev空\n");
            targetList = &((typeIndex == 0)
            ? MovieVal.AllPlanRadioList
            : PlanRadioLists[typeIndex - 1]);
        }else
        {
            qDebug("MovieVal.AllPlanRadioList_Dev非空\n");
            targetList = &((typeIndex == 0)
            ? MovieVal.AllPlanRadioList_Dev
            : PlanRadioLists[typeIndex - 1]);
        }
        if(targetList->isEmpty())
        {
            ui->stackedWidget->setCurrentWidget(ui->page_planEmpty);
            updateSize(); // 切到空态页立即按当前窗口尺寸重算图片比例
        }else
        {
            ui->stackedWidget->setCurrentWidget(ui->page_cloud);
        }
        updateRadioButtonLayout(*targetList, layout_All, scrollArea_All,
                                content_All, row_Allrb, col_Allrb,
                                fullScreenMode);
        spacer_Temp_All = nullptr;
    } else {
        if(AllPlanRadioList_Temp.isEmpty())
        {
            ui->stackedWidget->setCurrentWidget(ui->page_searchEmpty);
            updateSize(); // 切到空态页立即按当前窗口尺寸重算图片比例
        }else
        {
            ui->stackedWidget->setCurrentWidget(ui->page_cloud);
            updateRadioButtonLayout(AllPlanRadioList_Temp, layout_All, scrollArea_All,
                                    content_All, row_Allrb_Temp, col_Allrb_Temp,
                                    fullScreenMode);
            spacer_Temp_Search = nullptr;
        }

    }
}

// ========== 通用按钮布局全量刷新 ==========
void SpeakerSet::updateRadioButtonLayout(QList<NewRadioBtn*>& radioList,
                                         QGridLayout* layout,
                                         QScrollArea* scrollArea,
                                         QWidget* contentWidget,
                                         int& rowVar, int& colVar,
                                         int fullScreenMode)
{
    if (!contentWidget || !scrollArea) return;

    // 防止刷新期间闪烁
    contentWidget->setUpdatesEnabled(false);

    // 安全清空布局
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget()) {
            w->hide();
        }
        delete item;
    }

    // 固定间距与边距
    constexpr int spacing = 13;
    constexpr int marginHorizontal = 30;
    layout->setSpacing(spacing);
    layout->setContentsMargins(marginHorizontal, 0, marginHorizontal, 0);

    // 计算可用宽度
    const int viewportWidth = scrollArea->viewport()->width();
    const int cloudWidth = ui->widget_CloudPlan->width();
    const int targetWidth = (fullScreenMode == 1)
                                ? qMax(viewportWidth, cloudWidth)
                                : qMin(viewportWidth, cloudWidth);
    const int effectiveWidth = (targetWidth > 1071) ? targetWidth : 1071;
    const int availableWidth = effectiveWidth
                               - layout->contentsMargins().left()
                               - layout->contentsMargins().right();


    // 计算按钮尺寸与列数
    if (fullScreenMode == 1) {
        calculateOptimalButtonSize(availableWidth, spacing,
                                   btnWight_rb, btnMaxWight_rb);
    } else {
        CurrentBtnWight_rb = btnWight_rb;
        const int totalPerButton = CurrentBtnWight_rb + spacing;
        // 每行最后一个方案后面不需要13px间距（行末只需保持30px边框边距），故列数按 (可用宽+间距) 计算
        COLUMN_COUNT_rb = qMax(1, (availableWidth + spacing) / totalPerButton);
    }
    CurrentBtnHight_rb = btnHight_rb;


    qDebug()<<"updateRadioButtonLayout COLUMN_COUNT_rb:"<<COLUMN_COUNT_rb
             <<"effectiveWidth" <<effectiveWidth
        <<"availableWidth" <<availableWidth
        <<"viewportWidth" <<viewportWidth
        <<"cloudWidth" <<cloudWidth;

    // 重置列属性
    for (int c = 0; c < layout->columnCount(); ++c) {
        layout->setColumnStretch(c, 0);
        layout->setColumnMinimumWidth(c, 0);
    }
    for (int c = 0; c < COLUMN_COUNT_rb; ++c) {
        layout->setColumnStretch(c, 0);
        layout->setColumnMinimumWidth(c, CurrentBtnWight_rb);
    }

    // 无按钮时提前返回
    if (radioList.isEmpty()) {
        rowVar = 0;
        colVar = 0;
        contentWidget->setUpdatesEnabled(true);
        contentWidget->adjustSize();
        return;
    }

    // 重新排列按钮
    int buttonIndex = 0;
    const int total = radioList.size();
    int row = 0, col = 0;

    qDebug()<<"updateRadioButtonLayout SelDev_DeviceName2:"<<SelDev_DeviceName;
    qDebug()<<"updateRadioButtonLayout buttonIndex2:"<<buttonIndex;
    qDebug()<<"updateRadioButtonLayout total2:"<<total;

    while (buttonIndex < total) {
        col = 0;
        while (col < COLUMN_COUNT_rb && buttonIndex < total) {
            NewRadioBtn* radio = radioList[buttonIndex];
            // const QString dev = radio->lab1->text();//获取标签1
            // //判断该按钮是否应该显示
            // bool shouldShow = false;

            // if(!SelDev_DeviceName.isEmpty())
            // {
            //     shouldShow = isDeviceMatchingPlanDev(dev);
            // }else
            // {
            //     qDebug()<<"updateRadioButtonLayout SelDev_DeviceName:"<<SelDev_DeviceName;
            // }

            // qDebug()<<"dev"<<dev;

            bool shouldShow = true;
            if (shouldShow) {
                // 添加到网格
                radio->setFixedSize(CurrentBtnWight_rb, CurrentBtnHight_rb);
                radio->updateElidedText(radio->property("fullText").toString(),
                                        radio->lab_name->text());
                layout->addWidget(radio, row, col, 1, 1, Qt::AlignLeft | Qt::AlignTop);
                radio->show();
                ++col;
            } else {
                radio->hide();
            }

            ++buttonIndex;   // 推进索引，避免死循环

        }

        // 本行未满则添加弹簧
        if (col < COLUMN_COUNT_rb) {
            layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding,
                                            QSizePolicy::Minimum),
                            row, col, 1, COLUMN_COUNT_rb - col);

        }
        ++row;
    }

    // 行不满（方案少）时保持按钮间距固定：
    // 所有按钮行固定高度（stretch=0），多余垂直空间由底部弹性行（stretch=1）吸收，
    // 避免 QGridLayout 将多余空间均分给各行导致行间间距变大
    for (int r = 0; r <= qMax(row, m_stretchRow_All); ++r) {
        layout->setRowStretch(r, 0);
    }
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
                    row, 0, 1, COLUMN_COUNT_rb);
    layout->setRowStretch(row, 1);
    m_stretchRow_All = row;

    // 尾部弹性列：吸收多余水平空间，保证按钮之间水平间距固定为13
    // （列 stretch 全为 0 时多余宽度会被均分给各列，导致按钮间距变大）
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum),
                    0, COLUMN_COUNT_rb, qMax(1, row), 1);
    layout->setColumnStretch(COLUMN_COUNT_rb, 1);

    // 更新行列位置指针
    rowVar = row - 1;
    colVar = col;
    if (colVar >= COLUMN_COUNT_rb) {
        colVar = 0;
        ++rowVar;
    }

    contentWidget->setUpdatesEnabled(true);
    contentWidget->adjustSize();
}

// ========== 计算最佳按钮大小（全屏） ==========
void SpeakerSet::calculateOptimalButtonSize(int availableWidth, int spacing,
                                            int minWidth, int maxWidth)
{
    float bestUtilization = 0.0f;
    int bestWidth = minWidth;
    int bestCols = 1;

    const int maxPossibleCols = qMin(20, (availableWidth + spacing) / (minWidth + spacing));

    for (int cols = 1; cols <= maxPossibleCols; ++cols) {
        const int totalSpacing = (cols - 1) * spacing;
        const int widthForButtons = availableWidth - totalSpacing;
        int actualWidth = qBound(minWidth, widthForButtons / cols, maxWidth);
        const int usedWidth = actualWidth * cols + totalSpacing;
        const float utilization = static_cast<float>(usedWidth) / availableWidth;

        if (utilization > bestUtilization + 0.001f) {
            bestUtilization = utilization;
            bestWidth = actualWidth;
            bestCols = cols;
        } else if (qAbs(utilization - bestUtilization) < 0.001f && cols > bestCols) {
            bestWidth = actualWidth;
            bestCols = cols;
        }
    }

    CurrentBtnWight_rb = bestWidth;
    COLUMN_COUNT_rb = bestCols;
}

// ========== 增量添加单个按钮（日常方案新建时使用） ==========
void SpeakerSet::appendRadioToLayout(NewRadioBtn* radio,
                                     QGridLayout* layout,
                                     int& currentRow,
                                     int& currentCol,
                                     QSpacerItem*& spacerItem,
                                     int columnCount,
                                     int buttonWidth,
                                     int buttonHeight)
{
    if (!radio || !layout) return;

    // 维护底部弹性行：旧弹性行降级为固定高度（该行可能已落入新按钮）
    if (m_stretchRow_All >= 0) {
        layout->setRowStretch(m_stretchRow_All, 0);
        m_stretchRow_All = -1;
    }

    // 移除旧弹簧
    if (spacerItem) {
        layout->removeItem(spacerItem);
        delete spacerItem;
        spacerItem = nullptr;
    }

    // 设置按钮尺寸和文字
    radio->setFixedSize(buttonWidth, buttonHeight);
    radio->updateElidedText(radio->property("fullText").toString(),radio->lab_name->text());
    radio->show();
    ui->stackedWidget->setCurrentWidget(ui->page_cloud);

    // 添加到网格
    layout->addWidget(radio, currentRow, currentCol, 1, 1,
                      Qt::AlignLeft | Qt::AlignTop);

    // qDebug("方案添加到currentRow：%d,currentCol:%d\n",currentRow,currentCol);
    // 更新位置
    currentCol++;
    if (currentCol >= columnCount) {
        currentCol = 0;
        currentRow++;
    }

    // 放置新弹簧
    spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addItem(spacerItem, currentRow, currentCol,
                    1, columnCount - currentCol);

    // 新底部弹性行：下一个空闲位置所在行吸收多余垂直空间，保证按钮行间距固定
    layout->setRowStretch(currentRow, 1);
    m_stretchRow_All = currentRow;

    // 尾部弹性列：与全量刷新保持一致，保证按钮间水平间距固定
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum),
                    0, columnCount, qMax(1, currentRow), 1);
    layout->setColumnStretch(columnCount, 1);
}


/*删除一个按钮后，将其后的按钮依次前移，避免全量刷新
 * (目标网格布局,
 * 按钮列表（顺序与布局顺序一致）,
 * 被删按钮所在行（删除前）,被删按钮所在列（删除前）,
 * 当前行指针（用于后续追加，会更新）,当前列指针（用于后续追加，会更新）,
 * 固定列数,
 * 按钮宽度,按钮高度,
 * 末尾弹簧（需要更新）)*/
void SpeakerSet::incrementalRearrangeAfterDelete(QGridLayout* layout,
                                                 QList<NewRadioBtn*>& radioList,
                                                 int deletedRow, int deletedCol,
                                                 int& currentRow, int& currentCol,
                                                 int columnCount,
                                                 int buttonWidth, int buttonHeight,
                                                 QSpacerItem*& spacerItem)
{
    if (!layout || radioList.isEmpty()) {
        // 无按钮时清空布局并重置位置
        if (spacerItem) {
            layout->removeItem(spacerItem);
            delete spacerItem;
            spacerItem = nullptr;
        }
        currentRow = 0;
        currentCol = 0;
        return;
    }

    // 1. 如果被删按钮不在最后位置，需要将其后的所有按钮重新放置到布局中
    // 计算被删按钮在列表中的原始索引（删除前的索引）
    // 注意：radioList 此时已经移除了要删除的按钮，所以 radioList 中保存的是剩下的按钮
    // 但我们知道删除前的位置 (deletedRow, deletedCol)，可以算出从哪个索引开始需要重排
    int startIndex = deletedRow * columnCount + deletedCol;  // 被删按钮在删除前的全局序号

    // 由于 radioList 已经移除该按钮，radioList 中索引 startIndex 的元素原本是删除前的后一个按钮
    // 如果 startIndex >= radioList.size()，说明删除的是最后一个按钮，无需重排后续按钮
    if (startIndex < radioList.size()) {
        // 2. 临时隐藏并移除从 startIndex 开始的所有按钮（它们会被重新添加）
        QList<NewRadioBtn*> buttonsToRelocate;
        for (int i = startIndex; i < radioList.size(); ++i) {
            NewRadioBtn* btn = radioList[i];
            layout->removeWidget(btn);
            btn->hide();
            buttonsToRelocate.append(btn);
        }

        // 3. 从被删位置开始，重新按行主序添加这些按钮
        int newRow = deletedRow;
        int newCol = deletedCol;
        for (NewRadioBtn* btn : buttonsToRelocate) {
            btn->setFixedSize(buttonWidth, buttonHeight);
            btn->updateElidedText(btn->property("fullText").toString(), btn->lab_name->text());
            layout->addWidget(btn, newRow, newCol, 1, 1, Qt::AlignLeft | Qt::AlignTop);
            btn->show();
            newCol++;
            if (newCol >= columnCount) {
                newCol = 0;
                newRow++;
            }
        }

        // 4. 更新 currentRow/currentCol 为下一个空闲位置
        currentRow = newRow;
        currentCol = newCol;
    } else {
        // 删除的是最后一个按钮，只需调整行列指针
        currentRow = deletedRow;
        currentCol = deletedCol;
        // // 如果删除后 currentCol 变为 0，且前面还有行，需要回退一行（边界处理）
        // if (currentCol == 0 && currentRow > 0) {
        //     qDebug("都减，currentRow:%d,currentCol:%d\n",currentRow,currentCol);
        //     currentRow--;
        //     currentCol = columnCount - 1;
        // } else if (currentCol > 0) {
        //     qDebug("只减currentCol，currentRow:%d,currentCol:%d\n",currentRow,currentCol);
        //     currentCol--;
        // }
    }

    // 5. 处理末尾弹簧（保证弹簧始终在最后未满的位置）
    if (spacerItem) {
        layout->removeItem(spacerItem);
        delete spacerItem;
        spacerItem = nullptr;
    }
    // 在当前行列位置添加新的弹簧
    spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout->addItem(spacerItem, currentRow, currentCol, 1, columnCount - currentCol);
}





//双击跳转到EQ，按钮移动
bool SpeakerSet::eventFilter(QObject *obj, QEvent *event){
    //qDebug("eventFilter\n");
    if (obj->inherits("QRadioButton"))
    {
        // 当前方案指示器（NewRadioBtnText，非 NewRadioBtn）：双击跳转 EQ 界面
        if (obj == ui->rBt_currentPlan)
        {
            if (event->type() == QEvent::MouseButtonDblClick)
            {
                QMouseEvent *me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton && currentPlanRadio)
                {
                    qDebug("rBt_currentPlan 双击跳转EQ\n");
                    emit EQpageChange(currentPlanRadio->GetDataVisibleEn());
                    return true; // 事件已处理
                }
            }
            return QObject::eventFilter(obj, event);
        }

        NewRadioBtn *btn = qobject_cast<NewRadioBtn*>(obj);
        if (!btn)
            return QObject::eventFilter(obj, event);

        // 1. 鼠标双击：跳转 EQ 界面
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            // 终止正在进行的拖拽状态
            if (m_dragging) {
                m_dragging = false;
                // 清除高亮记录
                if (m_lastToggledButton) {
                    m_lastToggledButton = nullptr;
                }
            }

            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                qDebug("左键双击\n");
                emit EQpageChange(btn->GetDataVisibleEn());
                return true; // 事件已处理
            }
        }
        // 2. 鼠标移动：编辑模式 + 拖拽过程中，高亮经过的按钮
        else if (event->type() == QEvent::MouseMove)
        {
            if (editEn)
            {
                m_dragging = true;
                QMouseEvent *me = static_cast<QMouseEvent*>(event);
                if (me->buttons() & Qt::LeftButton)   // 左键按下
                {
                    // 获取父容器，假设所有按钮都在同一个父窗口下
                    QWidget *parent = btn->parentWidget();
                    if (parent)
                    {
                        QPoint pos = parent->mapFromGlobal(me->globalPos());
                        QWidget *child = parent->childAt(pos);
                        NewRadioBtn *targetBtn = qobject_cast<NewRadioBtn*>(child);

                        // 仅当鼠标进入一个「新的」按钮时才切换
                        if (targetBtn && targetBtn != m_lastToggledButton)
                        {
                            // 切换选中状态：若已为 true 则设为 false，反之设为 true
                            bool en = !targetBtn->AllpBt_check->isChecked();
                            targetBtn->AllpBt_check->setChecked(en);
                            // if(en)
                            // {
                            //     MovieVal.AllPlanRadioList_Check.append(targetBtn);
                            // }
                            m_lastToggledButton = targetBtn;
                        }
                    }
                }
                else
                {
                    // 左键未按下，重置记录
                    m_lastToggledButton = nullptr;
                }
            }
            return false;
        }
    }else
    {
        //双击给分类重命名，只有group_AlltypeBtn的1-8才可进入eventFilter
        if (event->type() == QEvent::MouseButtonDblClick) {
            if(ui->pBt_EditType->isChecked())
            {
            QPushButton *btn = qobject_cast<QPushButton*>(obj);
            if (btn && group_AlltypeBtn->buttons().contains(btn)) {
                int idx = group_AlltypeBtn->id(btn) - 1;   // 获取按钮对应的 id（1~8）-1
                // 弹出输入框，默认显示当前文本
                if(!CPlansType)
                {
                    CPlansType = new EditPlansType(m);
                }

                CPlansType->EditTitle(1);//重命名分类
                CPlansType->hidePrompt();
                QString oldName = PlansTypes[idx].Name;
                CPlansType->ShowEditName(idx,PlansTypes[idx].Name);
                int res = CPlansType->exec();
                if(res == QDialog::Accepted)
                {

                    typeButtons[idx]->setText(PlansTypes[idx].Name);

                    // if(!Mplan)
                    // {
                    //     Mplan = new MovePlan(m);
                    // }
                    // Mplan->delType(oldName);
                    // Mplan->addType(PlansTypes[idx].Name);

                    //保存方案分类
                    QVariantMap plansMap;
                    QVariantList Name;
                    QVariantList Enable;
                    for (auto& plan : PlansTypes) {
                        Name.append(plan.Name);
                        Enable.append(plan.en);
                    }
                    plansMap["TName"] = Name;
                    plansMap["TEn"] = Enable;
                    globalSettings->setValue("PlansTypes",plansMap);
                }
                CPlansType->close();

                return true; // 事件已处理
            }

            }
        }
        return false;
    }


    return false;
}

void SpeakerSet::InitUIInformation() {
    {
        // 模糊面板圆角
        ui->widget_2->setCornerRadius(10);
        ui->widget_3->setCornerRadius(10);
        ui->widget_right->setCornerRadius(10);
        ui->widget_all->setCornerRadius(10);
        ui->page_plan->setCornerRadius(10);
        ui->page_plan->setObjectName("SpeakerSet_page_plan");
        ui->page_plan->setStyleSheet(R"(
            #SpeakerSet_page_plan {
                border-radius: 10px;
                background-color: rgba(81, 96, 122, 0.2);
            }
        )");
    }
    {

        // ui->pBt_GameListen 完整设置
        ui->pBt_GameListen->setCl_min_size(QSize(88, 88));
        ui->pBt_GameListen->setCl_icon_size(QSize(34, 25), QSize(34, 25));
        ui->pBt_GameListen->setCl_icon_text_spacing(7);
        ui->pBt_GameListen->setCl_icon_point(QPoint(27, 32), QPoint(27, 25));
        ui->pBt_GameListen->setCl_bg_default_color(QColor(81, 96, 122, 51));
        ui->pBt_GameListen->setCl_bg_hover_color(QColor(255, 255, 255, 25));
        ui->pBt_GameListen->setCl_border_radius(10);
        // 显示信息 后手设置
        ui->pBt_GameListen->setCl_pixmap(
            QPixmap(":/Skin/Images/soundTest/soundTest_icon_2x.png"));
        ui->pBt_GameListen->setCl_classification_name(tr("试听"));
        ui->pBt_GameListen->repaint(); // 立即重绘
    }
}

void SpeakerSet::InitMember() {}

void SpeakerSet::InitConnect() {}

//点击试听
void SpeakerSet::on_pBt_GameListen_clicked(bool checked)
{
    //跳转试听界面
    emit pageChange();

}

/*//跳转预设界面(动画：从下往上出现)
void SpeakerSet::on_rBt_currentPlan_clicked()
{
    //防止动画被多次执行
    if (m_pageAnimating) return;
    m_pageAnimating = true;

    // 入口日志：线程ID、当前页面、计划模式
    emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan [mode=%1, currentWidget=%2]")
                    .arg(currentPlanMode)
                    .arg(ui->stackedWidget_Speaker->currentWidget() ?
                             ui->stackedWidget_Speaker->currentWidget()->objectName() : "null"));


    qDebug("on_rBt_currentPlan_clicked()\n");
    // ui->stackedWidget_Speaker->setCurrentWidget(ui->page_plan);
    //实现动画效果，界面从下往上移动出来
    //显示当前预设方案所属的预设界面（我的\所有）
    if(currentPlanMode == 0)
    {
        emit ApoManager::instance()->requestlogWithTime("uiA on_rBt_currentPlan: calling on_pBt_All");
        on_pBt_All_clicked();
    }else
    {
        emit ApoManager::instance()->requestlogWithTime("uiA on_rBt_currentPlan: calling on_pBt_Type1");
        on_pBt_Type1_clicked();
    }


    // 获取当前页面
    QWidget *currentPage = ui->stackedWidget_Speaker->currentWidget();

    emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan after pBt click: currentPage=%1, target=page_plan")
                    .arg(currentPage ? currentPage->objectName() : "null"));


    if (currentPage == ui->page_plan)
    {
        emit ApoManager::instance()->requestlogWithTime("uiA on_rBt_currentPlan: already page_plan, switching to page_System first");

        //若已是目标页，切换一下，使能再次动态出现
        ui->stackedWidget_Speaker->setCurrentWidget(ui->page_System);
        currentPage = ui->stackedWidget_Speaker->currentWidget();
        //return; // 如果目标页面已经是当前页面，则直接返回

        emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan after reset: currentPage=%1")
                        .arg(currentPage ? currentPage->objectName() : "null"));

    }


    // 检查主线程
    if (QThread::currentThread() != QApplication::instance()->thread()) {
        emit ApoManager::instance()->requestlogWithTime("uiA ERROR: on_rBt_currentPlan NOT in main thread!");
    }

    // 记录 hide 前页面的状态
    emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan before hide: currentPage visible=%1 size=%2x%3")
                    .arg(currentPage->isVisible())
                    .arg(currentPage->width()).arg(currentPage->height()));



    currentPage->hide();

    // 获取堆叠窗口的高度和宽度
    int height = ui->stackedWidget_Speaker->height();
    int width = ui->stackedWidget_Speaker->width();

    emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan stack size: %1x%2").arg(width).arg(height));

    // 记录 page_plan 初始状态
    QString geoStr = QString("(%1,%2 %3x%4)")
                         .arg(ui->page_plan->geometry().x())
                         .arg(ui->page_plan->geometry().y())
                         .arg(ui->page_plan->geometry().width())
                         .arg(ui->page_plan->geometry().height());
    emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan page_plan after show: visible=%1 geometry=%2")
                         .arg(ui->page_plan->isVisible())
                         .arg(geoStr));


    // 设置目标页面的位置：在堆叠窗口的下方
    ui->page_plan->setGeometry(0, height, width, height);
    ui->page_plan->show(); // 确保目标页面显示


    geoStr = QString("(%1,%2 %3x%4)")
                         .arg(ui->page_plan->geometry().x())
                         .arg(ui->page_plan->geometry().y())
                         .arg(ui->page_plan->geometry().width())
                         .arg(ui->page_plan->geometry().height());
    emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan page_plan after show: visible=%1 geometry=%2")
                         .arg(ui->page_plan->isVisible())
                         .arg(geoStr));

    // 动画开始前记录
    emit ApoManager::instance()->requestlogWithTime("uiA on_rBt_currentPlan animation group starting...");



    // 创建当前页面的动画：从当前位置移动到(0, -height)
    QPropertyAnimation *animationCurrent = new QPropertyAnimation(currentPage, "geometry", this);
    animationCurrent->setDuration(300); // 动画时间300毫秒
    animationCurrent->setStartValue(currentPage->geometry());
    animationCurrent->setEndValue(QRect(0, -height, width, height));

    // 创建目标页面的动画：从当前位置(0, height)移动到(0,0)
    QPropertyAnimation *animationNext = new QPropertyAnimation(ui->page_plan, "geometry", this);
    animationNext->setDuration(300);
    animationNext->setStartValue(ui->page_plan->geometry());
    animationNext->setEndValue(QRect(0, 0, width, height));

    // 使用动画组，并行执行两个动画
    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(animationCurrent);
    animationGroup->addAnimation(animationNext);

    // 连接动画结束信号
    connect(animationGroup, &QParallelAnimationGroup::finished, this, [=]() {
        emit ApoManager::instance()->requestlogWithTime("uiA on_rBt_currentPlan animation finished");
        m_pageAnimating = false;

        // 动画结束后，将堆叠窗口的当前页面设置为目标页面
        ui->stackedWidget_Speaker->setCurrentWidget(ui->page_plan);
        emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan after setCurrentWidget: currentIndex=%1")
                        .arg(ui->stackedWidget_Speaker->currentIndex()));
        // 隐藏旧页面（实际上，在setCurrentWidget中，旧页面会被自动隐藏，但为了确保，我们可以手动隐藏）
        currentPage->hide();
        currentPage->setGeometry(0, 0, width, height);

        // 检查最终页面状态
        emit ApoManager::instance()->requestlogWithTime(QString("uiA on_rBt_currentPlan final page_plan: visible=%1 size=%2x%3 updatesEnabled=%4")
                        .arg(ui->page_plan->isVisible())
                        .arg(ui->page_plan->width()).arg(ui->page_plan->height())
                        .arg(ui->page_plan->updatesEnabled()));

        // 删除动画对象（使用deleteLater）
        animationGroup->deleteLater();

        // 延迟布局更新，避免在动画回调中再次阻塞
        scheduleLayoutUpdate();
    });

    // setupRadioButtons(FullScreenEn);
    // 开始动画
    animationGroup->start();

    // 如果之前没计划更新布局，这里再补一次请求（确保最终更新）
    scheduleLayoutUpdate();
    emit ApoManager::instance()->requestlogWithTime("uiA  on_rBt_currentPlan exit (animation running)");

    //单次定时器，800ms 后确认页面是否真的显示
    QTimer::singleShot(800, this, [this]() {
        if (m_pageAnimating) {
            emit ApoManager::instance()->requestlogWithTime("uiA CRITICAL: m_pageAnimating still true, force reset");
            m_pageAnimating = false;
        }

        if (ui->stackedWidget_Speaker->currentWidget() != ui->page_plan) {
            emit ApoManager::instance()->requestlogWithTime("uiA POST-CHECK FAIL: page_plan is not current widget after 500ms");
        }
        if (!ui->page_plan->isVisible()) {
            emit ApoManager::instance()->requestlogWithTime("uiA POST-CHECK FAIL: page_plan is not visible after 500ms");
        }
        if (ui->page_plan->size().isEmpty()) {
            emit ApoManager::instance()->requestlogWithTime("uiA POST-CHECK FAIL: page_plan size is zero after 500ms");
        }
    });


}*/


//跳转所有预设
void SpeakerSet::on_pBt_All_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_cloud);

    if(ui->lEdit_search->text().isEmpty())
    {
        // setupRadioButtons(FullScreenEn);
        // 不再直接调用 setupRadioButtons，而是异步请求
        scheduleLayoutUpdate();
    }else
    {
        UpdateSearchPlanPosition();
    }

}

//所有收藏按钮是否可用(已收藏满八个时，其他收藏按钮不可用)
void SpeakerSet::setFavPbtEn(bool en)
{
    if(!en)
    {
        for(int i = 0; i < MovieVal.AllPlanRadioList.count(); i++)
        {
            if(!MovieVal.AllPlanRadioList.at(i)->IsAdded)//未收藏
            {
                MovieVal.AllPlanRadioList.at(i)->AllpBt_fav->setEnabled(false);
            }
            // else
            // {
            //     AllPlanRadioList.at(i)->AllpBt_fav->setEnabled(true);
            // }
        }

    }else
    {
        for(int i = 0; i < MovieVal.AllPlanRadioList.count(); i++)
        {
            if(!MovieVal.AllPlanRadioList.at(i)->IsAdded)//未收藏
            {
                MovieVal.AllPlanRadioList.at(i)->AllpBt_fav->setEnabled(true);
            }
        }
    }
}


//添加方案（所有预设）（参数：名称、方案描述、是否加载文件、数据、是否被收藏、收藏id、是否立即生效音效、是否保存初始值（系统方案和导入保存）、标签1、标签2、分类(0:无，1:分类1...)、是否系统方案、是否合并（把1.9以及之前版本分为三种模式合并一起）、分享码方案ID、分享码）
void SpeakerSet::addAllPlan(QString name,QString desc,bool IsLoad, PlanVal val, bool IsAdded, int favIdx, bool TakeEffect, bool saveAsInit,QStringList Lab1,QString Lab2,int PlanTypeIdx, bool sysEn, bool ifMerge, QString ShareCodeId,QString ShareCode)
{
    //需要合并的是旧方案，其实不包含标签
    if(ifMerge)
    {
        {
            //分机型显示时
            // if(MovieVal.AllPlanRadioHash.contains(qMakePair(name,Lab1[0])))
        }
        //不分机型显示时
        if(MovieVal.AllPlanRadioHash.contains(qMakePair(name,QString(""))))
        {
            name = name + " (" + QString::number(suffix) + ")";
            suffix++;
        }
    }



    //PlanVal aVal = {};
    NewRadioBtn *radio = new NewRadioBtn(val,content_All);

    // radio->setFixedSize(CurrentBtnWight_rb,CurrentBtnHight_rb);
    radio->updateElidedText(desc,name);
    radio->setIsLoad(IsLoad);
    //radio->setCheckable(true);
    radio->IsSys = sysEn;

    if(Lab1.isEmpty())
    {
        radio->lab1->hide();
    }else
    {
        radio->lab1->show();
        radio->lab1->setText(Lab1[0]);//填写
        radio->setLabDevs(Lab1);

        //设置标签1的样式
        // QString txt = GetCurrentDeviceIdentifier();
        radio->setLab1Style(Lab1[0]);

    }

    if(Lab2.isEmpty())
    {
        radio->lab2->hide();
    }else
    {
        radio->lab2->show();
        radio->setLabel2(Lab2);
    }




    radio->setIsAddedEn(IsAdded,favIdx);
    if(IsAdded)
    {
        //加入EightFavPlan
        EightFavPlan[favIdx/*EightFavPlanIndex*/].PlanMode = 0;//0:所有预设，1：我的预设
        EightFavPlan[favIdx/*EightFavPlanIndex*/].PName = radio->lab_name->text();
        EightFavPlan[favIdx/*EightFavPlanIndex*/].IsloadEn = radio->IsLoad;
        EightFavPlan[favIdx/*EightFavPlanIndex*/].label_Devs = Lab1;
        EightFavPlan[favIdx/*EightFavPlanIndex*/].label_Scene = Lab2;

        // radio->setIsAddedEn(true,EightFavPlanIndex);

        EightFavPlanIndex++;
        qDebug("All收藏EightFavPlanIndex：%d\n",EightFavPlanIndex);
        //EightFavPlanCnt++;
        if(EightFavPlanIndex == 8)
        {
            setFavPbtEn(false);//所有收藏不可选
        }
    }




    MovieVal.AllPlanRadioList.append(radio);

    buttonGroup->addButton(radio);

    {
        //分机型显示时
        //     for (const QString& dev : Lab1)
        //     {
        //         MovieVal.AllPlanRadioHash.insert(qMakePair(name,dev),radio);//同步把按钮插入哈希表，方案名称作为键值
        //     }
    }
    {
        //不分机型显示时
        MovieVal.AllPlanRadioHash.insert(qMakePair(name, QString()), radio);//同步把按钮插入哈希表，方案名称作为键值
    }


    if(!sysEn)
    {
        MovieVal.MyPlanRadioList.append(radio);
        {
            //分机型显示时
            // for (const QString& dev : Lab1)
            // {
            //     MovieVal.MyPlanRadioHash.insert(qMakePair(name,dev),radio);//同步把按钮插入哈希表，方案名称作为键值
            // }
        }

        {
            //不分机型显示时
            MovieVal.AllPlanRadioHash.insert(qMakePair(name, QString()), radio);//同步把按钮插入哈希表，方案名称作为键值
        }

    }else
    {
        SysPlanVal.SysPlanRadioList_Mode.append(radio);
        {
            //分机型显示时
            // for (const QString& dev : Lab1)
        }
        //不分机型显示时
        QString dev = QString();
        {
            SysPlanVal.SysPlanRadioHash_Mode.insert(qMakePair(name,dev),radio);//同步把按钮插入哈希表，方案名称作为键值
        }

    }

    // if (PlanTypeIdx >= 1 && PlanTypeIdx <= 8) {
    //     PlanRadioLists[PlanTypeIdx - 1].append(radio);
    // }


    radio->installEventFilter(this);

    if(saveAsInit)
    {
        //保存系统方案和加载方案的初始值，方便初始化
        SysPlanVal_Init.insert(name,val);
        // SysPlanVal_Index++;
        SysPlanVal_Index = SysPlanVal_Init.size() - 1; // 更新索引（若需要）
        emit RealTimeSaveSysPlanValInit_S();
    }

    if(sysEn)
    {
        radio->AllpBt_edit->hide();//系统方案不可编辑，数据保密
        // radio->setChecked(false);
    }else
    {
        // radio->setChecked(true);
    }


    radio->PlanPageSel = PlanTypeIdx;
    //搜索时新建文件后显示
    if(!ui->lEdit_search->text().isEmpty())
    {
        if((group_AlltypeBtn->checkedId()!=0) && (PlanTypeIdx != group_AlltypeBtn->checkedId()))
        {
            radio->setVisible(false);
        }else
        {
            // 获取搜索文本（转换为小写进行不区分大小写匹配）
            QString searchText = ui->lEdit_search->text().toLower();
            // 检查文本是否包含搜索内容且属于当前分类
            bool shouldShow = searchText.isEmpty() || radio->lab_name->text().toLower().contains(searchText);
            if(shouldShow)
            {
                AllPlanRadioList_Temp.append(radio);
                //直接追加到搜索布局末尾，不再全量刷新
                appendRadioToLayout(radio, layout_All,
                                    row_Allrb_Temp, col_Allrb_Temp, spacer_Temp_Search,
                                    COLUMN_COUNT_rb, CurrentBtnWight_rb, CurrentBtnHight_rb);
            }else
            {
                radio->setVisible(shouldShow);
            }
        }
    }else
    {

        if((group_AlltypeBtn->checkedId()!=0) && (PlanTypeIdx != group_AlltypeBtn->checkedId()))
        {
            radio->setVisible(false);
        }



        //增量添加按钮到布局末尾
        appendRadioToLayout(radio, layout_All,
                            row_Allrb, col_Allrb, spacer_Temp_All,
                            COLUMN_COUNT_rb, CurrentBtnWight_rb, CurrentBtnHight_rb);

    }


    //是否创建即生效，重新创建系统方案时，创建不生效
    if(TakeEffect)
    {
        ui->rBt_currentPlan->setIndicatorText(radio->lab_name->text(),radio->lab2->text());
        currentPlanRadio = radio;
        currentPlanVal = radio->getAllPlanValue();

        // MovieVal.AllPlanRadioList_Dev.append(radio);
        if (PlanTypeIdx >= 1 && PlanTypeIdx <= 8) {
            PlanRadioLists[PlanTypeIdx - 1].append(radio);
        }


        radio->setChecked(true);
        emit ApoManager::instance()->requestlogWithTime("addAllPlan if(TakeEffect) ShowcurrentPlanVal");
        ShowcurrentPlanVal();
        MovieVal.C_PlanName = name;
        MovieVal.C_PlanDev = Lab1;
        MovieVal.C_PlanPageSel = PlanTypeIdx;
        ui->widget_eight->AllDisChecked();//取消所有收藏按钮的选中
    }


    //点击收藏,连接 toggled 信号：参数为 true 表示选中，false 表示取消选中
    connect(radio->AllpBt_fav, &QPushButton::toggled, [radio,this](bool checked) {
        qDebug()<<("进入收藏槽函数")<<radio->lab_name->text();
        if(checked)
        {
            //加入EightFavPlan
            EightFavPlan[EightFavPlanIndex].PlanMode = 0;//0:所有预设，1：我的预设
            EightFavPlan[EightFavPlanIndex].PName = radio->lab_name->text();
            EightFavPlan[EightFavPlanIndex].IsloadEn = radio->IsLoad;
            EightFavPlan[EightFavPlanIndex].label_Devs = radio->getLabDevs();
            EightFavPlan[EightFavPlanIndex].label_Scene = radio->lab2->text();
            radio->setIsAddedEn(true,EightFavPlanIndex);

            EightFavPlanIndex++;
            qDebug("All收藏EightFavPlanIndex：%d\n",EightFavPlanIndex);
            //EightFavPlanCnt++;
            if(EightFavPlanIndex == 8)
            {
                setFavPbtEn(false);//所有收藏不可选
            }
        }else
        {
            int idx = radio->favIdx;
            //从EightFavPlan中剔除 ,EightFavPlanIndex 为0则代表没有收藏
            if (idx >=0 && idx < 7) {
                std::move(EightFavPlan + idx + 1, // 源起始：第EightFavPlanIndex + 1项
                          EightFavPlan + 8,                     // 源结束：末尾
                          EightFavPlan + idx);    // 目标：从第EightFavPlanIndex项开始覆盖
                EightFavPlan[7] = {};

                //EightFavPlanCnt--;
                radio->setIsAddedEn(false,-1);

                EightFavPlanIndex--;

            } else if (idx == 7) {
                // 删除最后一项，无需移动
                EightFavPlanIndex--;
                //EightFavPlanCnt--;
                radio->setIsAddedEn(false,-1);
            }
            if(EightFavPlanIndex < 8)
            {
                setFavPbtEn(true);//所有收藏可选
            }

        }
        ui->widget_eight->ShowEightFavorite(true);
        if(radio->IsSys)
        {
            //保存系统预设
            emit RealTimeSaveSysPlan_S();
        }else
        {
            //保存我的预设
            emit RealTimeSaveModeVal_S();
        }


    });
    // 点击修改
    connect(radio->A_rename, &QAction::triggered, [this, radio]() {
        PlanCopy *copy = new PlanCopy(m);
        copy->ShowDev();
        copy->EditTitle(tr("修改方案"));
        copy->showCurName(radio->lab_name->text(),radio->property("fullText").toString(),radio->getLabDevs(),radio->lab2->text());


        copy->setModal(true);
        int result = copy->exec();
        if(result == QDialog::Accepted)
        {
            //qDebug("点击了确认,名称为%s\n",MyPlanName.toUtf8().constData());
            if(MyPlanName != radio->lab_name->text())
            {
                {
                    //分机型显示时
                    // MovieVal.AllPlanRadioHash.remove(qMakePair(radio->lab_name->text(),radio->lab1->text()));//同步把按钮插入哈希表，方案名称作为键值，把旧的清除
                    // MovieVal.MyPlanRadioHash.remove(qMakePair(radio->lab_name->text(),radio->lab1->text()));
                    // SysPlanVal.SysPlanRadioHash_Mode.remove(qMakePair(radio->lab_name->text(),radio->lab1->text()));
                }

                //不分机型显示时
                MovieVal.AllPlanRadioHash.remove(qMakePair(radio->lab_name->text(),QString()));//同步把按钮插入哈希表，方案名称作为键值，把旧的清除
                MovieVal.MyPlanRadioHash.remove(qMakePair(radio->lab_name->text(),QString()));
                SysPlanVal.SysPlanRadioHash_Mode.remove(qMakePair(radio->lab_name->text(),QString()));

                radio->updateElidedText(MyPlanDesc,MyPlanName);

                {
                    //分机型显示时
                    // for (const QString& dev : MyPlanLab1)
                    // {
                    //     MovieVal.AllPlanRadioHash.insert(qMakePair(MyPlanName,dev),radio);
                    //     MovieVal.MyPlanRadioHash.insert(qMakePair(MyPlanName,dev),radio);
                    //     SysPlanVal.SysPlanRadioHash_Mode.insert(qMakePair(MyPlanName,dev),radio);
                    // }
                }
                {
                    //不分机型显示时
                    MovieVal.AllPlanRadioHash.insert(qMakePair(MyPlanName,QString()),radio);
                    MovieVal.MyPlanRadioHash.insert(qMakePair(MyPlanName,QString()),radio);
                    SysPlanVal.SysPlanRadioHash_Mode.insert(qMakePair(MyPlanName,QString()),radio);
                }



                if(radio->IsAdded)//被收藏，删除收藏
                {
                    int idx = radio->favIdx;
                    EightFavPlan[idx].PName = MyPlanName;
                    ui->widget_eight->Rename(idx);
                }
                if(radio->isChecked())
                {
                    ui->rBt_currentPlan->setIndicatorText(MyPlanName,MyPlanLab2);
                }

            }
            if(MyPlanDesc != radio->property("fullText").toString())
            {
                radio->updateElidedText(MyPlanDesc,MyPlanName);
            }
            if(MyPlanLab1 != radio->getLabDevs())
            {
                radio->setLabDevs(MyPlanLab1);
            }
            if(MyPlanLab2 != radio->lab2->text())
            {
                radio->setLabel2(MyPlanLab2);//切换场景以及切换为对应场景的图标
            }

            if(radio->IsSys)
            {
                //保存系统预设
                emit RealTimeSaveSysPlan_S();
            }else
            {
                //保存我的预设
                emit RealTimeSaveModeVal_S();
            }

        }
        copy->deleteLater();


    });

    // 点击复制
    connect(radio->A_copy, &QAction::triggered, [this, radio]() {
        PlanCopy *copy = new PlanCopy(m);
        copy->ShowDev();
        copy->EditTitle(tr("复制方案"));
        copy->showCurName(radio->lab_name->text(),radio->property("fullText").toString(),radio->getLabDevs(),radio->lab2->text());


        copy->setModal(true);
        int result = copy->exec();
        if(result == QDialog::Accepted)
        {
            //qDebug("点击了确认,名称为%s\n",MyPlanName.toUtf8().constData());
            bool IsAdded = false;
            int favIdx = -1;
            int PlanTypeIdx = copy->get_cBox_PlanType_Idx_currentIndex () + 1;
            addAllPlan(MyPlanName,MyPlanDesc,0,radio->getAllPlanValue(),IsAdded,favIdx,true,false,MyPlanLab1,MyPlanLab2,PlanTypeIdx,false,false,"","");

        }
        copy->deleteLater();

        emit RealTimeSaveModeVal_S();
    });

    //点击删除按钮
    connect(radio->A_del, &QAction::triggered, [this, radio,PlanTypeIdx]() {
        //删除导入的方案
        DelReset *del = new DelReset(m);
        del->editText(0);

        del->setModal(true);
        int result = del->exec();
        if(result == QDialog::Accepted)
        {
            if(radio->IsAdded)//被收藏，删除收藏
            {
                radio->AllpBt_fav->setChecked(false);
            }


            QString Name = radio->lab_name->text();
            {
                //分机型显示时
                // QString dev = radio->lab1->text();
            }
            //不分机型显示时
            QString dev = QString();
            // 1. 使用哈希表查找目标按钮（优先主哈希表，再临时哈希表）
            NewRadioBtn* target = MovieVal.AllPlanRadioHash.value(qMakePair(Name,dev), nullptr);

            if (!target) {
                // 如果都找不到，说明按钮不在当前管理的列表中，直接返回
                return;
            }


            // 获取布局位置（删除前）
            int layoutIndex = layout_All->indexOf(radio);
            int row = -1, col = -1;
            if (layoutIndex != -1) {
                int rowSpan, colSpan;
                layout_All->getItemPosition(layoutIndex, &row, &col, &rowSpan, &colSpan);
            }


            // 从所有容器中移除（哈希表和列表）
            {
                //分机型显示时
               // for (const QString& dev : radio->getLabDevs())
            }
            //不分机型显示时
            {
                MovieVal.AllPlanRadioHash.remove(qMakePair(Name,dev));
                MovieVal.MyPlanRadioHash.remove(qMakePair(Name,dev));
            }


            // MovieVal.AllPlanRadioList_Dev.removeAll(radio);
            MovieVal.AllPlanRadioList.removeAll(radio);
            MovieVal.MyPlanRadioList.removeAll(radio);
            AllPlanRadioList_Temp.removeAll(radio);
            if (PlanTypeIdx >= 1 && PlanTypeIdx <= 8) {
                PlanRadioLists[radio->PlanPageSel-1].removeAll(radio);//0-7代表分类,这里不包含所有
            }


            //处理选中状态（如果删除的是当前选中的）
            if (radio->isChecked())
            {
                //若根据机型显示方案，则再次方案中选中第一个方案。否则在所有方案中选中第一个方案
                if (!MovieVal.AllPlanRadioList_Dev.isEmpty())
                {
                    MovieVal.AllPlanRadioList_Dev.first()->setChecked(true);
                }else if (!MovieVal.AllPlanRadioList.isEmpty())
                {
                    MovieVal.AllPlanRadioList.first()->setChecked(true);
                }
            }

            // 6. 从布局中移除并删除控件
            layout_All->removeWidget(radio);
            radio->deleteLater();   // 只调用一次

            // 7. 根据当前是否搜索模式，调用对应的增量重排函数
            // 重排必须使用与布局当前显示一致的列表（当前标签页对应的列表），
            // 否则索引错位，删除后的空位补不上
            if (ui->lEdit_search->text().isEmpty())
            {
                const int viewType = group_AlltypeBtn->checkedId();
                QList<NewRadioBtn*>* viewList = (viewType == 0)
                        ? (MovieVal.AllPlanRadioList_Dev.isEmpty()
                           ? &MovieVal.AllPlanRadioList
                           : &MovieVal.AllPlanRadioList_Dev)
                        : &PlanRadioLists[viewType - 1];
                if (row >= 0 && col >= 0) {
                    incrementalRearrangeAfterDelete(layout_All,
                                                    *viewList,   // 注意：此时 radio 已从列表中移除
                                                    row, col,    // 删除前的行列
                                                    row_Allrb, col_Allrb,
                                                    COLUMN_COUNT_rb,
                                                    CurrentBtnWight_rb, CurrentBtnHight_rb,
                                                    spacer_Temp_All);
                }
            }
            else
            {
                // 搜索模式：使用临时列表和临时行列变量
                if (row >= 0 && col >= 0) {
                    incrementalRearrangeAfterDelete(layout_All,
                                                    AllPlanRadioList_Temp,
                                                    row, col,
                                                    row_Allrb_Temp, col_Allrb_Temp,
                                                    COLUMN_COUNT_rb,
                                                    CurrentBtnWight_rb, CurrentBtnHight_rb,
                                                    spacer_Temp_Search);
                }
            }


            if(radio->IsSys)
            {
                //保存系统预设
                emit RealTimeSaveSysPlan_S();
            }else
            {
                //保存我的预设
                emit RealTimeSaveModeVal_S();
            }


        }
        del->deleteLater();


    });
    //点击移动到
    connect(radio->A_move, &QAction::triggered, [this, radio]() {
        if(!Mplan)
        {
            Mplan = new MovePlan(m);
        }
        Mplan->delAllType();
        for (int i = 0; i < 8; i++)
        {
            if ((!PlansTypes[i].Name.isEmpty()) && PlansTypes[i].en)
            {
                if(!Mplan)
                {
                    Mplan = new MovePlan(m);
                }
                Mplan->addType(PlansTypes[i].Name);
            }
        }

        int idx = radio->PlanPageSel;
        if(idx != 0)
        {
            Mplan->showType(PlansTypes[idx-1].Name);
        }
        Mplan->setModal(true);
        int result = Mplan->exec();
        if(result == QDialog::Accepted)
        {
            if(MyPlanTypeIdx+1 == idx)
            {
                return;
            }
            if(idx!=0)
            {
                PlanRadioLists[idx-1].removeAll(radio);//0-7代表分类,这里不包含所有
            }

            for (int i = 0; i < 8; i++)
            {
                if(PlansTypes[i].Name == MyPlanType)
                {
                    radio->PlanPageSel = i+1;//0代表所有，1-8代表分类
                    PlanRadioLists[i].append(radio);//0-7代表分类,这里不包含所有
                    group_AlltypeBtn->button(MyPlanTypeIdx+1)->setChecked(true); //跳转到对应分类
                    // if(ui->lEdit_search->text().isEmpty())
                    // {
                    //     scheduleLayoutUpdate();
                    // }else
                    // {
                    //     UpdateSearchPlanPosition();
                    // }
                }

            }
        }
        Mplan->close();



    });

    //点击勾选框（用于编辑时）
    connect(radio->AllpBt_check, &QRadioButton::toggled, [this, radio](bool checked) {
        qDebug("进入勾选框\n");
        if(checked)
        {
            MovieVal.AllPlanRadioList_Check.append(radio);
        }else
        {
            MovieVal.AllPlanRadioList_Check.removeAll(radio);
        }
    });

    //点击（双击进入均衡器界面）
    connect(radio, &QRadioButton::toggled, [this, radio,PlanTypeIdx,sysEn](){
        if(radio->isChecked())
        {
            if(sysEn)
            {
                ui->pBt_ExportPlan->setEnabled(false);
            }else
            {
                ui->pBt_ExportPlan->setEnabled(true);
            }

            qDebug("选中AllPlan\n");
            MovieVal.C_PlanName = radio->lab_name->text();
            MovieVal.C_PlanDev = radio->getLabDevs();

            if(radio->IsAdded)
            {
                ui->widget_eight->PlanCheckedUpdate(radio->favIdx);
            }else
            {
                ui->widget_eight->AllDisChecked();//取消所有收藏按钮的选中
            }
            ui->rBt_currentPlan->setIndicatorText(radio->lab_name->text(),radio->lab2->text());

            currentPlanRadio = radio;
            currentPlanRadio->PlanPageSel = PlanTypeIdx;
            currentPlanVal = radio->getAllPlanValue();

            LOG_INFO("[Plan] 应用方案: name={} scene={} dev={}",
                     radio->lab_name->text().toStdString(), radio->lab2->text().toStdString(),
                     radio->lab1->text().toStdString());

            if(ifShowPlan)
            {
                qDebug("点击显示 ShowcurrentPlanVal\n");
                emit ApoManager::instance()->requestlogWithTime("addAllPlan clicked ShowcurrentPlanVal");
                ShowcurrentPlanVal();
                qDebug("点击显示 ShowcurrentPlanVal 完毕\n");
            }else
            {
                ifShowPlan = true;
            }


            if(radio->IsSys)
            {
                //保存系统预设
                emit RealTimeSaveSysPlan_S();
            }else
            {
                //保存我的预设
                emit RealTimeSaveModeVal_S();
            }
        }
    });
}


//重新排列所有预设
void SpeakerSet::RearrangeAllPlanAfterDel(int idx)
{
    // 遍历所有 radio
    for (int i = idx; i < MovieVal.AllPlanRadioList/*AllPlanRadioList_Dev*/.count(); i++) {
        NewRadioBtn* currentRadio = MovieVal.AllPlanRadioList/*AllPlanRadioList_Dev*/.at(i);

        //计算当前按钮在网格中的位置,并左上角对齐
        layout_All->addWidget(currentRadio, row_Allrb, col_Allrb, 1, 1, Qt::AlignLeft | Qt::AlignTop);

        // 更新行列位置
        col_Allrb++;
        if (col_Allrb >= COLUMN_COUNT_rb) {
            col_Allrb = 0;
            row_Allrb++;
        }
    }
    // if(AllPlanRadioList.count() > 0)
    // {
    //     AllPlanRadioList.at(0)->setChecked(true);
    // }

}

//重新排列搜索后的所有预设
void SpeakerSet::RearrangeAllPlanAfterSearch(int idx)
{
    // 遍历所有 radio
    for (int i = idx; i < AllPlanRadioList_Temp.count(); i++) {
        NewRadioBtn* currentRadio = AllPlanRadioList_Temp.at(i);

        //计算当前按钮在网格中的位置,并左上角对齐
        layout_All->addWidget(currentRadio, row_Allrb_Temp, col_Allrb_Temp, 1, 1, Qt::AlignLeft | Qt::AlignTop);

        // 更新行列位置
        col_Allrb_Temp++;
        if (col_Allrb_Temp >= COLUMN_COUNT_rb) {
            col_Allrb_Temp = 0;
            row_Allrb_Temp++;
        }
    }
    // if(AllPlanRadioList_Temp.count() > 0)
    // {
    //     AllPlanRadioList_Temp.at(0)->setChecked(true);
    // }
}

//点击保存按钮(所有预设的方案则重命名，我的预设则保存覆盖原数据)
void SpeakerSet::on_pBt_save_clicked()
{
    if (currentPlanRadio == nullptr) {
        return; // 防御：方案未就绪（设备未连接/未加载）时跳过保存
    }
    currentPlanRadio->updateAllPlanValue(currentPlanVal);
    if(currentPlanRadio->IsSys)
    {
        //保存系统预设
        emit RealTimeSaveSysPlan_S();
    }else
    {
        emit RealTimeSaveModeVal_S();
    }
}


void SpeakerSet::PlanSave()
{
    on_pBt_save_clicked();


}

//重置
void SpeakerSet::ResetValue(int type)
{
    DelReset *reset = new DelReset(m);
    reset->editText(type+1);
    reset->setModal(true);
    int result = reset->exec();
    if(result == QDialog::Accepted)
    {
        QString name = currentPlanRadio->lab_name->text();
        {
            //分机型显示时
           // QString dev = currentPlanRadio->lab1->text();
        }

        //不分机型显示时
        QString dev = QString();



        switch(type)
        {
        case 0:
            //重置均衡器
            if(currentPlanVal.DataVisibleEn)
            {
                //系统和导入方案存在初始值,系统初始值是不会包含二创内容的，导入方案不含二创内容
                bool find = false;
                if(currentPlanRadio->IsSys || currentPlanRadio->IsLoad)
                {

                    NewRadioBtn* btn = SysPlanVal.SysPlanRadioHash_Mode.value(qMakePair(name, dev),nullptr);
                    if (btn)
                    {
                        find = true;
                        emit ApoManager::instance()->requestlogWithTime("AllPlanRadioHash isTarget true");
                        PlanVal val = SysPlanVal_Init.value(name);

                        memcpy(currentPlanVal.freqVal, val.freqVal, sizeof(val.freqVal));
                        memcpy(currentPlanVal.eqVal, val.eqVal, sizeof(val.eqVal));
                        memcpy(currentPlanVal.qVal, val.qVal, sizeof(val.qVal));
                        memcpy(currentPlanVal.filterVal, val.filterVal, sizeof(val.filterVal));

                        //二创方案内容修改为以下部分
                        std::fill(std::begin(currentPlanVal.freqVal_deriv), std::end(currentPlanVal.freqVal_deriv), 20000.0);
                        std::fill(std::begin(currentPlanVal.eqVal_deriv),   std::end(currentPlanVal.eqVal_deriv),   0.0);
                        std::fill(std::begin(currentPlanVal.qVal_deriv),    std::end(currentPlanVal.qVal_deriv),    0.5);
                        std::fill(std::begin(currentPlanVal.filterVal_deriv),    std::end(currentPlanVal.filterVal_deriv),    0);
                    }
                }
                if (!find)
                {
                    if (!currentPlanVal.ParentPlanName.isEmpty())
                    {
                        //二创方案，只改
                        double values[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
                        memcpy(currentPlanVal.freqVal_deriv, values, sizeof(values));
                        double Eqvalues[10] = {0,0,0,0,0,0,0,0,0,0};
                        memcpy(currentPlanVal.eqVal_deriv, Eqvalues, sizeof(Eqvalues));
                        double Qvalues[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
                        memcpy(currentPlanVal.qVal_deriv, Qvalues, sizeof(Qvalues));
                        std::fill(std::begin(currentPlanVal.filterVal_deriv),    std::end(currentPlanVal.filterVal_deriv),    0);
                    } else {
                        //不是系统和加载方案，且不是二创方案
                        double values[10] = {30,60,120,250,500,1000,2000,4000,8000,16000};
                        memcpy(currentPlanVal.freqVal, values, sizeof(values));
                        double Eqvalues[10] = {0,0,0,0,0,0,0,0,0,0};
                        memcpy(currentPlanVal.eqVal, Eqvalues, sizeof(Eqvalues));
                        double Qvalues[10] = {0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5};
                        memcpy(currentPlanVal.qVal, Qvalues, sizeof(Qvalues));
                        std::fill(std::begin(currentPlanVal.filterVal),    std::end(currentPlanVal.filterVal),    0);

                        //二创方案内容修改为以下部分
                        std::fill(std::begin(currentPlanVal.freqVal_deriv), std::end(currentPlanVal.freqVal_deriv), 20000.0);
                        std::fill(std::begin(currentPlanVal.eqVal_deriv),   std::end(currentPlanVal.eqVal_deriv),   0.0);
                        std::fill(std::begin(currentPlanVal.qVal_deriv),    std::end(currentPlanVal.qVal_deriv),    0.5);
                        std::fill(std::begin(currentPlanVal.filterVal_deriv),    std::end(currentPlanVal.filterVal_deriv),    0);
                    }
                }


            }

            break;
        case 1:
            //重置算法
            //额外EQ
            memset(currentPlanVal.ExtraEq,0,sizeof(currentPlanVal.ExtraEq));
            //低音
            currentPlanVal.lowVal = 0;
            //增益
            currentPlanVal.GainVal = 0;
            //灵晞算法
            currentPlanVal.drcVal = 0;

            break;
        case 2:
            //重置空间
            currentPlanVal.spaceVal = 0;
            currentPlanVal.spaceReverb = 0;
            currentPlanVal.spaceSize = 0;//0:小，1：中，2：大
            break;
        default:
            break;
        }

        ShowcurrentPlanVal();
        on_pBt_save_clicked();
        emit PlanReset_S();

    }
    reset->deleteLater();
}

//创建新方案
void SpeakerSet::CreateNewMyPlan(QString txt)
{
    PlanCopy *copy = new PlanCopy(m);
    copy->ShowDev();
    copy->EditTitle(txt);
    if(txt == "添加二创方案")
    {
        copy->showSysName(currentPlanRadio->lab_name->text());
    }
    /// WBLIU: 先更新分类标签信息
    QStringList t_planTypes;
    for (int i = 0; i < 8; i++)
    {
        if ((!PlansTypes[i].Name.isEmpty()) && PlansTypes[i].en)
        {
            t_planTypes << PlansTypes[i].Name;
        }
    }
    copy->updateUI_cBox_PlanType_Idx(t_planTypes);  // 更新分类标签信息

    copy->setModal(true);
    int result = copy->exec();
    if(result == QDialog::Accepted)
    {
        //qDebug("点击了确认,名称为%s\n",MyPlanName.toUtf8().constData());
        bool IsAdded = false;
        int favIdx = -1;
        if(txt == "添加二创方案")
        {
            PlanVal temp;
            temp.DataVisibleEn = true;
            temp.ParentPlanName = currentPlanRadio->lab_name->text();
            temp.AlgoOpenEn = false;
            temp.spaceOpenEn = false;
            temp.eqOpenEn = true;
            memset(temp.ExtraEq,0,sizeof(temp.ExtraEq));
            temp.lowVal = 0;
            temp.drcVal = 0;
            temp.GainVal = 0;
            temp.spaceVal = 0;
            temp.spaceReverb = 0;
            temp.spaceSize = 0;
            memcpy(temp.freqVal,currentPlanVal.freqVal,sizeof(currentPlanVal.freqVal));
            memcpy(temp.eqVal,currentPlanVal.eqVal,sizeof(currentPlanVal.eqVal));
            memcpy(temp.qVal,currentPlanVal.qVal,sizeof(currentPlanVal.qVal));
            memcpy(temp.filterVal,currentPlanVal.filterVal,sizeof(currentPlanVal.filterVal));
            //二创
            const double src[10] = {30.0, 60.0, 120.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 16000.0};
            memcpy(temp.freqVal_deriv, src, sizeof(temp.freqVal_deriv));
            memset(temp.eqVal_deriv,0,sizeof(temp.eqVal_deriv));
            std::fill(std::begin(temp.qVal_deriv), std::end(temp.qVal_deriv), 0.5);
            std::fill(std::begin(temp.filterVal_deriv),    std::end(temp.filterVal_deriv),    0);

            int PlanTypeIdx = copy->get_cBox_PlanType_Idx_currentIndex () + 1;
            addAllPlan(MyPlanName,MyPlanDesc,0,temp,IsAdded,favIdx,true,false,MyPlanLab1,MyPlanLab2,PlanTypeIdx,false,false,"",""); //copy->get_cBox_PlanType_Idx_currentIndex () + 1 <== 0 为所有预设
        }else
        {
            int PlanTypeIdx = copy->get_cBox_PlanType_Idx_currentIndex () + 1;

            addAllPlan(MyPlanName,
                       MyPlanDesc,
                       0,
                       {true,
                        "",
                        false,
                        false,
                        true,
                        false,
                        {0, 0, 0, 0, 0, 0, 0},
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        {30, 60, 120, 250, 500, 1000, 2000, 4000, 8000, 16000},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5},
                        {0,0,0,0,0,0,0,0,0,0},
                        {20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000},
                        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                        {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5},
                        {0,0,0,0,0,0,0,0,0,0}
                       },
                       IsAdded,
                       favIdx,
                       true,
                       false,
                       MyPlanLab1,
                       MyPlanLab2,
                       PlanTypeIdx,
                       false,
                       false,
                       "",
                       "");
        }

        // emit FavEQpageChange(currentPlanRadio->getAllPlanValue().DataVisibleEn);//跳转到新创建方案的均衡器页面 /// WBLIU:改为不跳转
        try {
            group_AlltypeBtn->button(copy->get_cBox_PlanType_Idx_currentIndex() + 1)->setChecked(true); // /// WBLIU:跳转到对应分类
        } catch (...) {
            qDebug() << "跳转到指定分类失败" << __FILE__ << __FUNCTION__ <<__LINE__;
        }

        emit RealTimeSaveModeVal_S();
        ui->pBt_ExportPlan->setEnabled(true);

        //使用零延迟定时器，让函数在当前事件循环所有任务（包括布局更新）都完成后再执行。否则ensureWidgetVisible无效果
        QTimer::singleShot(0, this, [this](){
            qDebug()<<"name:"<<currentPlanRadio->lab_name->text();
            scrollArea_All->ensureWidgetVisible(currentPlanRadio);
        });


    }
    copy->deleteLater();
}

void SpeakerSet::CreateDerivPlanSlot()
{
    CreateNewMyPlan("添加二创方案");
}

//新建方案
void SpeakerSet::on_pBt_NewPlan_clicked()
{
    CreateNewMyPlan("新建方案");
}

//搜索(只要包含arg1就显示，不区分大小写)
void SpeakerSet::on_lEdit_search_textChanged(const QString &arg1)
{
    // 获取搜索文本（转换为小写进行不区分大小写匹配）
    // QString searchText = arg1.toLower();

    UpdateSearchPlanPosition();

}
//
void SpeakerSet::UpdateSearchPlanPosition()
{
    const QString searchText = ui->lEdit_search->text().toLower();
    const int idx = group_AlltypeBtn->checkedId();

    // 重置状态
    row_Allrb_Temp = 0;
    col_Allrb_Temp = 0;
    AllPlanRadioList_Temp.clear();

    // 遍历所有按钮
    for (NewRadioBtn* radio : qAsConst(MovieVal.AllPlanRadioList/*AllPlanRadioList_Dev*/)) {
        if (!radio) continue;

        const QString radioText = radio->lab_name->text().toLower();
        const bool textMatch = searchText.isEmpty() || radioText.contains(searchText);
        const bool categoryMatch = (idx == 0) || (radio->PlanPageSel == idx);

        if (textMatch && categoryMatch) {
            AllPlanRadioList_Temp.append(radio);
        } else {
            radio->setVisible(false);  // 不匹配则隐藏
        }
    }

    // 重新布局
    setupRadioButtons(FullScreenEn);
}
/*//使用AllPlanRadioHash无顺序
void SpeakerSet::UpdateSearchPlanPosition()
{
    QString searchText = ui->lEdit_search->text().toLower();
    int idx = group_AlltypeBtn->checkedId();

    // 重置临时列表
    row_Allrb_Temp = 0;
    col_Allrb_Temp = 0;
    AllPlanRadioList_Temp.clear();

    // 遍历哈希表中的所有按钮
    for (NewRadioBtn* radio : MovieVal.AllPlanRadioHash.values()) {
        if (!radio) continue;

        QString radioText = radio->lab_name->text().toLower();
        bool textMatch = searchText.isEmpty() || radioText.contains(searchText);//搜索
        bool categoryMatch = (idx == 0) || (radio->PlanPageSel == idx);//分类

        if (textMatch && categoryMatch) {
            AllPlanRadioList_Temp.append(radio);
        } else {
            radio->setVisible(false);
        }
    }

    // 重新布局
    setupRadioButtons(FullScreenEn);
}*/
//点击收藏跳转到EQ界面
void SpeakerSet:: FavShowEQpageChange()
{

    emit FavEQpageChange(currentPlanRadio->getAllPlanValue().DataVisibleEn);
    // if(currentPlanRadio->getAllPlanValue().DataVisibleEn)
    // {

    // }else
    // {
    //     ui->stackedWidget_Speaker->setCurrentWidget(ui->page_System);//数据保密，不可显示,用户可二次创作
    // }


    updateSize();

}

//修改为在eq界面
void SpeakerSet::ShowcurrentPlanVal()
{
    emit SetApoVal();
}
//SelDev_DeviceName转换为对应的标签1
QString SpeakerSet::GetCurrentDeviceIdentifier()
{
    if (SelDev_DeviceName.contains("T10", Qt::CaseInsensitive)) {
        if (SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive))
            return QStringLiteral("T10无线");
        else
            return QStringLiteral("T10有线");
    }
    if (SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive) && ((SelDev_PID == 0xF016)|| (SelDev_PID == 0xF017)) )
        return QStringLiteral("K03S超竞版");
    if (SelDev_DeviceName.contains("K06S", Qt::CaseInsensitive))
        return QStringLiteral("K06S");
    if (SelDev_DeviceName.contains("T7 GT", Qt::CaseInsensitive))
        return QStringLiteral("T7 GT");
    if (SelDev_DeviceName.contains("T7", Qt::CaseInsensitive))
        return QStringLiteral("T7");
    if (SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive))
        return QStringLiteral("K03S");
    if (SelDev_DeviceName.contains("S21", Qt::CaseInsensitive))
        return QStringLiteral("S21无线智充版");

    return QString();  // 未识别设备返回空字符串
}
bool SpeakerSet::isDeviceMatchingPlanDev(const QString& planDev)
{
    {
        //分机型显示时
        // if (planDev.contains("T10无线", Qt::CaseInsensitive)) {
        //     return SelDev_DeviceName.contains("T10", Qt::CaseInsensitive) &&
        //            SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive);
        // }
        // if (planDev.contains("T10有线", Qt::CaseInsensitive)) {
        //     return SelDev_DeviceName.contains("T10", Qt::CaseInsensitive) &&
        //            !SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive);
        // }
        // if (planDev.contains("K03S超竞版", Qt::CaseInsensitive)) {
        //     return SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive) &&
        //            ((SelDev_PID == 0xF016)|| (SelDev_PID == 0xF017));
        // }
        // if (planDev.contains("K06S", Qt::CaseInsensitive)) {
        //     return SelDev_DeviceName.contains("K06S", Qt::CaseInsensitive);
        // }
        // if (planDev.contains("T7", Qt::CaseInsensitive)) {
        //     return SelDev_DeviceName.contains("T7", Qt::CaseInsensitive);
        // }
        // if (planDev.contains("K03S", Qt::CaseInsensitive)) {
        //     // 必须放在“超竞版”判断之后，避免误判
        //     return SelDev_DeviceName.contains("K03S", Qt::CaseInsensitive) &&
        //            (SelDev_PID != 0xF016) && (SelDev_PID != 0xF017);
        // }
        // return false;
    }
    //不分机型显示时
    return true;

}
void SpeakerSet::selectFirstPresetForDevice(const QString& dev)
{
    // 遍历所有方案按钮，找到第一个 lab2 文本与设备标识匹配的项
    for (NewRadioBtn* btn : MovieVal.AllPlanRadioList) {
        if (QString::compare(btn->lab1->text(), dev, Qt::CaseInsensitive) == 0) {
            btn->setChecked(true);
            ui->rBt_currentPlan->setIndicatorText(btn->lab_name->text(), btn->lab2->text());
            return;
        }
    }

    // 未找到该设备的方案：清空指示器，且不选中任何按钮（原逻辑）
    ui->rBt_currentPlan->setIndicatorText(QString(), QString());
}
//整合和兼容新旧版本方案，仅当前设备的方案显示
void SpeakerSet::IntegratePlansAndCompatible()
{
    //仅第一次打开软件时执行，解决新旧版本兼容
    if(!RetrievePlan)
    {
        for (int i = 0; i < 8; ++i) {
            PlanRadioLists[i].clear();
        }

        ui->lEdit_search->setText("");

        // AllPlanRadioList = TempVal.AllPlanRadioList ;
        // AllPlanRadioHash =  TempVal.AllPlanRadioHash;
        //重新布局
        col_Myrb = 0;
        row_Myrb = 0;
        while (QLayoutItem *item = layout_All->takeAt(0)) {
            if (QWidget *widget = item->widget()) {
                widget->hide(); // 隐藏，如果后续还要使用
            }
            delete item;
        }



        emit ApoManager::instance()->requestlogWithTime(QString("TempVal.C_PlanPageSel:%1").arg(MovieVal.C_PlanPageSel));

        QString name = MovieVal.C_PlanName;
        {
            //分机型显示时
            // for (const QString& dev : MovieVal.C_PlanDev)
        }
        //不分机型显示时
        QString dev = QString();
        {

            QString currentDev = GetCurrentDeviceIdentifier();

            if(!MovieVal.C_PlanName.isEmpty() && isDeviceMatchingPlanDev(dev))
            {
                //设置选中状态，不使用for循环，减少冗余
                NewRadioBtn* target = MovieVal.AllPlanRadioHash.value(qMakePair(name, dev), nullptr);
                if (target) {
                    target->setChecked(true);
                    ui->rBt_currentPlan->setIndicatorText(MovieVal.C_PlanName,target->lab2->text());
                    // break;//分机型显示时
                }else
                {
                    ui->rBt_currentPlan->setIndicatorText("","");
                    // selectFirstPresetForDevice(currentDev);
                }
            }else
            {
                ui->rBt_currentPlan->setIndicatorText("","");
                // selectFirstPresetForDevice(currentDev);
            }
        }

       // 清空 EightFavPlan 数组
        for (int i = 0; i < 8; ++i) {
            EightFavPlan[i] = {};  // 安全、清晰、兼容 Qt 类型
        }
        EightFavPlanIndex = 0;
        setFavPbtEn(true);
         //是否机型显示，收藏显示
        for(int j = 0; j < MovieVal.AllPlanRadioList.count(); j++)
        {
            bool shouldShow = true;
            {
                //根据机型显示，判断该按钮是否应该显示
                // bool shouldShow = false;
                // if(!SelDev_DeviceName.isEmpty())
                // {
                //     for (const QString& dev : MovieVal.AllPlanRadioList.at(j)->getLabDevs())
                //     {

                //         shouldShow = isDeviceMatchingPlanDev(dev);
                //         if(shouldShow)
                //         {
                //             break;
                //         }
                //     }
                // }
            }
            if(shouldShow)
            {
                //分机型显示时
                {
                   // MovieVal.AllPlanRadioList_Dev.append(MovieVal.AllPlanRadioList.at(j));
                }


                int PlanTypeIdx = MovieVal.AllPlanRadioList.at(j)->PlanPageSel;
                if (PlanTypeIdx >= 1 && PlanTypeIdx <= 8) {
                    PlanRadioLists[PlanTypeIdx - 1].append(MovieVal.AllPlanRadioList.at(j));
                }

            }

            if(MovieVal.AllPlanRadioList.at(j)->IsAdded)
            {

                if(shouldShow)
                {
                    QString name = MovieVal.AllPlanRadioList.at(j)->lab_name->text();
                    emit ApoManager::instance()->requestlogWithTime(QString("系统收藏：%1,名称：%2，位置：%3").arg(j).arg(name).arg(MovieVal.AllPlanRadioList.at(j)->favIdx));


                    if(EightFavPlan[MovieVal.AllPlanRadioList.at(j)->favIdx].PName.isEmpty())
                    {
                        MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->blockSignals(true);
                        MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->setChecked(true);
                        MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->blockSignals(false);
                        EightFavPlan[MovieVal.AllPlanRadioList.at(j)->favIdx].PlanMode = 0;//0:所有预设，1：我的预设
                        EightFavPlan[MovieVal.AllPlanRadioList.at(j)->favIdx].PName = name;
                        EightFavPlan[MovieVal.AllPlanRadioList.at(j)->favIdx].IsloadEn = MovieVal.AllPlanRadioList.at(j)->IsLoad;
                        EightFavPlan[MovieVal.AllPlanRadioList.at(j)->favIdx].label_Devs = MovieVal.AllPlanRadioList.at(j)->getLabDevs();
                        EightFavPlan[MovieVal.AllPlanRadioList.at(j)->favIdx].label_Scene = MovieVal.AllPlanRadioList.at(j)->lab2->text();
                        EightFavPlanIndex++;
                        if(EightFavPlanIndex == 8)
                        {
                            setFavPbtEn(false);//所有收藏按钮不可用
                        }
                    }else
                    {
                        for(int x = 0; x < 8; x++)
                        {
                            if(EightFavPlan[x].PName.isEmpty())
                            {
                                MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->blockSignals(true);
                                MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->setChecked(true);
                                MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->blockSignals(false);
                                MovieVal.AllPlanRadioList.at(j)->favIdx = x;
                                EightFavPlan[x].PlanMode = 0;//0:所有预设，1：我的预设
                                EightFavPlan[x].PName = name;
                                EightFavPlan[x].IsloadEn = MovieVal.AllPlanRadioList.at(j)->IsLoad;
                                EightFavPlan[x].label_Devs = MovieVal.AllPlanRadioList.at(j)->getLabDevs();
                                EightFavPlan[x].label_Scene = MovieVal.AllPlanRadioList.at(j)->lab2->text();

                                //if(x >= EightFavPlanIndex)
                                {
                                    EightFavPlanIndex++;
                                    if(EightFavPlanIndex == 8)
                                    {
                                        setFavPbtEn(false);//所有收藏按钮不可用
                                    }
                                }
                            }
                            break;
                        }
                    }
                }


            }else
            {
                MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->blockSignals(true);
                MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->setChecked(false);
                MovieVal.AllPlanRadioList.at(j)->AllpBt_fav->blockSignals(false);
            }
        }


        // 压缩 EightFavPlan，消除空洞，让非空项连续放置在前部
        int dest = 0;
        for (int src = 0; src < 8; ++src) {
            if (!EightFavPlan[src].PName.isEmpty()) {
                if (src != dest) {
                    // 移动收藏项
                    EightFavPlan[dest] = EightFavPlan[src];
                    EightFavPlan[src] = FavPlan(); // 清空原位置

                    // 更新对应按钮的 favIdx
                    FavPlan &moved = EightFavPlan[dest];
                    // 所有预设
                    for (int k = 0; k < MovieVal.AllPlanRadioList.count(); ++k) {
                        NewRadioBtn *btn = MovieVal.AllPlanRadioList.at(k);
                        if (btn->lab_name->text() == moved.PName &&
                            btn->IsAdded) {
                            btn->favIdx = dest;
                            break;
                        }
                    }

                }
                ++dest;
            }
        }
        EightFavPlanIndex = dest;
        // 根据实际收藏数量，更新收藏按钮的使能状态
        setFavPbtEn(EightFavPlanIndex < 8);


        RetrievePlan = true;

        scheduleLayoutUpdate();
    }


}
//根据本地文件显示预设库界面
void SpeakerSet::SwitchMode()
{
    //IsSwitch = true;//切换，若切换过去，模式开关为关闭状态，则音效保持之前不变
    //qDebug("SwitchMode mode:%d\n",mode);


    emit ApoManager::instance()->requestlogWithTime(QString("EightFavPlanIndex:%1").arg(EightFavPlanIndex));
    ui->widget_eight->ShowEightFavorite(true);

    scrollArea_All->ensureWidgetVisible(currentPlanRadio);

    //eq界面设置界面


    // //跳转到EQ界面,并设置EQ数据是否可见
    // emit EQpageChange(currentPlanVal.DataVisibleEn);
    // // if(currentPlanVal.DataVisibleEn)
    // // {
    // //     emit EQpageChange(currentPlanVal.DataVisibleEn);
    // // }else
    // // {
    // //     ui->stackedWidget_Speaker->setCurrentWidget(ui->page_System);//数据保密，不可显示,用户可二次创作
    // // }


}


//导入方案
void SpeakerSet::on_pBt_LoadPlan_clicked()
{
    cl_dialog_loadPlan_->cl_lineEdit_->clear();
    cl_dialog_loadPlan_->cl_error_icon_->hide();
    cl_dialog_loadPlan_->cl_error_message_->hide();

    int ret_result = cl_dialog_loadPlan_->exec();
    if (ret_result == QDialog::Accepted) {
        // 检查返回的状态码，新增方案前，确保文件已下载
        if (cl_dialog_loadPlan_->resp.code == "unauthorized") {
            // 显示一下错误信息:未登录
            g_shareCodeCopyHint->setText(tr("%1").arg(cl_dialog_loadPlan_->ret_info));
            g_shareCodeCopyHint->show();
            QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
            return;
        }

        // 文件已下载完毕 且 耳机机型校验通过，检查一下文件是否存在
        QFileInfo fileInfo(cl_dialog_loadPlan_->savePath);
        if (fileInfo.exists()) {
            readExportPlanIni(cl_dialog_loadPlan_->savePath,true);
            on_pBt_All_clicked();

            // 读取完成后删除文件
            if (QFile::remove(cl_dialog_loadPlan_->savePath)) {
                qDebug() << "文件删除成功:" << cl_dialog_loadPlan_->savePath;
            } else {
                qDebug() << "文件删除失败，可能被占用或权限不足";
            }
        }

        // 显示正常提示信息
        g_shareCodeCopyHint->setText(tr("%1").arg (cl_dialog_loadPlan_->ret_info));
        g_shareCodeCopyHint->show();
        QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
    }


    /// wbliu:旧版导入
    // QFileDialog fdialog; //.sdt
    // fdialog.setNameFilter(QString(
    //     "ini file(*.ini)")); //在win中过滤显示.ini文件，而且保存的文件的后缀都为.sdt；在Ubuntu中只是过滤显示文件
    // fdialog.setViewMode(QFileDialog::Detail);
    // fdialog.setAcceptMode(
    //     QFileDialog::AcceptOpen); /*设置操作模式定义对话框是用于打开还是保存文件，默认情况下，此属性设置为AcceptOpen，标题为打开*/
    // fdialog.setDirectory(QString("/")); //因为"/"在Ubuntu中代表的根目录
    // fdialog.setWindowTitle("导入方案");
    // QString filePath;
    // if (fdialog.exec() == QDialog::Accepted) {
    //     filePath = fdialog.selectedFiles()[0];
    //     // QString fileName = fdialog.selectedFiles().first();
    //     //方案加入到所有预设界面
    //     QFileInfo fileInfo(filePath);
    //     if (fileInfo.exists()) {
    //         //QString fileName = fileInfo.baseName();
    //         readExportPlanIni(filePath);
    //         on_pBt_All_clicked();
    //     }
    //     // bool IsAdded[3] = {false,false,false};
    //     // int favIdx[3] = {-1,-1,-1};
    //     // addAllPlan(fileName,1,{},IsAdded,favIdx);
    // }

}
//导出方案  on_pBt_LoadPlan_clickedy cc
void SpeakerSet::on_pBt_ExportPlan_clicked()
{
    // 居中于父窗口 m
    if (m) {
        const QRect parentRect = m->geometry();
        const QSize dlgSize = eqHiddenDlg->sizeHint(); // 或 size()
        int x = parentRect.x() + (parentRect.width() - dlgSize.width()) / 2;
        int y = parentRect.y() + (parentRect.height() - dlgSize.height()) / 2;
        eqHiddenDlg->move(x, y);
    }
    int res = eqHiddenDlg->exec();
    if(res == QDialog::Accepted)
    {

        // token 由 ApiClient/AuthStore 自动附加（2026-08-04 起不再手动读 globalSettings）

        // // 2 秒后自动隐藏
        // g_shareCodeCopyHint->setText (tr("方案分享码已复制"));
        // g_shareCodeCopyHint->show();
        // QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);

        QString upload_file_path = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/upload_tmp.ini";  // 上传文件路径,上传完毕后删除

        // QString filePath = "C:/Users/28215/Desktop/T10三角洲音效焚决BetaV2.0.ini";

        // 确保目录存在
        QDir dir = QFileInfo(upload_file_path).absoluteDir();
        if (!dir.exists()) {
            dir.mkpath(".");
        }



        writeExportPlanIni(upload_file_path, eqHiddenDlg->GetEqShowEn());
        qDebug() << "File to save:" << upload_file_path;

        // 服务器不存在可分享文件
        if (currentPlanRadio->ShareCodeId.isEmpty()) {
            /// 上传文件，然后获取分享码
            /// 上传文件指定ini文件

            // // 将当前配置信息写入文件
            QFile * file = new QFile(upload_file_path);
            if (!file->open(QIODevice::ReadOnly)) {
                emit ApoManager::instance()->requestlogWithTime(
                    QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg("file open failed"));
                delete file;
                return;
            }


            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QVariant("form-data; name=\"file\"; filename=\""
                                        + QFileInfo(upload_file_path).fileName() + "\""));
            filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                               QVariant("application/octet-stream")); // INI 文件的 MIME 类型

            filePart.setBodyDevice(file);
            file->setParent(multiPart);
            multiPart->append(filePart);

            QNetworkReply *reply = HttpClient::instance().upload("/user/uploads", multiPart, RequestOptions{}.withTag("user"));
            multiPart->setParent(reply);

            QObject::connect(reply, &QNetworkReply::finished, this, [=]() mutable {
                reply->deleteLater();

                file->close();
                file->deleteLater();

                // 不管上传成不成功，都删除文件，下次重新打开再写
                if (QFile::remove(upload_file_path)) {
                    qDebug() << "本地 upload_tmp.ini 文件删除成功:" << upload_file_path;
                } else {
                    qDebug() << "本地 upload_tmp.ini 文件删除失败:" << upload_file_path;
                }

                if (reply->error() != QNetworkReply::NoError) {
                    emit ApoManager::instance()->requestlogWithTime(QString("%1 %2 netWorkReply error:%3")
                                                                        .arg(__FUNCTION__)
                                                                        .arg(__LINE__)
                                                                        .arg(reply->errorString()));

                    return;
                }

                if (reply->error() == QNetworkReply::NoError) {
                    /// 读取响应数据
                    QByteArray responseData = reply->readAll();
                    qDebug() << "文件上传请求 回显原始数据:" << QString::fromUtf8(responseData);

                    /// 解析 JSON
                    QJsonParseError parseError;
                    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
                    if (parseError.error != QJsonParseError::NoError) {
                        QString errorMsg = "JSON解析错误: " + parseError.errorString();
                        emit ApoManager::instance()->requestlogWithTime(
                            QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg(errorMsg));

                        return;
                    }

                    /// 检查业务状态码
                    QJsonObject rootObj = jsonDoc.object();
                    QString code = rootObj["code"].toString();
                    if (code != "success") {
                        QString message = rootObj["message"].toString();
                        QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
                        emit ApoManager::instance()->requestlogWithTime(QString("上传文件 %1").arg(errorMsg));
                        return;
                    }

                    DeSheng::FileUploadsResponse ret_info; /// 文件上传回显信息
                    if (DeSheng::ProcessFileUploadsResult(ret_info, jsonDoc)) {
                        /// 文件上传成功后，调用 创建方案库分享码 接口
                        /// 再将返回的 分享码 复制到 粘贴板

                        DeSheng::CreateShareCodeRequest req;
                        req.url = ret_info.data.url;                    // 文件上传回显信息中的 URL
                        req.title = currentPlanRadio->lab_name->text(); // 方案标题(方案名称)
                        req.description = currentPlanRadio->property("fullText").toString(); // 方案描述
                        req.device_name = SelDev_DeviceName;            //设备名称
                        req.device_type = DevType;                      //设备类型

                        QJsonObject body = CreateShareCodeRequestToJson(req);
                        QJsonDocument doc(body);

                        QByteArray bodyData = doc.toJson(QJsonDocument::Compact);
                        qDebug() << "请求体:" << QString::fromUtf8(bodyData);

                        // 统一走新栈 ApiClient（token 由 AuthStore 自动附加，路由按 tag "schemes"）
                        QNetworkReply *createShareCode_reply
                            = HttpClient::instance().post("/schemes/share-code",
                                RequestOptions{}.withBody(bodyData).withTag("schemes"));
                        connect(createShareCode_reply, &QNetworkReply::finished, [=]() {
                            if (createShareCode_reply->error() != QNetworkReply::NoError) {
                                // 网络错误属内部细节，走日志不展示给用户
                                LOG_WARN("创建分享码请求失败: {}", createShareCode_reply->errorString().toStdString());
                                if (g_shareCodeCopyHint) {
                                    g_shareCodeCopyHint->setText(tr("创建分享码失败"));
                                    g_shareCodeCopyHint->show();
                                    QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                                }
                                return;
                            }

                            DeSheng::CreateShareCodeResponse resp;
                            QByteArray retArray = createShareCode_reply->readAll();
                            QJsonDocument doc = QJsonDocument::fromJson(retArray);

                            qDebug() << "回显信息：" << retArray;

                            if (ProcessCreateShareCodeResult(resp, doc)) {
                                // ProcessCreateShareCodeResult 只校验响应结构，业务是否成功须再校验 code
                                if (resp.code != "success" || resp.data.share_code.isEmpty()) {
                                    // 业务失败原因（code/message）属内部细节，走日志；用户只看到简洁失败提示
                                    LOG_WARN("创建分享码业务失败: code={} message={}",
                                             resp.code.toStdString(), resp.message.toStdString());
                                    if (g_shareCodeCopyHint) {
                                        g_shareCodeCopyHint->setText(tr("创建分享码失败"));
                                        g_shareCodeCopyHint->show();
                                        QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                                    }
                                    return;
                                }

                                // 创建分享码成功
                                QString shareCode = resp.data.share_code;
                                int64_t ShareCodeId = resp.data.id;

                                qDebug() << "创建分享码成功" << resp.data.title << "创建分享码后文件url："<<resp.data.url;

                                // 更新当前选中项的相关信息

                                currentPlanRadio->ShareCodeId = QString::number(ShareCodeId);
                                currentPlanRadio->ShareCode = shareCode;

                                // 拷贝到粘贴板
                                QClipboard *clipboard = QApplication::clipboard();

                                // 拼接成指定格式: 名称+机型+场景+ys分享码
                                QStringList devs = currentPlanRadio->getLabDevs();
                                QString devStr = devs.join("+");
                                QString target_text = currentPlanRadio->lab_name->text()
                                                      + "+" + devStr
                                                      + "+" + currentPlanRadio->lab2->text()
                                                      + "+ys" + shareCode;
                                clipboard->setText(target_text);

                                // 提示用户
                                // 2 秒后自动隐藏
                                g_shareCodeCopyHint->setText(tr("方案分享码已复制"));
                                g_shareCodeCopyHint->show();
                                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                                // qDebug() << "分享码已复制到粘贴板:" << shareCode;
                            } else {
                                // 响应解析失败（结构异常）属内部细节，走日志；用户只看到简洁失败提示
                                LOG_WARN("创建分享码响应解析失败");
                                if (g_shareCodeCopyHint) {
                                    g_shareCodeCopyHint->setText(tr("创建分享码失败"));
                                    g_shareCodeCopyHint->show();
                                    QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                                }
                            }
                        });

                    } else {
                        ///请求回显消息异常处理
                        qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                                        .arg(ret_info.code)
                                        .arg(ret_info.message);
                    };
                }
            });

        } else {
            // 该方案已上传过服务器,重新上传方案文件后 转 更新方案 接口
            QFile *file = new QFile(upload_file_path);
            if (!file->open(QIODevice::ReadOnly)) {
                emit ApoManager::instance()->requestlogWithTime(
                    QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg("file open failed"));
                delete file;
                return;
            }

            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QVariant("form-data; name=\"file\"; filename=\""
                                        + QFileInfo(upload_file_path).fileName() + "\""));
            filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                               QVariant("application/octet-stream")); // INI 文件的 MIME 类型

            filePart.setBodyDevice(file);
            file->setParent(multiPart);
            multiPart->append(filePart);

            QNetworkReply *reply = HttpClient::instance().upload("/user/uploads", multiPart, RequestOptions{}.withTag("user"));
            multiPart->setParent(reply);

            QObject::connect(reply, &QNetworkReply::finished, this, [=]() mutable {
                reply->deleteLater();

                if (file) {
                    if (file->isOpen()) {
                        file->close();
                    }
                    file->deleteLater();
                }

                // 不管上传成不成功，都删除文件，下次重新打开再写
                if (QFile::remove(upload_file_path)) {
                    qDebug() << "本地 upload_tmp.ini 文件删除成功:" << upload_file_path;
                } else {
                    qDebug() << "本地 upload_tmp.ini 文件删除失败:" << upload_file_path;
                }

                if (reply->error() != QNetworkReply::NoError) {
                    emit ApoManager::instance()->requestlogWithTime(QString("%1 %2 netWorkReply error:%3")
                                                                        .arg(__FUNCTION__)
                                                                        .arg(__LINE__)
                                                                        .arg(reply->errorString()));

                    return;
                }

                if (reply->error() == QNetworkReply::NoError) {

                    /// 读取响应数据
                    QByteArray responseData = reply->readAll();
                    qDebug() << "文件上传请求 回显原始数据:" << QString::fromUtf8(responseData);

                    /// 解析 JSON
                    QJsonParseError parseError;
                    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
                    if (parseError.error != QJsonParseError::NoError) {
                        QString errorMsg = "JSON解析错误: " + parseError.errorString();
                        emit ApoManager::instance()->requestlogWithTime(
                            QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg(errorMsg));

                        return;
                    }

                    /// 检查业务状态码
                    QJsonObject rootObj = jsonDoc.object();
                    QString code = rootObj["code"].toString();
                    if (code != "success") {
                        QString message = rootObj["message"].toString();
                        QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
                        emit ApoManager::instance()->requestlogWithTime(QString("上传文件 %1").arg(errorMsg));

                        return;
                    }

                    DeSheng::FileUploadsResponse ret_info; /// 文件上传回显信息
                    if (DeSheng::ProcessFileUploadsResult(ret_info, jsonDoc)) {
                        /// 文件上传成功后，调用 更新方案 接口
                        /// 再将返回的 分享码 复制到 粘贴板
                        DeSheng::UpdateSchemeRequest update_scheme_request;

                        update_scheme_request.id = currentPlanRadio->ShareCodeId
                                                       .toLongLong(); //分享码 对应的id
                        update_scheme_request.title = currentPlanRadio->lab_name
                                                          ->text(); // 方案标题(方案名称)
                        update_scheme_request.description = currentPlanRadio->lab_description
                                                                ->text(); // 方案描述
                        update_scheme_request.url = ret_info.data.url;    // 文件上传回显信息中的 URL
                        update_scheme_request.device_name = SelDev_DeviceName; //设备名称
                        update_scheme_request.device_type = DevType;           //设备类型

                        QJsonObject body = UpdateSchemeRequestToJson(update_scheme_request);
                        QJsonDocument doc(body);

                        QString t_path = QString("/schemes/%1").arg(update_scheme_request.id);
                        QNetworkReply *reply = HttpClient::instance().put(t_path, RequestOptions{}.withBody(doc.toJson()).withTag("schemes"));
                        connect(reply, &QNetworkReply::finished, [=]() {
                            DeSheng::UpdateSchemeResponse resp;
                            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                            if (ProcessUpdateSchemeResult(resp, doc)) {
                                // 方案更新成功
                                qDebug() << "方案更新成功" << resp.data.updated_at << "更新后文件url："<<resp.data.url;
                                // 更新当前选中项的相关信息
                                QString shareCode = resp.data.share_code;
                                int64_t ShareCodeId = resp.data.id;

                                // 更新当前选中项的相关信息
                                currentPlanRadio->ShareCodeId = QString::number(ShareCodeId);
                                currentPlanRadio->ShareCode = shareCode;

                                // 拷贝到粘贴板
                                QClipboard *clipboard = QApplication::clipboard();

                                // 拼接成指定格式: 名称+机型+场景+ys分享码
                                QStringList devs = currentPlanRadio->getLabDevs();
                                QString devStr = devs.join("+");
                                QString target_text = currentPlanRadio->lab_name->text()
                                                      + "+" + devStr
                                                      + "+" + currentPlanRadio->lab2->text()
                                                      + "+ys" + shareCode;
                                clipboard->setText(target_text);

                                // 提示用户
                                // 2 秒后自动隐藏
                                g_shareCodeCopyHint->setText(tr("方案分享码已复制"));
                                g_shareCodeCopyHint->show();
                                QTimer::singleShot(2000, g_shareCodeCopyHint, &QLabel::hide);
                                // qDebug() << "分享码已复制到粘贴板:" << shareCode;
                            }
                        });
                    }
                }
            });
        }



        /// WBLIU: 旧版导出
        // QFileDialog fdialog;//.sdt
        // fdialog.setNameFilter(QString("ini file(*.ini)")); //在win中过滤显示.ini文件，而且保存的文件的后缀都为.sdt；在Ubuntu中只是过滤显示文件
        // fdialog.setDefaultSuffix("ini");// 设置默认后缀 —— 当用户输入不带扩展名的文件名时，自动添加 .ini
        // fdialog.setViewMode(QFileDialog::Detail);// 显示详细视图
        // fdialog.setAcceptMode(QFileDialog::AcceptSave);/*设置文件对话框为保存模式，默认标题另存为；操作模式定义对话框是用于打开还是保存文件，默认情况下，此属性设置为AcceptOpen，标题为打开*/
        // fdialog.setDirectory(QString("/"));    //因为"/"在Ubuntu中代表的根目录
        // fdialog.setWindowTitle("导出方案");
        // QString filePath;
        // if(fdialog.exec() == QDialog::Accepted){
        //     filePath = fdialog.selectedFiles()[0];
        //     // 注意：即使过滤器是 *.ini，用户仍可能输入任意名字
        //     // 我们确保如果没有扩展名或不是 .ini，则补上 .ini
        //     QFileInfo info(filePath);
        //     if (info.suffix().isEmpty()) {
        //         filePath += ".ini"; // 强制加上 .ini 后缀
        //     } else if (info.suffix() != "ini") {
        //         // 如果用户选择了 .ini 或其他格式，你可以提示或保留原意
        //         // 这里我们直接使用用户选择的路径
        //         filePath += ".ini"; // 强制加上 .ini 后缀
        //     }

        //     writeExportPlanIni(filePath);
        //     qDebug() << "File to save:" << filePath;
        // }
    }
}

//保存预设到文件
int SpeakerSet::writeExportPlanIni(QString filePath, bool ShowEqEn)
{
    QSettings settings(filePath,QSettings::IniFormat);
    //保存当前选中预设 相关信息
    if (currentPlanRadio)
    {
        // 保存按钮属性
        //名称
        settings.setValue(QString("LoadPlan/Name"),currentPlanRadio->lab_name->text());
        //描述
        settings.setValue(QString("LoadPlan/Description"),currentPlanRadio->property("fullText").toString());
        //标签1
        settings.setValue("LoadPlan/Lab1",currentPlanRadio->getLabDevs());
        //标签2
        settings.setValue(QString("LoadPlan/Lab2"),currentPlanRadio->lab2->text());

        //保密
        QVariantMap planValMap;
        planValMap["DataVisibleEn"] = ShowEqEn;//currentPlanRadio->getAllPlanValue().DataVisibleEn;
        planValMap["ParentPlanName"] = currentPlanRadio->getAllPlanValue().ParentPlanName;
        planValMap["AlgoOpenEn"] = currentPlanRadio->getAllPlanValue().AlgoOpenEn;
        planValMap["spaceOpenEn"] = currentPlanRadio->getAllPlanValue().spaceOpenEn;
        planValMap["eqOpenEn"] = currentPlanRadio->getAllPlanValue().eqOpenEn;
        planValMap["drcOpenEn"] = currentPlanRadio->getAllPlanValue().drcOpenEn;
        planValMap["lowVal"] = currentPlanRadio->getAllPlanValue().lowVal;
        planValMap["spaceVal"] = currentPlanRadio->getAllPlanValue().spaceVal;
        planValMap["spaceReverb"] = currentPlanRadio->getAllPlanValue().spaceReverb;
        planValMap["GainVal"] = currentPlanRadio->getAllPlanValue().GainVal;
        planValMap["drcVal"] = currentPlanRadio->getAllPlanValue().drcVal;

        // WBLIU：添加 ExtraEq数据
        QVariantList t_ExtraEq;
        for (int extraEq : currentPlanRadio->getAllPlanValue().ExtraEq) {
            t_ExtraEq.append(extraEq);
        }
        planValMap["ExtraEq"] = t_ExtraEq;

        //添加频点值
        QVariantList freqValList;
        for (int j = 0; j < 10; ++j) {
            freqValList.append(currentPlanRadio->getAllPlanValue().freqVal[j]);
        }
        planValMap["freqVal"] = freqValList;
        // 添加eqVal数组 int eqVal[10]
        QVariantList eqValList;
        for (int j = 0; j < 10; ++j) {
            eqValList.append(currentPlanRadio->getAllPlanValue().eqVal[j]);
        }
        planValMap["eqVal"] = eqValList;
        // 添加qVal数组 double qVal[10]
        QVariantList qValList;
        for (int j = 0; j < 10; ++j) {
            qValList.append(currentPlanRadio->getAllPlanValue().qVal[j]);
        }
        planValMap["qVal"] = qValList;
        //添加滤波器
        QVariantList filterValList;
        for (int j = 0; j < 10; ++j) {
            filterValList.append(currentPlanRadio->getAllPlanValue().filterVal[j]);
        }
        planValMap["filterVal"] = filterValList;


        //二创内容
        //添加频点值
        QVariantList freqValList_deriv;
        for (int j = 0; j < 10; ++j) {
            freqValList_deriv.append(currentPlanRadio->getAllPlanValue().freqVal_deriv[j]);
        }
        planValMap["freqVal_deriv"] = freqValList_deriv;
        // 添加eqVal数组 int eqVal[10]
        QVariantList eqValList_deriv;
        for (int j = 0; j < 10; ++j) {
            eqValList_deriv.append(currentPlanRadio->getAllPlanValue().eqVal_deriv[j]);
        }
        planValMap["eqVal_deriv"] = eqValList_deriv;
        // 添加qVal数组 double qVal[10]
        QVariantList qValList_deriv;
        for (int j = 0; j < 10; ++j) {
            qValList_deriv.append(currentPlanRadio->getAllPlanValue().qVal_deriv[j]);
        }
        planValMap["qVal_deriv"] = qValList_deriv;
        //添加滤波器
        QVariantList filterValList_deriv;
        for (int j = 0; j < 10; ++j) {
            filterValList_deriv.append(currentPlanRadio->getAllPlanValue().filterVal_deriv[j]);
        }
        planValMap["filterVal_deriv"] = filterValList_deriv;

        settings.setValue("WritePlan",planValMap);

        /*//未加密
        settings.setValue(QString("LoadPlan/DataVisibleEn"), currentPlanRadio->getAllPlanValue().DataVisibleEn);
        settings.setValue(QString("LoadPlan/AlgoOpenEn"), currentPlanRadio->getAllPlanValue().AlgoOpenEn);
        settings.setValue(QString("LoadPlan/spaceOpenEn"), currentPlanRadio->getAllPlanValue().spaceOpenEn);
        settings.setValue(QString("LoadPlan/eqOpenEn"),currentPlanRadio->getAllPlanValue().eqOpenEn);

        settings.setValue(QString("LoadPlan/lowVal"), currentPlanRadio->getAllPlanValue().lowVal);
        //qDebug("save lowVal:%d\n",planValMap["lowVal"].toInt());
        settings.setValue(QString("LoadPlan/spaceVal"), currentPlanRadio->getAllPlanValue().spaceVal);

        settings.setValue(QString("LoadPlan/GainVal"), currentPlanRadio->getAllPlanValue().GainVal);
        settings.setValue(QString("LoadPlan/lowEqVal"), currentPlanRadio->getAllPlanValue().lowEqVal);
        settings.setValue(QString("LoadPlan/middleEqVal"), currentPlanRadio->getAllPlanValue().middleEqVal);
        settings.setValue(QString("LoadPlan/highEqVal"), currentPlanRadio->getAllPlanValue().highEqVal);
        //添加freqVal数组 int freqVal[10]
        QVariantList freqValList;
        for (int j = 0; j < 10; ++j) {
            freqValList.append(currentPlanRadio->getAllPlanValue().freqVal[j]);
        }
        settings.setValue(QString("LoadPlan/freqVal"), freqValList);
        // 添加eqVal数组 int eqVal[10]
        QVariantList eqValList;
        for (int j = 0; j < 10; ++j) {
            eqValList.append(currentPlanRadio->getAllPlanValue().eqVal[j]);
        }
        settings.setValue(QString("LoadPlan/eqVal"), eqValList);
        // 添加qVal数组 double qVal[10]
        QVariantList qValList;
        for (int j = 0; j < 10; ++j) {
            qValList.append(currentPlanRadio->getAllPlanValue().qVal[j]);
        }
        settings.setValue(QString("LoadPlan/qVal"), qValList);
        */
    }

    return 1;
}
//读取加载的文件预设(路径，是否立马生效)
int SpeakerSet::readExportPlanIni(QString filePath, bool TakeEffect)
{
    QSettings settings(filePath,QSettings::IniFormat);
    // 保存按钮属性
    QString name = settings.value("LoadPlan/Name").toString();
    QString desc = settings.value("LoadPlan/Description","").toString();
    QStringList Lab1 = settings.value("LoadPlan/Lab1","").toStringList();
    QString Lab2 = settings.value("LoadPlan/Lab2","").toString();

    //若名称已存在，则自动生成新的
    QString originalName = name;
    // 检查并生成唯一名称
    bool nameExists = false;
    // 检查并生成唯一名称
    {
        //分机型显示时
       // for (const QString& dev : Lab1)
    }

    //不分机型显示时
    QString dev = QString();
    {
        nameExists = MovieVal.AllPlanRadioHash.contains(qMakePair(name, dev));
        if (nameExists) {
            // 生成新名称
            int counter = 1;
            do {
                name = QString("%1-%2").arg(originalName).arg(counter);
                counter++;

                // 重新检查新名称是否已存在
                nameExists = MovieVal.AllPlanRadioHash.contains(qMakePair(name, dev));

            } while (nameExists && counter <= 1000);

            qDebug() << "名称" << originalName << "已存在，重命名为:" << name;
        }
    }

    bool IsLoad = true;//settings.value("LoadPlan/IsLoad").toBool();

    bool RIsAdded= 0;
    int favIdx = -1;


    QVariantMap planValMap = settings.value("WritePlan").toMap();
    PlanVal RVal;
    RVal.DataVisibleEn = planValMap["DataVisibleEn"].toBool();
    RVal.ParentPlanName = planValMap["ParentPlanName"].toString();
    RVal.AlgoOpenEn = planValMap["AlgoOpenEn"].toBool();
    RVal.spaceOpenEn = planValMap["spaceOpenEn"].toBool();
    RVal.eqOpenEn = planValMap["eqOpenEn"].toBool();
    RVal.drcOpenEn = planValMap["drcOpenEn"].toBool();
    RVal.lowVal = planValMap["lowVal"].toInt();
    //qDebug("read lowVal:%d\n",planValMap["lowVal").toInt());
    RVal.spaceVal = planValMap["spaceVal"].toInt();
    RVal.spaceReverb = planValMap["spaceReverb"].toInt();
    RVal.GainVal = planValMap["GainVal"].toInt();
    RVal.drcVal = planValMap["drcVal"].toInt(); // 读取灵犀算法值


    // WBLIU：读取 ExtraEq 数据
    QVariantList t_ExtraEq = planValMap["ExtraEq"].toList();
    int t_extra_count = qMin(t_ExtraEq.size(), 7);
    for (int j = 0; j < t_extra_count; ++j) {
        RVal.ExtraEq[j] = t_ExtraEq.at(j).toInt();
    }
    QVariantList freqValList = planValMap["freqVal"].toList();
    int count = freqValList.size();
    for (int j = 0; j < 10; ++j) {
        RVal.freqVal[j] = j < count ? freqValList[j].toDouble() : 20000.0;
    }
    QVariantList eqValList = planValMap["eqVal"].toList();
    count = eqValList.size();
    for (int j = 0; j < 10; ++j) {
        RVal.eqVal[j] = j < count ? eqValList[j].toDouble() : 0.0;
    }
    QVariantList qValList = planValMap["qVal"].toList();
    count = qValList.size();
    for (int j = 0; j < 10; ++j) {
        RVal.qVal[j] = j < count ? qValList[j].toDouble() : 0.7;
    }
    QVariantList filterValList = planValMap["filterVal"].toList();
    count = filterValList.size();
    for (int j = 0; j < 10; ++j) {
        RVal.filterVal[j] = j < count ? filterValList[j].toInt() : 0;
    }



    //二创内容
    //频点值
    QVariantList freqValList_deriv = planValMap["freqVal_deriv"].toList();
    count = freqValList_deriv.size();
    for (int j = 0; j < 10; ++j) {
        if (j < count) {
            RVal.freqVal_deriv[j] = freqValList_deriv[j].toDouble();
        } else {
            RVal.freqVal_deriv[j] = 20000.0;
        }
    }

    QVariantList eqValList_deriv = planValMap["eqVal_deriv"].toList();
    count = eqValList_deriv.size();
    for (int j = 0; j < 10; ++j) {
        if (j < count) {
            RVal.eqVal_deriv[j] = eqValList_deriv[j].toDouble();
        } else {
            RVal.eqVal_deriv[j] = 0;
        }
    }

    QVariantList qValList_deriv = planValMap["qVal_deriv"].toList();
    count = qValList_deriv.size();
    for (int j = 0; j < 10; ++j) {
        if (j < count) {
            RVal.qVal_deriv[j] = qValList_deriv[j].toDouble();
        } else {
            RVal.qVal_deriv[j] = 0.7;
        }
    }


    QVariantList filterValList_deriv = planValMap["filterVal_deriv"].toList();
    count = filterValList_deriv.size();
    for (int j = 0; j < 10; ++j) {
        if (j < count) {
            RVal.filterVal_deriv[j] = filterValList_deriv[j].toInt();
        } else {
            RVal.filterVal_deriv[j] = 0;
        }
    }
    /* 未加密
    PlanVal RVal;
    RVal.DataVisibleEn = settings.value("LoadPlan/DataVisibleEn").toBool();
    RVal.AlgoOpenEn = settings.value("LoadPlan/AlgoOpenEn").toBool();
    RVal.spaceOpenEn = settings.value("LoadPlan/spaceOpenEn").toBool();
    RVal.eqOpenEn = settings.value("LoadPlan/eqOpenEn").toBool();
    RVal.lowVal = settings.value("LoadPlan/lowVal").toInt();
    //qDebug("read lowVal:%d\n",settings.value("LoadPlan/lowVal").toInt());
    RVal.spaceVal = settings.value("LoadPlan/spaceVal").toInt();
    RVal.GainVal = settings.value("LoadPlan/GainVal").toInt();
    RVal.lowEqVal = settings.value("LoadPlan/lowEqVal").toInt();
    RVal.middleEqVal = settings.value("LoadPlan/middleEqVal").toInt();
    RVal.highEqVal = settings.value("LoadPlan/highEqVal").toInt();
    QVariantList freqValList = settings.value("LoadPlan/freqVal").toList();
    for (int j = 0; j < 10; ++j) {
        RVal.freqVal[j] = freqValList[j].toDouble();
    }
    QVariantList eqValList = settings.value("LoadPlan/eqVal").toList();
    for (int j = 0; j < 10; ++j) {
        RVal.eqVal[j] = eqValList[j].toDouble();
    }
    QVariantList qValList = settings.value("LoadPlan/qVal").toList();
    for (int j = 0; j < 10; ++j) {
        RVal.qVal[j] = qValList[j].toDouble();
    }*/

    addAllPlan(name,desc,IsLoad,RVal,RIsAdded,favIdx,TakeEffect,true,Lab1,Lab2,0,false,false,"","");


    group_AlltypeBtn->button(0)->setChecked(true); //跳转到所有分类

    // QAbstractButton *btn = group_AlltypeBtn->button(0);
    // if (btn) {
    //     btn->setChecked(true);   // 选中所有预设按钮，进入所有预设中
    // }

    //emit RealTimeSaveSysPlan_S();

    return 1;
}
//显示分类标题
void SpeakerSet::ShowPlansType()
{
    // ui->widget_CreateType->setVisible(false);


    for (int i = 0; i < 8; i++)
    {
        if (!PlansTypes[i].Name.isEmpty())
        {
            typeButtons[i]->setText(PlansTypes[i].Name);
            typeButtons[i+8]->setVisible(false);

            if(!Mplan)
            {
                Mplan = new MovePlan(m);
            }
            Mplan->addType(PlansTypes[i].Name);

            PlansTypeIdx++;
        }
        typeWidgets[i]->setVisible(PlansTypes[i].en);
    }


}
//创建分类
void SpeakerSet::on_pBt_CreateType_clicked()
{

    if(!CPlansType)
    {
        CPlansType = new EditPlansType(m);
    }

    CPlansType->EditTitle(0);
    CPlansType->hidePrompt();
    CPlansType->ShowEditName(-1,"");
    int res = CPlansType->exec();
    if(res == QDialog::Accepted)
    {

        typeWidgets[PlansTypeIdx]->setVisible(true);
        typeButtons[PlansTypeIdx+8]->setVisible(true);
        typeButtons[PlansTypeIdx]->setText(PlansTypes[PlansTypeIdx].Name);

        // if(!Mplan)
        // {
        //     Mplan = new MovePlan(m);
        // }
        // Mplan->addType(PlansTypes[PlansTypeIdx].Name);

        if(editEn)
        {
            typeButtons[PlansTypeIdx+8]->setVisible(true);
        }else
        {
            typeButtons[PlansTypeIdx+8]->setVisible(false);
        }

        PlansTypeIdx++;
        if(PlansTypeIdx >= 8)
        {
            ui->widget_CreateType->hide();
        }







        //保存方案分类
        QVariantMap plansMap;
        QVariantList Name;
        QVariantList Enable;
        for (auto& plan : PlansTypes) {
            Name.append(plan.Name);
            Enable.append(plan.en);
        }
        plansMap["TName"] = Name;
        plansMap["TEn"] = Enable;
        globalSettings->setValue("PlansTypes",plansMap);
    }
    CPlansType->close();
}
//编辑分类
void SpeakerSet::on_pBt_EditType_clicked()
{
    editEn = true;
    ui->stackedWidget_right->setCurrentWidget(ui->page_editing);
    setTypeBtnStyle(true);
    ShowPlansCheckBox(true);


    if(PlansTypeIdx >= 8)
    {
        ui->widget_CreateType->setVisible(false);
    }else
    {
        ui->widget_CreateType->setVisible(true);
    }


    for(int i = 0; i< PlansTypeIdx;i++)
    {
        qDebug()<< "typeButtons[i+8] name:" << typeButtons[i+8]->objectName();
        typeButtons[i+8]->setVisible(true);
    }
}
//取消编辑
void SpeakerSet::on_pBt_CancelEditType_clicked()
{
    editEn = false;
    ui->stackedWidget_right->setCurrentWidget(ui->page_noEdit);
    setTypeBtnStyle(false);
    ShowPlansCheckBox(false);
    if(PlansTypeIdx >= 8)
    {
        ui->widget_CreateType->setVisible(false);
    }else
    {
        ui->widget_CreateType->setVisible(true);
    }
    for(int i = 0; i< PlansTypeIdx;i++)
    {
        typeButtons[i+8]->setVisible(false);
    }
    if(!MovieVal.AllPlanRadioList_Check.isEmpty())
    {
        int cnt = MovieVal.AllPlanRadioList_Check.count();
        for (int i = 0; i < cnt; i++)
        {
            NewRadioBtn* currentRadio = MovieVal.AllPlanRadioList_Check.at(i);
            currentRadio->AllpBt_check->blockSignals(true);
            currentRadio->AllpBt_check->setChecked(false);
            currentRadio->AllpBt_check->blockSignals(false);
        }
        MovieVal.AllPlanRadioList_Check.clear();
    }
}
//设置分类的样式
void SpeakerSet::setTypeBtnStyle(bool checked)
{
    QString buttonStyle;
    if(!checked)
    {
        // 为组内所有按钮设置统一的样式表
        buttonStyle = R"(
            QPushButton {
                border-radius: 6px;
                color: #A1A8B3;
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                background: transparent;
            }
            QPushButton:checked {
                color: rgb(255, 255, 255);
                background: #0091DA;
            }
            QPushButton:hover:!checked {
                color: #A1A8B3;
                background: rgba(255, 255, 255, 0.1);
            }
        )";
    }else
    {
        buttonStyle = R"(
            QPushButton {
                border-radius: 6px;
                color: #A1A8B3;
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                background: rgba(255, 255, 255, 0.1);
            }
            QPushButton:checked {
                color: rgb(255, 255, 255);
                background: #0091DA;
            }
            QPushButton:hover:!checked {
                color: #A1A8B3;
                background: rgba(255, 255, 255, 0.2);
            }
        )";

    }
    for (auto *btn : group_AlltypeBtn->buttons()) {
        auto *pushBtn = qobject_cast<QPushButton*>(btn);
        if (pushBtn) {
            pushBtn->setStyleSheet(buttonStyle);
        }
    }

}
//显示所有方案的勾选框(用于编辑)
void SpeakerSet::ShowPlansCheckBox(bool en)
{
    for(int i = 0; i < MovieVal.MyPlanRadioList/*AllPlanRadioList_Dev*/.count(); i++)
    {
        NewRadioBtn* target = MovieVal.MyPlanRadioList/*AllPlanRadioList_Dev*/.at(i);
        target->AllpBt_check->setVisible(en);
        target->AllpBt_fav->setVisible(!en);
        if(!target->IsSys)
        {
            target->AllpBt_edit->setVisible(!en);
        }

    }

}

//删除所有勾选的方案
void SpeakerSet::on_pBt_DelPlans_clicked()
{
    if(MovieVal.AllPlanRadioList_Check.isEmpty())
    {
        return;
    }

    DelReset *del = new DelReset(m);
    del->editText(5);

    del->setModal(true);
    int result = del->exec();
    if(result == QDialog::Accepted)
    {
        bool save1 = false,save2 = false;

        int cnt = MovieVal.AllPlanRadioList_Check.count();
        //从最后一个元素往前遍历，删除当前元素不影响前面索引。
        for (int i = cnt - 1; i >= 0; i--)
        {
            NewRadioBtn* currentRadio = MovieVal.AllPlanRadioList_Check.at(i);
            if(currentRadio->IsAdded)//被收藏，删除收藏
            {
                currentRadio->AllpBt_fav->setChecked(false);
            }
            if(currentRadio->IsSys)
            {
                continue;//系统方案不可删除
            }


            QString Name = currentRadio->lab_name->text();
            {
                //分机型显示时
                // QString dev = currentRadio->lab1->text();
            }

            //不分机型显示时
            QString dev = QString();
            // 1. 使用哈希表查找目标按钮（优先主哈希表，再临时哈希表）
            NewRadioBtn* target = MovieVal.AllPlanRadioHash.value(qMakePair(Name,dev), nullptr);

            if (!target) {
                // 如果都找不到，说明按钮不在当前管理的列表中，直接返回
                return;
            }

            // 获取布局位置（删除前）
            int layoutIndex = layout_All->indexOf(currentRadio);
            int row = -1, col = -1;
            if (layoutIndex != -1) {
                int rowSpan, colSpan;
                layout_All->getItemPosition(layoutIndex, &row, &col, &rowSpan, &colSpan);
            }

            // 从所有容器中移除（哈希表和列表）
            MovieVal.AllPlanRadioHash.remove(qMakePair(Name,dev));
            MovieVal.MyPlanRadioHash.remove(qMakePair(Name,dev));

            // MovieVal.AllPlanRadioList_Dev.removeAll(currentRadio);
            MovieVal.AllPlanRadioList.removeAll(currentRadio);
            MovieVal.MyPlanRadioList.removeAll(currentRadio);
            AllPlanRadioList_Temp.removeAll(currentRadio);
            int PlanTypeIdx = currentRadio->PlanPageSel;
            if (PlanTypeIdx >= 1 && PlanTypeIdx <= 8) {
                PlanRadioLists[PlanTypeIdx-1].removeAll(currentRadio);//0-7代表分类,这里不包含所有
            }
            MovieVal.AllPlanRadioList_Check.removeAt(i);

            // AllPlanRadioList_Load.removeAll(radio);

            // 处理选中状态（如果删除的是当前选中的）
            if (currentRadio->isChecked())
            {
                if (!MovieVal.AllPlanRadioList_Dev.isEmpty())
                {
                    MovieVal.AllPlanRadioList_Dev.first()->setChecked(true);
                }
                else if (!MovieVal.AllPlanRadioList.isEmpty())
                {
                    MovieVal.AllPlanRadioList.first()->setChecked(true);
                }
            }

            // 从布局中移除并删除控件
            layout_All->removeWidget(currentRadio);
            currentRadio->deleteLater();   // 只调用一次

            // 根据当前是否搜索模式，调用对应的增量重排函数
            // 重排必须使用与布局当前显示一致的列表（当前标签页对应的列表），
            // 否则索引错位，删除后的空位补不上
            if (ui->lEdit_search->text().isEmpty())
            {
                const int viewType = group_AlltypeBtn->checkedId();
                QList<NewRadioBtn*>* viewList = (viewType == 0)
                        ? (MovieVal.AllPlanRadioList_Dev.isEmpty()
                           ? &MovieVal.AllPlanRadioList
                           : &MovieVal.AllPlanRadioList_Dev)
                        : &PlanRadioLists[viewType - 1];
                if(viewList->isEmpty())
                {
                    ui->stackedWidget->setCurrentWidget(ui->page_planEmpty);
                    updateSize(); // 切到空态页立即按当前窗口尺寸重算图片比例
                }else
                {
                    ui->stackedWidget->setCurrentWidget(ui->page_cloud);
                }
                if (row >= 0 && col >= 0) {
                    incrementalRearrangeAfterDelete(layout_All,
                                                    *viewList,   // 注意：此时 radio 已从列表中移除
                                                    row, col,    // 删除前的行列
                                                    row_Allrb, col_Allrb,
                                                    COLUMN_COUNT_rb,
                                                    CurrentBtnWight_rb, CurrentBtnHight_rb,
                                                    spacer_Temp_All);
                }
            }
            else
            {
                if(AllPlanRadioList_Temp.isEmpty())
                {
                    ui->stackedWidget->setCurrentWidget(ui->page_planEmpty);
                    updateSize(); // 切到空态页立即按当前窗口尺寸重算图片比例
                }else
                {
                    ui->stackedWidget->setCurrentWidget(ui->page_cloud);
                }
                // 搜索模式：使用临时列表和临时行列变量
                if (row >= 0 && col >= 0) {
                    incrementalRearrangeAfterDelete(layout_All,
                                                    AllPlanRadioList_Temp,
                                                    row, col,
                                                    row_Allrb_Temp, col_Allrb_Temp,
                                                    COLUMN_COUNT_rb,
                                                    CurrentBtnWight_rb, CurrentBtnHight_rb,
                                                    spacer_Temp_Search);
                }
            }

            if(currentRadio->IsSys)
            {
                save1 = true;
            }else
            {
                save2 = true;
            }
        }
        if(save1)
        {
            //保存系统预设
            emit RealTimeSaveSysPlan_S();
        }
        if(save2)
        {
            //保存我的预设
            emit RealTimeSaveModeVal_S();
        }
        // MovieVal.AllPlanRadioList_Check.clear();
    }
    del->deleteLater();


}

//移动所有勾选的方案
void SpeakerSet::on_pBt_MoveToType_clicked()
{
    if(MovieVal.AllPlanRadioList_Check.isEmpty())
    {
        return;
    }

    if(!Mplan)
    {
        Mplan = new MovePlan(m);
    }
    Mplan->delAllType();
    for (int i = 0; i < 8; i++)
    {
        if ((!PlansTypes[i].Name.isEmpty()) && PlansTypes[i].en)
        {
            if(!Mplan)
            {
                Mplan = new MovePlan(m);
            }
            Mplan->addType(PlansTypes[i].Name);
        }
    }

    int idx = MovieVal.AllPlanRadioList_Check.at(0)->PlanPageSel;
    if(idx != 0)
    {
        Mplan->showType(PlansTypes[idx-1].Name);
    }
    Mplan->setModal(true);
    int result = Mplan->exec();
    if(result == QDialog::Accepted)
    {
        for (int i = 0; i < MovieVal.AllPlanRadioList_Check.count(); i++)
        {
            NewRadioBtn* currentRadio = MovieVal.AllPlanRadioList_Check.at(i);
            int index = currentRadio->PlanPageSel;
            qDebug("当前按钮之前所在分类：%d,选中分类：%d\n",index,MyPlanTypeIdx+1);
            if(MyPlanTypeIdx+1 == index)
            {
                continue;
            }
            if(index != 0)
            {
                PlanRadioLists[index-1].removeAll(currentRadio);//0-7代表分类,这里不包含所有
            }

            for (int i = 0; i < 8; i++)
            {
                if(PlansTypes[i].Name == MyPlanType)
                {
                    currentRadio->PlanPageSel = i+1;//0代表所有，1-8代表分类
                    PlanRadioLists[i].append(currentRadio);//0-7代表分类,这里不包含所有
                }

            }
        }

        group_AlltypeBtn->button(MyPlanTypeIdx+1)->setChecked(true); //跳转到对应分类
        // if(ui->lEdit_search->text().isEmpty())
        // {
        //     scheduleLayoutUpdate();
        // }else
        // {
        //     UpdateSearchPlanPosition();
        // }
    }
    Mplan->close();


}

//切换主题
void SpeakerSet::setThemeAndPanelTransparency_SpeakerSet(int idx,int PValue)
{

}
//设置面板透明度（参数：主题，透明度）
void SpeakerSet::setPanelTransparency_SpeakerSet(int idx,int PValue)
{
    double PanelTransparency = PValue / 100.0;   //面板透明度(默认值是0.2)
    int r, g, b;
    QString suffix;
    switch (idx)
    {
    case 0: suffix = ""/*"_darkBlue"*/; break;//深蓝色（还未修改主题图片）
    case 1: suffix = "_white";  break;//白色
    case 2: suffix = "_black";  break;//黑色
    default: suffix = "";      break;
    }

    //当前预设
    ui->rBt_currentPlan->setThemeAndPanelTransparency(idx,PValue);

    switch (idx) {
    case 0:
        r = 81; g = 96; b = 122; break;// 深蓝色
    case 1:
        r = 81; g = 96; b = 122; break;// 白色
    case 2:
        r = 81; g = 96; b = 122; break; // 黑色
    default:
        r = 81; g = 96; b = 122; break;
    }

    QString colorStr = QString("rgba(%1, %2, %3, %4)")
                           .arg(r).arg(g).arg(b).arg(PanelTransparency);
    QColor background = QColor(r, g, b, PanelTransparency);
    switch (idx) {
    case 0:
        r = 255; g = 255; b = 255; break;// 深蓝色
    case 1:
        r = 255; g = 255; b = 255; break;// 白色
    case 2:
        r = 255; g = 255; b = 255; break; // 黑色
    default:
        r = 255; g = 255; b = 255; break;
    }
    QString colorStr2 = QString("rgba(%1, %2, %3, %4)")
                           .arg(r).arg(g).arg(b).arg(PanelTransparency);
    //我的收藏
    ui->widget_all->setStyleSheet(QString("background-color: %1;border-radius: 10px;").arg(colorStr));
    //方案库
    ui->pBt_Plans->setStyleSheet(
        QString("QPushButton"
                "{"
                "border-radius: 10px;"
                "background: %1;"
                "image: url(:/Skin/Images/Headphones/plans-ch%2.png);"
                "}")
            .arg(colorStr).arg(suffix));

    //试听
    ui->pBt_GameListen->setBackground(background);

    //预设库
    ui->stackedWidget_Speaker->setStyleSheet(
        QString("background: %1;"
                "border-radius: 6px;"
                "border-image: none;")
            .arg(colorStr)
        );

}
//设置面板模糊度
void SpeakerSet::setPanelBlur_SpeakerSet(int PValue)
{

}
