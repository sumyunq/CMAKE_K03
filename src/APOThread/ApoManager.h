#ifndef APOMANAGER_H
#define APOMANAGER_H

#include "LoadApoDLL.h"
#include <QObject>
#include <QThread>
#include <QVector>
#include <QString>

class ApoWorker;

class ApoManager : public QObject
{
    Q_OBJECT
public:
    static ApoManager* instance();
    void start();   // 启动工作线程
    void stop();    // 停止工作线程

signals:
    // 对外的所有信号，对应 ApoWorker 的槽
    void requestSetProcessEffectOption(unsigned int option);
    void requestSetLhdcDevice(const QString &deviceGUID);
    void requestSetRenderState(bool enable);
    void requestSetArEffectState(bool enable);

    void requestSetSurroundState(bool enable);
    void requestSetDistance(int scale);//空间环绕
    void requestSetReverbState(bool enable);
    void requestSetReverbFilter(int roomType);
    void requestSetReverbActivateRoomType(int nRoomType);
    void requestSetArReverbRatio(double ratio);//空间混响

    void requestSetBassBoostState(bool enable);
    void requestSetCompBassGain(int value);
    void requestSetCompBassCenterFrequency(double freq);
    void requestSetBassBoostGain(int value);

    void requestSetDrcState(bool enable);
    void requestSetDrcThreshold(double threshold);
    void requestSetDrcRatio(double ratio);
    void requestSetDrcAttackTime(double attackTime);
    void requestSetDrcReleaseTime(double releaseTime);
    void requestSetDrcMakeupEnable(uint makeupEnable);
    void requestSetDrcInputGain(double inputGain);
    void requestSetDrcOutputGain(double outputGain);
    void requestSetDrcLimiterEnable(uint limiterEnable);
    void requestSetDrcLimiterThreshold(double limiterThreshold);

    void requestSetGlobalInputGainDb(int value);

    void requestSetExtendEqState(uint index, bool enable);
    void requestSetExtendEqualizerGain(uint index_EQ, uint index_band, double dbValue);
    void requestSetExtendEqualizerGainEx(uint index_EQ, const QVector<double> &gains);
    void requestSetExtendEqualizerCenterFrequency(uint index_EQ, uint index_band, double freq);
    void requestSetExtendEqualizerCenterFrequencyEx(uint index_EQ, const QVector<double> &freqs);
    void requestSetExtendEqualizerBandEnable(uint index_EQ, uint index_band, bool enable);
    void requestSetExtendEqualizerBandEnableEx(uint index_EQ, const QVector<bool> &enables);
    void requestSetExtendEqualizerBandQuality(uint index_EQ, uint index_band, double quality);
    void requestSetExtendEqualizerBandQualityEx(uint index_EQ, const QVector<double> &qualities);
    void requestResetExtendEqualizerSetting(uint index_EQ);


    void requestSetExtendEqualizerBandFilter(uint index_EQ, uint index_band, EqualizerFilter filter);
    void requestSetExtendEqualizerBandFilterEx(uint index_EQ, const QVector<EqualizerFilter>& filters);


    void requestSetVocalEffectsEnable(int en);
    void requestSetRichVocalsEnable(int en);
    void requestSetAINSEnable(int en);
    void requestSetAINSLevel(int percent_level);

    void requestlogWithTime(const QString &msg);

private:
    explicit ApoManager(QObject *parent = nullptr);
    ~ApoManager();
    Q_DISABLE_COPY(ApoManager)

    static ApoManager* m_instance;
    ApoWorker* m_worker;
    QThread* m_workerThread;
};

#endif // APOMANAGER_H
