#ifndef CUSTOM_QSCROLLAREA_BACKGROUND_IMAGES_COMPONENT_H
#define CUSTOM_QSCROLLAREA_BACKGROUND_IMAGES_COMPONENT_H

#include <QHash>
#include <QHBoxLayout>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QWidget>

#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/InterfaceSettingCustomUI/custom_QWidget_background_images.h" ///< 内部子部件(单个可设置背景图片)
#include "modules/GeneralCustomUI/custom_QDialog_general_tips.h"   ///< 自定义控件： 弹窗

namespace Ui {
class BackgroundComponentView;
}

/// \brief 背景图片设置区域
/// 子控件：
///     - 内部网格布局排列 CustomQWidgetBackgroundImages 背景图部件
/// 通过 resizeEvent 动态计算列数，自适应窗口大小
class CustomQScrollAreaBackgroundComponent : public QScrollArea
{
    Q_OBJECT
public:
    explicit CustomQScrollAreaBackgroundComponent(QWidget *parent = nullptr, int theme = 0);
    ~CustomQScrollAreaBackgroundComponent();

    void applyTheme(int theme);              ///< 按主题更新样式

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽
    void showDeleteDialog(CustomQWidgetBackgroundImages *widget); ///< 弹出删除确认弹窗（exec() 安全封装）

public:
    void updateView();                                              ///< 更新视图,调整网格布局
    void refreshList();                                             ///< 从数据模型重新加载 cl_background_widget_list_
    void refreshListIfDirty();                                      ///< 仅在数据变更时刷新（避免每切 tab 都重建）
    void markDirty();                                               ///< 标记数据已变更（登录后/增删壁纸时调用）
    QPixmap loadWallpaperThumb(const QString &path);                ///< 缩放解码 + 缓存（避免全尺寸解码卡顿）
    void removeWidgetFromList(CustomQWidgetBackgroundImages *widget); ///< 从 list 中移除 widget 并清理关联数据
    QPointer<CustomQWidgetBackgroundImages> getCurrentItem() const; ///< 返回当前选中的项
    void setCurrentItem(CustomQWidgetBackgroundImages *item);       ///< 设置当前选中的项
    void updateSelectedItemTransparency(qreal value);               ///< 更新选中项的透明度
    int cl_theme_ = 0;                                              ///< 当前主题

    // ── 子控件
    CustomQWidgetBackgroundImages *cl_background_widget_header_ = nullptr;    ///< 第一个默认为系统背景（不进行持久化）
    QList<CustomQWidgetBackgroundImages *> cl_background_widget_list_; ///< 背景图部件集合（需进行持久化）
    CustomQWidgetBackgroundImages *cl_background_widget_tail_ = nullptr;    ///< 最后一个默认为添加背景（不进行持久化）

signals:
    void changeSliderValue(qreal value);               ///< 通知透明度滑块更新
    void backgroundChanged(const QString &path);       ///< 选中自定义/系统壁纸，携带图片路径
    void defaultBackgroundRestored();                  ///< 选中默认背景（header）

private slots:
    void dealwithBackgroundWidgetClicked(); ///< 处理背景图部件点击

private:
    Ui::BackgroundComponentView *ui;
    QGridLayout *cl_grid_layout_ = nullptr;                        ///< 内部网格布局
    QWidget *cl_content_widget_ = nullptr;                         ///< 内容控件
    QList<QString> cl_background_view_files_;             ///< 背景视图文件列表（预留）
    QHash<QString, QPixmap> cl_wallpaper_thumb_cache_;    ///< 壁纸路径 → 缩略图（缩放解码缓存）
    int cl_column_count_ = 1;                                      ///< 当前屏幕宽度可容纳的列数
    QPointer<CustomQWidgetBackgroundImages> cl_current_item_;      ///< 缓存当前选中的项
    bool cl_list_dirty_ = true;                                      ///< 列表需刷新标记（初始 true，登录后首次切到界面设置即可刷新）

    CustomQDialogGeneralTips* clp_dialog_tips_ = nullptr;   ///< 自定义弹窗

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QSCROLLAREA_BACKGROUND_IMAGES_COMPONENT_H
