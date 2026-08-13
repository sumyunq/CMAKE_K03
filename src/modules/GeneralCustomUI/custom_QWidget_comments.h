#ifndef CUSTOM_QWIDGET_COMMENTS_H
#define CUSTOM_QWIDGET_COMMENTS_H

#include <QLabel>
#include <QList>
#include <QMap>
#include <QWidget>

class CustomQLabelTag;

/// \brief 评论区标签流式布局
class CustomQWidgetComments : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetComments(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetComments() override;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void addTag(int key, CustomQLabelTag *tag);     ///< 添加标签（key 唯一，重复则覆盖旧标签）
    void removeTag(int key);                               ///< 移除标签
    void clearTags();                                      ///< 清空全部标签
    CustomQLabelTag *tag(int key) const;             ///< 按 key 取标签

    bool cl_expanded() const;                              ///< 获取展开状态
    void setCl_expanded(bool expanded);                    ///< 设置展开状态

    int cl_h_spacing() const;                              ///< 获取水平间距
    void setCl_h_spacing(int s);                           ///< 设置水平间距

    int cl_v_spacing() const;                              ///< 获取垂直间距
    void setCl_v_spacing(int s);                           ///< 设置垂直间距

    int cl_tag_height() const;                             ///< 获取标签行高
    void setCl_tag_height(int h);                          ///< 设置标签行高

signals:
    void expandedChanged(bool expanded);                   ///< 展开/收缩状态变化，通知外部调整高度
    void heightChanged(int newHeight);                     ///< 内容高度变化，通知外部更新面板
    void tagClicked(int key);                              ///< 标签被点击，传递标签 key

protected:
    void paintEvent(QPaintEvent *event) override;           ///< 自绘展开/收起按钮
    void resizeEvent(QResizeEvent *event) override;        ///< 宽度变化时重新流式摆放
    void mousePressEvent(QMouseEvent *event) override;     ///< 检测"展开/收起"标签点击
    void mouseMoveEvent(QMouseEvent *event) override;      ///< 检测展开/收起按钮悬停
    void leaveEvent(QEvent *event) override;               ///< 鼠标离开时清除悬停态
    bool event(QEvent *event) override;                    ///< 拦截 LayoutRequest 触发 doLayout

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽
    void doLayout();          ///< 执行流式摆放
    void updateArrowIcon();   ///< 更新展开/收起箭头图标（expand × hover 四态）

public:
    void applyTheme(int theme); ///< 应用主题样式

public:
    QMap<int, CustomQLabelTag *> cl_tag_map_;       ///< 标签 map
    QList<int> cl_tag_order_;                       ///< 标签添加顺序

private:
    bool cl_expanded_ = false;                             ///< 当前展开状态
    int cl_h_spacing_ = 6;                                 ///< 水平间距
    int cl_v_spacing_ = 6;                                 ///< 垂直间距
    int cl_tag_height_ = 20;                               ///< 标签行高
    int cl_cached_height_ = 0;                             ///< 缓存的布局总高度
    int cl_theme_ = 0;                                     ///< 当前主题

    QLabel *clp_arrow_icon_ = nullptr;                     ///< 展开/收起箭头图标（8×4）
    QRect  cl_expand_rect_;                                 ///< 展开/收起按钮区域（缓存，用于悬停检测）
    bool   cl_is_hover_expand_ = false;                     ///< 悬停在展开/收起按钮上
};
#endif // CUSTOM_QWIDGET_COMMENTS_H
