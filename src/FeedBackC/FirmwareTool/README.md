# FirmwareTool — USB 设备固件升级

## 概述

动态加载第三方 Windows DLL `UsbCliBridge.dll`，封装其 HID 固件升级函数。提供从设备读取固件版本的能力。烧录固件文件（`openFWFile()`）已有实现但无调用点（零调用死代码）。

## 文件

| 文件 | 职责 |
|------|------|
| `firmware_tool.h/cpp` | `FirmwareTool` 类 + C 结构体 `FirmwareProcessInfoC` |

## FirmwareTool

- **基类**：`QObject`
- **线程**：主线程，同步阻塞（`Sleep(50)` 等设备数据）

### API

| 方法 | 说明 |
|------|------|
| `openFWFile()` | 打开文件对话框选 `.SOT` 文件 → 调用 `pfn_Usb_ProcessFirmware` 烧录。**当前零调用者，为死代码。** |
| `GetDeviceFirmwareVersion(vid, pid, fwVersion, libVersion)` | 通过 VID/PID 连接 HID 设备：`HidConnect` → `HidIsConnected` → `HidGetDeviceInfo` → `Sleep(50)` → `HidExportDeviceInfo`，从 offset 0x20（库版本）和 0x24（固件版本）解析版本号字符串 |

### 函数指针（11 个）

从 `UsbCliBridge.dll` 动态解析，均为 public 成员：

| # | 指针 | 签名 |
|---|------|------|
| 1 | `pfn_Usb_Open` | `UsbCli_Open_t` |
| 2 | `pfn_Usb_Close` | `UsbCli_Close_t` |
| 3 | `pfn_Usb_GetDllVersion` | `UsbCli_GetDllVersion_t` |
| 4 | `pfn_Usb_HidConnect` | `UsbCli_HidConnect_t` |
| 5 | `pfn_Usb_HidDisConnect` | `UsbCli_HidDisConnect_t` |
| 6 | `pfn_Usb_HidExportDeviceInfo` | `UsbCli_HidExportDeviceInfo_t` |
| 7 | `pfn_Usb_HidGetDeviceInfo` | `UsbCli_HidGetDeviceInfo_t` |
| 8 | `pfn_Usb_HidIsConnected` | `UsbCli_HidIsConnected_t` |
| 9 | `pfn_Usb_ProcessFirmware` | `UsbCli_ProcessFirmware_t` |
| 10 | `pfn_Usb_ReleaseResource` | `UsbCli_ReleaseResource_t` |
| 11 | `pfn_Usb_Request` | `UsbCli_Request_t` |

## FirmwareProcessInfoC

C 结构体，持有固件处理缓冲区和元数据（`pucFwBuf`、`puchFwVer`、`pucLibVer` 等）。提供 `Create(firmwarePath, bufferBytes)` / `Destroy()` 工厂方法，内部用 `new[]`/`delete[]` 管理堆内存。

## 依赖

- Windows API：`LoadLibraryW`、`GetProcAddress`、`FreeLibrary`、`Sleep`
- 第三方 DLL：`UsbCliBridge.dll`（从 `QCoreApplication::applicationDirPath()` 加载，非 `wmfengine.dll`）
- Qt：`QObject`、`QFileDialog`、`QCoreApplication`、`QLibrary`

## 命名规范

- 部分遵循 `cl_` 约定（`cl_act_hid_lib_`）
- 函数指针用 `pfn_Usb_` 前缀
- 结构体用 C 风格 `FirmwareProcessInfoC`

## 已知问题

- `openFWFile()` 实现完整但零调用者，为死代码。实际固件升级入口可能在其他模块（如 `UpdateOTA` 页面），未经过本类。
