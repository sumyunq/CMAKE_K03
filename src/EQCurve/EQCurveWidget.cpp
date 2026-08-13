#include "EQCurve/EQCurveWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QDebug>
#include <QTimer>
#include <QGuiApplication>
#include <QScreen>
#include <QMoveEvent>
#include <complex>
#include <cmath>

// ==================== 常量定义 ====================
static const double SAMPLE_RATE = 48000.0;      // 采样率 (Hz)，应与音频处理引擎一致

// 滤波器类型常量 (与 EQBand::filterType 对应)
// static const int FILTER_LOW_PASS   = 0;//LOW Pass
// static const int FILTER_HIGH_PASS  = 1;//High Pass
// static const int FILTER_NOTCH  = 2;//Notch Filter
// static const int FILTER_PEAKING    = 3;//PeakingEq
// static const int FILTER_LOW_SHELF  = 4;//Low Shelving
// static const int FILTER_HIGH_SHELF = 5;//High Shelving

static const int FILTER_PEAKING    = 0;//PeakingEq
static const int FILTER_HIGH_PASS  = 1;//High Pass
static const int FILTER_LOW_PASS   = 2;//LOW Pass
static const int FILTER_HIGH_SHELF = 3;//High Shelving
static const int FILTER_LOW_SHELF  = 4;//Low Shelving
static const int FILTER_NOTCH  = 5;//Notch Filter

// 频率响应计算参数
static const double MIN_FREQ = 20.0;// 最小频率 20Hz
static const double MAX_FREQ = 20000.0;// 最大频率 20kHz
static const double MIN_GAIN = -12.0; // 最小增益 -12dB
static const double MAX_GAIN = 12.0;// 最大增益 +12dB
static const int SAMPLE_POINTS = 500;// 采样点数

static const double PLOT_MIN_GAIN = -14.0;// y轴最小增益 -14dB
static const double PLOT_MAX_GAIN = 15.0;// y轴最大增益 +14dB
static const double PLOT_MIN_FREQ = 19.0;// x轴最小频率 15Hz
static const double PLOT_MAX_FREQ = 21000.0;// x轴最大频率 25kHz
// static const double PLOT_MIN_FREQ = 19.0;// x轴最小频率 15Hz
// static const double PLOT_MAX_FREQ = 21000.0;// x轴最大频率 25kHz

// 绘图区域边距（与坐标轴标签距离边框的空白）
static const int PLOT_LEFT_MARGIN = 65;// 左侧留白，用于Y轴标签
static const int PLOT_RIGHT_MARGIN = 30;// 右侧留白
static const int PLOT_TOP_MARGIN = 20;// 顶部留白
static const int PLOT_BOTTOM_MARGIN = 60;// 底部留白，用于X轴标签和标题

// ==================== 辅助函数（双二阶系数与频率响应） ====================

/**
 * 根据滤波器类型、频率、增益、Q值、采样率计算双二阶滤波器系数
 * 基于 Audio EQ Cookbook 公式
 */
static void computeBiquadCoefficients(int type, double f0, double G_dB, double Q, double fs,
                                      double &b0, double &b1, double &b2,
                                      double &a0, double &a1, double &a2)
{
    double omega = 2.0 * M_PI * f0 / fs;
    double sn = sin(omega);
    double cs = cos(omega);
    double alpha = sn / (2.0 * Q);
    double A = 0.0;

    switch (type) {
    case FILTER_PEAKING:
        A = std::pow(10.0, G_dB / 40.0);
        b0 = 1.0 + alpha * A;
        b1 = -2.0 * cs;
        b2 = 1.0 - alpha * A;
        a0 = 1.0 + alpha / A;
        a1 = -2.0 * cs;
        a2 = 1.0 - alpha / A;
        // qDebug("FILTER_PEAKING\n");
        break;

    case FILTER_LOW_SHELF:
        A = std::pow(10.0, G_dB / 20.0);
        b0 = A * ((A + 1.0) - (A - 1.0) * cs + 2.0 * sqrt(A) * alpha);
        b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cs);
        b2 = A * ((A + 1.0) - (A - 1.0) * cs - 2.0 * sqrt(A) * alpha);
        a0 = (A + 1.0) + (A - 1.0) * cs + 2.0 * sqrt(A) * alpha;
        a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cs);
        a2 = (A + 1.0) + (A - 1.0) * cs - 2.0 * sqrt(A) * alpha;
        // qDebug("FILTER_LOW_SHELF\n");
        break;

    case FILTER_HIGH_SHELF:
        A = std::pow(10.0, G_dB / 20.0);
        b0 = A * ((A + 1.0) + (A - 1.0) * cs + 2.0 * sqrt(A) * alpha);
        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
        b2 = A * ((A + 1.0) + (A - 1.0) * cs - 2.0 * sqrt(A) * alpha);
        a0 = (A + 1.0) - (A - 1.0) * cs + 2.0 * sqrt(A) * alpha;
        a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
        a2 = (A + 1.0) - (A - 1.0) * cs - 2.0 * sqrt(A) * alpha;
        // qDebug("FILTER_HIGH_SHELF\n");
        break;

    case FILTER_LOW_PASS:
        // 截止频率 f0，Q 控制斜率
        b0 = (1.0 - cs) / 2.0;
        b1 = 1.0 - cs;
        b2 = (1.0 - cs) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cs;
        a2 = 1.0 - alpha;
        // qDebug("FILTER_LOW_PASS\n");
        break;

    case FILTER_HIGH_PASS:
        b0 = (1.0 + cs) / 2.0;
        b1 = -(1.0 + cs);
        b2 = (1.0 + cs) / 2.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cs;
        a2 = 1.0 - alpha;
        // qDebug("FILTER_HIGH_PASS\n");
        break;

    case FILTER_NOTCH:
        // 陷波器：在 f0 处形成极深衰减，无需增益参数
        b0 = 1.0;
        b1 = -2.0 * cs;
        b2 = 1.0;
        a0 = 1.0 + alpha;
        a1 = -2.0 * cs;
        a2 = 1.0 - alpha;
        // qDebug("FILTER_NOTCH\n");
        break;

    default:   // 默认 Peaking
        A = std::pow(10.0, G_dB / 40.0);
        b0 = 1.0 + alpha * A;
        b1 = -2.0 * cs;
        b2 = 1.0 - alpha * A;
        a0 = 1.0 + alpha / A;
        a1 = -2.0 * cs;
        a2 = 1.0 - alpha / A;
        // qDebug("default\n");
        break;
    }
}

/**
 * 计算双二阶滤波器在频率 f (Hz) 处的幅度响应（线性值）
 */
