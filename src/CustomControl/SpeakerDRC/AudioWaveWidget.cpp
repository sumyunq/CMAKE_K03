/*#include "audiowavewidget.h"
#include "qpainterpath.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//全局安全辅助函数
//正弦
inline float safeSin(float x) { return (float)std::sin((double)x); }
//浮点取余
inline float safeFmod(float x, float y) { return (float)std::fmod((double)x, (double)y); }
//自然指数
inline float safeExp(float x) { return (float)std::exp((double)x); }
//绝对值
inline float safeAbs(float x) { return x < 0.0f ? -x : x; }

AudioWaveWidget::AudioWaveWidget(QWidget *parent)
    : QWidget(parent)
      , m_timer(new QTimer(this))
      , m_time(0.0f)
      , m_level(3)
      , m_scanAngle(0.0f)
      , m_scanSpeed(0.0f)
      , m_currentAmp(0.4f)
      , m_targetAmp(0.4f)
      , m_currentNoise(0.05f)
      , m_targetNoise(0.05f)
      , m_currentScanSpeed(0.0f)
      , m_targetScanSpeed(0.0f)
      , m_currentScanAlpha(0.4f)
      , m_targetScanAlpha(0.4f)
      , m_enabled(true)               // 默认开启
      , m_currentEnabledFactor(1.0f)  // 当前开关因子
      , m_targetEnabledFactor(1.0f)   // 目标开关因子
      , m_buttonGroup(new QButtonGroup(this))
{
    setMinimumSize(933, 428);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            // 开启透明背景
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    m_waveData.resize(800);
    m_bgWaveData.resize(800);

            // ===预生成循环纹理 ===
    m_bgTexture.resize(BG_TEXTURE_SIZE);
    for (int i = 0; i < BG_TEXTURE_SIZE; ++i) {
        float x = (float)i / (float)BG_TEXTURE_SIZE;
        float TWO_PI = (float)(M_PI * 2.0);
        // 单一频率 2.0，均匀起伏；首尾完美循环
        m_bgTexture[i] = safeSin(x * TWO_PI * 2.0f) * 0.85f;
    }

            // === 创建按钮组与五个 QPushButton ===
    m_buttonGroup->setExclusive(true);  // 互斥模式

    QString labels[] = {
        QString::fromUtf8("轻微"),
        QString::fromUtf8("适度"),
        QString::fromUtf8("强力"),
        QString::fromUtf8("深度"),
        QString::fromUtf8("极限")
    };

    for (int i = 0; i < 5; ++i) {
        QPushButton *btn = new QPushButton(labels[i], this);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);

                // 按钮文本字体：#FFFFFF 14px weight:500
        QFont bf = btn->font();
        bf.setFamily("Noto Sans S Chinese");
        bf.setPixelSize(14);
        bf.setWeight(QFont::Medium);
        btn->setFont(bf);

                // 设置按钮样式：未选中时背景半透明黑，选中时蓝色背景，白色文字，圆角 8px
        btn->setStyleSheet(
            "QPushButton {"
            "  background-color: rgba(0, 0, 0, 51);"
            "  color: #FFFFFF;"
            "  border-radius: 8px;"
            "  border: none;"
            "}"
            "QPushButton:checked {"
            "  background-color: #009FEF;"
            "}"
            "QPushButton:checked:disabled{"
            " background-color: #0F6796;"
            "}"
            );
        //按钮点击事件
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            setSuppressionLevel(i + 1);
            emit
        });

        m_buttons.append(btn);
        m_buttonGroup->addButton(btn, i + 1);
    }

    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_time += 0.022f;// 时间前进，驱动所有动画
        m_scanAngle += m_currentScanSpeed;// 时间前进，驱动所有动画
        if (m_scanAngle >= 360.0f) m_scanAngle -= 360.0f;

                // 多路 lerp 平滑过渡：以 0.08 系数逼近目标值
        const float lerpSpeed = 0.08f;
        m_currentAmp += (m_targetAmp - m_currentAmp) * lerpSpeed;
        m_currentNoise += (m_targetNoise - m_currentNoise) * lerpSpeed;
        m_currentScanSpeed += (m_targetScanSpeed - m_currentScanSpeed) * lerpSpeed;
        m_currentScanAlpha += (m_targetScanAlpha - m_currentScanAlpha) * lerpSpeed;

                // 开关因子平滑过渡（0.0=直线  1.0=正常波形）
        m_currentEnabledFactor += (m_targetEnabledFactor - m_currentEnabledFactor) * lerpSpeed;

        updateWaveData();
        update();
    });
    m_timer->start(16);

    setSuppressionLevel(3);

            // 初始布局按钮位置
    updateBtnGeometry();
}

AudioWaveWidget::~AudioWaveWidget()
{
}
//等级控制
void AudioWaveWidget::setSuppressionLevel(int level)
{
    if (level < 1) level = 1;
    if (level > 5) level = 5;
    if (m_level == level) return;

    m_level = level;
    m_targetAmp = getAmplitude();
    m_targetNoise = getNoiseStrength();
    m_targetScanSpeed = getScanSpeed();
    m_targetScanAlpha = getScanAlpha();

            // 同步 QPushButton 选中状态
    if (m_buttons.size() == 5) {
        m_buttons[m_level - 1]->setChecked(true);
    }

    emit DrcLevelChanged(m_level);
    update();
}
//控制主波形峰高,等级 1→1.0, 2→0.7, 3→0.4, 4→0.2, 5→0.08。
float AudioWaveWidget::getAmplitude() const
{
    switch (m_level) {
        case 1: return 1.0f;
        case 2: return 0.7f;
        case 3: return 0.4f;
        case 4: return 0.2f;
        case 5: return 0.08f;
        default: return 0.4f;
    }
}
//控制噪声（未使用）,等级 1→0.15, 5→0.01。
float AudioWaveWidget::getNoiseStrength() const
{
    switch (m_level) {
        case 1: return 0.15f;
        case 2: return 0.10f;
        case 3: return 0.05f;
        case 4: return 0.02f;
        case 5: return 0.01f;
        default: return 0.05f;
    }
}
//等级越高扫描越快（360°/(分钟数)）
float AudioWaveWidget::getScanSpeed() const
{
    switch (m_level) {
        case 1: return 360.0f / (21.0f * 60.0f);
        case 2: return 360.0f / (17.0f * 60.0f);
        case 3: return 360.0f / (13.0f * 60.0f);
        case 4: return 360.0f / (9.0f * 60.0f);
        case 5: return 360.0f / (5.0f * 60.0f);
        default: return 360.0f / (13.0f * 60.0f);
    }
}
//等级越高扫描线越不透明 0.2→0.6
float AudioWaveWidget::getScanAlpha() const
{
    switch (m_level) {
        case 1: return 0.20f;
        case 2: return 0.30f;
        case 3: return 0.40f;
        case 4: return 0.50f;
        case 5: return 0.60f;
        default: return 0.40f;
    }
}
//等级标题（轻微/适度/强力/深度/极限压制）
QString AudioWaveWidget::getLevelTitle() const
{
    switch (m_level) {
        case 1: return QString::fromUtf8("轻微压制");
        case 2: return QString::fromUtf8("适度压制");
        case 3: return QString::fromUtf8("强力压制");
        case 4: return QString::fromUtf8("深度压制");
        case 5: return QString::fromUtf8("极限压制");
        default: return QString::fromUtf8("强力压制");
    }
}
//等级功能描述
QString AudioWaveWidget::getLevelDesc() const
{
    switch (m_level) {
        case 1: return QString::fromUtf8("最小化算法介入，保留原始音质动态，适合需要极高音频还原度的场景。");
        case 2: return QString::fromUtf8("均衡处理音频脉冲，在保持环境感的同时削弱刺耳的高频瞬态声音。");
        case 3: return QString::fromUtf8("AI 深度介入处理，强效压制突发爆炸声与枪声，推荐在多数竞技环境使用。");
        case 4: return QString::fromUtf8("大幅度削减音频峰值，环境背景音被极度压缩，适合嘈杂或极端爆炸环境。");
        case 5: return QString::fromUtf8("极致的波形整形算法，提供最高强度的压制效果，但可能会轻微影响脚步声的清晰度。");
        default: return QString("");
    }
}


// 计算波形数据
void AudioWaveWidget::updateWaveData()
{
    const int count = m_waveData.size();
    const float TWO_PI = (float)(M_PI * 2.0);

    const float pulseSpeed = 0.18f;//脉冲包络移动速度
    float pulseCenter = std::fmod(m_time * pulseSpeed, 1.0f);//高斯脉冲中心在 [0,1] 区间内的位置，随时间循环
    const float sigma = 0.05f;//高斯包络宽度，越小脉冲越尖锐
    const float invSigma2 = 1.0f / (sigma * sigma);

    float bgOffset = m_time * 0.15f * BG_TEXTURE_SIZE;//背景纹理采样偏移量，速度系数，0.03f适中

    for (int i = 0; i < count; ++i) {
        float x = (float)i / (float)(count - 1);

                // === 主波形（动态）,三个不同频率的正弦波叠加，形成主波形基线===
        float base = safeSin(x * TWO_PI * 2.0f + m_time * 1.2f) * 0.45f
                     + safeSin(x * TWO_PI * 3.5f - m_time * 0.7f) * 0.28f
                     + safeSin(x * TWO_PI * 1.8f + m_time * 0.5f) * 0.22f;

                // === 脉冲 ===
        float dx = x - pulseCenter;
        float dist = safeAbs(dx);
        float gaussianEnv = safeExp(-dist * dist * invSigma2);
        float carrier = (float)std::cos((double)(dx * TWO_PI * 150.0f));
        float pulse = carrier * gaussianEnv;

        float pulseStrength = m_currentAmp * 0.80f;  // 0.55f -> 0.80f，峰高变大
        float noise = 0.0f;

        m_waveData[i] = base * 0.45f * m_currentAmp + pulse * pulseStrength + noise * 0.12f;

                // === 背景波浪：均匀高度，完美循环滚动 ===
        float texPos = x * BG_TEXTURE_SIZE + bgOffset;
        float wrapped = safeFmod(texPos, (float)BG_TEXTURE_SIZE);
        if (wrapped < 0.0f) wrapped += (float)BG_TEXTURE_SIZE;

        int idx0 = (int)wrapped;
        int idx1 = (idx0 + 1) % BG_TEXTURE_SIZE;
        float t = wrapped - (float)idx0;

        m_bgWaveData[i] = m_bgTexture[idx0] * (1.0f - t) + m_bgTexture[idx1] * t;

                // 关闭时慢慢变直（趋向水平线）
        m_waveData[i] *= m_currentEnabledFactor;
        m_bgWaveData[i] *= m_currentEnabledFactor;
    }
}

void AudioWaveWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBgWaveform(p);// 背景实心光带
    drawWaveform(p);// 主波形线条
    drawLevelSelector(p);//底部 UI（标题、描述）
}
//底部 UI（标题、描述）
void AudioWaveWidget::drawLevelSelector(QPainter &p)
{
    int waveBottom = height() * 55 / 100;

    const int textX = 40;
    const int textY = waveBottom + 35;
    const int descY = textY + 32;
    const int descHeight = 50;

            // === 上方文本信息 ===
    // 蓝色指示圆点：直径12，rgba(0,159,239,0.7)
    p.setBrush(QColor(0, 159, 239, 179));
    p.setPen(Qt::NoPen);
    p.drawEllipse(textX, textY + 6, 12, 12);  // 垂直居中微调

            // 标题：rgba(0,159,239,0.7) 16px weight:500
    QFont tf = p.font();
    tf.setFamily("Noto Sans S Chinese");
    tf.setPixelSize(16);
    tf.setWeight(QFont::Medium);  // 500
    p.setFont(tf);
    p.setPen(QColor(0, 159, 239, 179));
    p.drawText(textX + 20, textY, width() - textX - 40, 26,
               Qt::AlignLeft | Qt::AlignVCenter, getLevelTitle());

            // 描述：rgba(161,168,179,0.5) 14px weight:500
    QFont df = p.font();
    df.setFamily("Noto Sans S Chinese");
    df.setPixelSize(14);
    df.setWeight(QFont::Medium);
    p.setFont(df);
    p.setPen(QColor(161, 168, 179, 128));
    p.drawText(textX, descY, width() - textX - 40, descHeight,
               Qt::AlignLeft | Qt::AlignTop, getLevelDesc());

}
//按钮位置放置
void AudioWaveWidget::updateBtnGeometry()
{
    int waveBottom = height() * 55 / 100;

    const int leftMargin  = 147;
    const int rightMargin = 146;
    const int btnSpacing  = 10;
    const int btnHeight   = 44;

    int availableWidth = width() - leftMargin - rightMargin;
    int btnWidth = (availableWidth - btnSpacing * 4) / 5;

    const int textX = 40;
    const int textY = waveBottom + 35;
    const int descY = textY + 32;
    const int descHeight = 50;
    int btnY = descY + descHeight + 33;

    for (int i = 0; i < m_buttons.size(); ++i) {
        int bx = leftMargin + i * (btnWidth + btnSpacing);
        m_buttons[i]->setGeometry(bx, btnY, btnWidth, btnHeight);
    }
}

void AudioWaveWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBtnGeometry();  // 窗口大小变化时更新按钮位置
}

void AudioWaveWidget::mousePressEvent(QMouseEvent *event)
{
    // 按钮点击检测已由 QPushButton 自行处理，此处无需检测按钮区域
    QWidget::mousePressEvent(event);
}

// 边缘淡出
static void drawWithFadeEdges(QPainter &p, const QImage &img, int width)
{
    QImage mask(width, img.height(), QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    QPainter mp(&mask);
    QLinearGradient g(0, 0, width, 0);
    g.setColorAt(0.0, QColor(255, 255, 255, 0));
    g.setColorAt(0.1, QColor(255, 255, 255, 255));
    g.setColorAt(0.9, QColor(255, 255, 255, 255));
    g.setColorAt(1.0, QColor(255, 255, 255, 0));
    mp.fillRect(img.rect(), g);
    mp.end();

    QPainter mp2(&mask);
    mp2.setCompositionMode(QPainter::CompositionMode_SourceIn);
    mp2.drawImage(0, 0, img);
    mp2.end();

    p.drawImage(0, 0, mask);
}

//背景波形绘制
void AudioWaveWidget::drawBgWaveform(QPainter &p)
{
    int waveTop = height() * 15 / 100;
    int waveBottom = height() * 55 / 100;
    int waveCenter = (waveTop + waveBottom) / 2;
    int waveHeight = waveBottom - waveTop;

    float halfThick = qMax(50.0f, waveHeight * 0.22f);

    QPainterPath path;
    bool first = true;
    for (int i = 0; i < m_bgWaveData.size(); ++i) {
        float x = (float)i / (float)(m_bgWaveData.size() - 1) * (float)width();
        float waveY = m_bgWaveData[i] * (float)waveHeight * 0.12f;//边界起伏系数
        float y = (float)waveCenter + waveY - halfThick;
        if (first) { path.moveTo(x, y); first = false; }
        else       { path.lineTo(x, y); }
    }
    for (int i = m_bgWaveData.size() - 1; i >= 0; --i) {
        float x = (float)i / (float)(m_bgWaveData.size() - 1) * (float)width();
        float waveY = m_bgWaveData[i] * (float)waveHeight * 0.12f;
        float y = (float)waveCenter + waveY + halfThick;
        path.lineTo(x, y);
    }
    path.closeSubpath();

            // 水平渐变：两端 10% alpha 从 0 渐入到 13
    QLinearGradient hGrad(0, 0, width(), 0);
    hGrad.setColorAt(0.0, QColor(95, 220, 255, 0));
    hGrad.setColorAt(0.1, QColor(95, 220, 255, 13));
    hGrad.setColorAt(0.9, QColor(95, 220, 255, 13));
    hGrad.setColorAt(1.0, QColor(95, 220, 255, 0));

    p.fillPath(path, hGrad);
}

//主波形绘制
void AudioWaveWidget::drawWaveform(QPainter &p)
{
    int waveTop = height() * 15 / 100;
    int waveBottom = height() * 55 / 100;
    int waveCenter = (waveTop + waveBottom) / 2;
    int waveHeight = waveBottom - waveTop;

    QPainterPath path;
    bool first = true;
    for (int i = 0; i < m_waveData.size(); ++i) {
        float x = (float)i / (float)(m_waveData.size() - 1) * (float)width();
        float y = (float)waveCenter + m_waveData[i] * (float)waveHeight * 0.38f;
        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }

            // 先画到临时 QImage（所有 glow + 主线）
    QImage img(width(), height(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter ip(&img);
    ip.setRenderHint(QPainter::Antialiasing);

            // glow 层
    ip.setPen(QPen(QColor(0, 130, 220, 30), 14));
    ip.drawPath(path);
    ip.setPen(QPen(QColor(0, 145, 230, 60), 8));
    ip.drawPath(path);
    ip.setPen(QPen(QColor(0, 155, 240, 100), 4));
    ip.drawPath(path);

            // 主线 #009FEF，粗细 2
    ip.setPen(QPen(QColor(0, 159, 239, 230), 2));
    ip.drawPath(path);

            // 核心高亮
    ip.setPen(QPen(QColor(140, 220, 255, 255), 1));
    ip.drawPath(path);

            // === 下方实心填充已删除 ===

    ip.end();

            // 统一应用两端 10% 渐入渐出
    drawWithFadeEdges(p, img, width());
}


//  开关函数(按钮不可用。动画逐渐变平,波形颜色变为 #0F6796)
void AudioWaveWidget::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    m_targetEnabledFactor = enabled ? 1.0f : 0.0f;
    update();
}
//设置DRC
void AudioWaveWidget::setDrcLevel(int level)
{
    setSuppressionLevel(level);
}
*/

