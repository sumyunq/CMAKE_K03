#include "DeviceSel.h"
#include <QFile>
#include <QSettings>
#include "APOThread/ApoManager.h"
#include "DeviceManager/DefaultOutput.h"
#include "LoadLib.h"
#include "ui_DeviceSel.h"

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include <QDebug>

#include <QFuture>
#include <QtConcurrent>

unsigned short SelDev_VID;
unsigned short SelDev_PID;
QString SelDev_DeviceName;
QString SelDev_DeviceGuid;
bool devSelEn = true; //true:进入机型选择页面  false:跳过机型选择界面

QHash<QString, QHash<QString, QString>> ERdevs = {};
QString fP_HasOpened; //存放之前打开过的设备信息的文件路径
bool retB = false;
bool General = false; //非查找通用模式下设备

DeviceSel::DeviceSel(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DeviceSel)
{
    ui->setupUi(this);
    // 设置无边框
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    // QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    // shadow->setOffset(0, 0);             // 设置阴影偏移
    // shadow->setColor(QColor("#000000")); // 设置阴影颜色
    // shadow->setBlurRadius(30);           // 设置模糊半径

    // ui->widget_device_section_page->setGraphicsEffect(shadow);
    //ui->widget->setContentsMargins(30, 30, 30, 30); // 设置边距以显示阴影

    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

DeviceSel::~DeviceSel()
{
    delete ui;
}

void DeviceSel::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // 什么都不做，忽略 ESC 键
        event->accept(); // 或 ignore()
        return;
    }
    // 其他键交给父类处理
    QWidget::keyPressEvent(event);
}

void DeviceSel::LanguageSet()
{
    //刷新文本
    ui->retranslateUi(this);
}

void DeviceSel::DevSelInitialization()
{
    ERdevs = DefaultOutput::enumDevices2(eRender);
    if (retB) {
        //devs = lolib->EnumeDev();
        //判断当前电脑是否打开过上位机且打开的是那个设备，若该设备存在则直接跳到ui->page_Selmain，否则打开page_devSel
        // fP_HasOpened = QApplication::applicationDirPath()+QString("/ProgramData/setting.ini");

        fP_HasOpened = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath()
                       + "/XIBERIA X HUB/ProgramData/setting.ini";
        // if (!QFile::exists(fP_HasOpened))
        // {
        //     fP_HasOpened = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).first() + QString("/ProgramData/%1/setting.ini").arg(SelDev_DeviceName);
        // }

        // 检查文件是否存在
        QFile configFile(fP_HasOpened);
        if (configFile.exists()) {
            //qDebug() << "配置文件存在，判断设备是否存在";
            bool deviceFound = false;
            QSettings settings(fP_HasOpened, QSettings::IniFormat);
            unsigned short T_VID = settings.value("Device/VID").toUInt();
            unsigned short T_PID = settings.value("Device/PID").toUInt();
            QString T_Name = settings.value("Device/Name").toString();
            for (QHash<QString, QHash<QString, QString>>::iterator iter = ERdevs.begin();
                 iter != ERdevs.end();
                 iter++) {
                QHash<QString, QString> deviceInfo = iter.value();
                // 比较设备信息的每个字段
                bool vidMatch = (deviceInfo.value("vid").toUInt() == T_VID);
                bool pidMatch = (deviceInfo.value("pid").toUInt() == T_PID);
                bool nameMatch = (deviceInfo.value("friendlyName") == T_Name);

                // qDebug() << "=== DevSelInitialization设备信息 ===";
                // qDebug() << "设备名称:" << deviceInfo.value("friendlyName");
                // qDebug() << "VID:" << deviceInfo.value("vid").toUInt();
                // qDebug() << "PID:" << deviceInfo.value("pid").toUInt();
                // qDebug() << "================";

                // 如果所有字段都匹配
                if (vidMatch && pidMatch && nameMatch) {
                    SelDev_VID = deviceInfo["vid"].toUShort(nullptr, 16);
                    SelDev_PID = deviceInfo["pid"].toUShort(nullptr, 16);
                    SelDev_DeviceName = deviceInfo["friendlyName"];
                    SelDev_DeviceGuid = iter.key();

                    //qDebug() << "=== 设备信息 ===";
                    //qDebug() << "设备名称:" << SelDev_DeviceName;
                    //qDebug() << "VID:" << SelDev_VID;
                    //qDebug() << "PID:" << SelDev_PID;
                    //qDebug() << "================";

                    deviceFound = true;
                    break;
                }
            }
            if (!deviceFound) {
                qDebug() << "没有找到匹配的设备";
                devSelEn = true; //进入机型选择界面

            } else {
                //找到匹配设备
                devSelEn = false; //跳过机型选择界面
            }

        } else {
            devSelEn = true; //进入机型选择界面
        }

        //选择设备界面显示
        // CreateButton(); ///WBLIU:旧版
        UpdateDeviceSelectionMainPage(); ///WBLIU:新版

        //qDebug("选择设备界面显示devSelEn:%d\n",devSelEn);
    }

    //Sleep(1000);
}