static double computeMagnitudeResponse(double b0, double b1, double b2,
                                       double a0, double a1, double a2,
                                       double f, double fs)
{
    double omega = 2.0 * M_PI * f / fs;
    std::complex<double> z = std::complex<double>(cos(omega), -sin(omega)); // e^(-jω)
    std::complex<double> numerator   = b0 + b1 * z + b2 * z * z;
    std::complex<double> denominator = a0 + a1 * z + a2 * z * z;
    return std::abs(numerator / denominator);
}

// ==================== EQCurveWidget 实现 ====================

EQCurveWidget::EQCurveWidget(QWidget *parent)
    : QWidget(parent)
    , m_selectedBandIndex(-1)
    , m_draggingQ(false)
    , m_dragStartQ(0.5)        // 任意默认值
    , m_dragStartPos(0, 0)
{
    setMinimumSize(1071, 482);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);// 启用鼠标跟踪

    m_editPanel = new EditPanelTip(this);
    m_editPanel->hide();
    m_editPanel->installEventFilter(this);

    // 计算绘图区域
    int left = PLOT_LEFT_MARGIN;
    int right = PLOT_RIGHT_MARGIN;
    int top = PLOT_TOP_MARGIN;
    int bottom = PLOT_BOTTOM_MARGIN;
    m_plotRect = QRect(left, top, width() - left - right, height() - top - bottom);

    // 连接面板编辑信号
    connect(m_editPanel, &EditPanelTip::frequencyChanged, this, &EQCurveWidget::onFreqChangedFromPanel);
    connect(m_editPanel, &EditPanelTip::gainChanged, this, &EQCurveWidget::onGainChangedFromPanel);
    connect(m_editPanel, &EditPanelTip::qChanged, this, &EQCurveWidget::onQChangedFromPanel);
    connect(m_editPanel, &EditPanelTip::filterChanged, this, &EQCurveWidget::onFilterChangedFromPanel);

}
//设置 EQ 所有频段的参数（启用、频率、增益、Q 值）
void EQCurveWidget::setBands(const QVector<EQBand> &bands)
{
    m_bands = bands;
    m_draggedBandIndex = -1;//重置拖拽索引
    m_hoveredBandIndex = -1;//重置悬浮索引
    m_selectedBandIndex = -1;//重置选中的索引
    updateCurvePoints();
    update();// 触发重绘
}

void EQCurveWidget::hideEditPanelTip()
{
    m_editPanel->hide();
}

void EQCurveWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 计算绘图区域
    int left = PLOT_LEFT_MARGIN;
    int right = PLOT_RIGHT_MARGIN;
    int top = PLOT_TOP_MARGIN;
    int bottom = PLOT_BOTTOM_MARGIN;
    m_plotRect = QRect(left, top, width() - left - right, height() - top - bottom);


    // 绘制底部圆角背景（整体 widget 背景）
    QPainterPath backgroundPath;
    QRectF r = rect();
    qreal radius = 10;

    // 从左上角开始，顺时针绘制
    backgroundPath.moveTo(r.left(), r.top());                              // 左上角
    backgroundPath.lineTo(r.right(), r.top());                             // 上边 → 右上角
    backgroundPath.lineTo(r.right(), r.bottom() - radius);                 // 右边向下至右下圆角起点
    // 右下圆角：用二次贝塞尔曲线（控制点在右下角顶点）
    backgroundPath.quadTo(r.right(), r.bottom(),
                          r.right() - radius, r.bottom());
    backgroundPath.lineTo(r.left() + radius, r.bottom());                  // 底边至左下圆角起点
    // 左下圆角：用二次贝塞尔曲线（控制点在左下角顶点）
    backgroundPath.quadTo(r.left(), r.bottom(),
                          r.left(), r.bottom() - radius);
    backgroundPath.lineTo(r.left(), r.top());                              // 左边直线回到左上角

    // painter.fillPath(backgroundPath, QColor(81, 96, 122, 51));   // 整体背景alpha = 0.2 * 255 ≈ 51
    painter.fillPath(backgroundPath, Qt::transparent);   // alpha = 0.2 * 255 ≈ 51


    painter.fillRect(m_plotRect, QColor(0, 0, 0,76)); // 绘图区域

    // 绘制背景网格和坐标轴
    drawGrid(&painter);

    painter.save();
    painter.setPen(Qt::NoPen);
    // // 60Hz ~ 500Hz 区域
    // QRectF band1(freqToX(60.0), m_plotRect.top(),
    //              freqToX(500.0) - freqToX(60.0), m_plotRect.height());
    // painter.fillRect(band1, QColor(237, 175, 3, 25));   // rgba(237,175,3,0.1)

    // // 800Hz ~ 4000Hz 区域
    // QRectF band2(freqToX(800.0), m_plotRect.top(),
    //              freqToX(4000.0) - freqToX(800.0), m_plotRect.height());
    // painter.fillRect(band2, QColor(237, 69, 3, 25));    // rgba(237,69,3,0.1)

    // // 6000Hz ~ 10000Hz 区域
    // QRectF band3(freqToX(6000.0), m_plotRect.top(),
    //              freqToX(10000.0) - freqToX(6000.0), m_plotRect.height());
    // painter.fillRect(band3, QColor(3, 206, 237, 25));   // rgba(3,206,237,0.1)

    // 60Hz ~ 500Hz 区域：渐变 #fed259，整体不透明度 5%
    {
        QRectF band1(freqToX(60.0), m_plotRect.top(),
                     freqToX(500.0) - freqToX(60.0), m_plotRect.height());
        QColor yellow("#fed259");
        yellow.setAlphaF(0.05);                     // 底部 5% 不透明度
        QColor yellowTransparent = yellow;
        yellowTransparent.setAlpha(0);              // 顶部完全透明

        QLinearGradient grad1(band1.topLeft(), band1.bottomLeft());
        grad1.setColorAt(0.0, yellowTransparent);
        grad1.setColorAt(1.0, yellow);
        painter.fillRect(band1, grad1);
    }

    // 800Hz ~ 4000Hz 区域：渐变 #ff753f，整体不透明度 5%
    {
        QRectF band2(freqToX(800.0), m_plotRect.top(),
                     freqToX(4000.0) - freqToX(800.0), m_plotRect.height());
        QColor orange("#ff753f");
        orange.setAlphaF(0.05);
        QColor orangeTransparent = orange;
        orangeTransparent.setAlpha(0);

        QLinearGradient grad2(band2.topLeft(), band2.bottomLeft());
        grad2.setColorAt(0.0, orangeTransparent);
        grad2.setColorAt(1.0, orange);
        painter.fillRect(band2, grad2);
    }

    // 6000Hz ~ 10000Hz 区域：渐变 #4ee8ff，整体不透明度 5%
    {
        QRectF band3(freqToX(6000.0), m_plotRect.top(),
                     freqToX(10000.0) - freqToX(6000.0), m_plotRect.height());
        QColor cyan("#4ee8ff");
        cyan.setAlphaF(0.05);
        QColor cyanTransparent = cyan;
        cyanTransparent.setAlpha(0);

        QLinearGradient grad3(band3.topLeft(), band3.bottomLeft());
        grad3.setColorAt(0.0, cyanTransparent);
        grad3.setColorAt(1.0, cyan);
        painter.fillRect(band3, grad3);
    }

    painter.restore();// 恢复绘制频段背景前的状态

    // ===== 绘制顶部频段标签矩形（位于绘图区域上方） =====
    {
        struct FreqLabel {
            double startFreq;
            double endFreq;
            QString text;
        };
        const FreqLabel freqLabels[] = {
            { PLOT_MIN_FREQ, 60.0,   QString() },
            { 60.0,          500.0,  QStringLiteral("脚步声") },
            { 500.0,         800.0,  QString() },
            { 800.0,         4000.0, QStringLiteral("环境音") },
            { 4000.0,        6000.0, QString() },
            { 6000.0,        10000.0,QStringLiteral("枪声") },
            { 10000.0,       PLOT_MAX_FREQ, QString() }
        };

        const int labelHeight = 24;          // 矩形高度
        const int spacing = 1;               // 矩形间水平间距（像素）
        const int spacing2 = 3;               // 矩形与绘图区域的间距（像素）
        const double radius = 4.0;           // 圆角半径
        QColor bgColor(0, 0, 0, 77);        // 矩形颜色，rgba(0,0,0,0.3)
        QColor textColor(161, 166, 173, 153);     // 文字颜色，rgba(161, 166, 173, 0.6)

        QFont labelFont;
        labelFont.setFamily("Noto Sans S Chinese");   // 字体族名称
        labelFont.setWeight(QFont::Medium);
        labelFont.setPixelSize(10);
        // QColor labelColor(161, 166, 173, 153);

        painter.setFont(labelFont);
        painter.setPen(textColor);

        for (const auto &fl : freqLabels) {
            double x1 = freqToX(fl.startFreq);
            double x2 = freqToX(fl.endFreq) - spacing;  // 右侧扣除间距
            if (x2 <= x1) continue;

            // 关键修改：Y 坐标上移 labelHeight，使矩形位于绘图区上方
            QRectF rect(x1, m_plotRect.top() - labelHeight - spacing2, x2 - x1, labelHeight);

            QPainterPath path;
            path.addRoundedRect(rect, radius, radius);
            painter.fillPath(path, bgColor);

            // if (!fl.text.isEmpty()) {
            //     painter.drawText(rect, Qt::AlignCenter, fl.text);
            // }
            if (!fl.text.isEmpty()) {
                QFontMetrics fm(labelFont);
                // 获取文字的像素宽度
                int textWidth = fm.horizontalAdvance(fl.text);
                // 获取文字的像素高度（ascent + descent）
                int textHeight = fm.height();
                // 计算水平居中的 X
                qreal x = rect.center().x() - textWidth / 2.0;
                // 计算视觉垂直居中的 Y：矩形中心向下偏移一半高度，再加回基线以上部分（ascent）
                qreal y = rect.center().y() - textHeight / 2.0/* + fm.ascent()*/ + labelHeight/2;
                painter.drawText(QPointF(x, y), fl.text);
            }
        }
    }

   /* // ---------- 绘制竖直文字（字符正立） ----------
    QFont labelFont;
    labelFont.setFamily("Noto Sans S Chinese");   // 字体族名称
    labelFont.setWeight(QFont::Medium);
    labelFont.setPixelSize(52);
    QColor labelColor(0, 0, 0, 25);   // 10% 黑色
    painter.setFont(labelFont);
    painter.setPen(labelColor);

    auto drawVerticalLabel = [&](const QRectF &bandRect, const QString &text) {
        painter.save();
        QFontMetrics fm(labelFont);
        int charHeight = fm.height();               // 单个字符的高度，用作行距
        int totalHeight = charHeight * text.length();

        // 起始 Y 坐标：从区域中心向上偏移一半总高度，使整列文字垂直居中
        qreal startY = bandRect.center().y() - totalHeight / 2.0 + charHeight / 2.0;

        for (int i = 0; i < text.length(); ++i) {
            QString ch = text.at(i);
            int charWidth = fm.horizontalAdvance(ch);
            qreal x = bandRect.center().x() - charWidth / 2.0; // 水平居中
            qreal y = startY + i * charHeight;
            // 以 (x, y) 为中心绘制单个字符
            painter.drawText(QRectF(x, y - charHeight / 2.0, charWidth, charHeight),
                             Qt::AlignCenter, ch);
        }
        painter.restore();
    };

    drawVerticalLabel(band1, QStringLiteral("脚步声"));
    drawVerticalLabel(band2, QStringLiteral("环境音"));
    drawVerticalLabel(band3, QStringLiteral("枪声"));
*/
    // drawAxes(&painter);//绘制坐标轴线
    drawAxisLabels(&painter);//绘制坐标轴标签

    if (m_curvePoints.isEmpty())
        return;

    //曲线区域
    // 绘制总曲线
    QPainterPath path;
    bool firstPoint = true;
    // for (const QPointF &point : m_curvePoints) {
    for (const QPointF &point : std::as_const(m_curvePoints))
    {
        double x = freqToX(point.x());
        double y = gainToY(point.y());
        if (m_plotRect.contains(QPoint(x, y))){  //整个QWidget范围内 || (x >= m_plotRect.left() && x <= m_plotRect.right())) {
            if (firstPoint) {
                path.moveTo(x, y);
                firstPoint = false;
            } else {
                path.lineTo(x, y);
            }
        } else {
            // 如果点超出绘图区域，断开路径
            firstPoint = true;
        }
    }
    painter.setPen(QPen(Qt::white, 1.5));
    painter.drawPath(path);

    // 颜色数组
    static const QColor bandColors[] = {
        QColor(QRgb(0xD5695F)), QColor(QRgb(0xF6B044)), QColor(QRgb(0x51BE83)),
        QColor(QRgb(0x92DFD1)), QColor(QRgb(0x5FACE2)), QColor(QRgb(0xA469BD)),
        QColor(QRgb(0x8B746A)), QColor(QRgb(0xFEBCAA)), QColor(QRgb(0x484E97)),
        QColor(QRgb(0xFFFF84))
    };

    int highlightIndex = (m_selectedBandIndex != -1) ? m_selectedBandIndex : m_hoveredBandIndex;

    // 绘制频点标记（每个频点使用对应的颜色）
    for (int i = 0; i < m_bands.size(); ++i) {
        const EQBand &band = m_bands[i];
        if (band.enabled && band.frequency >= MIN_FREQ && band.frequency <= MAX_FREQ) {
            double x = freqToX(band.frequency);
            double y = gainToY(band.gain); // 用户设定的增益
            if (m_plotRect.contains(QPoint(x, y))) {
                painter.setBrush(bandColors[i % 10]);
                painter.setPen(QPen(bandColors[i % 10], 2, Qt::SolidLine));
                if (i == highlightIndex) {
                    //该频点高亮显示时（鼠标悬浮），⚪️——🟠——⚪️
                    // const double offset = 45.0;// 圆心距（边缘间距36 + 左半径3.5 + 中半径5.5）
                    const double offset = knobOffset(band.q);
                    double leftCenterX = x - offset;
                    double rightCenterX = x + offset;
                    // 绘制直线连接圆心
                    painter.drawLine(QPointF(leftCenterX, y), QPointF(x, y));
                    painter.drawLine(QPointF(x, y), QPointF(rightCenterX, y));
                     // 左圆（直径7）
                    painter.drawEllipse(QRectF(leftCenterX - 3.5, y - 3.5, 7, 7));
                    // 中圆（直径11）
                    painter.drawEllipse(QRectF(x - 5.5, y - 5.5, 11, 11));
                    // 右圆（直径7）
                    painter.drawEllipse(QRectF(rightCenterX - 3.5, y - 3.5, 7, 7));
                } else {
                    //该频点非高亮显示时，🟠，单圆（直径11）
                    painter.drawEllipse(QRectF(x - 5.5, y - 5.5, 11, 11));
                }
            }
        }
    }

    // 绘制高亮频点的单独曲线（带半透明填充）
    if (highlightIndex >= 0 && highlightIndex < m_bands.size()) {
        const EQBand &highlightedBand = m_bands[highlightIndex];
        if (highlightedBand.enabled) {
            QVector<EQBand> soloBands = { highlightedBand };
            QVector<QPointF> soloPoints = calculateCurvePointsForBands(soloBands);
            if (!soloPoints.isEmpty()) {
                // 构建曲线路径和填充路径
                QPainterPath soloPath, fillPath;
                bool first = true;
                for (const QPointF &point : soloPoints) {
                    double x = freqToX(point.x());
                    double y = gainToY(point.y());
                    if (first) {
                        soloPath.moveTo(x, y);
                        fillPath.moveTo(x, y);
                        first = false;
                    } else {
                        soloPath.lineTo(x, y);
                        fillPath.lineTo(x, y);
                    }
                }
                 // 闭合填充路径：连接到绘图区域右下角、左下角
                fillPath.lineTo(m_plotRect.bottomRight());
                fillPath.lineTo(m_plotRect.bottomLeft());
                fillPath.closeSubpath();

                 // 设置半透明画刷（颜色与曲线一致，alpha=80）
                QColor fillColor = bandColors[highlightIndex % 10];
                fillColor.setAlpha(25);
                painter.setBrush(fillColor);
                painter.setPen(Qt::NoPen);// 填充区域不要轮廓
                painter.drawPath(fillPath);
                // 绘制虚线曲线（线宽2，虚线样式）
                painter.setPen(QPen(bandColors[highlightIndex % 10], 2, Qt::DashLine));
                painter.setBrush(Qt::NoBrush);// 确保曲线不填充
                painter.drawPath(soloPath);
            }
        }
    }

    // 处于遮罩状态，在绘图区域覆盖半透明黑色
    if (m_disableOverlay) {
        painter.fillRect(m_plotRect, QColor(0, 0, 0, 128));  // 50% 不透明度
    }
}

void EQCurveWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateCurvePoints();// 窗口大小改变时重新采样曲线点
    if (m_editPanel && m_editPanel->isVisible()) {
        int activeIndex = (m_selectedBandIndex != -1) ? m_selectedBandIndex : m_hoveredBandIndex;
        if (activeIndex != -1 && activeIndex < m_bands.size()) {
            showEditPanelForBand(activeIndex);
        }
    }
}
//将频率值（Hz，对数坐标）转换为绘图区域中的 X 坐标（像素）
double EQCurveWidget::freqToX(double freq) const
{
    double logFreq = std::log10(freq);
    double logMin = std::log10(PLOT_MIN_FREQ);
    double logMax = std::log10(PLOT_MAX_FREQ);
    double t = (logFreq - logMin) / (logMax - logMin);
    return m_plotRect.left() + t * m_plotRect.width();
}
//将增益值（dB）转换为绘图区域中的 Y 坐标（像素）
double EQCurveWidget::gainToY(double gain) const
{
    double t = (gain - PLOT_MIN_GAIN) / (PLOT_MAX_GAIN - PLOT_MIN_GAIN);
    return m_plotRect.bottom() - t * m_plotRect.height();
}
// X 坐标转换为频率值
double EQCurveWidget::xToFreq(double x) const
{
    double t = (x - m_plotRect.left()) / m_plotRect.width();
    double logFreq = std::log10(PLOT_MIN_FREQ) + t * (std::log10(MAX_FREQ) - std::log10(PLOT_MIN_FREQ));
    return std::pow(10.0, logFreq);
}

