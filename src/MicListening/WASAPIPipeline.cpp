#include "MicListening/WASAPIPipeline.h"
#include <QDebug>
#include <QThread>
#include <vector>

static const char* hresultToString(HRESULT hr) {
    switch (hr) {
    case S_OK:                         return "S_OK";
    case AUDCLNT_E_DEVICE_IN_USE:      return "AUDCLNT_E_DEVICE_IN_USE";
    case AUDCLNT_E_UNSUPPORTED_FORMAT: return "AUDCLNT_E_UNSUPPORTED_FORMAT";
    case AUDCLNT_E_ENDPOINT_CREATE_FAILED: return "AUDCLNT_E_ENDPOINT_CREATE_FAILED";
    case E_POINTER:                    return "E_POINTER";
    case E_INVALIDARG:                 return "E_INVALIDARG";
    default:                           return "Unknown HRESULT";
    }
}

WASAPIPipeline::WASAPIPipeline()
    : m_commonFormat(nullptr)
    , m_running(false)
    , m_comInitialized(false)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        m_comInitialized = true;
    } else if (hr == RPC_E_CHANGED_MODE) {
        qDebug() << "COM already initialized in different mode, continuing...";
    } else {
        qDebug() << "CoInitializeEx failed:" << hr;
    }
}

WASAPIPipeline::~WASAPIPipeline()
{
    stop();
    if (m_commonFormat) {
        CoTaskMemFree(m_commonFormat);
    }
    if (m_comInitialized) {
        CoUninitialize();
    }
}

bool WASAPIPipeline::initialize()
{
    // 创建设备枚举器
    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    if (FAILED(hr)) {
        qDebug() << "CoCreateInstance failed:" << hresultToString(hr);
        return false;
    }

    // 获取默认麦克风和扬声器
    hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &m_captureDevice);
    if (FAILED(hr)) {
        qDebug() << "GetDefaultAudioEndpoint(Capture) failed:" << hresultToString(hr);
        return false;
    }
    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_renderDevice);
    if (FAILED(hr)) {
        qDebug() << "GetDefaultAudioEndpoint(Render) failed:" << hresultToString(hr);
        return false;
    }

    // 定义通用格式列表（优先尝试高兼容性格式）
    std::vector<WAVEFORMATEX> formats;

    // 16位 44100Hz 立体声
    WAVEFORMATEX fmt1 = {};
    fmt1.wFormatTag = WAVE_FORMAT_PCM;
    fmt1.nChannels = 2;
    fmt1.nSamplesPerSec = 44100;
    fmt1.wBitsPerSample = 16;
    fmt1.nBlockAlign = fmt1.nChannels * fmt1.wBitsPerSample / 8;
    fmt1.nAvgBytesPerSec = fmt1.nSamplesPerSec * fmt1.nBlockAlign;
    fmt1.cbSize = 0;
    formats.push_back(fmt1);

    // 16位 48000Hz 立体声
    WAVEFORMATEX fmt2 = fmt1;
    fmt2.nSamplesPerSec = 48000;
    fmt2.nAvgBytesPerSec = fmt2.nSamplesPerSec * fmt2.nBlockAlign;
    formats.push_back(fmt2);

    // 16位 44100Hz 单声道
    WAVEFORMATEX fmt3 = fmt1;
    fmt3.nChannels = 1;
    fmt3.nBlockAlign = fmt3.nChannels * fmt3.wBitsPerSample / 8;
    fmt3.nAvgBytesPerSec = fmt3.nSamplesPerSec * fmt3.nBlockAlign;
    formats.push_back(fmt3);

    // 16位 48000Hz 单声道
    WAVEFORMATEX fmt4 = fmt3;
    fmt4.nSamplesPerSec = 48000;
    fmt4.nAvgBytesPerSec = fmt4.nSamplesPerSec * fmt4.nBlockAlign;
    formats.push_back(fmt4);

    // 依次尝试每个格式，直到两个客户端都初始化成功
    for (const auto& fmt : formats) {
        // 尝试用此格式同时初始化捕获和渲染客户端
        ComPtr<IAudioClient> capClient, rndClient;
        bool capOK = false, rndOK = false;

        // 初始化捕获客户端
        hr = m_captureDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                                       nullptr, (void**)&capClient);
        if (SUCCEEDED(hr)) {
            REFERENCE_TIME hnsBufferDuration = 10 * 10000; // 10ms
            hr = capClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                       AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
                                       hnsBufferDuration, 0, &fmt, nullptr);
            if (SUCCEEDED(hr)) {
                capOK = true;
            } else {
                qDebug() << "Capture init failed for"
                         << fmt.nSamplesPerSec << "Hz," << fmt.nChannels << "ch:"
                         << hresultToString(hr);
            }
        }

        // 初始化渲染客户端（如果捕获成功，继续尝试渲染）
        if (capOK) {
            hr = m_renderDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                                          nullptr, (void**)&rndClient);
            if (SUCCEEDED(hr)) {
                REFERENCE_TIME hnsBufferDuration = 10 * 10000;
                hr = rndClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                           AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM,
                                           hnsBufferDuration, 0, &fmt, nullptr);
                if (SUCCEEDED(hr)) {
                    rndOK = true;
                } else {
                    qDebug() << "Render init failed for"
                             << fmt.nSamplesPerSec << "Hz," << fmt.nChannels << "ch:"
                             << hresultToString(hr);
                }
            }
        }

        if (capOK && rndOK) {
            // 成功！保存客户端和格式
            m_captureClient = capClient;
            m_renderClient = rndClient;
            m_commonFormat = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));
            memcpy(m_commonFormat, &fmt, sizeof(WAVEFORMATEX));
            qDebug() << "Successfully initialized with format:"
                     << m_commonFormat->nSamplesPerSec << "Hz,"
                     << m_commonFormat->nChannels << "ch,"
                     << m_commonFormat->wBitsPerSample << "bits";
            break;
        }
    }

    if (!m_commonFormat) {
        qDebug() << "No common format found for both devices.";
        return false;
    }

    // 获取 IAudioCaptureClient 和 IAudioRenderClient 接口
    HRESULT hrCap = m_captureClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_capture);
    HRESULT hrRnd = m_renderClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_render);
    if (FAILED(hrCap) || FAILED(hrRnd)) {
        qDebug() << "GetService failed. Capture:" << hresultToString(hrCap)
        << "Render:" << hresultToString(hrRnd);
        return false;
    }

    return true;
}

