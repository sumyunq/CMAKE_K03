#ifndef DELRESET_H
#define DELRESET_H

/********************* 删除重置弹窗 *********************/

#include <QDialog>

namespace Ui {
class DelReset;
}

class DelReset : public QDialog
{
    Q_OBJECT

public:
    explicit DelReset(QWidget *parent = nullptr);
    ~DelReset();

    void editText(int idx);

    void setTheme_DelReset(int idx);

private:
    Ui::DelReset *ui;
};

#endif // DELRESET_H
