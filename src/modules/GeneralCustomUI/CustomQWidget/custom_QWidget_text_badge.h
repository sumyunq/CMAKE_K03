#ifndef CUSTOM_QWIDGET_TEXT_BADGE_H
#define CUSTOM_QWIDGET_TEXT_BADGE_H

#include <QColor>
#include <QFont>
#include <QWidget>

class QTimer;
class QShowEvent;
class QHideEvent;
class QPainter;

///
/// \brief 文字徽章配置参数
struct CustomQWidgetTextBadgeConfig
{
    QString text;                                ///< 显示文字
    QColor  bg_color = QColor(161, 168, 179, 51);        ///< 背景色
    QColor  text_color = QColor("#A1A8B3");      ///< 文字颜色
    QFont   font;                                ///< 文字字体（空=默认 Noto Sans 10px）
    qreal   radius = 8.5;                        ///< 背景圆角
    int     padding_left = 8;                   ///< 左边距
    int     padding_top = 2;                     ///< 上边距
    int     padding_right = 8;                  ///< 右边距
    int     padding_bottom = 2;                  ///< 下边距
    Qt::Alignment alignment = Qt::AlignCenter;    ///< 文字对齐方式
};

///
/// \brief 自绘文字徽章（paintEvent 绘制圆角背景 + 文字，自动适配尺寸）
///
/// 适用场景：状态标签、分类标识、计数徽章等需要带背景色的短文本。
/// 所有视觉属性均可独立设置，setCl_text() 触发布局更新。
///
/// \code
/// auto *t_badge = new CustomQWidgetTextBadge(parent);
/// t_badge->setCl_text(tr("新消息"));
/// t_badge->setCl_bg_color(QColor("#FF6600"));
/// t_badge->setCl_radius(8);
/// \endcode
class CustomQWidgetTextBadge : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetTextBadge(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetTextBadge();

    void applyTheme(int theme);                         ///< 应用主题样式

    void setCl_text(const QString &text);               ///< 设置文字（自动更新尺寸）
    void setCl_bg_color(const QColor &color);           ///< 设置背景色（火焰模式下作为火焰主色，黄→橙→红自动衍生）
    void setCl_text_color(const QColor &color);         ///< 设置文字颜色
    void setCl_font(const QFont &font);                 ///< 设置字体
    void setCl_radius(qreal radius);                    ///< 设置背景圆角
    void setCl_padding(int left, int top, int right, int bottom); ///< 设置内边距
    void setCl_alignment(Qt::Alignment align);          ///< 设置文字对齐
    void setCl_fire_enabled(bool en);                   ///< 开/关火焰动画背景
    void setCl_fire_speed(int ms);                      ///< 火焰动画更新间隔（ms），默认 50

    QString cl_text() const;                            ///< 获取文字
    QColor  cl_bg_color() const;                        ///< 获取背景色
    QColor  cl_text_color() const;                      ///< 获取文字颜色
    bool    cl_fire_enabled() const;                    ///< 是否已开启火焰模式

    QSize sizeHint() const override;                    ///< 根据文字+边距计算推荐尺寸
    QSize minimumSizeHint() const override;             ///< 最小尺寸

protected:
    void paintEvent(QPaintEvent *event) override;       ///< 自绘：圆角背景 + 文字（火焰模式下绘制火焰背景）
    void changeEvent(QEvent *event) override;           ///< 字体变更时刷新尺寸
    void showEvent(QShowEvent *event) override;         ///< 显示时启动火焰定时器
    void hideEvent(QHideEvent *event) override;         ///< 隐藏时停止火焰定时器

private slots:
    void onFireTick();                                  ///< 火焰动画帧 → update()

private:
    void InitUIInformation(int theme);                  ///< 初始化UI的默认信息
    void InitMember();                                  ///< 初始化内部成员
    void InitConnect();                                 ///< 连接默认的信号槽

    void updateSizeFromText();                          ///< 根据文字和边距更新控件尺寸
    void drawSolidBg(QPainter &t_painter, const QRect &t_rect); ///< 绘制纯色背景
    void drawFireBg(QPainter &t_painter, const QRect &t_rect);  ///< 绘制火焰动画背景

private:
    CustomQWidgetTextBadgeConfig cl_cfg_;               ///< 配置参数
    int cl_theme_ = 0;                                  ///< 主题

    // 火焰动画
    QTimer *clp_fire_timer_ = nullptr;                  ///< 火焰动画定时器
    qreal  cl_fire_phase_ = 0.0;                        ///< 火焰相位（0~2π），每帧递增
    bool   cl_fire_enabled_ = false;                    ///< 火焰模式开关
};

#endif // CUSTOM_QWIDGET_TEXT_BADGE_H
