#ifndef CUSTOM_QWIDGET_DOWNLOAD_PROGRESS_RING_H
#define CUSTOM_QWIDGET_DOWNLOAD_PROGRESS_RING_H

#include <QWidget>

///
/// \brief 下载进度圆环控件（自绘）
/// 子控件：无
/// 14×14 固定尺寸，paintEvent 绘制背景弧 + 进度弧（12点方向顺时针）
class CustomQWidgetDownloadProgressRing : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetDownloadProgressRing(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetDownloadProgressRing();

    int cl_progress() const;                        ///< 获取当前进度 (0~100)
    void setCl_progress(int percent);               ///< 设置进度 (0~100)，触发 repaint

    void applyTheme(int theme);                     ///< 应用主题样式
    void reset();                                   ///< 重置进度为 0

protected:
    void paintEvent(QPaintEvent *event) override;   ///< 自绘圆环

private:
    void InitUIInformation(int theme);              ///< 初始化UI的默认信息
    void InitMember();                              ///< 初始化内部成员
    void InitConnect();                             ///< 连接默认的信号槽

private:
    QSize cl_min_size_ = QSize(14, 14);             ///< 固定最小尺寸

private:
    int cl_theme_ = 0;                              ///< 当前主题
    int cl_progress_ = 0;                           ///< 当前进度 0~100
    int cl_ring_width_ = 2;                         ///< 圆环线宽（px）
    QColor cl_ring_color_ = QColor("#009FEF");      ///< 进度弧颜色（项目 accent 蓝）
    QColor cl_bg_color_ = QColor("#3A3F47");        ///< 背景弧颜色（深灰）
};

#endif // CUSTOM_QWIDGET_DOWNLOAD_PROGRESS_RING_H
