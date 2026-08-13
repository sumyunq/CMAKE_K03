/*#ifndef AUDIOWAVEWIDGET_H
#define AUDIOWAVEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QVector>
#include <QRect>

class AudioWaveWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit AudioWaveWidget(QWidget *parent = nullptr);
    ~AudioWaveWidget();

    void setSuppressionLevel(int level);
    int suppressionLevel() const { return m_level; }

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

  signals:
    void DrcLevelChanged(int level);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private:
    void updateWaveData();
    void drawBgWaveform(QPainter &p);
    void drawWaveform(QPainter &p);
    void drawLevelSelector(QPainter &p);
    void updateBtnGeometry();

    float getAmplitude() const;
    float getNoiseStrength() const;
    float getScanSpeed() const;
    float getScanAlpha() const;
    QString getLevelTitle() const;
    QString getLevelDesc() const;

    QTimer *m_timer;//驱动动画的定时器，每 16ms 触发一次（约 60fps）
    float m_time;//全局动画时间轴，所有动态效果的基础变量
    int m_level;//压制等级（1~5）

    float m_scanAngle;//扫描线角度（当前代码中未实际绘制扫描线，但保留状态）
    float m_scanSpeed;//扫描线角速度

    float m_currentAmp;//当前振幅，用于平滑过渡（lerp）
    float m_targetAmp;//目标振幅，用于平滑过渡（lerp）
    float m_currentNoise;//当前噪声强度
    float m_targetNoise;//目标噪声强度
    float m_currentScanSpeed;//当前扫描速度
    float m_targetScanSpeed;//目标扫描速度
    float m_currentScanAlpha;//当前扫描透明度
    float m_targetScanAlpha;//目标扫描透明度

    QVector<float> m_waveData;//主波形采样点数
    QVector<float> m_bgWaveData;//背景波形采样点数
    QVector<QRect> m_levelRects;

    QVector<float> m_bgTexture;
    static constexpr int BG_TEXTURE_SIZE = 1600;

    //使能，关闭是逐渐变平滑
    bool m_enabled;//开关状态
    float m_currentEnabledFactor;//开关过渡当前值（0.0=直线，1.0=正常）
    float m_targetEnabledFactor;//开关过渡目标值
};

#endif // AUDIOWAVEWIDGET_H
*/

#ifndef AUDIOWAVEWIDGET_H
#define AUDIOWAVEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QVector>
#include <QRect>
#include <QPushButton>
#include <QButtonGroup>

class AudioWaveWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit AudioWaveWidget(QWidget *parent = nullptr);
    ~AudioWaveWidget();

    void setSuppressionLevel(int level);
    int suppressionLevel() const { return m_level; }

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    void setDrcLevel(int level);

  signals:
    void DrcLevelChanged(int level);
    void setOpenDrcEn(bool en);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private:
    void updateWaveData();
    void drawBgWaveform(QPainter &p);
    void drawWaveform(QPainter &p);
    void drawLevelSelector(QPainter &p);
    void updateBtnGeometry();  // 更新按钮几何位置

    float getAmplitude() const;
    float getNoiseStrength() const;
    float getScanSpeed() const;
    float getScanAlpha() const;
    QString getLevelTitle() const;
    QString getLevelDesc() const;

    QTimer *m_timer;//驱动动画的定时器，每 16ms 触发一次（约 60fps）
    float m_time;//全局动画时间轴，所有动态效果的基础变量
    int m_level;//压制等级（1~5）

    float m_scanAngle;//扫描线角度（当前代码中未实际绘制扫描线，但保留状态）
    float m_scanSpeed;//扫描线角速度

    float m_currentAmp;//当前振幅，用于平滑过渡（lerp）
    float m_targetAmp;//目标振幅，用于平滑过渡（lerp）
    float m_currentNoise;//当前噪声强度
    float m_targetNoise;//目标噪声强度
    float m_currentScanSpeed;//当前扫描速度
    float m_targetScanSpeed;//目标扫描速度
    float m_currentScanAlpha;//当前扫描透明度
    float m_targetScanAlpha;//目标扫描透明度

    QVector<float> m_waveData;//主波形采样点数
    QVector<float> m_bgWaveData;//背景波形采样点数

    // === 在类声明（.h）中新增 ===
    QVector<float> m_bgTexture;
    static constexpr int BG_TEXTURE_SIZE = 1600;

    // 按钮组相关（新增）
    QButtonGroup *m_buttonGroup;      // 按钮组，互斥
    QVector<QPushButton*> m_buttons;  // 五个等级按钮

    //使能，关闭是逐渐变平滑
    bool m_enabled;//开关状态
    float m_currentEnabledFactor;//开关过渡当前值（0.0=直线，1.0=正常）
    float m_targetEnabledFactor;//开关过渡目标值
};

#endif // AUDIOWAVEWIDGET_H


