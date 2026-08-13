#ifndef FACTORYRESET_H
#define FACTORYRESET_H

/********************* 恢复出厂（恢复默认）弹窗 *********************/

#include <QDialog>

namespace Ui {
class FactoryReset;
}

class FactoryReset : public QDialog
{
    Q_OBJECT

public:
    explicit FactoryReset(QWidget *parent = nullptr);
    ~FactoryReset();

    void startTimer();//开启4s倒计时
    //根据主题设置样式
    void setTheme_FactoryReset(int idx);

private:
    Ui::FactoryReset *ui;

    QTimer *m_countdownTimer = nullptr;
    int m_countdownValue;
};

#endif // FACTORYRESET_H
