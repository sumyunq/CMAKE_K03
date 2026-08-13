#ifndef CUSTOM_QLABEL_TAG_H
#define CUSTOM_QLABEL_TAG_H

#include <QLabel>

///
/// \brief 标签样式枚举
enum class TagLabelStyle {
    selected = 0, ///< 选中（蓝底白字）
    not_selected, ///< 未选中（灰底灰字）
};

///
/// \brief The CustomQLabelTag class
/// 自定义标签控件：文字 + 数字，尺寸由内容动态决定
/// 数字显示规则：
///   万以下（<10000）：原样数字
///   万以上（≥10000）：X.Yw 一位小数
class CustomQLabelTag : public QLabel
{
    Q_OBJECT

public:
    explicit CustomQLabelTag(QWidget *parent = nullptr, int theme = 0);
    ~CustomQLabelTag() override;

    void updateTag(const QString &text, int number); ///< 批量更新接口：同时设置文字和数字

    QString cl_tag_text() const;                     ///< 获取标签文字
    void setCl_tag_text(const QString &text);        ///< 设置标签文字

    int cl_tag_number() const;                       ///< 获取标签数字
    void setCl_tag_number(int number);               ///< 设置标签数字

    TagLabelStyle cl_tag_style() const;              ///< 获取标签样式
    void setCl_tag_style(TagLabelStyle style);       ///< 设置标签样式

    int cachedWidth() const;                         ///< 获取缓存的标签宽度（供流式布局高频查询）

    QFont cl_font() const;                           ///< 获取字体
    void setCl_font(const QFont &font);              ///< 设置字体

signals:
    void clicked();                                      ///< 标签被点击

protected:
    void paintEvent(QPaintEvent *event) override;      ///< 自绘圆角背景 + 文字
    void mousePressEvent(QMouseEvent *event) override; ///< 点击切换选中态并增减数字

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void refreshLabel();      ///< 刷新标签显示文本并触发重绘
    static QString formatNumber(int n); ///< 数字转显示字符串

public:
    void applyTheme(int theme); ///< 应用主题样式

private:
    QString cl_tag_text_;                                ///< 标签文字
    int cl_tag_number_ = 0;                              ///< 标签数字
    TagLabelStyle cl_tag_style_ = TagLabelStyle::not_selected; ///< 当前样式
    int cl_theme_ = 0;                                   ///< 当前主题
    QFont cl_font_ = QFont("Noto Sans S Chinese");    ///< 字体
    mutable int cl_cached_width_ = 0;                    ///< 缓存的 sizeHint 宽度
};
#endif // CUSTOM_QLABEL_TAG_H
