#ifndef NEWRADIOBTN_H
#define NEWRADIOBTN_H

/********************* 预设库和上传方案处的方案窗体 *********************/

#include "GlobalDefinition.h"
#include <QRadioButton>
#include <QPushButton>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include "CustomControl/CustomRadioButton/AutoResizeLabel.h"
//QRadioButton内部右上/下角添加一个按钮

class NewRadioBtn : public QRadioButton
{
    Q_OBJECT
public:
    explicit NewRadioBtn(const PlanVal& val, QWidget *parent = nullptr);
    QPushButton *AllpBt_fav;//收藏
    QPushButton *AllpBt_edit;//编辑（修改、复制、删除、移动到）
    QPushButton *AllpBt_check;//勾选（用于批量删除和移动）

    QWidget *container;//放置两个标签的控件
    QHBoxLayout *HLayout_label;//水平布局
    AutoResizeLabel *lab1;//标签1 机型
    AutoResizeLabel *lab2;//标签2 场景
    void setLabel2(const QString &label2);

    QLabel *lab_name;//方案名称（最大九个字）

    QWidget *PlanDescription;//放置方案描述的控件
    QHBoxLayout *HLayout_description;//水平布局
    QLabel *lab_description;//方案描述（最大50字符）,外部获得方案描述使用property("fullText").toString()

    QMenu *eMenu;
    QAction *A_rename;
    QAction *A_copy;
    QAction *A_del;
    QAction *A_move;

    void updateElidedText(QString m_fullText,QString PlanName);//方案描述，方案名称
    void updateAllPlanValue(const PlanVal& newVal);
    PlanVal getAllPlanValue();

    void setIsAddedEn(bool en, int idx);
    bool IsAdded;//是否已收藏
    int favIdx;//收藏后的id
    bool IsSys;//是否系统方案

    bool IsLoad;
    void setIsLoad(bool en);
    void setStyle(bool IsAdded);

    bool GetDataVisibleEn();

    int PlanPageSel;//属于哪个分类（0：所有预设，1：我的预设，2：分类2, 3：分类3...8:分类8）
    int GetPlanPageSel();

    QString ShareCodeId; //分享方案文件id:为空则需要上传文件，不为空表示文件已存在服务器，只需要更新
    QString ShareCode;   //分享码


    void setLab1Style(QString DeviceName);//设置标签1的样式
    void setLabDevsOne(const QString& dev);// 添加单个设备
    void setLabDevs(const QStringList& devs);// 设置设备列表
    const QStringList& getLabDevs() const;// 获取设备列表


protected:
    // 窗口大小变化时更新位置
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void updateButtonPosition();

    PlanVal m_planValue;

    QLabel *m_customTooltip = nullptr;

    QStringList lab_devs;

    QWidget *m_tooltipWidget ;//多机型悬浮框
    QVBoxLayout *m_tooltipLayout;// 悬浮框布局
    void updateTooltip(); // 更新悬浮框内容

    NewCustomToolTip *tip_des;
};

#endif // NEWRADIOBTN_H
