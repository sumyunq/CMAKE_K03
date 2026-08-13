#include "APOThread/ApoManager.h"
#include "APOThread/ApoWorker.h"
#include "LoadApoDLL.h"

ApoManager* ApoManager::m_instance = nullptr;

ApoManager::ApoManager(QObject *parent)
    : QObject(parent), m_worker(nullptr), m_workerThread(new QThread(this))
{
    // 注册复杂类型用于跨线程传递
    qRegisterMetaType<QVector<double>>("QVector<double>");
    qRegisterMetaType<QVector<bool>>("QVector<bool>");
}

ApoManager* ApoManager::instance()
{
    if (!m_instance)
        m_instance = new ApoManager();
    return m_instance;
}

void ApoManager::start()
{
    if (m_worker) return;

    m_worker = new ApoWorker();
    m_worker->setApo(apo);   // 把 apo 交给 worker
    m_worker->moveToThread(m_workerThread);

    // 将 Manager 的信号连接到 Worker 的槽（跨线程自动使用 QueuedConnection）
    connect(this, &ApoManager::requestSetProcessEffectOption, m_worker, &ApoWorker::setProcessEffectOption);
    connect(this, &ApoManager::requestSetLhdcDevice, m_worker, &ApoWorker::setLhdcDevice);
    connect(this, &ApoManager::requestSetRenderState, m_worker, &ApoWorker::setRenderState);
    connect(this, &ApoManager::requestSetArEffectState, m_worker, &ApoWorker::setArEffectState);

    connect(this, &ApoManager::requestSetSurroundState, m_worker, &ApoWorker::setSurroundState);
    connect(this, &ApoManager::requestSetDistance, m_worker, &ApoWorker::setDistance);
    connect(this, &ApoManager::requestSetReverbState, m_worker, &ApoWorker::setReverbState);
    connect(this, &ApoManager::requestSetReverbFilter, m_worker, &ApoWorker::setReverbFilter);
    connect(this, &ApoManager::requestSetReverbActivateRoomType, m_worker, &ApoWorker::setReverbActivateRoomType);
    connect(this, &ApoManager::requestSetArReverbRatio, m_worker, &ApoWorker::setArReverbRatio);

    connect(this, &ApoManager::requestSetBassBoostState, m_worker, &ApoWorker::setBassBoostState);
    connect(this, &ApoManager::requestSetCompBassGain, m_worker, &ApoWorker::setCompBassGain);
    connect(this, &ApoManager::requestSetCompBassCenterFrequency, m_worker, &ApoWorker::setCompBassCenterFrequency);
    connect(this, &ApoManager::requestSetBassBoostGain, m_worker, &ApoWorker::setBassBoostGain);

    connect(this, &ApoManager::requestSetDrcState, m_worker, &ApoWorker::setDrcState);
    connect(this, &ApoManager::requestSetDrcThreshold, m_worker, &ApoWorker::setDrcThreshold);
    connect(this, &ApoManager::requestSetDrcRatio, m_worker, &ApoWorker::setDrcRatio);
    connect(this, &ApoManager::requestSetDrcAttackTime, m_worker, &ApoWorker::setDrcAttackTime);
    connect(this, &ApoManager::requestSetDrcReleaseTime, m_worker, &ApoWorker::setDrcReleaseTime);
    connect(this, &ApoManager::requestSetDrcMakeupEnable, m_worker, &ApoWorker::setDrcMakeupEnable);
    connect(this, &ApoManager::requestSetDrcInputGain, m_worker, &ApoWorker::setDrcInputGain);
    connect(this, &ApoManager::requestSetDrcOutputGain, m_worker, &ApoWorker::setDrcOutputGain);
    connect(this, &ApoManager::requestSetDrcLimiterEnable, m_worker, &ApoWorker::setDrcLimiterEnable);
    connect(this, &ApoManager::requestSetDrcLimiterThreshold, m_worker, &ApoWorker::setDrcLimiterThreshold);

    connect(this, &ApoManager::requestSetGlobalInputGainDb, m_worker, &ApoWorker::setGlobalInputGainDb);

    connect(this, &ApoManager::requestSetExtendEqState, m_worker, &ApoWorker::setExtendEqState);
    connect(this, &ApoManager::requestSetExtendEqualizerGain, m_worker, &ApoWorker::setExtendEqualizerGain);
    connect(this, &ApoManager::requestSetExtendEqualizerGainEx, m_worker, &ApoWorker::setExtendEqualizerGainEx);
    connect(this, &ApoManager::requestSetExtendEqualizerCenterFrequency, m_worker, &ApoWorker::setExtendEqualizerCenterFrequency);
    connect(this, &ApoManager::requestSetExtendEqualizerCenterFrequencyEx, m_worker, &ApoWorker::setExtendEqualizerCenterFrequencyEx);
    connect(this, &ApoManager::requestSetExtendEqualizerBandEnable, m_worker, &ApoWorker::setExtendEqualizerBandEnable);
    connect(this, &ApoManager::requestSetExtendEqualizerBandEnableEx, m_worker, &ApoWorker::setExtendEqualizerBandEnableEx);
    connect(this, &ApoManager::requestSetExtendEqualizerBandQuality, m_worker, &ApoWorker::setExtendEqualizerBandQuality);
    connect(this, &ApoManager::requestSetExtendEqualizerBandQualityEx, m_worker, &ApoWorker::setExtendEqualizerBandQualityEx);
    connect(this, &ApoManager::requestResetExtendEqualizerSetting, m_worker, &ApoWorker::resetExtendEqualizerSetting);

    connect(this, &ApoManager::requestSetExtendEqualizerBandFilter, m_worker, &ApoWorker::SetExtendEqualizerBandFilter);
    connect(this, &ApoManager::requestSetExtendEqualizerBandFilterEx, m_worker, &ApoWorker::SetExtendEqualizerBandFilterEx);

    connect(this, &ApoManager::requestSetVocalEffectsEnable, m_worker, &ApoWorker::setVocalEffectsEnable);
    connect(this, &ApoManager::requestSetRichVocalsEnable, m_worker, &ApoWorker::setRichVocalsEnable);
    connect(this, &ApoManager::requestSetAINSEnable, m_worker, &ApoWorker::setAINSEnable);
    connect(this, &ApoManager::requestSetAINSLevel, m_worker, &ApoWorker::setAINSLevel);

    connect(this, &ApoManager::requestlogWithTime, m_worker, &ApoWorker::logWithTime);

    // Worker 随线程结束自动删除
    connect(m_workerThread, &QThread::finished, this, []() {
        delete apo;
    });

    m_workerThread->start();
}

void ApoManager::stop()
{
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

ApoManager::~ApoManager()
{
    stop();
}
