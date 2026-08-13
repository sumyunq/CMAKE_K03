#ifndef EQCURVEWIDGET_H
#define EQCURVEWIDGET_H

/********************* EQ波形图 *********************/

#include <QWidget>
#include <QVector>
#include "EQCurve/EditPanelTip.h"

// EQ 频段数据结构
struct EQBand {
    bool enabled = true;        // 是否启用该频段
    double frequency = 1000.0;  // 中心频率 (Hz)
    double gain = 0.0;          // 增益 (dB)
    double q = 1.0;             // Q值 (品质因数)
    int filterType = 0;         //滤波器
};
enum HitPart { HitNone = 0, HitCenter, HitLeftKnob, HitRightKnob, HitLine };

class EQCurveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EQCurveWidget(QWidget *parent = nullptr);


    // ========== 供撤销回退调用的内部修改接口（改数据，刷新 UI，发射信号） ==========
    void setBandGainInternal(int index, double gain);
    void setBandFrequencyInternal(int index, double freq);
    void setBandQInternal(int index, double q);
    void setBandFilterInternal(int index, int filterType);

    void setDisableOverlay(bool disable); // true：覆盖  false：不覆盖

    void hideEditPanelTip();//隐藏数据面板
    // 设置所有 EQ 频段
    void setBands(const QVector<EQBand> &bands);

    void AllFreq(QVector<double> &FVal);
    void AllQVal(QVector<double> &QVal);

    double GetIndexGain(int idx);
    double GetIndexFreq(int idx);
    double GetIndexQ(int idx);
    int GetIndexFilter(int idx);

    void hideEditPanel();//隐藏面板

    // 交互辅助
    int hitTestBand(const QPoint &pos) const;

signals:
    void bandGainChanged(int index, double gain);
    void bandFrequencyChanged(int index, double freq);
    void bandQChanged(int index, double q);
    void bandFilterTypeChanged(int index, int filterType);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onFreqChangedFromPanel(int index, double freq);
    void onGainChangedFromPanel(int index, double gain);
    void onQChangedFromPanel(int index, double q);
    void onFilterChangedFromPanel(int index, int filterType);

private:

    // 绘图辅助
    void drawGrid(QPainter *painter);
    void drawAxes(QPainter *painter);
    void drawAxisLabels(QPainter *painter);

    // 坐标转换（基于当前 m_plotRect）
    double freqToX(double freq) const;
    double gainToY(double gain) const;
    double xToFreq(double x) const;
    double yToGain(double y) const;

    // 曲线计算
    double calculateBandGain(const EQBand &band, double f) const;
    double calculateTotalGain(double f) const;
    void updateCurvePoints();
    QVector<QPointF> calculateCurvePointsForBands(const QVector<EQBand> &bands) const;

    // 交互辅助
    // int hitTestBand(const QPoint &pos) const;    // WBLIU:转 public
    void showEditPanelForBand(int index);// 将编辑面板定位到频点正上方

    //高亮显示时，移动旁边两个🟠，修改Q值
    //返回点击部件的类型
    HitPart hitTestBandPart(const QPoint &pos, int bandIndex) const;


    // 绘图区域矩形（缓存）
    QRect m_plotRect;

    // 数据
    QVector<EQBand> m_bands;
    QVector<QPointF> m_curvePoints;// 总曲线采样点 (freq, gain)

    // 交互状态
    int m_draggedBandIndex = -1;// 正在拖拽的频点索引
    int m_hoveredBandIndex = -1;// 鼠标悬浮的频点索引
    int m_selectedBandIndex = -1;// 点击锁定的频点索引
    EditPanelTip *m_editPanel = nullptr;

    double m_dragStartGain;   // 拖拽前的增益
    double m_dragStartFreq;   // 拖拽前的频率

    bool m_disableOverlay = false; // 是否显示遮罩并禁用交互， true：覆盖  false：不覆盖


    bool m_draggingQ;          // 是否正在拖动 Q 值
    double m_dragStartQ;       // 拖动开始时的 Q 值
    QPoint m_dragStartPos;     // 拖动开始时的鼠标位置（用于计算位移）
    double knobOffset(double q) const;//根据 Q 值计算小圆偏移距离
};

#endif // EQCURVEWIDGET_H
