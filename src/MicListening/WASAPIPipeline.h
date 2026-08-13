//WASAPI 管道实现
#ifndef WASAPIPIPELINE_H
#define WASAPIPIPELINE_H

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <wrl/client.h>
#include <atomic>
#include <thread>

using Microsoft::WRL::ComPtr;

class WASAPIPipeline
{
public:
    WASAPIPipeline();
    ~WASAPIPipeline();

    bool initialize();
    void start();
    void stop();

private:
    void audioLoop();
    bool initWithFormat(const WAVEFORMATEX* format);

    ComPtr<IMMDevice> m_captureDevice;
    ComPtr<IMMDevice> m_renderDevice;
    ComPtr<IAudioClient> m_captureClient;
    ComPtr<IAudioClient> m_renderClient;
    ComPtr<IAudioCaptureClient> m_capture;
    ComPtr<IAudioRenderClient> m_render;

    WAVEFORMATEX* m_commonFormat;   // 统一使用的格式
    std::atomic<bool> m_running;
    std::thread m_audioThread;
    bool m_comInitialized;
};

#endif // WASAPIPIPELINE_H
