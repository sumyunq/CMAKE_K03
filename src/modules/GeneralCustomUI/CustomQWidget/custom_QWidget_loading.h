#ifndef CUSTOM_QWIDGET_LOADING_H
#define CUSTOM_QWIDGET_LOADING_H

#include <QColor>
#include <QRect>
#include <QWidget>

class QTimer;

///
/// \brief 加载动画配置参数
struct CustomQWidgetLoadingConfig
{
    QColor arc_head_color = QColor(255, 255, 255, 127); ///< 弧线渐变头部
    QColor arc_tail_color = QColor(255, 255, 255, 0);   ///< 弧线渐变尾部
    QColor text_color = QColor("#FFFFFF");           ///< 文字颜色
    QColor bg_color = Qt::transparent;               ///< 背景色
    QRect  arc_rect;                                 ///< 弧线区域（空=自动）
    QRect  text_rect;                                ///< 文字区域（空=自动）
    qreal  arc_width = 2.0;                          ///< 弧线宽度
    int    arc_span = 360;                           ///< 弧线跨度（°），缺口=360-span
    int    radius = 0;                               ///< 弧线半径（0=取宽高较小值）
    int    period_ms = 500;                          ///< 旋转周期（ms）
    int    clip_radius = 0;                          ///< 裁剪圆角（0=不裁剪）
    QString text;                                    ///< 文字
    bool   text_visible = true;                      ///< 文字可见
    bool   clockwise = true;                         ///< 旋转方向
};

///
/// \brief 加载中动画控件（自绘旋转弧线 + 可选文字）
class CustomQWidgetLoading : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetLoading(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetLoading();

    void start();                             ///< 启动旋转
    void stop();                              ///< 停止旋转
    bool cl_running() const;                  ///< 是否旋转中

    void setCl_config(const CustomQWidgetLoadingConfig &cfg); ///< 批量设置参数
    CustomQWidgetLoadingConfig cl_config() const;             ///< 获取参数

    void setCl_text(const QString &text);           ///< 设置文字
    void setCl_arc_color(const QColor &color); ///< 弧线颜色（便捷接口，头尾同色）

    void applyTheme(int theme);               ///< 应用主题样式

    QSize sizeHint() const override;          ///< 返回推荐尺寸

protected:
    void paintEvent(QPaintEvent *event) override; ///< 自绘弧线 + 文字

private slots:
    void tick();                              ///< 定时器回调

private:
    void InitUIInformation(int theme);        ///< 初始化UI的默认信息
    void InitMember();                        ///< 初始化内部成员
    void InitConnect();                       ///< 连接默认的信号槽

private:
    QTimer *clp_timer_ = nullptr;             ///< 定时器
    CustomQWidgetLoadingConfig cl_cfg_;       ///< 配置参数
    qreal cl_angle_ = 0;                      ///< 当前角度
    int cl_theme_ = 0;                        ///< 主题
};

#endif // CUSTOM_QWIDGET_LOADING_H