#include "audiowavewidget.h"
#include "qpainterpath.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//全局安全辅助函数
//正弦
inline float safeSin(float x) { return (float)std::sin((double)x); }
//浮点取余
inline float safeFmod(float x, float y) { return (float)std::fmod((double)x, (double)y); }
//自然指数
inline float safeExp(float x) { return (float)std::exp((double)x); }
//绝对值
inline float safeAbs(float x) { return x < 0.0f ? -x : x; }

static inline QColor lerpColor(const QColor &from, const QColor &to, float t)
{
    return QColor(
        (int)(from.red()   + (to.red()   - from.red())   * t),
        (int)(from.green() + (to.green() - from.green()) * t),
        (int)(from.blue()  + (to.blue()  - from.blue())  * t),
        (int)(from.alpha() + (to.alpha() - from.alpha()) * t)
        );
}

AudioWaveWidget::AudioWaveWidget(QWidget *parent)
    : QWidget(parent)
      , m_timer(new QTimer(this))
      , m_time(0.0f)
      , m_level(3)
      , m_scanAngle(0.0f)
      , m_scanSpeed(0.0f)
      , m_currentAmp(0.4f)
      , m_targetAmp(0.4f)
      , m_currentNoise(0.05f)
      , m_targetNoise(0.05f)
      , m_currentScanSpeed(0.0f)
      , m_targetScanSpeed(0.0f)
      , m_currentScanAlpha(0.4f)
      , m_targetScanAlpha(0.4f)
      , m_enabled(true)               // 默认开启
      , m_currentEnabledFactor(1.0f)  // 当前开关因子
      , m_targetEnabledFactor(1.0f)   // 目标开关因子
      , m_buttonGroup(new QButtonGroup(this))
{
    setMinimumSize(933, 428);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            // 开启透明背景
    setAttribute(Qt::WA_TranslucentBackground);
    setAutoFillBackground(false);

    m_waveData.resize(800);
    m_bgWaveData.resize(800);

            // ===预生成循环纹理 ===
    m_bgTexture.resize(BG_TEXTURE_SIZE);
    for (int i = 0; i < BG_TEXTURE_SIZE; ++i) {
        float x = (float)i / (float)BG_TEXTURE_SIZE;
        float TWO_PI = (float)(M_PI * 2.0);
        // 单一频率 2.0，均匀起伏；首尾完美循环
        m_bgTexture[i] = safeSin(x * TWO_PI * 2.0f) * 0.85f;
    }

            // === 创建按钮组与五个 QPushButton ===
    m_buttonGroup->setExclusive(true);  // 互斥模式

    QString labels[] = {
        QString::fromUtf8("轻微"),
        QString::fromUtf8("适度"),
        QString::fromUtf8("强力"),
        QString::fromUtf8("深度"),
        QString::fromUtf8("极限")
    };

    for (int i = 0; i < 5; ++i) {
        QPushButton *btn = new QPushButton(labels[i], this);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setCheckable(true);
        btn->setCursor(Qt::PointingHandCursor);

                // 按钮文本字体：#FFFFFF 14px weight:500
        QFont bf = btn->font();
        bf.setFamily("Noto Sans S Chinese");
        bf.setPixelSize(14);
        bf.setWeight(QFont::Medium);
        btn->setFont(bf);

                // 设置按钮样式：未选中时背景半透明黑，选中时蓝色背景，白色文字，圆角 8px
        btn->setStyleSheet(
            "QPushButton {"
            "  background-color: rgba(0, 0, 0, 51);"
            "  color: #FFFFFF;"
            "  border-radius: 8px;"
            "  border: none;"
            "}"
            "QPushButton:checked {"
            "  background-color: #009FEF;"
            "}"
            "QPushButton:checked:disabled{"
            " background-color: #0F6796;"
            "color: #ACACAC;"
            "}"
            );
        //按钮点击事件
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            setSuppressionLevel(i + 1);
        });

        m_buttons.append(btn);
        m_buttonGroup->addButton(btn, i + 1);
    }

    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_time += 0.022f;// 时间前进，驱动所有动画
        m_scanAngle += m_currentScanSpeed;// 时间前进，驱动所有动画
        if (m_scanAngle >= 360.0f) m_scanAngle -= 360.0f;

                // 多路 lerp 平滑过渡：以 0.08 系数逼近目标值
        const float lerpSpeed = 0.08f;
        m_currentAmp += (m_targetAmp - m_currentAmp) * lerpSpeed;
        m_currentNoise += (m_targetNoise - m_currentNoise) * lerpSpeed;
        m_currentScanSpeed += (m_targetScanSpeed - m_currentScanSpeed) * lerpSpeed;
        m_currentScanAlpha += (m_targetScanAlpha - m_currentScanAlpha) * lerpSpeed;

                // 开关因子平滑过渡（0.0=直线  1.0=正常波形）
        m_currentEnabledFactor += (m_targetEnabledFactor - m_currentEnabledFactor) * lerpSpeed;

        updateWaveData();
        update();
    });
    m_timer->start(16);

    setSuppressionLevel(3);

            // 初始布局按钮位置
    updateBtnGeometry();
}

