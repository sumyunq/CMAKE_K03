#ifndef UPDATEOTAFIND_H
#define UPDATEOTAFIND_H

/********************* OTA可升级回退版本弹窗 *********************/

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class UpdateOTAFind;
}

class UpdateOTAFind : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateOTAFind(QWidget *parent = nullptr);
    ~UpdateOTAFind();

    void updateTitle(int themeidx,int type, QString version);
    void startTimer();//开启4s倒计时
    //根据主题设置样式
    void setTheme_UpdateOTAFind(int idx);
private:
    Ui::UpdateOTAFind *ui;

    QTimer *m_countdownTimer = nullptr;
    int m_countdownValue;
    QString BtnTxt;//按钮文本（升级，回退）
};

#endif // UPDATEOTAFIND_H
