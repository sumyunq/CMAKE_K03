#ifndef CUSTOM_QWIDGET_GRADE_STATUS_H
#define CUSTOM_QWIDGET_GRADE_STATUS_H

#include <QLabel>
#include <QPoint>
#include <QProgressBar>
#include <QSize>
#include <QWidget>

///
/// \brief 等级状态控件 — 展示用户等级、经验进度、当前/升级所需经验
/// 子控件：
///     QLabel      等级文字（Lv.X）
///     QProgressBar 经验进度条
///     QLabel      当前经验/升级所需经验 比例文字
class CustomQWidgetGradeStatus : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetGradeStatus(QWidget *parent = nullptr);

    void applyTheme(int theme); ///< 按主题更新样式

    void setCl_grade_level(int level);                        ///< 设置等级文字（Lv.X）
    void setCl_progress(int value, int maximum);              ///< 设置经验进度
    void setCl_empirical_text(const QString &text);           ///< 设置经验比例文字
    void setCl_empirical_value(int currentXp, int requiredXp); ///< 设置经验数值（自动格式化）

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public:
    /// 子控件（可供外部直接访问）
    QLabel *clp_grade_ = nullptr;                          ///< 等级文字
    QProgressBar *clp_grade_progressBar_ = nullptr;         ///< 经验进度条
    QLabel *clp_grade_empirical_value_proportion_ = nullptr; ///< 经验比例文字

    /// 等级文字 布局参数
    QSize cl_grade_current_size_ = QSize(25, 13);
    QSize cl_grade_min_size_ = QSize(25, 13);
    QSize cl_grade_max_size_ = QSize(25, 13);
    QPoint cl_grade_default_point_ = QPoint(39, 1);

    /// 进度条 布局参数
    QSize cl_grade_progressBar_current_size_ = QSize(166, 4);
    QSize cl_grade_progressBar_min_size_ = QSize(166, 4);
    QSize cl_grade_progressBar_max_size_ = QSize(166, 4);
    QPoint cl_grade_progressBar_default_point_ = QPoint(74, 6);

    /// 经验比例文字 布局参数
    QSize cl_grade_empirical_value_proportion_current_size_ = QSize(50, 14);
    QSize cl_grade_empirical_value_proportion_min_size_ = QSize(50, 14);
    QSize cl_grade_empirical_value_proportion_max_size_ = QSize(50, 14);
    QPoint cl_grade_empirical_value_proportion_default_point_ = QPoint(242, 0);

    /// 整体布局参数
    int cl_theme_ = 0; ///< 当前主题

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override; ///< 窗口大小变化时更新子控件布局
};

#endif // CUSTOM_QWIDGET_GRADE_STATUS_H
