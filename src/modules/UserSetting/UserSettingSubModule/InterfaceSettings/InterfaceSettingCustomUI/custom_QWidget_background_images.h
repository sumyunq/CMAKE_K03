#ifndef CUSTOM_QWIDGET_BACKGROUND_IMAGES_H
#define CUSTOM_QWIDGET_BACKGROUND_IMAGES_H

#include <QLabel>
#include <memory>
#include <QMouseEvent>
#include <QPushButton>
#include <QSharedPointer>
#include <QVariantAnimation>
#include <QWidget>

#include "data/api_global.h" ///< BackgroundImageMode

/// \brief 背景图片部件
/// 五种形态，通过枚举控制内部子控件和交互行为
class CustomQWidgetBackgroundImages : public QWidget
{
    Q_OBJECT

public:
    explicit CustomQWidgetBackgroundImages(
        BackgroundImageMode t_mode = BackgroundImageMode::DefaultTheme,
        QWidget *parent = nullptr,
        int theme = 0);
    CustomQWidgetBackgroundImages(
        BackgroundImageMode t_mode,
        QSharedPointer<QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>> map,
        int index,
        QWidget *parent = nullptr,
        int theme = 0);
    ~CustomQWidgetBackgroundImages();

    void applyTheme(int theme); ///< 按主题更新样式

    void setBackground(const QPixmap &pixmap, const QString &filePath = QString()); ///< 设置背景图
    void setCenterIcon(const QPixmap &pixmap);        ///< 设置中心图标
    void setCenterText(const QString &text);          ///< 设置中心文字
    void setCl_icon_size(const QSize &size);          ///< 设置图标尺寸
    void setCl_text_height(int h);                    ///< 设置文字高度
    void setCl_icon_default_point(const QPoint &point);   ///< 设置图标默认坐标
    void setCl_icon_hover_point(const QPoint &point);     ///< 设置图标悬停坐标
    void setCl_text_default_point(const QPoint &point);   ///< 设置文字默认坐标
    void setCl_text_hover_point(const QPoint &point);     ///< 设置文字悬停坐标
    void setCl_delete_btn_size(const QSize &size);        ///< 设置删除按钮尺寸
    void switchMode(BackgroundImageMode targetMode);     ///< 切换为指定模式

signals:
    void defaultClicked();                    ///< DefaultTheme → 恢复默认背景
    void systemClicked();                     ///< SystemTheme → 选中系统壁纸
    void customClicked();                     ///< Custom → 选中自定义壁纸
    void deleteRequested();                   ///< Custom 删除按钮
    void backgroundAdded(const QString &path); ///< AddCustom 添加背景后触发

private:
    void InitUIInformation(int theme); ///< 初始化 UI 默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认信号槽

public:
    // ── 形态 / 主题 ──
    BackgroundImageMode cl_mode_ = BackgroundImageMode::DefaultTheme; ///< 当前形态
    int cl_theme_ = 0;                                                ///< 当前主题
    bool cl_is_selected_ = false;                                     ///< 是否选中

    // ── 背景 ──
    QString cl_file_path_;                         ///< 背景图片文件路径
    QPixmap cl_background_pixmap_;                  ///< 背景图原图
    QPixmap cl_cached_scaled_bg_;                   ///< 缩放后缓存（避免每帧 paintEvent 重缩放）

    // ── 壁纸数据共享 ──
    QSharedPointer<QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>> clp_wallpaper_map_;
    int cl_wallpaper_index_ = -1;

    // ── 图标尺寸 / 坐标（三档：default / hover / current）──
    QSize cl_icon_size_ = QSize(44, 44);            ///< 图标尺寸
    QPoint cl_icon_default_point_ = QPoint(90, 45); ///< 图标默认坐标
    QPoint cl_icon_hover_point_ = QPoint(90, 34);   ///< 图标悬停坐标
    QPoint cl_icon_current_point_ = QPoint(90, 45); ///< 图标当前坐标（动画驱动）

    // ── 文字尺寸 / 坐标 / 透明度（三档：default / hover / current）──
    QSize cl_delete_btn_size_ = QSize(32, 32);       ///< 删除按钮尺寸
    QSize cl_text_size_ = QSize(200, 20);            ///< 文字尺寸
    QPoint cl_text_default_point_ = QPoint(12, 102);  ///< 文字默认坐标
    QPoint cl_text_hover_point_ = QPoint(12, 91);    ///< 文字悬停坐标
    QPoint cl_text_current_point_ = QPoint(12, 102);  ///< 文字当前坐标（动画驱动）
    qreal cl_text_opacity_ = 0.0;                    ///< 文字当前透明度（动画驱动）

private:
    QLabel *clp_background_ = nullptr;
    QLabel *clp_hover_mask_ = nullptr;
    QLabel *clp_center_icon_ = nullptr;
    QLabel *clp_center_text_ = nullptr;
    QPushButton *clp_delete_btn_ = nullptr;
    std::unique_ptr<QVariantAnimation> clp_icon_anim_;
    std::unique_ptr<QVariantAnimation> clp_text_pos_anim_;
    std::unique_ptr<QVariantAnimation> clp_text_opacity_anim_;

    bool cl_is_hover_ = false;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
};

#endif // CUSTOM_QWIDGET_BACKGROUND_IMAGES_H
