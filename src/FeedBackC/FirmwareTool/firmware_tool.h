#ifndef FIRMWARE_TOOL_H
#define FIRMWARE_TOOL_H

#include <windows.h> // Windows API

#include <cstring> // std::memset



// 定义函数指针类型
typedef int(__stdcall *UsbCli_Open_t)(const char *path);
typedef int(__stdcall *UsbCli_Close_t)();
typedef int(__stdcall *UsbCli_GetDllVersion_t)();
typedef int(__stdcall *UsbCli_HidConnect_t)(unsigned short vid,
                                            unsigned short pid,
                                            unsigned char param3,
                                            unsigned char param4);
typedef int(__stdcall *UsbCli_HidDisConnect_t)();
typedef int(__stdcall *UsbCli_HidExportDeviceInfo_t)(unsigned char *buffer,
                                                     unsigned int bufferSize,
                                                     unsigned int *bytesReturned);
typedef int(__stdcall *UsbCli_HidGetDeviceInfo_t)();
typedef int(__stdcall *UsbCli_HidIsConnected_t)(unsigned char *connected);

///目前用不上
// typedef int(__stdcall *UsbCli_HidGetFuncSupp_t)(SA9762_FUNC_SUPP_T *funcSupp);
// typedef int(__stdcall *UsbCli_HidGetEffectStatus_t)(SA9762_Effect_status *effectStatus);
// typedef int(__stdcall *UsbCli_HidSetEffectStatus_t)(SA9762_Effect_status *effectStatus);

typedef int(__stdcall *UsbCli_ProcessFirmware_t)(const wchar_t *filePath,
                                                 unsigned char *buffer,
                                                 unsigned int bufferSize,
                                                 unsigned int *bytesWritten,
                                                 unsigned char *chipId,
                                                 unsigned short *vid,
                                                 unsigned short *pid,
                                                 unsigned char *fwVer,
                                                 unsigned char *libVer);
typedef int(__stdcall *UsbCli_ReleaseResource_t)();
typedef int(__stdcall *UsbCli_Request_t)(int, unsigned char *, int, unsigned char *, int);
#include <iostream>

#pragma pack(push, 1) // 确保与 C# StructLayout 对齐一致

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdint>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct FirmwareProcessInfoC
{
    const wchar_t *pszFwFilePathSrc;
    unsigned int uiFwBufBytes;
    unsigned char *pucFwBuf;
    unsigned int uiFwBufWriteBytes;
    unsigned char ucChipId;
    unsigned short ucUsbVendorId;
    unsigned short ucUsbProductId;
    unsigned char *puchFwVer;
    unsigned char *pucLibVer;

    static FirmwareProcessInfoC Create(const wchar_t *firmwarePath, uint32_t bufferBytes)
    {
        FirmwareProcessInfoC info;

        info.pszFwFilePathSrc = firmwarePath;
        info.uiFwBufBytes = bufferBytes;

        // 分配 buffer
        info.pucFwBuf = new uint8_t[bufferBytes];
        std::memset(info.pucFwBuf, 0, bufferBytes);

        info.uiFwBufWriteBytes = 0;
        info.ucChipId = 0;
        info.ucUsbVendorId = 0;
        info.ucUsbProductId = 0;

        info.puchFwVer = new uint8_t[4]{0};
        info.pucLibVer = new uint8_t[4]{0};

        return info;
    }
    void Destroy()
    {
        delete[] pucFwBuf;
        delete[] puchFwVer;
        delete[] pucLibVer;

        pucFwBuf = nullptr;
        puchFwVer = nullptr;
        pucLibVer = nullptr;
    }
} FirmwareProcessInfoC;

#ifdef __cplusplus
}
#endif

#pragma pack(pop)



#include <QtDebug>

#include <QMessageBox>
#include <QCoreApplication.h>
#include <QDebug.h>
#include <QFileDialog.h>
#include <QLibrary.h>

class FirmwareTool : public QObject
{
    Q_OBJECT
public:
    FirmwareTool(QObject *parent = nullptr);
    ~FirmwareTool();

    HINSTANCE cl_act_hid_lib_; ///< windowsAPI返回

    ///对应的函数指针
    UsbCli_Open_t pfn_Usb_Open;
    UsbCli_Close_t pfn_Usb_Close;
    UsbCli_GetDllVersion_t pfn_Usb_GetDllVersion;
    UsbCli_HidConnect_t pfn_Usb_HidConnect;
    UsbCli_HidDisConnect_t pfn_Usb_HidDisConnect;
    UsbCli_HidExportDeviceInfo_t pfn_Usb_HidExportDeviceInfo;
    UsbCli_HidGetDeviceInfo_t pfn_Usb_HidGetDeviceInfo;
    UsbCli_HidIsConnected_t pfn_Usb_HidIsConnected;
    // UsbCli_HidGetFuncSupp_t pfn_Usb_HidGetFuncSupp;
    // UsbCli_HidGetEffectStatus_t pfn_Usb_HidGetEffectStatus;
    // UsbCli_HidSetEffectStatus_t pfn_Usb_HidSetEffectStatus;
    UsbCli_ProcessFirmware_t pfn_Usb_ProcessFirmware;
    UsbCli_ReleaseResource_t pfn_Usb_ReleaseResource;
    UsbCli_Request_t pfn_Usb_Request;

    bool openFWFile();  ///打开升级文件

    bool GetDeviceFirmwareVersion(unsigned short vid,
                                  unsigned short pid,
                                  QString &fwVersion,
                                  QString &libVersion);
};

#endif // FIRMWARE_TOOL_H