void DeviceSel::HandleDeviceChecked(const DeSheng::DeviceInfo &deviceInfo)
{
    //if(checked)
    {
        SelDev_VID = deviceInfo.SelDev_VID;
        SelDev_PID = deviceInfo.SelDev_PID;
        SelDev_DeviceName = deviceInfo.DeviceSysTypeName;
        SelDev_DeviceGuid = deviceInfo.DeviceGuid;

        writeDevIni();

        // qDebug() << "\n=== 设备信息 ===";
        // qDebug() << "设备名称:" << SelDev_DeviceName;
        // qDebug() << "VID:" << SelDev_VID;
        // qDebug() << "PID:" << SelDev_PID;
        // qDebug() << "================";

        emit ApoManager::instance()->requestlogWithTime("\n=== 选择设备的信息 ===");
        emit ApoManager::instance()->requestlogWithTime(
            QString("设备名称:%1").arg(SelDev_DeviceName));
        emit ApoManager::instance()->requestlogWithTime(QString("VID:%1").arg(SelDev_VID));
        emit ApoManager::instance()->requestlogWithTime(QString("PID:%1").arg(SelDev_PID));
        emit ApoManager::instance()->requestlogWithTime("================");

        // //QPoint globalPos = button->mapToGlobal(QPoint(0, 0));
        // // 将按钮的位置转换为屏幕坐标
        // QPoint topLeft = button->mapToGlobal(QPoint(0, 0));
        // QRect globalRect(topLeft, button->size());
        // load = button->property("image").toString();
        // //load = ":/Skin/Images/DevSel/General-bk.png";
        // emit imageSelected(load, globalRect);

        QObject *senderObj = sender();
        if (senderObj) {
            CustomQWidgetSingleDeviceInfo *t_single_device_info_
                = qobject_cast<CustomQWidgetSingleDeviceInfo *>(senderObj);
            if (t_single_device_info_) {
                // 更新一下 device_selection_main_page 中的设备信息，方便 mainwindows 去获取
                clp_device_selection_mainPage_->setCl_selected_device_information (t_single_device_info_->cl_device_info_);

                QPoint topLeft = t_single_device_info_->cl_device_pixmap_label_->mapToGlobal(
                    QPoint(0, 0));
                QRect globalRect(topLeft, t_single_device_info_->cl_device_pixmap_label_->size());
                emit imageSelected(t_single_device_info_->cl_device_info_, globalRect);
            }
        }
    }
}

void DeviceSel::InitUIInformation()
{
    {
        // 设备选择区域
        clp_device_selection_mainPage_ = new DeviceSelectionMainPage(ui->widget_device_section_page);
        ui->widget_device_section_page->layout()->addWidget(clp_device_selection_mainPage_);
        ui->lab_empty->setStyleSheet("border-image: url(:/Skin/Images/GeneralIcon/Empty/DeviceEmpty.png);");
        ui->lab_warning->setStyleSheet(
            QString(
                "color: rgba(161, 168, 179, 0.3);"
                "font-family: \"Noto Sans S Chinese\"; "
                "font-weight: 500;"
                "font-size: 44px;"
                )
            );
    }
}