void WASAPIPipeline::start()
{
    if (m_running) return;

    HRESULT hr = m_captureClient->Start();
    if (FAILED(hr)) {
        qDebug() << "Start capture client failed:" << hresultToString(hr);
        return;
    }
    hr = m_renderClient->Start();
    if (FAILED(hr)) {
        qDebug() << "Start render client failed:" << hresultToString(hr);
        m_captureClient->Stop();
        return;
    }

    m_running = true;
    m_audioThread = std::thread(&WASAPIPipeline::audioLoop, this);
}

void WASAPIPipeline::stop()
{
    if (!m_running) return;
    m_running = false;
    if (m_audioThread.joinable())
        m_audioThread.join();

    if (m_captureClient) m_captureClient->Stop();
    if (m_renderClient) m_renderClient->Stop();
}

void WASAPIPipeline::audioLoop()
{
    // 获取缓冲区大小（以帧为单位）
    UINT32 captureBufferFrames = 0;
    UINT32 renderBufferFrames = 0;
    m_captureClient->GetBufferSize(&captureBufferFrames);
    m_renderClient->GetBufferSize(&renderBufferFrames);

    // 每次处理的最小帧数（取较小值）
    UINT32 framesToProcess = min(captureBufferFrames, renderBufferFrames);

    while (m_running) {
        BYTE* captureData = nullptr;
        UINT32 framesAvailable = 0;
        DWORD flags = 0;

        HRESULT hr = m_capture->GetBuffer(&captureData, &framesAvailable, &flags,
                                          nullptr, nullptr);
        if (FAILED(hr)) {
            QThread::msleep(1);
            continue;
        }

        if (framesAvailable > 0 && !(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
            // 确保不超过渲染缓冲区容量
            UINT32 framesToWrite = min(framesAvailable, framesToProcess);
            BYTE* renderData = nullptr;
            hr = m_render->GetBuffer(framesToWrite, &renderData);
            if (SUCCEEDED(hr)) {
                UINT32 bytesToCopy = framesToWrite * m_commonFormat->nBlockAlign;
                memcpy(renderData, captureData, bytesToCopy);
                m_render->ReleaseBuffer(framesToWrite, 0);
            }
        }
        m_capture->ReleaseBuffer(framesAvailable);
    }
}
