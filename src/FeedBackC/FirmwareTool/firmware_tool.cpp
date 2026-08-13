#include "FeedBackC/FirmwareTool/firmware_tool.h"
#include <QMessageBox>

FirmwareTool::FirmwareTool(QObject *parent)
{
    QString dllPath = QCoreApplication::applicationDirPath()
                      + "/UsbCliBridge.dll"; ///第三方dll路径
    qDebug() << "加载 DLL 路径:" << dllPath;

    cl_act_hid_lib_ = LoadLibraryW(dllPath.toStdWString().c_str());
    if (!cl_act_hid_lib_) {
        // QMessageBox::critical(this, "错误", "无法加载 UsbCliBridge.dll");
        qDebug() << "DLL 加载失败";
        // return;
    }
    qDebug() << "DLL 加载成功";

    // 导出对应的函数
    pfn_Usb_Open = (UsbCli_Open_t) GetProcAddress(cl_act_hid_lib_, "_UsbCli_Open@4");
    pfn_Usb_Close = (UsbCli_Close_t) GetProcAddress(cl_act_hid_lib_, "_UsbCli_Close@0");
    pfn_Usb_GetDllVersion = (UsbCli_GetDllVersion_t)
        GetProcAddress(cl_act_hid_lib_, "_UsbCli_GetDllVersion@0"); ///正常
    pfn_Usb_HidConnect = (UsbCli_HidConnect_t) GetProcAddress(cl_act_hid_lib_,
                                                              "_UsbCli_HidConnect@16"); ///正常
    pfn_Usb_HidDisConnect = (UsbCli_HidDisConnect_t) GetProcAddress(cl_act_hid_lib_,
                                                                    "_UsbCli_HidDisConnect@0");
    pfn_Usb_HidExportDeviceInfo = (UsbCli_HidExportDeviceInfo_t)
        GetProcAddress(cl_act_hid_lib_, "_UsbCli_HidExportDeviceInfo@12");
    pfn_Usb_HidGetDeviceInfo = (UsbCli_HidGetDeviceInfo_t)
        GetProcAddress(cl_act_hid_lib_, "_UsbCli_HidGetDeviceInfo@0");
    pfn_Usb_HidIsConnected = (UsbCli_HidIsConnected_t) GetProcAddress(cl_act_hid_lib_,
                                                                      "_UsbCli_HidIsConnected@4");
    // pfn_Usb_HidGetFuncSupp = (UsbCli_HidGetFuncSupp_t) GetProcAddress(cl_act_hid_lib_,
    //                                                                   "_UsbCli_HidGetFuncSupp@4");
    // pfn_Usb_HidGetEffectStatus = (UsbCli_HidGetEffectStatus_t)
    //     GetProcAddress(cl_act_hid_lib_, "_UsbCli_HidGetEffectStatus@4");
    // pfn_Usb_HidSetEffectStatus = (UsbCli_HidSetEffectStatus_t)
    //     GetProcAddress(cl_act_hid_lib_, "_UsbCli_HidSetEffectStatus@4");
    pfn_Usb_ProcessFirmware = (UsbCli_ProcessFirmware_t)
        GetProcAddress(cl_act_hid_lib_, "_UsbCli_ProcessFirmware@36");
    pfn_Usb_ReleaseResource = (UsbCli_ReleaseResource_t)
        GetProcAddress(cl_act_hid_lib_, "_UsbCli_ReleaseResource@0");
    pfn_Usb_Request = (UsbCli_Request_t) GetProcAddress(cl_act_hid_lib_, "_UsbCli_Request@20");

    // 检查关键函数
    if (!pfn_Usb_ProcessFirmware || !pfn_Usb_Open || !pfn_Usb_HidConnect) {
        qDebug() << "必要函数导出失败";
    }
}

FirmwareTool::~FirmwareTool() {
    // 释放 DLL
    if (cl_act_hid_lib_) {
        FreeLibrary(cl_act_hid_lib_);
        cl_act_hid_lib_ = nullptr;
    }

    // 清空函数指针（防止野指针误用）
    pfn_Usb_Open = nullptr;
    pfn_Usb_Close = nullptr;
    pfn_Usb_GetDllVersion = nullptr;

    pfn_Usb_HidConnect = nullptr;
    pfn_Usb_HidDisConnect = nullptr;
    pfn_Usb_HidExportDeviceInfo = nullptr;
    pfn_Usb_HidGetDeviceInfo = nullptr;
    pfn_Usb_HidIsConnected = nullptr;

    pfn_Usb_ProcessFirmware = nullptr;
    pfn_Usb_ReleaseResource = nullptr;
    pfn_Usb_Request = nullptr;
}