void DeviceSel::InitMember() {}

void DeviceSel::InitConnect() {}

void DeviceSel::UpdateDeviceSelectionMainPage()
{
    {
        qDeleteAll(
            clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_);
        clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.clear();

        // 遍历设备集合
        for (QHash<QString, QHash<QString, QString>>::iterator iter = ERdevs.begin();
             iter != ERdevs.end();
             iter++) {
            QHash<QString, QString> deviceInfo = iter.value();

            QString Name = deviceInfo.value("friendlyName");
            unsigned short VID = deviceInfo.value("vid").toUShort(nullptr, 16);
            unsigned short PID = deviceInfo.value("pid").toUShort(nullptr, 16);

            DeSheng::DeviceInfo t_deviceInfo;
            t_deviceInfo.SelDev_PID = PID;
            t_deviceInfo.SelDev_VID = VID;
            t_deviceInfo.DeviceSysTypeName = Name; ///< 不采用，会带前置序号
            t_deviceInfo.DeviceGuid = iter.key();

            AddTargetDeviceInfo(t_deviceInfo);

            // qDebug() << "=== CreateButton设备信息 ===";
            // qDebug() << "设备名称:" << Name;
            // qDebug() << "VID:" << VID;
            // qDebug() << "PID:" << PID;
            // qDebug() << "================";
            // int isBlue = deviceInfo.value("IsBluetooth").toInt();
            //36f9 f001
            // QString VidPid = deviceInfo.value("vid&pid");
            //if(isBlue == 0)
            {
                // QString VidPid;
                // bool isSupported = false;
                // if(retB)
                // {
                //     //是否支持LHDC(即APO是否支持该设备)
                //     isSupported = emit ApoManager::instance()->requestIsLhdcDeviceSupport(iter.key(), VidPid);
                // }
                // if(isSupported)
                // {
                //     //是否包含，不区分大小写
                //     if (Name.contains("T10", Qt::CaseInsensitive)) {
                //         if (Name.contains("Wireless", Qt::CaseInsensitive)) {
                //             button->setStyleSheet(
                //                 "QPushButton{"
                //                 "border:null;"
                //                 "background-image: "
                //                 "url(:/Skin/Images/DevSel/background.png);"
                //                 "color: rgb(255, 255, 255);"
                //                 "text-align: center;"
                //                 "padding-bottom: 50px;"
                //                 "border-image: "
                //                 "url(:/Skin/Images/DevSel/T10Wireless-bk.png);}"
                //                 "QPushButton:hover{"
                //                 "background-image: "
                //                 "url(:/Skin/Images/DevSel/checked.png);}");
                //             button->setProperty("image",
                //                                 ":/Skin/Images/home/T10Wireless-big.png");
                //         } else {
                //             button->setStyleSheet(
                //                 "QPushButton{"
                //                 "border:null;"
                //                 "background-image: "
                //                 "url(:/Skin/Images/DevSel/background.png);"
                //                 "color: rgb(255, 255, 255);"
                //                 "text-align: center;"
                //                 "padding-bottom: 50px;"
                //                 "border-image: url(:/Skin/Images/DevSel/T10-bk.png);}"
                //                 "QPushButton:hover{"
                //                 "background-image: "
                //                 "url(:/Skin/Images/DevSel/checked.png);}");
                //             //button->setProperty("image", ":/Skin/Images/DevSel/T10-bk.png");
                //             button->setProperty("image", ":/Skin/Images/home/T10-big.png");
                //         }

                //     } /*else if(Name.contains("K03S",Qt::CaseInsensitive))
                // {
                //     QMessageBox::button->setStyleSheet(
                //         "QPushButton{"
                //         "border:null;"
                //         "background-image: url(:/Skin/Images/DevSel/background.png);"
                //         "color: rgb(255, 255, 255);"
                //         "text-align: center;"
                //         "padding-bottom: 50px;"
                //         "border-image: url(:/Skin/Images/DevSel/K03S-bk.png);}"
                //         "QPushButton:hover{"
                //         "background-image: url(:/Skin/Images/DevSel/checked.png);}");
                //     //button->setProperty("image", ":/Skin/Images/DevSel/K03S-bk.png");
                //     button->setProperty("image", ":/Skin/Images/home/K03S-big.png");
                // }
                // */ else if (Name.contains("K03S", Qt::CaseInsensitive) && PID == 0xF016)
                // {
                //     button->setStyleSheet("QPushButton{"
                //                           "border:null;"
                //                           "background-image: "
                //                           "url(:/Skin/Images/DevSel/background.png);"
                //                           "color: rgb(255, 255, 255);"
                //                           "text-align: center;"
                //                           "padding-bottom: 50px;"
                //                           "border-image: "
                //                           "url(:/Skin/Images/DevSel/K03S-Super-bk.png);}"
                //                           "QPushButton:hover{"
                //                           "background-image: "
                //                           "url(:/Skin/Images/DevSel/checked.png);}");

                //     button->setProperty("image", ":/Skin/Images/home/K03S-Super-big.png");
                // }
                // else if (Name.contains("K06S", Qt::CaseInsensitive))
                // {
                //     button->setStyleSheet(
                //         "QPushButton{"
                //         "border:null;"
                //         "background-image: "
                //         "url(:/Skin/Images/DevSel/background.png);"
                //         "color: rgb(255, 255, 255);"
                //         "text-align: center;"
                //         "padding-bottom: 50px;"
                //         "border-image: url(:/Skin/Images/DevSel/K06S-bk.png);}"
                //         "QPushButton:hover{"
                //         "background-image: "
                //         "url(:/Skin/Images/DevSel/checked.png);}");
                //     //button->setProperty("image", ":/Skin/Images/DevSel/K06S-bk.png");
                //     button->setProperty("image", ":/Skin/Images/home/K06S-big.png");
                // }
                // else if (Name.contains("T7", Qt::CaseInsensitive))
                // {
                //     button->setStyleSheet(
                //         "QPushButton{"
                //         "border:null;"
                //         "background-image: "
                //         "url(:/Skin/Images/DevSel/background.png);"
                //         "color: rgb(255, 255, 255);"
                //         "text-align: center;"
                //         "padding-bottom: 50px;"
                //         "border-image: url(:/Skin/Images/DevSel/T7-bk.png);}"
                //         "QPushButton:hover{"
                //         "background-image: "
                //         "url(:/Skin/Images/DevSel/checked.png);}");
                //     //button->setProperty("image", ":/Skin/Images/DevSel/T7-bk.png");
                //     button->setProperty("image", ":/Skin/Images/home/T7-big.png");
                // }
                // else
                // {
                //     button->deleteLater();
                //     button = NULL;
                //     continue; //其他APO支持的耳机不显示，只开放T10有线和无线，K06S，T7
                //     // button->setStyleSheet("QPushButton{"
                //     //                       "border:null;"
                //     //                       "background-image: url(:/Skin/Images/DevSel/background.png);"
                //     //                       "color: rgb(255, 255, 255);"
                //     //                       "text-align: center;"
                //     //                       "padding-bottom: 50px;"
                //     //                       "border-image: url(:/Skin/Images/DevSel/General-bk.png);}"
                //     //                       "QPushButton:hover{"
                //     //                       "background-image: url(:/Skin/Images/DevSel/checked.png);}"
                //     //                       );
                //     // button->setProperty("image", ":/Skin/Images/home/General-big.png");
                // }

                // // 存储设备信息到按钮属性中
                // button->setProperty("VID", VID);
                // button->setProperty("PID", PID);
                // button->setProperty("DeviceName", Name);
                // button->setProperty("DeviceGuid", iter.key());
                // button->setCheckable(true);

                // 计算当前按钮在网格中的位置，靠左开始显示
            }
        }

        // 可支持设备数量为 0, 显示机型选择界面
        if (clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.size()== 0)
        {
            devSelEn = true; //需要显示机型选择页面,(否则值和之前一样即可)
            ui->stackedWidget->setCurrentWidget(ui->page_DevEmpty);


        } else {
            ui->stackedWidget->setCurrentWidget(ui->page_DevSel);
            clp_device_selection_mainPage_->clp_scrollArea_device_selection_->updateView(); ///< 更新设备滚动区域

            {
                // 右侧按键区域
                clp_device_selection_mainPage_->clp_scrollArea_roundbutton_->updateView();

                // 先同步按键列表
                clp_device_selection_mainPage_->syncRowButtons(); // 按行数同步右侧导航按键

                // 设备列表超过 1 行时才显示右侧按键区域
                if (clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_rowCount_
                    > 1) {
                    clp_device_selection_mainPage_->clp_scrollArea_roundbutton_->raise();
                    clp_device_selection_mainPage_->clp_scrollArea_roundbutton_
                        ->setGeometry(clp_device_selection_mainPage_->rect().width() - 31,
                                      0,
                                      8,
                                      clp_device_selection_mainPage_->rect().height());
                    clp_device_selection_mainPage_->clp_scrollArea_roundbutton_->show();
                } else {
                    clp_device_selection_mainPage_->clp_scrollArea_roundbutton_->hide();
                }
                // 右侧按键区域
                clp_device_selection_mainPage_->clp_scrollArea_roundbutton_->updateView();
            }
        }
    }
}

