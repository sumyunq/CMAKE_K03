# APOThread — 音频 DSP 工作线程

## 概述

APO（Audio Processing Object）线程子系统负责与 `Savitech3darEngineApo.dll` 的通信。所有 APO DLL 调用**必须在专用工作线程上执行**，避免阻塞 GUI 主线程。

## 架构

```
MainThread (GUI)                    WorkerThread (APO)
    │                                    │
    │  ApoManager::instance()            │
    │  → requestSetSurroundState(bool)   │
    │  → requestSetBassBoostState(bool)  │
    │  → requestSetDrcState(bool)        │
    │  → ...（42 个 request 信号）        │
    │         │                           │
    │         └──── QueuedConnection ────→ ApoWorker
    │                                      → LoadApoDLL::SetXxx(...)
    │                                      → Savitech3darEngineApo.dll
```

**关键约束**：所有 APO 调用是**发射后不管（fire-and-forget）**，没有同步返回值通道。调用方无法获知操作是否成功。

## 文件

| 文件 | 职责 |
|------|------|
| `ApoManager.h/cpp` | 饿汉单例，主线程 QObject。暴露 42 个 `request*` 信号作为公共 API |
| `ApoWorker.h/cpp` | 工作线程 QObject。接收信号 → 调 `LoadApoDLL` 方法 |

## ApoManager

**单例模式**：饿汉单例（线程安全），类位于全局命名空间

### 生命周期

```cpp
ApoManager::instance().start();  // main.cpp 启动时调用
// 创建 QThread → 创建 ApoWorker → moveToThread → 连接所有信号
ApoManager::instance().stop();   // main.cpp 退出时调用
// quit + wait → 删除 apo（LoadApoDLL*）
```

### 信号接口（公共 API）

| 类别 | 信号 | 参数 |
|------|------|------|
| **设备** | `requestSetLhdcDevice` | `QString deviceGUID` |
| **总开关** | `requestSetProcessEffectOption` | `uint option` 位掩码 |
| **空间音效** | `requestSetSurroundState` | `bool` |
| | `requestSetDistance` | `int` |
| **混响** | `requestSetReverbState` / `requestSetReverbFilter` / `requestSetReverbActivateRoomType` / `requestSetArReverbRatio` | `bool` / `int` / `int` / `double` |
| **低音** | `requestSetBassBoostState` / `requestSetCompBassGain` / `requestSetCompBassCenterFrequency` / `requestSetBassBoostGain` | `bool` / `int` / `double` / `int` |
| **DRC** | `requestSetDrcState` + 9 个参数信号 | `bool` / `int` / `double` |
| **扩展 EQ** | `requestSetExtendEqState(index, bool)` + 9 参数信号 | `uint, bool` |
| **麦克风** | `requestSetVocalEffectsEnable` / `requestSetRichVocalsEnable` / `requestSetAINSEnable` / `requestSetAINSLevel` | `int` ×4 |
| **日志** | `requestlogWithTime` | `QString` |

## ApoWorker

- 所有 slot 方法遵循相同模式：`if (m_apo) m_apo->SetXxx(args...)`
- 析构时 `delete m_apo`

### 潜在问题

- **P0 关机双重释放**：同一 `LoadApoDLL*`（即全局 `apo`）存在三处 `delete`：
  1. `ApoWorker::~ApoWorker()`（`ApoWorker.cpp:11`）：`delete m_apo`
  2. `ApoManager::start()`（`ApoManager.cpp:84`）：`QThread::finished` lambda 内 `delete apo`
  3. `mainwindow.cpp:1608-1624`：`QtConcurrent::run` lambda 内 `delete apo`
  三处均指向同一个 `LoadApoDLL*` 对象，关闭/退出时可能多次释放导致崩溃。

## 依赖

- `LoadApoDLL`：DLL 函数指针封装
- `Savitech3darEngineApo.dll`（37 MB，render APO）
- `SaviUIControl.dll`（capture/mic APO）
