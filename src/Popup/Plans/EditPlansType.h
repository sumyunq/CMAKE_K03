#ifndef EditPlansType_H
#define EditPlansType_H

/********************* 编辑方案库分类（增加、重命名） *********************/

#include <QDialog>

namespace Ui {
class EditPlansType;
}

class EditPlansType : public QDialog
{
    Q_OBJECT

public:
    explicit EditPlansType(QWidget *parent = nullptr);
    ~EditPlansType();

    void hidePrompt();

    void EditTitle(int idx);

    int updateId = -1;
    void ShowEditName(int Id,QString txt);

    void setTheme_EditPlansType(int idx);

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_pBt_ok_clicked();

private:
    Ui::EditPlansType *ui;
};

#endif // EditPlansType_H
