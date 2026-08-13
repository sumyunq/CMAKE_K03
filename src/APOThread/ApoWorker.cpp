#include "APOThread/ApoWorker.h"
#include "LoadApoDLL.h"    // 包含完整 LoadApoDLL 声明

ApoWorker::ApoWorker(QObject *parent)
    : QObject(parent), m_apo(nullptr)
{
}

ApoWorker::~ApoWorker()
{
    delete m_apo;
}

void ApoWorker::setApo(LoadApoDLL* apo)
{
    m_apo = apo;
}


// ---------- 实现所有公共槽 ----------
void ApoWorker::setProcessEffectOption(unsigned int option)
{
    if (m_apo) m_apo->SetProcessEffectOption(option);
}
void ApoWorker::setLhdcDevice(const QString &deviceGUID)
{
    if (m_apo) {
        // 需要非 const 参数适配原接口，这里传一份拷贝
        QString guid = deviceGUID;
        m_apo->SetLhdcDevice(guid);
    }
}
void ApoWorker::setRenderState(bool enable)
{
    if (m_apo) m_apo->SetRenderState(enable);
}
void ApoWorker::setArEffectState(bool enable)
{
    if (m_apo) m_apo->SetArEffectState(enable);
}

void ApoWorker::setSurroundState(bool enable)
{
    if (m_apo) m_apo->SetSurroundState(enable);
}
void ApoWorker::setDistance(int scale)
{
    if (m_apo) m_apo->SetDistance(scale);
}
void ApoWorker::setReverbState(bool enable)
{
    if (m_apo) m_apo->SetReverbState(enable);
}

void ApoWorker::setReverbFilter(int roomType)
{
    if (m_apo) m_apo->SetReverbFilter(static_cast<ReverbRoomType>(roomType));
}
void ApoWorker::setReverbActivateRoomType(int nRoomType)
{
    if (m_apo) m_apo->SetReverbActivateRoomType(nRoomType);
}
void ApoWorker::setArReverbRatio(double ratio)
{
    if (m_apo) m_apo->SetArReverbRatio(ratio);
}

void ApoWorker::setBassBoostState(bool enable)
{
    if (m_apo) m_apo->SetBassBoostState(enable);
}
void ApoWorker::setCompBassGain(int value)
{
    if (m_apo) m_apo->SetCompBassGain(value);
}
void ApoWorker::setCompBassCenterFrequency(double freq)
{
    if (m_apo) m_apo->SetCompBassCenterFrequency(freq);
}
void ApoWorker::setBassBoostGain(int value)
{
    if (m_apo) m_apo->SetBassBoostGain(value);
}

void ApoWorker::setDrcState(bool enable)
{
    if (m_apo) m_apo->SetDrcState(enable);
}
void ApoWorker::setDrcThreshold(double threshold)
{
    if (m_apo) m_apo->SetDrcThreshold(threshold);
}
void ApoWorker::setDrcRatio(double ratio)
{
    if (m_apo) m_apo->SetDrcRatio(ratio);
}
void ApoWorker::setDrcAttackTime(double attackTime)
{
    if (m_apo) m_apo->SetDrcAttackTime(attackTime);
}
void ApoWorker::setDrcReleaseTime(double releaseTime)
{
    if (m_apo) m_apo->SetDrcReleaseTime(releaseTime);
}
void ApoWorker::setDrcMakeupEnable(uint makeupEnable)
{
    if (m_apo) m_apo->SetDrcMakeupEnable(makeupEnable);
}
void ApoWorker::setDrcInputGain(double inputGain)
{
    if (m_apo) m_apo->SetDrcInputGain(inputGain);
}
void ApoWorker::setDrcOutputGain(double outputGain)
{
    if (m_apo) m_apo->SetDrcOutputGain(outputGain);
}
void ApoWorker::setDrcLimiterEnable(uint limiterEnable)
{
    if (m_apo) m_apo->SetDrcLimiterEnable(limiterEnable);
}
void ApoWorker::setDrcLimiterThreshold(double limiterThreshold)
{
    if (m_apo) m_apo->SetDrcLimiterThreshold(limiterThreshold);
}