void DeviceSel::AddTargetDeviceInfo(DeSheng::DeviceInfo deviceInfo)
{
    // T10有线/无线
    if (deviceInfo.DeviceSysTypeName.contains("T10", Qt::CaseInsensitive)) {
        if (deviceInfo.DeviceSysTypeName.contains("Wireless", Qt::CaseInsensitive)) { // T10无线
            CustomQWidgetSingleDeviceInfo *t_device_
                = new CustomQWidgetSingleDeviceInfo(tr("T10无线"), clp_device_selection_mainPage_, false);
            t_device_->updatePushButtonScrollArea();
            t_device_->setCheckedDevice();
            {
                // 更新内部 cl_device_info_ 信息，点击时通过信号槽传递
                t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
                t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
                t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
                t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
            }
            clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_
                .append(t_device_);
            QObject::connect(t_device_,
                             &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                             this,
                             &DeviceSel::HandleDeviceChecked);
        } else {
            //T10有线
            CustomQWidgetSingleDeviceInfo *t_device_
                = new CustomQWidgetSingleDeviceInfo(tr("T10有线"), clp_device_selection_mainPage_, false);
            t_device_->updatePushButtonScrollArea();
            t_device_->setCheckedDevice();
            {
                // 更新内部 cl_device_info_ 信息
                t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
                t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
                t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
                t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
            }
            clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_
                .append(t_device_);
            QObject::connect(t_device_,
                             &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                             this,
                             &DeviceSel::HandleDeviceChecked);
        }
        return;
    }

    // K03S
    if (deviceInfo.DeviceSysTypeName.contains("K03S")) {
        // K03S超竞版
        if (deviceInfo.DeviceSysTypeName.contains("K03S", Qt::CaseInsensitive)
            && ((deviceInfo.SelDev_PID == 0xF016) || (deviceInfo.SelDev_PID == 0xF017)) ) {
            CustomQWidgetSingleDeviceInfo *t_device_
                = new CustomQWidgetSingleDeviceInfo(tr("K03S超竞版"), clp_device_selection_mainPage_, false);
            t_device_->updatePushButtonScrollArea();
            t_device_->setCheckedDevice();
            {
                // 更新内部 cl_device_info_ 信息
                t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
                t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
                t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
                t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
            }
            clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_
                .append(t_device_);
            QObject::connect(t_device_,
                             &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                             this,
                             &DeviceSel::HandleDeviceChecked);
            // K03S 暂时忽略 不加入 clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_ 中进行显示
        } else {
            // CustomQWidgetSingleDeviceInfo *t_device_
            //     = new CustomQWidgetSingleDeviceInfo("K03S", clp_device_selection_mainPage_);
            // t_device_->updatePushButtonScrollArea();
            // t_device_->setCheckedDevice();
            // {
            //     // 更新内部 cl_device_info_ 信息
            //     t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
            //     t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
            //     t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
            // }
            // clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_
            //     .append(t_device_);
            // QObject::connect(t_device_,
            //                  &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
            //                  this,
            //                  &DeviceSel::HandleDeviceChecked);
        }
        return;
    }

    // K03有线版二代
    if (deviceInfo.DeviceSysTypeName.contains("K03")&& deviceInfo.SelDev_PID == 0xE003)
    {
      CustomQWidgetSingleDeviceInfo *t_device_ = new CustomQWidgetSingleDeviceInfo("K03有线版", clp_device_selection_mainPage_, true);
      t_device_->updatePushButtonScrollArea();
      t_device_->setCheckedDevice();
      {
        // 更新内部 cl_device_info_ 信息
        t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
        t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
        t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
        t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
      }
      clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.append(t_device_);
      QObject::connect(t_device_,
                       &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                       this,
                       &DeviceSel::HandleDeviceChecked);

      return;
    }

    // K06S
    if (deviceInfo.DeviceSysTypeName.contains("K06S")) {
        CustomQWidgetSingleDeviceInfo *t_device_
            = new CustomQWidgetSingleDeviceInfo("K06S", clp_device_selection_mainPage_, false);
        t_device_->updatePushButtonScrollArea();
        t_device_->setCheckedDevice();
        {
            // 更新内部 cl_device_info_ 信息
            t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
            t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
            t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
            t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
        }
        clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.append(
            t_device_);
        QObject::connect(t_device_,
                         &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                         this,
                         &DeviceSel::HandleDeviceChecked);
        return;
    }

    // T7
    if (deviceInfo.DeviceSysTypeName.contains("T7") && (deviceInfo.SelDev_PID == 0xF014 || deviceInfo.SelDev_PID == 0xF008)) {
        CustomQWidgetSingleDeviceInfo *t_device_
            = new CustomQWidgetSingleDeviceInfo("T7", clp_device_selection_mainPage_, true);
        t_device_->updatePushButtonScrollArea();
        t_device_->setCheckedDevice();
        {
            // 更新内部 cl_device_info_ 信息
            t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
            t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
            t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
            t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
        }
        clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.append(t_device_);
        QObject::connect(t_device_,
                         &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                         this,
                         &DeviceSel::HandleDeviceChecked);
        return;
    }

    // T7 GT
    if (deviceInfo.DeviceSysTypeName.contains("T7 GT") && (deviceInfo.SelDev_PID == 0xF015 || deviceInfo.SelDev_PID == 0xF009)) {
        CustomQWidgetSingleDeviceInfo *t_device_
            = new CustomQWidgetSingleDeviceInfo("T7 GT", clp_device_selection_mainPage_, false);
        t_device_->updatePushButtonScrollArea();
        t_device_->setCheckedDevice();
        {
            // 更新内部 cl_device_info_ 信息
            t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
            t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
            t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
            t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
        }
        clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.append(
            t_device_);
        QObject::connect(t_device_,
                         &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                         this,
                         &DeviceSel::HandleDeviceChecked);
        return;
    }

    // S21无线智充版
    if (deviceInfo.DeviceSysTypeName.contains("S21") && deviceInfo.SelDev_PID == 0xC001) {
        CustomQWidgetSingleDeviceInfo *t_device_
            = new CustomQWidgetSingleDeviceInfo("S21无线智充版", clp_device_selection_mainPage_, false);
        t_device_->updatePushButtonScrollArea();
        t_device_->setCheckedDevice();
        {
            // 更新内部 cl_device_info_ 信息
            t_device_->cl_device_info_.SelDev_PID = deviceInfo.SelDev_PID;
            t_device_->cl_device_info_.SelDev_VID = deviceInfo.SelDev_VID;
            t_device_->cl_device_info_.DeviceGuid = deviceInfo.DeviceGuid;
            t_device_->cl_device_info_.DeviceSysTypeName = deviceInfo.DeviceSysTypeName;
        }
        clp_device_selection_mainPage_->clp_scrollArea_device_selection_->cl_all_device_list_.append(
            t_device_);
        QObject::connect(t_device_,
                         &CustomQWidgetSingleDeviceInfo::sendSignalsDeviceInfo,
                         this,
                         &DeviceSel::HandleDeviceChecked);
        return;
    }
}

