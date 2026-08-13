#ifndef CUSTOM_QWIDGET_FEEDBACK_RETURN_PAGE_H
#define CUSTOM_QWIDGET_FEEDBACK_RETURN_PAGE_H

#include <QLabel>
#include <QMovie>
#include <QPixmap>
#include <QSize>
#include <QWidget>

namespace Ui {
class CustomQWidgetFeedBackReturnPage;
}

///
/// \brief 反馈结果状态枚举
enum class FeedBackResult {
    Submitting, ///< 提交中
    Success,    ///< 提交成功
    Failure,    ///< 提交失败
};

///
/// \brief 反馈提交结果 — 返回页面（成功/失败通用）
/// 子控件：
///     QLabel     图标（widget/label）
///     QLabel     标题文字（label_2）
///     QLabel     副标题文字（label_3）
///     QPushButton 操作按钮（pushButton）
class CustomQWidgetFeedBackReturnPage : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetFeedBackReturnPage(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetFeedBackReturnPage();

    void applyTheme(int theme); ///< 按主题更新样式

    /// 便捷接口 — 一步设置反馈结果
    void showSubmitting(); ///< 显示提交中状态
    void showSuccess();    ///< 显示反馈成功状态
    void showFailure();    ///< 显示反馈失败状态

    /// 状态查询
    FeedBackResult cl_result() const; ///< 获取当前结果状态

    /// 精细控制 — 各子控件内容
    QString cl_title_text() const;                    ///< 获取标题文字
    void setCl_title_text(const QString &text);       ///< 设置标题文字

    QString cl_subtitle_text() const;                  ///< 获取副标题文字
    void setCl_subtitle_text(const QString &text);     ///< 设置副标题文字

    QString cl_button_text() const;                    ///< 获取按钮文字
    void setCl_button_text(const QString &text);       ///< 设置按钮文字

    QPixmap cl_icon_pixmap() const;                    ///< 获取图标
    void setCl_icon_pixmap(const QPixmap &pixmap);     ///< 设置图标

signals:
    void actionButtonClicked(); ///< 操作按钮点击

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽

public:
    int cl_theme_ = 0; ///< 当前主题

private:
    Ui::CustomQWidgetFeedBackReturnPage *ui;

    QPixmap cl_icon_pixmap_;                        ///< 图标缓存
    QSize cl_min_size_ = QSize(375, 536);           ///< 最小尺寸
    FeedBackResult cl_result_ = FeedBackResult::Success; ///< 当前结果状态

    QLabel *clp_gif_label_ = nullptr;  ///< GIF 动画标签（提交中）

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override; ///< 窗口大小变化时更新图标
};

#endif // CUSTOM_QWIDGET_FEEDBACK_RETURN_PAGE_H
