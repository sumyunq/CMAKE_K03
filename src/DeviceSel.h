#ifndef DEVICESEL_H
#define DEVICESEL_H

#include <QButtonGroup>
#include <QDialog>

#include "modules/DeviceSelectionPage/device_selection_main_page.h" ///< 设备选择

namespace Ui {
class DeviceSel;
}

class DeviceSel : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSel(QWidget *parent = nullptr);
    ~DeviceSel();

    void LanguageSet();
    void DevSelInitialization();
    void UpdateDeviceSelectionMainPage(); ///< 更新设备选择界面
    void OnXiberiaAction(int result);     //回调函数

protected slots:
    void HandleDeviceChecked(const DeSheng::DeviceInfo &deviceInfo); ///< 设备图片被点击，进入主页

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void AddTargetDeviceInfo(
        DeSheng::DeviceInfo deviceInfo); ///< 根据设备名，在设备选择界面添加相关设备

public:
    DeviceSelectionMainPage *clp_device_selection_mainPage_ = nullptr; ///<设备选择主界面

private:
    Ui::DeviceSel *ui;
    int writeDevIni();

private slots:
    //void ButtonGroup_buttonToggled(QAbstractButton *button,bool checked);
    // void ButtonGroup_buttonClicked(
    //     QAbstractButton *
    //         button); /// WBLIU： 更新为 void HandleDeviceChecked(const DeSheng::DeviceInfo &deviceInfo); ///< 设备图片被点击，进入主页

signals:
    void imageSelected(const DeSheng::DeviceInfo &deviceInfo, const QRect &sourceGeometry);
    //void loadApoFailed();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // DEVICESEL_H