void ApoWorker::setGlobalInputGainDb(int value)
{
    if (m_apo) m_apo->SetGlobalInputGainDb(value);
}

void ApoWorker::setExtendEqState(uint index, bool enable)
{
    if (m_apo) m_apo->SetExtendEqState(index, enable);
}
void ApoWorker::setExtendEqualizerGain(uint index_EQ, uint index_band, double dbValue)
{
    if (m_apo) m_apo->SetExtendEqualizerGain(index_EQ, index_band, dbValue);
}
void ApoWorker::setExtendEqualizerGainEx(uint index_EQ, const QVector<double> &gains)
{
    if (m_apo) {
        QVector<double> g = gains;   // 原函数接受 QVector<double>&，需要非const
        m_apo->SetExtendEqualizerGainEx(index_EQ, g);
    }
}
void ApoWorker::setExtendEqualizerCenterFrequency(uint index_EQ, uint index_band, double freq)
{
    if (m_apo) m_apo->SetExtendEqualizerCenterFrequency(index_EQ, index_band, freq);
}
void ApoWorker::setExtendEqualizerCenterFrequencyEx(uint index_EQ, const QVector<double> &freqs)
{
    if (m_apo) {
        QVector<double> f = freqs;
        m_apo->SetExtendEqualizerCenterFrequencyEx(index_EQ, f);
    }
}
void ApoWorker::setExtendEqualizerBandEnable(uint index_EQ, uint index_band, bool enable)
{
    if (m_apo) m_apo->SetExtendEqualizerBandEnable(index_EQ, index_band, enable);
}
void ApoWorker::setExtendEqualizerBandEnableEx(uint index_EQ, const QVector<bool> &enables)
{
    if (m_apo) {
        QVector<bool> e = enables;
        m_apo->SetExtendEqualizerBandEnableEx(index_EQ, e);
    }
}
void ApoWorker::setExtendEqualizerBandQuality(uint index_EQ, uint index_band, double quality)
{
    if (m_apo) m_apo->SetExtendEqualizerBandQuality(index_EQ, index_band, quality);
}
void ApoWorker::setExtendEqualizerBandQualityEx(uint index_EQ, const QVector<double> &qualities)
{
    if (m_apo) {
        QVector<double> q = qualities;
        m_apo->SetExtendEqualizerBandQualityEx(index_EQ, q);
    }
}
void ApoWorker::resetExtendEqualizerSetting(uint index_EQ)
{
    if (m_apo) m_apo->ResetExtendEqualizerSetting(index_EQ);
}

void ApoWorker::SetExtendEqualizerBandFilter(uint index_EQ, uint index_band, EqualizerFilter filter)
{
    if (m_apo) m_apo->SetExtendEqualizerBandFilter(index_EQ,index_band,filter);
}
void ApoWorker::SetExtendEqualizerBandFilterEx(uint index_EQ, const QVector<EqualizerFilter>& filters)
{
    if (m_apo) m_apo->SetExtendEqualizerBandFilterEx(index_EQ,filters);
}


void ApoWorker::setVocalEffectsEnable(int en)
{
    if (m_apo) m_apo->SetVocalEffectsEnable(en);
}
void ApoWorker::setRichVocalsEnable(int en)
{
    if (m_apo) m_apo->SetRichVocalsEnable(en);
}
void ApoWorker::setAINSEnable(int en)
{
    if (m_apo) m_apo->SetAINSEnable(en);
}

void ApoWorker::setAINSLevel(int percent_level)
{
    if (m_apo) m_apo->SetAINSLevel(percent_level);
}

void ApoWorker::logWithTime(const QString &msg)
{
    if (m_apo) m_apo->logWithTime(msg);
}
