#ifndef MOVEPLAN_H
#define MOVEPLAN_H

/********************* 方案库移动分类弹窗 *********************/

#include <QDialog>
#include "CustomControl/NewComboBox.h"

//移动方案到某分类
namespace Ui {
class MovePlan;
}

class MovePlan : public QDialog
{
    Q_OBJECT

public:
    explicit MovePlan(QWidget *parent = nullptr);
    ~MovePlan();

    void addType(QString name);//添加分类到ComboBox
    void delType(QString name);//ComboBox删除分类
    void delAllType();//ComboBox删除所有分类
    void rnameType(QString oldName,QString newName);//重命名分类
    void showType(QString name);//显示已添加到的分类

    void setTheme_MovePlan(int idx);//根据主题设置样式

private slots:
    void on_pBt_ok_clicked();

private:
    Ui::MovePlan *ui;

    void M_SetCBoxShadow(NewComboBox *cBox);
};

#endif // MOVEPLAN_H