bool FirmwareTool::openFWFile()
{
    // ///固定文件路径
    // QString TestPath = "D:\\QT5WorkSpace\\LoadT10Lib\\out\\SA9012_SV1052_T10_XIBERIA_V0013."
    //                    "H01.0.21_20251121.SOT";

    QString TestPath = QFileDialog::getOpenFileName(nullptr,
                                                    tr("选择固件文件"),
                                                    ".",
                                                    tr("SOT 文件 (*.SOT)"));
    if (TestPath.isEmpty()) {
        qDebug() << "打开文件失败";
        return false;
    }
    // 转成 UTF-16
    std::wstring wpath = TestPath.toStdWString();
    // 用于传给 DLL
    const wchar_t *filePath = wpath.c_str();
    unsigned int bufferSize = 0x400000; // 4MB

    FirmwareProcessInfoC info = FirmwareProcessInfoC::Create(filePath, bufferSize);

    // 调用 pfn_Usb_ProcessFirmware 函数
    int ret = pfn_Usb_ProcessFirmware(info.pszFwFilePathSrc,
                                      info.pucFwBuf,
                                      info.uiFwBufBytes,
                                      &info.uiFwBufWriteBytes,
                                      &info.ucChipId,
                                      &info.ucUsbVendorId,
                                      &info.ucUsbProductId,
                                      info.puchFwVer,
                                      info.pucLibVer);

    qDebug() << "pfn_Usb_ProcessFirmware 处理结果:" << ret;
    if (ret == 0) {
        qDebug() << "芯片 ID:" << info.ucChipId;
        qDebug() << "VID:" << QString::number(info.ucUsbVendorId, 16);
        qDebug() << "PID:" << QString::number(info.ucUsbProductId, 16);
        qDebug() << "写入字节数:" << info.uiFwBufWriteBytes;
    }

    // 销毁结构体
    info.Destroy();
    return (ret == 0);
}

bool FirmwareTool::GetDeviceFirmwareVersion(unsigned short vid,
                                            unsigned short pid,
                                            QString &fwVersion,
                                            QString &libVersion)
{
    // HID 连接
    int ret = pfn_Usb_HidConnect(vid, pid, 0x8C, 0x02);
    if (ret != 0) {
        qDebug() << "HidConnect 失败:" << ret;
        pfn_Usb_Close();
        return false;
    }

    // 确认连接状态
    unsigned char connected = 0;
    ret = pfn_Usb_HidIsConnected(&connected);
    if (ret != 0 || connected == 0) {
        qDebug() << "设备未连接";
        pfn_Usb_HidDisConnect();
        pfn_Usb_Close();
        return false;
    }

    // 获取设备信息
    ret = pfn_Usb_HidGetDeviceInfo();
    if (ret != 0) {
        qDebug() << "HidGetDeviceInfo 失败:" << ret;
    } else {
        qDebug() << "HidGetDeviceInfo 成功，等待设备填充数据...";
        Sleep(50); // 等待 50 毫秒
    }

    // 导出设备详细信息（这里可以拿到版本号）
    const unsigned int DEV_INFO_SIZE = 568; ///不低于568
    unsigned char *devInfoBuffer = new unsigned char[DEV_INFO_SIZE];
    memset(devInfoBuffer, 0, DEV_INFO_SIZE);
    unsigned int bytesReturned = 0;

    ret = pfn_Usb_HidExportDeviceInfo(devInfoBuffer, DEV_INFO_SIZE, &bytesReturned);

    if (ret == 0) {
        // 偏移量：库版本 0x20，固件版本 0x24
        unsigned char *lib = devInfoBuffer + 0x20;
        unsigned char *fw = devInfoBuffer + 0x24;

        // 版本号存储在 [0] 和 [2] 位置
        int libMajor = lib[0];
        int libMinor = lib[2];
        int fwMajor = fw[0];
        int fwMinor = fw[2];

        fwVersion = QString("%1.%2").arg(fwMajor, 4, 16, QChar('0')).arg(fwMinor, 4, 16, QChar('0'));
        libVersion
            = QString("%1.%2").arg(libMajor, 4, 16, QChar('0')).arg(libMinor, 4, 16, QChar('0'));

        // qDebug() << "设备固件版本:" << fwVersion;
        // qDebug() << "设备库版本:" << libVersion;
    }

    delete[] devInfoBuffer;

    // 看是否需要断开
    pfn_Usb_HidDisConnect();
    pfn_Usb_Close();

    return (ret == 0);
}