//Y 坐标转换为增益值
double EQCurveWidget::yToGain(double y) const
{
    double t = (m_plotRect.bottom() - y) / m_plotRect.height();
    return PLOT_MIN_GAIN + t * (PLOT_MAX_GAIN - PLOT_MIN_GAIN);
}
//计算单个频段在频率 f 处的增益贡献（dB），十组最后累加
double EQCurveWidget::calculateBandGain(const EQBand &band, double f) const
{
    if (!band.enabled) return 0.0;

    double b0, b1, b2, a0, a1, a2;
    computeBiquadCoefficients(band.filterType, band.frequency, band.gain, band.q, SAMPLE_RATE,
                              b0, b1, b2, a0, a1, a2);

    double magnitude = computeMagnitudeResponse(b0, b1, b2, a0, a1, a2, f, SAMPLE_RATE);
    if (magnitude <= 0.0) return -120.0;
    return 20.0 * std::log10(magnitude);
}
//计算所有启用的频段在频率 f 处的总增益（dB）
double EQCurveWidget::calculateTotalGain(double f) const
{
    double total = 0.0;
    for (const EQBand &band : m_bands) {
        total += calculateBandGain(band, f);
    }
    return total;
}
//更新曲线采样点数组 m_curvePoints
void EQCurveWidget::updateCurvePoints()
{
    if (m_plotRect.width() <= 0 || m_plotRect.height() <= 0)
        return;

    m_curvePoints.clear();
    double logMin = std::log10(PLOT_MIN_FREQ);
    double logMax = std::log10(PLOT_MAX_FREQ);
    for (int i = 0; i <= SAMPLE_POINTS; ++i) {
        double t = static_cast<double>(i) / SAMPLE_POINTS;
        double logFreq = logMin + t * (logMax - logMin);
        double freq = std::pow(10.0, logFreq);
        double gain = calculateTotalGain(freq);
        m_curvePoints.append(QPointF(freq, gain));
    }
}
//绘制垂直水平虚线
void EQCurveWidget::drawGrid(QPainter *painter)
{
    if (!m_plotRect.isValid()) return;
    painter->save();
    // painter->setPen(QPen(QColor(255, 255, 255, 25), 1, Qt::SolidLine));
    painter->setPen(QPen(QColor(255, 255, 255, 8), 1, Qt::SolidLine));
    painter->setClipRect(m_plotRect);

    for (double gain = -12.0; gain <= 12.01; gain += 6.0) {
        double y = gainToY(gain);
        if (y >= m_plotRect.top() && y <= m_plotRect.bottom())
            painter->drawLine(m_plotRect.left(), y, m_plotRect.right(), y);
    }

    double freqs[] = {30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
    // double freqs[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    // double freqs[] = {30, 40, 50, 60, 80, 100, 200, 300, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 5000, 6000, 8000, 10000};
    for (double freq : freqs) {
        double x = freqToX(freq);
        if (x >= m_plotRect.left() && x <= m_plotRect.right())
            painter->drawLine(x, m_plotRect.top(), x, m_plotRect.bottom());
    }
    painter->restore();
}
//绘制x轴线与y轴线
void EQCurveWidget::drawAxes(QPainter *painter)
{
    painter->save();
    painter->setPen(QPen(Qt::black, 0));
    painter->drawLine(m_plotRect.left(), m_plotRect.bottom(), m_plotRect.right(), m_plotRect.bottom());
    painter->drawLine(m_plotRect.left(), m_plotRect.top(), m_plotRect.left(), m_plotRect.bottom());
    painter->restore();
}
//绘制标题与值
void EQCurveWidget::drawAxisLabels(QPainter *painter)
{
    painter->save();
    painter->setPen(QColor(161, 166, 173, 153));
    QFont font = painter->font();
    font.setFamily("Noto Sans S Chinese");
    font.setWeight(QFont::Medium);
    font.setPointSize(10);
    painter->setFont(font);

    for (double gain = -12.0; gain <= 12.01; gain += 6.0) {
        double y = gainToY(gain);
        if (y >= m_plotRect.top() - 5 && y <= m_plotRect.bottom() + 5) {
            // QString label = QString::number(gain, 'f', 0) ;
            QString label = (gain > 0 ? "+" : "") + QString::number(gain, 'f', 0);
            painter->drawText(m_plotRect.left() - 35, y + 5, label);
        }
    }

    double freqs[] = {30, 40, 50, 60, 80, 100, 200, 300, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 5000, 6000, 8000, 10000};
    // double freqs[] = {30, 40, 50, 60, 80, 100, 200, 300, 400, 500, 600, 800, 1000, 2000, 3000, 4000, 5000, 6000, 8000, 10000};
    for (double freq : freqs) {
        double x = freqToX(freq);
        if (x >= m_plotRect.left() - 10 && x <= m_plotRect.right() + 10)
        {
            // QString label = (freq >= 1000) ? QString::number(freq/1000, 'f', 0) + "kHz" : QString::number(freq, 'f', 0) + "Hz";
            QString label = (freq >= 1000) ? QString::number(freq/1000, 'f', 0) + "k" : QString::number(freq, 'f', 0);
            // painter->drawText(x - 10, m_plotRect.bottom() + 28, label);
            // 使用矩形边界居中绘制，确保对齐精准
            QRect rect(x - 20, m_plotRect.bottom() + 11, 40, 20);
            painter->drawText(rect, Qt::AlignHCenter | Qt::AlignTop, label);
        }
    }

    // 在 X 轴最右侧绘制单位 (Hz)，与刻度同一条线
    {
        int labelY = m_plotRect.bottom() + 11;
        int labelH = 17;
        int unitX = m_plotRect.right() - PLOT_RIGHT_MARGIN;          // 放在右边界外侧
        QRect unitRect(unitX, labelY, 30, labelH);
        painter->drawText(unitRect, Qt::AlignLeft | Qt::AlignTop, "(Hz)");
    }

    // 在 Y 轴最顶部绘制单位 (dB)，与刻度同一条线
    {
        int labelY = m_plotRect.top();
        int labelH = 17;
        int unitX = m_plotRect.left() - 35;          // 放在右边界外侧
        QRect unitRect(unitX, labelY, 30, labelH);
        painter->drawText(unitRect, Qt::AlignLeft | Qt::AlignTop, "(dB)");
    }


    painter->save();
    painter->translate(m_plotRect.left() - 40, m_plotRect.center().y());
    painter->rotate(-90);

    painter->restore();

}

QVector<QPointF> EQCurveWidget::calculateCurvePointsForBands(const QVector<EQBand> &bands) const
{
    QVector<QPointF> points;
    if (m_plotRect.width() <= 0 || m_plotRect.height() <= 0)
        return points;

    double logMin = std::log10(PLOT_MIN_FREQ);
    double logMax = std::log10(PLOT_MAX_FREQ);
    for (int i = 0; i <= SAMPLE_POINTS; ++i) {
        double t = static_cast<double>(i) / SAMPLE_POINTS;
        double logFreq = logMin + t * (logMax - logMin);
        double freq = std::pow(10.0, logFreq);
        double totalGain = 0.0;
        for (const EQBand &band : bands) {
            if (band.enabled)
                totalGain += calculateBandGain(band, freq);
        }
        points.append(QPointF(freq, totalGain));
    }
    return points;
}

void EQCurveWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_disableOverlay) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    int left = PLOT_LEFT_MARGIN;
    int right = PLOT_RIGHT_MARGIN;
    int top = PLOT_TOP_MARGIN;
    int bottom = PLOT_BOTTOM_MARGIN;
    QRect tempPlotRect(left, top, width() - left - right, height() - top - bottom);

    auto freqToXLocal = [this, tempPlotRect](double freq) -> double {
        double logFreq = std::log10(freq);
        double logMin = std::log10(PLOT_MIN_FREQ);
        double logMax = std::log10(PLOT_MAX_FREQ);
        double t = (logFreq - logMin) / (logMax - logMin);
        return tempPlotRect.left() + t * tempPlotRect.width();
    };
    auto gainToYLocal = [this, tempPlotRect](double gain) -> double {
        double t = (gain - PLOT_MIN_GAIN) / (PLOT_MAX_GAIN - PLOT_MIN_GAIN);
        return tempPlotRect.bottom() - t * tempPlotRect.height();
    };
    auto xToFreqLocal = [this, tempPlotRect](double x) -> double {
        double t = (x - tempPlotRect.left()) / tempPlotRect.width();
        double logMin = std::log10(PLOT_MIN_FREQ);
        double logMax = std::log10(PLOT_MAX_FREQ);
        double logFreq = logMin + t * (logMax - logMin);
        return std::pow(10.0, logFreq);
    };
    auto yToGainLocal = [this, tempPlotRect](double y) -> double {
        double t = (tempPlotRect.bottom() - y) / tempPlotRect.height();
        return PLOT_MIN_GAIN + t * (PLOT_MAX_GAIN - PLOT_MIN_GAIN);
    };

    //Q值改变（鼠标向中心靠拢Q值增加，远离中心Q值减少）
    if (m_draggingQ && m_draggedBandIndex != -1) {
        EQBand &band = m_bands[m_draggedBandIndex];
        // 当前频点的屏幕 X 坐标（中心圆）
        double centerX = freqToX(band.frequency);
        // 起始位置与中心的水平距离（带符号）
        double startDist = m_dragStartPos.x() - centerX;
        // 当前位置与中心的水平距离（带符号）
        double curDist = event->pos().x() - centerX;
        // 靠近中心：abs 变小 → delta 为正 → Q 增加
        // 远离中心：abs 变大 → delta 为负 → Q 减少
        double delta = std::abs(startDist) - std::abs(curDist);
        double qSensitivity = 0.2;   // 灵敏度（每像素改变 0.2 Q 值）
        double newQ = m_dragStartQ + delta * qSensitivity;
        newQ = qBound(0.1, newQ, 10.0);
        newQ = std::round(newQ * 10.0) / 10.0;

        if (!qFuzzyCompare(band.q, newQ)) {
            band.q = newQ;
            updateCurvePoints();
            update();                           // 重绘，偏移量自动更新
            showEditPanelForBand(m_draggedBandIndex);
        }
        QWidget::mouseMoveEvent(event);
        return;
    }

    //增益、频率改变
    if (m_draggedBandIndex != -1 && !m_draggingQ) {
        if (m_draggedBandIndex >= 0 && m_draggedBandIndex < m_bands.size()) {
            double newGain = yToGainLocal(event->pos().y());
            newGain = qBound(MIN_GAIN, newGain, MAX_GAIN);
            newGain = std::round(newGain * 10.0) / 10.0;
            double newFreq = xToFreqLocal(event->pos().x());
            newFreq = qBound(MIN_FREQ, newFreq, MAX_FREQ);
            newFreq = std::round(newFreq * 10.0) / 10.0;

            EQBand &band = m_bands[m_draggedBandIndex];
            bool gainChanged = !qFuzzyCompare(band.gain, newGain);
            bool freqChanged = !qFuzzyCompare(band.frequency, newFreq);

            if (gainChanged || freqChanged) {
                if (gainChanged) band.gain = newGain;
                if (freqChanged) band.frequency = newFreq;
                updateCurvePoints();
                update();

                // if (gainChanged) emit bandGainChanged(m_draggedBandIndex, newGain);//实时变化APO
                // if (freqChanged) emit bandFrequencyChanged(m_draggedBandIndex, newFreq);
            }

            if (m_selectedBandIndex != m_draggedBandIndex) {
                m_selectedBandIndex = m_draggedBandIndex;
                update();
            }
            if (m_hoveredBandIndex != m_draggedBandIndex) {
                m_hoveredBandIndex = m_draggedBandIndex;
            }
            showEditPanelForBand(m_draggedBandIndex);
        } else {
            m_draggedBandIndex = -1;
            setCursor(Qt::ArrowCursor);
        }
        QWidget::mouseMoveEvent(event);
        return;
    }


    // 悬停检测
    int newHoverIndex = -1;
    for (int i = 0; i < m_bands.size(); ++i) {
        HitPart part = hitTestBandPart(event->pos(), i);
        if (part != HitNone) {
            newHoverIndex = i;
            break;
        }
    }

    if (newHoverIndex != m_hoveredBandIndex) {
        m_hoveredBandIndex = newHoverIndex;
        if (m_hoveredBandIndex != -1 && m_hoveredBandIndex != m_selectedBandIndex) {
            m_selectedBandIndex = -1;
        }
        if (m_hoveredBandIndex != -1) {
            HitPart part = hitTestBandPart(event->pos(), m_hoveredBandIndex);
            if (part == HitLeftKnob || part == HitRightKnob)
                setCursor(Qt::SizeHorCursor);
            else
                setCursor(Qt::OpenHandCursor);
            showEditPanelForBand(m_hoveredBandIndex);
        } else {
            setCursor(Qt::ArrowCursor);
            if (m_selectedBandIndex == -1 && !m_editPanel->geometry().contains(event->globalPos())) {
                hideEditPanel();
            }
        }
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void EQCurveWidget::leaveEvent(QEvent *event)
{
    if (m_disableOverlay) {
        QWidget::leaveEvent(event);
        return;
    }
    if (m_hoveredBandIndex != -1) {
        m_hoveredBandIndex = -1;
        update();
        if (m_selectedBandIndex == -1) {
            hideEditPanel();
        }
    }
    QWidget::leaveEvent(event);
}

void EQCurveWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_disableOverlay) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        // 如果编辑面板可见，且点击位置在面板区域内，则不做任何处理（不清除选中）
        if (m_editPanel->isVisible() && m_editPanel->geometry().contains(event->pos())) {
            QWidget::mousePressEvent(event);
            return;
        }

        // 遍历所有频段，精确查找被点击的部分
        int foundIdx = -1;
        HitPart foundPart = HitNone;
        for (int i = 0; i < m_bands.size(); ++i) {
            HitPart part = hitTestBandPart(event->pos(), i);
            if (part != HitNone) {
                foundIdx = i;
                foundPart = part;
                break;
            }
        }

        if (foundIdx != -1) {

            if (foundPart == HitLeftKnob || foundPart == HitRightKnob) {
                // 开始拖动 Q 值
                m_draggedBandIndex = foundIdx;
                m_selectedBandIndex = foundIdx;
                m_draggingQ = true;
                m_dragStartQ = m_bands[foundIdx].q;
                m_dragStartPos = event->pos();
                setCursor(Qt::SizeHorCursor);
                showEditPanelForBand(foundIdx);
                update();
            } else if (foundPart == HitCenter || foundPart == HitLine) {
                // 开始拖动频率/增益
                m_draggedBandIndex = foundIdx;
                m_selectedBandIndex = foundIdx;
                m_draggingQ = false;
                m_dragStartGain = m_bands[foundIdx].gain;
                m_dragStartFreq = m_bands[foundIdx].frequency;
                setCursor(Qt::ClosedHandCursor);
                showEditPanelForBand(foundIdx);
                update();
            }
        } else {
            // 点击空白处，取消选中
            m_selectedBandIndex = -1;
            m_draggedBandIndex = -1;
            m_draggingQ = false;
            hideEditPanel();
            update();
        }
    }
    QWidget::mousePressEvent(event);
}

void EQCurveWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_disableOverlay) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton && m_draggedBandIndex != -1) {
        if (m_draggingQ) {
            int idx = m_draggedBandIndex;
            if (idx >= 0 && idx < m_bands.size()) {
                if (!qFuzzyCompare(m_dragStartQ, m_bands[idx].q))
                    emit bandQChanged(idx, m_bands[idx].q);
            }
            m_draggingQ = false;
            m_draggedBandIndex = -1;
            setCursor(Qt::ArrowCursor);
        } else{
            int idx = m_draggedBandIndex;
            if (idx >= 0 && idx < m_bands.size()) {
                const EQBand &band = m_bands[idx];
                // 增益变化则发射
                if (!qFuzzyCompare(m_dragStartGain, band.gain))
                    emit bandGainChanged(idx, band.gain);
                // 频率变化则发射
                if (!qFuzzyCompare(m_dragStartFreq, band.frequency))
                    emit bandFrequencyChanged(idx, band.frequency);
            }
            m_draggedBandIndex = -1;
            setCursor(Qt::ArrowCursor);
        }
    }
    QWidget::mouseReleaseEvent(event);
}
//面板上频点改变
void EQCurveWidget::onFreqChangedFromPanel(int index, double freq)
{
    if (index < 0 || index >= m_bands.size()) return;
    m_bands[index].frequency = freq;
    updateCurvePoints();
    update();
    emit bandFrequencyChanged(index, freq);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}
