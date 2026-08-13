#ifndef APOWORKER_H
#define APOWORKER_H

#include "LoadApoDLL.h"
#include <QObject>
#include <QVector>
#include <QString>


class LoadApoDLL;

class ApoWorker : public QObject
{
    Q_OBJECT
public:
    explicit ApoWorker(QObject *parent = nullptr);
    ~ApoWorker();

public slots:

    // 在主线程中调用，设置已初始化的 LoadApoDLL 指针
    void setApo(LoadApoDLL* apo);

    // ---- 通用设置 ----
    void setProcessEffectOption(unsigned int option);
    void setLhdcDevice(const QString &deviceGUID);
    void setRenderState(bool enable);
    void setArEffectState(bool enable);

    // ---- 空间 ----
    void setSurroundState(bool enable);
    void setDistance(int scale);
    void setReverbState(bool enable);
    void setReverbFilter(int roomType);          // ReverbRoomType
    void setReverbActivateRoomType(int nRoomType);
    void setArReverbRatio(double ratio);

    // ---- 低音 ----
    void setBassBoostState(bool enable);
    void setCompBassGain(int value);
    void setCompBassCenterFrequency(double freq);
    void setBassBoostGain(int value);

    // ---- DRC ----
    void setDrcState(bool enable);
    void setDrcThreshold(double threshold);
    void setDrcRatio(double ratio);
    void setDrcAttackTime(double attackTime);
    void setDrcReleaseTime(double releaseTime);
    void setDrcMakeupEnable(uint makeupEnable);
    void setDrcInputGain(double inputGain);
    void setDrcOutputGain(double outputGain);
    void setDrcLimiterEnable(uint limiterEnable);
    void setDrcLimiterThreshold(double limiterThreshold);

    // ---- 增益/主音量 ----
    void setGlobalInputGainDb(int value);

    // ---- 均衡器 ----
    void setExtendEqState(uint index, bool enable);
    void setExtendEqualizerGain(uint index_EQ, uint index_band, double dbValue);
    void setExtendEqualizerGainEx(uint index_EQ, const QVector<double> &gains);
    void setExtendEqualizerCenterFrequency(uint index_EQ, uint index_band, double freq);
    void setExtendEqualizerCenterFrequencyEx(uint index_EQ, const QVector<double> &freqs);
    void setExtendEqualizerBandEnable(uint index_EQ, uint index_band, bool enable);
    void setExtendEqualizerBandEnableEx(uint index_EQ, const QVector<bool> &enables);
    void setExtendEqualizerBandQuality(uint index_EQ, uint index_band, double quality);
    void setExtendEqualizerBandQualityEx(uint index_EQ, const QVector<double> &qualities);
    void resetExtendEqualizerSetting(uint index_EQ);//不用
    // --- 滤波器 ---
    void SetExtendEqualizerBandFilter(uint index_EQ, uint index_band, EqualizerFilter filter);
    void SetExtendEqualizerBandFilterEx(uint index_EQ, const QVector<EqualizerFilter>& filters);


    // ---- 上行处理 ----
    void setVocalEffectsEnable(int en);
    void setRichVocalsEnable(int en);
    void setAINSEnable(int en);
    void setAINSLevel(int percent_level);

    // ---- 工具 ----
    void logWithTime(const QString &msg);

private:
    LoadApoDLL *m_apo;
};

#endif // APOWORKER_H
