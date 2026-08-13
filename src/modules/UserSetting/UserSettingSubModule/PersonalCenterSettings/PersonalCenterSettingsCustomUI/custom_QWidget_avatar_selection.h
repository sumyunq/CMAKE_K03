#ifndef CUSTOM_QWIDGET_AVATAR_SELECTION_H
#define CUSTOM_QWIDGET_AVATAR_SELECTION_H

#include <QHash>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QWidget>

namespace Ui {
class CustomQWidgetAvatarSelection;
}

/// \brief 头像选择界面
/// 模态运行，整体固定大小 272×344
/// 子控件：
///     QLabel      图标
///     QLabel      标题文字
///     QPushButton 关闭按键
///     QWidget     内容容器（内置 4 列网格）
///         QLabel × 15  可选头像
///     QLabel      提示图标
///     QLabel      提示文字
class CustomQWidgetAvatarSelection : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetAvatarSelection(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetAvatarSelection();

    void applyTheme(int theme); ///< 按主题更新样式

signals:
    void avatarSelected(int index); ///< 头像选中信号，返回序号

private:
    void InitUIInformation(int theme); ///< 初始化 UI 默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认信号槽

public:
    /// 图标
    QLabel *clp_icon_label_ = nullptr;
    QSize cl_icon_label_size_ = QSize(16, 16);
    QPoint cl_icon_label_point_ = QPoint(27, 24);

    /// 标题
    QLabel *clp_title_label_ = nullptr;
    QSize cl_title_label_size_ = QSize(160, 20);
    QPoint cl_title_label_point_ = QPoint(53, 21);

    /// 关闭按键
    QPushButton *clp_close_button_ = nullptr;
    QSize cl_close_button_size_ = QSize(31, 31);
    QPoint cl_close_button_point_ = QPoint(231, 10);

    /// 提示图标
    QLabel *clp_tip_icon_label_ = nullptr;
    QSize cl_tip_icon_label_size_ = QSize(14, 14);
    QPoint cl_tip_icon_label_point_ = QPoint(25, 300);

    /// 提示文字
    QLabel *clp_tip_text_label_ = nullptr;
    QSize cl_tip_text_label_size_ = QSize(220, 16);
    QPoint cl_tip_text_label_point_ = QPoint(45, 298);

    QList<QLabel *> cl_avatar_list_; ///< 头像列表

    /// 头像资源映射表 — 序号 → 资源路径
    static inline QHash<int, QString> cl_avatar_res_map_;

    int cl_theme_ = 0; ///< 当前主题

private:
    Ui::CustomQWidgetAvatarSelection *ui;

    bool cl_is_dragging_ = false; ///< 是否正在拖动
    QPoint cl_drag_offset_;       ///< 拖拽偏移

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // CUSTOM_QWIDGET_AVATAR_SELECTION_H
