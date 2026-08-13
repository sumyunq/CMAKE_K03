#ifndef CUSTOM_QWIDGET_FUNCTION_BUTTON_WITH_DISPLAY_LABEL_H
#define CUSTOM_QWIDGET_FUNCTION_BUTTON_WITH_DISPLAY_LABEL_H

#include <QWidget>

class QStackedWidget;

///
/// \brief 按键功能类型枚举
enum class ButtonFuctionType {
    Like = 0,  ///< 点赞
    Dislike,   ///< 踩
    Download,  ///< 下载
    Share,     ///< 分享
    Reserved1, ///< 预留功能 1
    Reserved2, ///< 预留功能 2
    Reserved3, ///< 预留功能 3
};

namespace Ui {
class CustomQWidgetFunctionButtonWithDisplayLabel;
}

///
/// \brief 下载按钮状态
enum class DownloadState {
    Normal = 0,      ///< 正常态 — 显示下载图标
    Downloading,     ///< 下载中 — 显示进度圆环
    Done             ///< 下载完成 — 恢复下载图标
};

class CustomQWidgetDownloadProgressRing;

///
/// \brief The CustomQWidgetFunctionButtonWithDisplayLabel class
/// 功能按键（带显示标签），支持点赞/取消点赞/下载/分享等操作
/// 子控件：
///     QPushButton 图标按键
///     QLabel      计量数值标签
///     CustomQWidgetDownloadProgressRing 下载进度圆环（下载模式专属）
class CustomQWidgetFunctionButtonWithDisplayLabel : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetFunctionButtonWithDisplayLabel(
        QWidget *parent = nullptr, ButtonFuctionType type = ButtonFuctionType::Like, int theme = 0);
    ~CustomQWidgetFunctionButtonWithDisplayLabel();

    ButtonFuctionType cl_button_type() const;
    void setCl_button_type(ButtonFuctionType type);

    int cl_count() const;
    void setCl_count(int count);
    void setChecked(bool checked); ///< 设置内部按钮选中态

    void applyTheme(int theme); ///< 应用主题样式

    void setDownloadProgress(int percent);               ///< 设置下载进度 (0~100)，驱动圆环更新
    void setDownloadState(DownloadState state);          ///< 切换下载状态，管理 pushButton/圆环 显隐
    DownloadState downloadState() const;                 ///< 获取当前下载状态

signals:
    void liked();       ///< 点赞（选中态）
    void unliked();     ///< 取消点赞（非选中态）
    void disliked();    ///< 踩（选中态）
    void undisliked();  ///< 取消踩（非选中态）
    void download();    ///< 下载
    void share();       ///< 分享

private:
    void InitUIInformation(int theme);   ///< 初始化UI的默认信息
    void InitMember();                   ///< 初始化内部成员
    void InitConnect();                  ///< 连接默认的信号槽
    void onInternalButtonClicked();      ///< 内部按键点击，根据类型发射对应信号
    void applyStyleForType();            ///< 根据类型更新按键样式

private:
    Ui::CustomQWidgetFunctionButtonWithDisplayLabel *ui;

private:
    QStackedWidget *clp_stack_ = nullptr;                               ///< 堆叠容器，切换 pushButton / 进度圆环
    CustomQWidgetDownloadProgressRing *clp_progress_ring_ = nullptr;    ///< 下载进度圆环
    DownloadState cl_download_state_ = DownloadState::Normal;           ///< 当前下载状态

private:
    int cl_theme_ = 0;                                             ///< 当前主题
    ButtonFuctionType cl_button_type_ = ButtonFuctionType::Like;   ///< 按键功能类型
    int cl_count_ = 0;                                             ///< 计量数值
    QRect cl_min_rect_ = QRect(0, 0, 41, 14);                      ///< 最小尺寸
};

#endif // CUSTOM_QWIDGET_FUNCTION_BUTTON_WITH_DISPLAY_LABEL_H