// void DeviceSel::ButtonGroup_buttonClicked(QAbstractButton *button)
// //void DeviceSel::ButtonGroup_buttonToggled(QAbstractButton *button,bool checked)
// {
//     //if(checked)
//     {
//         SelDev_VID = button->property("VID").toUInt();
//         SelDev_PID = button->property("PID").toUInt();
//         SelDev_DeviceName = button->property("DeviceName").toString();
//         SelDev_DeviceGuid = button->property("DeviceGuid").toString();

//         writeDevIni();

//         // qDebug() << "\n=== 设备信息 ===";
//         // qDebug() << "设备名称:" << SelDev_DeviceName;
//         // qDebug() << "VID:" << SelDev_VID;
//         // qDebug() << "PID:" << SelDev_PID;
//         // qDebug() << "================";

//         emit ApoManager::instance()->requestlogWithTime("\n=== 选择设备的信息 ===");
//         emit ApoManager::instance()->requestlogWithTime(
//             QString("设备名称:%1").arg(SelDev_DeviceName));
//         emit ApoManager::instance()->requestlogWithTime(QString("VID:%1").arg(SelDev_VID));
//         emit ApoManager::instance()->requestlogWithTime(QString("PID:%1").arg(SelDev_PID));
//         emit ApoManager::instance()->requestlogWithTime("================");