AudioWaveWidget::~AudioWaveWidget()
{
}
//等级控制
void AudioWaveWidget::setSuppressionLevel(int level)
{
    if (level < 1) level = 1;
    if (level > 5) level = 5;
    if (m_level == level) return;

    m_level = level;
    m_targetAmp = getAmplitude();
    m_targetNoise = getNoiseStrength();
    m_targetScanSpeed = getScanSpeed();
    m_targetScanAlpha = getScanAlpha();

            // 同步 QPushButton 选中状态
    if (m_buttons.size() == 5) {
        m_buttons[m_level - 1]->setChecked(true);
    }

    emit DrcLevelChanged(m_level);
    update();
}
//控制主波形峰高,等级 1→1.0, 2→0.7, 3→0.4, 4→0.2, 5→0.08。
float AudioWaveWidget::getAmplitude() const
{
    switch (m_level) {
        case 1: return 1.0f;
        case 2: return 0.7f;
        case 3: return 0.4f;
        case 4: return 0.2f;
        case 5: return 0.08f;
        default: return 0.4f;
    }
}
//控制噪声（未使用）,等级 1→0.15, 5→0.01。
float AudioWaveWidget::getNoiseStrength() const
{
    switch (m_level) {
        case 1: return 0.15f;
        case 2: return 0.10f;
        case 3: return 0.05f;
        case 4: return 0.02f;
        case 5: return 0.01f;
        default: return 0.05f;
    }
}
//等级越高扫描越快（360°/(分钟数)）
float AudioWaveWidget::getScanSpeed() const
{
    switch (m_level) {
        case 1: return 360.0f / (21.0f * 60.0f);
        case 2: return 360.0f / (17.0f * 60.0f);
        case 3: return 360.0f / (13.0f * 60.0f);
        case 4: return 360.0f / (9.0f * 60.0f);
        case 5: return 360.0f / (5.0f * 60.0f);
        default: return 360.0f / (13.0f * 60.0f);
    }
}
//等级越高扫描线越不透明 0.2→0.6
float AudioWaveWidget::getScanAlpha() const
{
    switch (m_level) {
        case 1: return 0.20f;
        case 2: return 0.30f;
        case 3: return 0.40f;
        case 4: return 0.50f;
        case 5: return 0.60f;
        default: return 0.40f;
    }
}
//等级标题（轻微/适度/强力/深度/极限压制）
QString AudioWaveWidget::getLevelTitle() const
{
    switch (m_level) {
        case 1: return QString::fromUtf8("轻微压制");
        case 2: return QString::fromUtf8("适度压制");
        case 3: return QString::fromUtf8("强力压制");
        case 4: return QString::fromUtf8("深度压制");
        case 5: return QString::fromUtf8("极限压制");
        default: return QString::fromUtf8("强力压制");
    }
}
//等级功能描述
QString AudioWaveWidget::getLevelDesc() const
{
    switch (m_level) {
        case 1: return QString::fromUtf8("最小化算法介入，保留原始音质动态，适合需要极高音频还原度的场景。");
        case 2: return QString::fromUtf8("均衡处理音频脉冲，在保持环境感的同时削弱刺耳的高频瞬态声音。");
        case 3: return QString::fromUtf8("AI 深度介入处理，强效压制突发爆炸声与枪声，推荐在多数竞技环境使用。");
        case 4: return QString::fromUtf8("大幅度削减音频峰值，环境背景音被极度压缩，适合嘈杂或极端爆炸环境。");
        case 5: return QString::fromUtf8("极致的波形整形算法，提供最高强度的压制效果，但可能会轻微影响脚步声的清晰度。");
        default: return QString("");
    }
}