//面板上增益值改变
void EQCurveWidget::onGainChangedFromPanel(int index, double gain)
{
    if (index < 0 || index >= m_bands.size()) return;
    m_bands[index].gain = gain;
    updateCurvePoints();
    update();
    emit bandGainChanged(index, gain);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}
//面板上Q值改变
void EQCurveWidget::onQChangedFromPanel(int index, double q)
{
    if (index < 0 || index >= m_bands.size()) return;
    m_bands[index].q = q;
    updateCurvePoints();
    update();
    emit bandQChanged(index, q);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}
//面板上滤波器改变
void EQCurveWidget::onFilterChangedFromPanel(int index, int filterType)
{
    qDebug() << "Before switch:" << m_bands[index].frequency << m_bands[index].gain << m_bands[index].q;
    if (index < 0 || index >= m_bands.size()) return;
    m_bands[index].filterType = filterType;
    updateCurvePoints();
    update();
    emit bandFilterTypeChanged(index, filterType);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }

    // ... 执行切换 ...
    qDebug() << "After filter change, bands:";
    for (int i = 0; i < m_bands.size(); ++i) {
        const auto &b = m_bands[i];
        qDebug() << i << b.enabled << b.frequency << b.gain << b.q << b.filterType;
    }
}

void EQCurveWidget::showEditPanelForBand(int index)
{
    if (index < 0 || index >= m_bands.size()) return;
    const EQBand &band = m_bands[index];

    int left = PLOT_LEFT_MARGIN;
    int right = PLOT_RIGHT_MARGIN;
    int top = PLOT_TOP_MARGIN;
    int bottom = PLOT_BOTTOM_MARGIN;
    QRect tempPlotRect(left, top, width() - left - right, height() - top - bottom);

    auto freqToXLocal = [tempPlotRect](double freq) -> double {
        double logFreq = std::log10(freq);
        double logMin = std::log10(PLOT_MIN_FREQ);
        double logMax = std::log10(PLOT_MAX_FREQ);
        double t = (logFreq - logMin) / (logMax - logMin);
        return tempPlotRect.left() + t * tempPlotRect.width();
    };
    auto gainToYLocal = [tempPlotRect](double gain) -> double {
        double t = (gain - PLOT_MIN_GAIN) / (PLOT_MAX_GAIN - PLOT_MIN_GAIN);
        return tempPlotRect.bottom() - t * tempPlotRect.height();
    };

    double px = freqToXLocal(band.frequency);
    double py = gainToYLocal(band.gain);
    QPoint localPos(static_cast<int>(px), static_cast<int>(py));

    m_editPanel->setBandData(index, band.frequency, band.gain, band.q, band.filterType);
    m_editPanel->adjustSize();

    int panelWidth = m_editPanel->width();
    int panelHeight = m_editPanel->height();

    int localX = localPos.x() - panelWidth / 2;
    int localY = localPos.y() - panelHeight - 10;

    if (localX < 0)
        localX = 0;
    else if (localX + panelWidth > width())
        localX = width() - panelWidth;
    if (localY < 0)
        localY = localPos.y() + 10;

    m_editPanel->move(localX, localY);
    m_editPanel->show();
    m_editPanel->raise();
}

void EQCurveWidget::hideEditPanel()
{
    if (m_editPanel && m_editPanel->isVisible()) {
        m_editPanel->hide();
    }
}

//用于检测给定的鼠标位置（QPoint）是否落在某个有效频段的圆形标记上，并返回该频段的索引。如果未命中则返回 -1
int EQCurveWidget::hitTestBand(const QPoint &pos) const
{
    int left = PLOT_LEFT_MARGIN;
    int right = PLOT_RIGHT_MARGIN;
    int top = PLOT_TOP_MARGIN;
    int bottom = PLOT_BOTTOM_MARGIN;
    QRect tempPlotRect(left, top, width() - left - right, height() - top - bottom);

    auto freqToXLocal = [this, tempPlotRect](double freq) -> double {
        double logFreq = std::log10(freq);
        double logMin = std::log10(PLOT_MIN_FREQ);
        double logMax = std::log10(PLOT_MAX_FREQ);
        double t = (logFreq - logMin) / (logMax - logMin);
        return tempPlotRect.left() + t * tempPlotRect.width();
    };
    auto gainToYLocal = [this, tempPlotRect](double gain) -> double {
        double t = (gain - PLOT_MIN_GAIN) / (PLOT_MAX_GAIN - PLOT_MIN_GAIN);
        return tempPlotRect.bottom() - t * tempPlotRect.height();
    };

    const double threshold = 5.0;
    for (int i = 0; i < m_bands.size(); ++i) {
        const EQBand &band = m_bands[i];
        if (!band.enabled) continue;
        double x = freqToXLocal(band.frequency);
        double y = gainToYLocal(band.gain);
        double dx = pos.x() - x;
        double dy = pos.y() - y;
        if (std::sqrt(dx*dx + dy*dy) < threshold)
            return i;
    }
    return -1;
}

