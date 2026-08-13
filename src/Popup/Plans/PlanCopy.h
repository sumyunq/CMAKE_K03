#ifndef PLANCOPY_H
#define PLANCOPY_H

/********************* 新建、复制方案弹窗 *********************/

#include <QDialog>
#include "LoadLib.h"
#include "APOThread/ApoManager.h"
#include "CustomControl/NewComboBox.h"
namespace Ui {
class PlanCopy;
}

class PlanCopy : public QDialog
{
    Q_OBJECT

public:
    explicit PlanCopy(QWidget *parent = nullptr);
    ~PlanCopy();

    void EditTitle(QString titleName);
    void showSysName(QString name);
    void showCurName(QString name,QString desc,QStringList lab_devs,QString lab2);
    void ShowDev();

    void updateUI_cBox_PlanType_Idx(QStringList planTypes);  /// 更新分类下拉框信息
    int get_cBox_PlanType_Idx_currentIndex();  /// 返回当前分类下拉框index

    void setTheme_PlanCopy(int idx);//根据主题设置样式

private slots:
    void on_pBt_ok_clicked();

    void on_pBt_close_clicked();

    void on_pushButton_clicked();

    void on_lEdit_Name_textChanged(const QString &arg1);

    void on_pTextEdit_Description_textChanged();

    void on_pBt_addDev_clicked();

    void on_pBt_Dev2_clicked();

    void on_pBt_Dev3_clicked();

    void on_cBox_Scene_currentIndexChanged(int index);

    void on_pBt_cancle_clicked();

private:
    Ui::PlanCopy *ui;

    void enforceByteLimit();//限制字节

    void M_SetCBoxShadow(NewComboBox *cBox);

    QButtonGroup *group_dev;

    int checkedCount = 0;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // PLANCOPY_H