// 计算波形数据
void AudioWaveWidget::updateWaveData()
{
    const int count = m_waveData.size();
    const float TWO_PI = (float)(M_PI * 2.0);

    const float pulseSpeed = 0.18f;//脉冲包络移动速度
    float pulseCenter = std::fmod(m_time * pulseSpeed, 1.0f);//高斯脉冲中心在 [0,1] 区间内的位置，随时间循环
    const float sigma = 0.05f;//高斯包络宽度，越小脉冲越尖锐
    const float invSigma2 = 1.0f / (sigma * sigma);

    float bgOffset = m_time * 0.15f * BG_TEXTURE_SIZE;//背景纹理采样偏移量，速度系数，0.03f适中

    for (int i = 0; i < count; ++i) {
        float x = (float)i / (float)(count - 1);

                // === 主波形（动态）,三个不同频率的正弦波叠加，形成主波形基线===
        float base = safeSin(x * TWO_PI * 2.0f + m_time * 1.2f) * 0.45f
                     + safeSin(x * TWO_PI * 3.5f - m_time * 0.7f) * 0.28f
                     + safeSin(x * TWO_PI * 1.8f + m_time * 0.5f) * 0.22f;

                // === 脉冲 ===
        float dx = x - pulseCenter;
        float dist = safeAbs(dx);
        float gaussianEnv = safeExp(-dist * dist * invSigma2);
        float carrier = (float)std::cos((double)(dx * TWO_PI * 150.0f));
        float pulse = carrier * gaussianEnv;

        float pulseStrength = m_currentAmp * 0.80f;  // 0.55f -> 0.80f，峰高变大
        float noise = 0.0f;

        m_waveData[i] = base * 0.45f * m_currentAmp + pulse * pulseStrength + noise * 0.12f;

                // === 背景波浪：均匀高度，完美循环滚动 ===
        float texPos = x * BG_TEXTURE_SIZE + bgOffset;
        float wrapped = safeFmod(texPos, (float)BG_TEXTURE_SIZE);
        if (wrapped < 0.0f) wrapped += (float)BG_TEXTURE_SIZE;

        int idx0 = (int)wrapped;
        int idx1 = (idx0 + 1) % BG_TEXTURE_SIZE;
        float t = wrapped - (float)idx0;

        m_bgWaveData[i] = m_bgTexture[idx0] * (1.0f - t) + m_bgTexture[idx1] * t;

                // 关闭时慢慢变直（趋向水平线）
        m_waveData[i] *= m_currentEnabledFactor;
        m_bgWaveData[i] *= m_currentEnabledFactor;
    }
}

void AudioWaveWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawBgWaveform(p);// 背景实心光带
    drawWaveform(p);// 主波形线条
    drawLevelSelector(p);//底部 UI（标题、描述）
}
//底部 UI（标题、描述）
void AudioWaveWidget::drawLevelSelector(QPainter &p)
{
    int waveBottom = height() * 55 / 100;

    const int textX = 40;
    const int textY = waveBottom + 35;
    const int descY = textY + 32;
    const int descHeight = 50;

            // === 上方文本信息 ===
    QColor titleOn(0, 159, 239, 179);
    QColor titleOff(15, 103, 150, 179);
    QColor titleColor = lerpColor(titleOff, titleOn, m_currentEnabledFactor);

            // 蓝色指示圆点：直径12，rgba(0,159,239,0.7)
    p.setBrush(lerpColor(QColor(15, 103, 150, 179), QColor(0, 159, 239, 179), m_currentEnabledFactor));
    p.setPen(Qt::NoPen);
    p.drawEllipse(textX, textY + 6, 12, 12);  // 垂直居中微调

            // 标题：rgba(0,159,239,0.7) 16px weight:500
    QFont tf = p.font();
    tf.setFamily("Noto Sans S Chinese");
    tf.setPixelSize(16);
    tf.setWeight(QFont::Medium);  // 500
    p.setFont(tf);
    p.setPen(titleColor);
    p.drawText(textX + 20, textY, width() - textX - 40, 26,
               Qt::AlignLeft | Qt::AlignVCenter, getLevelTitle());

            // 描述：rgba(161,168,179,0.5) 14px weight:500
    QFont df = p.font();
    df.setFamily("Noto Sans S Chinese");
    df.setPixelSize(14);
    df.setWeight(QFont::Medium);
    p.setFont(df);
    p.setPen(QColor(161, 168, 179, 128));
    p.drawText(textX, descY, width() - textX - 40, descHeight,
               Qt::AlignLeft | Qt::AlignTop, getLevelDesc());

}
//按钮位置放置
void AudioWaveWidget::updateBtnGeometry()
{
    int waveBottom = height() * 55 / 100;

    const int leftMargin  = 147;
    const int rightMargin = 146;
    const int btnSpacing  = 10;
    const int btnHeight   = 44;

    int availableWidth = width() - leftMargin - rightMargin;
    int btnWidth = (availableWidth - btnSpacing * 4) / 5;

    const int textX = 40;
    const int textY = waveBottom + 35;
    const int descY = textY + 32;
    const int descHeight = 50;
    int btnY = descY + descHeight + 33;

    for (int i = 0; i < m_buttons.size(); ++i) {
        int bx = leftMargin + i * (btnWidth + btnSpacing);
        m_buttons[i]->setGeometry(bx, btnY, btnWidth, btnHeight);
    }
}

void AudioWaveWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateBtnGeometry();  // 窗口大小变化时更新按钮位置
}

//若按钮在不可用时被点击，则切换按钮且打开开关
void AudioWaveWidget::mousePressEvent(QMouseEvent *event)
{
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (!m_buttons[i]->isEnabled() && m_buttons[i]->geometry().contains(event->pos())) {
            setEnabled(true);
            setSuppressionLevel(i + 1);
            emit setOpenDrcEn(true);
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

// 边缘淡出
static void drawWithFadeEdges(QPainter &p, const QImage &img, int width)
{
    QImage mask(width, img.height(), QImage::Format_ARGB32_Premultiplied);
    mask.fill(Qt::transparent);
    QPainter mp(&mask);
    QLinearGradient g(0, 0, width, 0);
    g.setColorAt(0.0, QColor(255, 255, 255, 0));
    g.setColorAt(0.1, QColor(255, 255, 255, 255));
    g.setColorAt(0.9, QColor(255, 255, 255, 255));
    g.setColorAt(1.0, QColor(255, 255, 255, 0));
    mp.fillRect(img.rect(), g);
    mp.end();

    QPainter mp2(&mask);
    mp2.setCompositionMode(QPainter::CompositionMode_SourceIn);
    mp2.drawImage(0, 0, img);
    mp2.end();

    p.drawImage(0, 0, mask);
}

//背景波形绘制
void AudioWaveWidget::drawBgWaveform(QPainter &p)
{
    int waveTop = height() * 15 / 100;
    int waveBottom = height() * 55 / 100;
    int waveCenter = (waveTop + waveBottom) / 2;
    int waveHeight = waveBottom - waveTop;

    float halfThick = qMax(50.0f, waveHeight * 0.22f);

    QPainterPath path;
    bool first = true;
    for (int i = 0; i < m_bgWaveData.size(); ++i) {
        float x = (float)i / (float)(m_bgWaveData.size() - 1) * (float)width();
        float waveY = m_bgWaveData[i] * (float)waveHeight * 0.12f;//边界起伏系数
        float y = (float)waveCenter + waveY - halfThick;
        if (first) { path.moveTo(x, y); first = false; }
        else       { path.lineTo(x, y); }
    }
    for (int i = m_bgWaveData.size() - 1; i >= 0; --i) {
        float x = (float)i / (float)(m_bgWaveData.size() - 1) * (float)width();
        float waveY = m_bgWaveData[i] * (float)waveHeight * 0.12f;
        float y = (float)waveCenter + waveY + halfThick;
        path.lineTo(x, y);
    }
    path.closeSubpath();

            // 水平渐变：两端 10% alpha 从 0 渐入到 13
    QLinearGradient hGrad(0, 0, width(), 0);
    hGrad.setColorAt(0.0, QColor(95, 220, 255, 0));
    hGrad.setColorAt(0.1, QColor(95, 220, 255, 13));
    hGrad.setColorAt(0.9, QColor(95, 220, 255, 13));
    hGrad.setColorAt(1.0, QColor(95, 220, 255, 0));

    p.fillPath(path, hGrad);
}

//主波形绘制
void AudioWaveWidget::drawWaveform(QPainter &p)
{
    int waveTop = height() * 15 / 100;
    int waveBottom = height() * 55 / 100;
    int waveCenter = (waveTop + waveBottom) / 2;
    int waveHeight = waveBottom - waveTop;

    QPainterPath path;
    bool first = true;
    for (int i = 0; i < m_waveData.size(); ++i) {
        float x = (float)i / (float)(m_waveData.size() - 1) * (float)width();
        float y = (float)waveCenter + m_waveData[i] * (float)waveHeight * 0.38f;
        if (first) {
            path.moveTo(x, y);
            first = false;
        } else {
            path.lineTo(x, y);
        }
    }

            // 先画到临时 QImage（所有 glow + 主线）
    QImage img(width(), height(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter ip(&img);
    ip.setRenderHint(QPainter::Antialiasing);

    QColor cOn1(0, 130, 220, 30);
    QColor cOff1(15, 103, 150, 30);
    QColor c1 = lerpColor(cOff1, cOn1, m_currentEnabledFactor);

    QColor cOn2(0, 145, 230, 60);
    QColor cOff2(15, 103, 150, 60);
    QColor c2 = lerpColor(cOff2, cOn2, m_currentEnabledFactor);

    QColor cOn3(0, 155, 240, 100);
    QColor cOff3(15, 103, 150, 100);
    QColor c3 = lerpColor(cOff3, cOn3, m_currentEnabledFactor);

    QColor cOnMain(0, 159, 239, 230);
    QColor cOffMain(15, 103, 150, 230);
    QColor cMain = lerpColor(cOffMain, cOnMain, m_currentEnabledFactor);

    QColor cOnHigh(140, 220, 255, 255);
    QColor cOffHigh(15, 103, 150, 255);
    QColor cHigh = lerpColor(cOffHigh, cOnHigh, m_currentEnabledFactor);

            // glow 层
    ip.setPen(QPen(c1, 14));
    ip.drawPath(path);
    ip.setPen(QPen(c2, 8));
    ip.drawPath(path);
    ip.setPen(QPen(c3, 4));
    ip.drawPath(path);

            // 主线 #009FEF，粗细 2
    ip.setPen(QPen(cMain, 2));
    ip.drawPath(path);

            // 核心高亮
    ip.setPen(QPen(cHigh, 1));
    ip.drawPath(path);

            // === 下方实心填充已删除 ===

    ip.end();

            // 统一应用两端 10% 渐入渐出
    drawWithFadeEdges(p, img, width());
}


//  开关函数(按钮不可用。动画逐渐变平,波形颜色变为 #0F6796)
void AudioWaveWidget::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;
    m_enabled = enabled;
    m_targetEnabledFactor = enabled ? 1.0f : 0.0f;

    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i]->setEnabled(enabled);
        //使鼠标事件传递给下方的父窗口
         m_buttons[i]->setAttribute(Qt::WA_TransparentForMouseEvents, !enabled);
    }

    update();
}
//设置DRC
void AudioWaveWidget::setDrcLevel(int level)
{
    setSuppressionLevel(level);
}