bool EQCurveWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_editPanel) {
        if (event->type() == QEvent::Leave) {
            if (m_hoveredBandIndex == -1 && m_selectedBandIndex == -1) {
                hideEditPanel();
            }
        }
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

//获得所有点的频率
void EQCurveWidget::AllFreq(QVector<double> &FVal)
{
    int i = 0;
    for(i = 0; i < 10; i++)
    {
        FVal.append(m_bands[i].frequency);
    }
    for(;i < 20; i++)
    {
        FVal.append(20000.0); //参数不能为0，会破音卡顿
    }
}
//获得所有Q值
void EQCurveWidget::AllQVal(QVector<double> &QVal)
{
    int i = 0;

    for(i = 0; i < m_bands.size(); i++)
    {
        QVal.append(m_bands[i].q);
    }
    for(;i < 20; i++)
    {
        QVal.append(0.7); //参数不能为0，会破音卡顿
    }
}
//获得某一个频点的增益值
double EQCurveWidget::GetIndexGain(int idx)
{
    if (idx >= 0 && idx < m_bands.size())
        return m_bands[idx].gain;

    // 越界时返回安全默认值（例如 0 dB）
    return 0.0;
}
//获得某一个频点的频率值
double EQCurveWidget::GetIndexFreq(int idx)
{
    if (idx >= 0 && idx < m_bands.size())
        return m_bands[idx].frequency;

    // 越界时返回安全默认值（例如 0 dB）
    return 0.0;
}
//获得某一个频点的q值
double EQCurveWidget::GetIndexQ(int idx)
{
    if (idx >= 0 && idx < m_bands.size())
        return m_bands[idx].q;

    // 越界时返回安全默认值（例如 0 dB）
    return 0.0;
}
//获得某一个频点的滤波器值
int EQCurveWidget::GetIndexFilter(int idx)
{
    if (idx >= 0 && idx < m_bands.size())
        return m_bands[idx].filterType;

    // 越界时返回安全默认值（例如 0 dB）
    return 5;
}

//撤销回退--设置某一点的增益
void EQCurveWidget::setBandGainInternal(int index, double gain)
{
    if (index < 0 || index >= m_bands.size()) return;
    if(m_bands[index].gain == gain) return;
    m_bands[index].gain = gain;
    updateCurvePoints();
    update();
    emit bandGainChanged(index, gain);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}
//撤销回退--设置某一点的频率
void EQCurveWidget::setBandFrequencyInternal(int index, double freq)
{
    if (index < 0 || index >= m_bands.size()) return;
    if(m_bands[index].frequency == freq) return;
    m_bands[index].frequency = freq;
    updateCurvePoints();
    update();
    emit bandFrequencyChanged(index, freq);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}
//撤销回退--设置某一点的Q值
void EQCurveWidget::setBandQInternal(int index, double q)
{
    if (index < 0 || index >= m_bands.size()) return;
    if(m_bands[index].q == q) return;
    m_bands[index].q = q;
    updateCurvePoints();
    update();
    emit bandQChanged(index, q);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}
//撤销回退--设置某一点的滤波器值
void EQCurveWidget::setBandFilterInternal(int index, int filterType)
{
    if (index < 0 || index >= m_bands.size()) return;
    if(m_bands[index].filterType == filterType) return;
    m_bands[index].filterType = filterType;
    updateCurvePoints();
    update();
    emit bandFilterTypeChanged(index, filterType);
    if (m_editPanel->isVisible() && (m_selectedBandIndex == index || m_hoveredBandIndex == index)) {
        showEditPanelForBand(index);
    }
}

void EQCurveWidget::setDisableOverlay(bool disabled)
{
    m_disableOverlay = disabled;
    update();  // 触发重绘
}


//返回点击部位类型
HitPart EQCurveWidget::hitTestBandPart(const QPoint &pos, int bandIndex) const
{
    if (bandIndex < 0 || bandIndex >= m_bands.size())
        return HitNone;
    const EQBand &band = m_bands[bandIndex];
    if (!band.enabled) return HitNone;

    // 计算频点在屏幕上的坐标
    double x = freqToX(band.frequency);
    double y = gainToY(band.gain);
    QPointF center(x, y);

    // const double offset = 45.0;
    const double offset = knobOffset(band.q);
    const double leftCenterX = x - offset;
    const double rightCenterX = x + offset;
    const double centerRadius = 5.5;
    const double knobRadius = 3.5;
    const double hitThreshold = 5.0;  // 额外的点击容差

    auto dist = [](const QPointF &a, const QPointF &b) -> double {
        return std::sqrt((a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y()));
    };

    QPointF leftKnob(leftCenterX, y);
    QPointF rightKnob(rightCenterX, y);
    QPointF centerPt(x, y);

    double distLeft = dist(pos, leftKnob);
    double distRight = dist(pos, rightKnob);
    double distCenter = dist(pos, centerPt);

    // 优先检测左右小圆（半径小，容易误触，所以先判断）
    if (distLeft <= knobRadius + hitThreshold && distLeft < distCenter)
        return HitLeftKnob;
    if (distRight <= knobRadius + hitThreshold && distRight < distCenter)
        return HitRightKnob;

    // 再检测中心圆
    if (distCenter <= centerRadius + hitThreshold)
        return HitCenter;

    // 检测连接线（水平线附近，避免太远）
    // 垂直线附近但未命中任何圆时，也可视为线上
    if (pos.x() >= leftCenterX - hitThreshold && pos.x() <= rightCenterX + hitThreshold &&
        std::abs(pos.y() - y) <= hitThreshold) {
        return HitLine;
    }

    return HitNone;
}
//根据 Q 值计算小圆偏移距离
double EQCurveWidget::knobOffset(double q) const
{
    //Q 值增加（带宽变窄）,左右小圆向中心靠拢，连线距离变小。
    //Q 值减少（带宽变宽）,左右小圆向外扩散，连线距离变大。
    const double minQ = 0.1, maxQ = 10.0;
    const double minOffset = 15.0, maxOffset = 85.0;
    double t = (q - minQ) / (maxQ - minQ);
    return maxOffset - t * (maxOffset - minOffset);
}
