#ifndef CUSTOM_QSCROLLAREA_FOR_ADVERTISEMENT_PUSHBUTTON_H
#define CUSTOM_QSCROLLAREA_FOR_ADVERTISEMENT_PUSHBUTTON_H

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTimer>

#include "modules/AdvertisementSelectionPage/AdvertisementSelectionPageCustomUI/custom_QPushbutton_for_single_advertisement.h" ///< 子控件：单个按键

namespace Ui {
class CustomQScrollAreaForAdvertisementPushButton;
}

///
/// \brief The CustomQScrollAreaForAdvertisementPushButton class
/// 广告界面底部 按键区域
class CustomQScrollAreaForAdvertisementPushButton : public QScrollArea
{
    Q_OBJECT

public:
    explicit CustomQScrollAreaForAdvertisementPushButton(QWidget *parent = nullptr);
    ~CustomQScrollAreaForAdvertisementPushButton();
    void updateView();

    void onAdvertisementClicked(int index);

signals:
    void changeGameTypeVideos(int index); ///发送信息体或者index

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    QList<CustomQPushButtonForSingleAdvertisement *> cl_all_advertisement_list_; ///< 所有的 按键集合
    QButtonGroup *cl_all_advertisement_buttonGroup_ = nullptr;                   ///<  广告 按键组

    QWidget *cl_content_widget_ = nullptr; ///< 内容显示区域
    QHBoxLayout *cl_hBoxLayout_ = nullptr; ///< 水平布局

private:
    ///布局属性
    int left_margin_ = 0;
    int top_margin_ = 0;
    int right_margin_ = 0;
    int bottom_margin_ = 0;
    int spacing_ = 10; ///内部部件见间距

private:
    Ui::CustomQScrollAreaForAdvertisementPushButton *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QSCROLLAREA_FOR_ADVERTISEMENT_PUSHBUTTON_H
