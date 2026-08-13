# dlls/ — 项目自有 DLL 依赖

本目录存放 **XIBERIA X HUB 项目自有的 DLL 依赖文件**（非第三方开源库，属于 Savitech 硬件方案的一部分）。

> 第三方开源库（FFmpeg、spdlog、VLC）的头文件/库文件放在 `third_party_library/` 的其他子目录，与本目录区分。

## 文件清单

| 文件名 | 源码引用 | 说明 |
|--------|---------|------|
| `ActHID.dll` | 有（`src/LoadLib.cpp`、`src/FeedBackC/FirmwareTool/firmware_tool.cpp`） | HID 设备 I/O（设备读写、固件下发） |
| `hidapi.dll` | 有（`src/LoadLib.cpp`） | USB HID 枚举 |
| `Savitech3darEngineApo.dll` | 有（`src/LoadApoDLL.cpp`） | APO 音频 DSP 引擎 |
| `wmfengine.dll` | 无 | 仅随项目分发，未在源码中直接引用 |

## 说明

- 前三个 DLL 为运行时 `LoadLibrary` 动态加载（无编译期链接依赖），运行时可执行文件目录需包含这些 DLL
- `wmfengine.dll` 源码无直接引用，仅做迁移保管
- 32 位（x86）构建，勿混用 64 位 DLL
