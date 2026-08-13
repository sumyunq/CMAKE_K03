#ifndef CUSTOM_QSCROLLAREA_ROUNDBUTTON_H
#define CUSTOM_QSCROLLAREA_ROUNDBUTTON_H

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QPushButton_roundbutton.h" ///<子部件

namespace Ui {
class CustomQScrollAreaRoundbutton;
}

class CustomQScrollAreaRoundbutton : public QScrollArea
{
    Q_OBJECT

public:
    explicit CustomQScrollAreaRoundbutton(QWidget *parent = nullptr);
    explicit CustomQScrollAreaRoundbutton(int showMode, QWidget *parent = nullptr); ///< 指定布局模式: 0=网格, 1=水平, 2=垂直
    ~CustomQScrollAreaRoundbutton();

    void updateView();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QList<CustomQPushButtonRoundButton *> cl_all_rows_CustomQPushButtonRoundButton_; ///< 行数对应的按键
    QButtonGroup *cl_buttonGroup_ = nullptr; ///< 按键互斥组

private:
    QWidget *cl_content_widget_ = nullptr; /// 内容显示区域
    QGridLayout *cl_gridLayout_ = nullptr; ///网格布局
    QHBoxLayout *cl_hBoxLayout_ = nullptr; ///水平布局
    QVBoxLayout *cl_vBoxLayout_ = nullptr; ///垂直布局

    int cl_show_mode_ = 0;        ///< 0: 网格布局; 1: 水平布局; 2: 垂直布局
    int cl_columnCount_ = 3;      ///< 网格布局模式下的列数
    int cl_itemMinWidth_ = 8;   ///< 单列最小宽度

private:
    Ui::CustomQScrollAreaRoundbutton *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QSCROLLAREA_ROUNDBUTTON_H