//         QString load;
//         //QPoint globalPos = button->mapToGlobal(QPoint(0, 0));
//         // 将按钮的位置转换为屏幕坐标
//         QPoint topLeft = button->mapToGlobal(QPoint(0, 0));
//         QRect globalRect(topLeft, button->size());
//         load = button->property("image").toString();
//         //load = ":/Skin/Images/DevSel/General-bk.png";
//         emit imageSelected(load, globalRect);
//     }
// }

int DeviceSel::writeDevIni()
{
    QSettings settings(fP_HasOpened, QSettings::IniFormat);
    //VID
    settings.setValue("Device/VID", QString::number(SelDev_VID, 16));
    //PID
    settings.setValue("Device/PID", QString::number(SelDev_PID, 16));
    //Name
    settings.setValue("Device/Name", SelDev_DeviceName);

    settings.sync(); //立即写入

    return 1;
}

/*void DeviceSel::on_pBt_SelOK_clicked()
{
    SelDev_VID = DevGroup->checkedButton()->property("VID").toUInt();
    SelDev_PID = DevGroup->checkedButton()->property("PID").toUInt();
    SelDev_DeviceName = DevGroup->checkedButton()->property("DeviceName").toString();

    qDebug() << "=== 设备信息 ===";
    qDebug() << "设备名称:" << SelDev_DeviceName;
    qDebug() << "VID:" << SelDev_VID;
    qDebug() << "PID:" << SelDev_PID;
    qDebug() << "================";

    writeDevIni();

    emit SelDevOk();

}*/
