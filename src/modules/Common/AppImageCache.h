#ifndef APP_IMAGE_CACHE_H
#define APP_IMAGE_CACHE_H

#include <QHash>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QSize>
#include <QPainter>
#include <QPainterPath>
#include <QNetworkReply>

#include <functional>

///
/// \brief 全局图片缓存单例
///
/// 存放壁纸预缩放缓存等运行时渲染资源。
/// 任何模块通过 AppImageCache::instance() 直接访问，无需穿透 MainWindow。
///
/// ## Thread Safety
/// - **写路径**（updateBackgroundCache）：`cl_mutex_` 保护，任意线程安全。
/// - **读路径**（paintEvent 中读取缓存 pixmap）：假定主线程独占访问，无锁。
///   `QPixmap` 非线程安全，读必须在主线程（GUI 线程）执行。
///
/// \code
/// auto &t_cache = AppImageCache::instance();
/// t_cache.updateBackgroundCache(size());          // 任意线程安全
/// t_painter.drawPixmap(0, 0, t_cache.cl_background_scaled_cache_); // 仅主线程
/// \endcode
class AppImageCache
{
public:
    static AppImageCache &instance(); ///< 单例访问（C++11 Magic Static，线程安全）

    /// \brief 按视口尺寸预缩放所有壁纸缓存（线程安全）
    /// \param t_viewSize 视口尺寸（通常为 MainWindow 或 centralwidget 的 size()）
    void updateBackgroundCache(const QSize &t_viewSize);

    /// \brief 重算面板毛玻璃模糊快照（合成默认底图+壁纸后整体模糊，线程安全）
    /// \param t_viewSize 视口尺寸
    /// \param t_blur_radius 模糊半径（逻辑像素，≤0 = 清空缓存）
    /// \param t_wallpaper_opacity 壁纸叠加不透明度（与 paintEvent 一致）
    void updateBlurredBackdrop(const QSize &t_viewSize, int t_blur_radius, double t_wallpaper_opacity);

    // ── 壁纸缓存（运行时，不持久化）──
    // 读路径假定主线程独占，写路径由 updateBackgroundCache() 内的 cl_mutex_ 保护
    QPixmap cl_background_pixmap_;        ///< 用户壁纸原图（空 = 无壁纸）
    QPixmap cl_background_scaled_cache_;  ///< 用户壁纸预缩放缓存
    QPixmap cl_default_background_cache_; ///< 默认底部背景预缩放缓存
    /// \brief 按全局坐标路径从模糊缓存裁剪，返回切片（主线程调用）
    QPixmap clippedBlur(const QPainterPath &t_path) const;

    QPixmap cl_background_blurred_cache_; ///< 整窗背景合成图的模糊快照（面板毛玻璃用）

    // ── 头像缓存 ──
    /// \brief 请求作者头像（url → pixmap），有缓存直接回调，否则异步下载后回调
    /// \param url 头像 URL
    /// \param callback 回调（主线程执行），空 pixmap = 下载失败
    void requestAvatar(const QString &url, std::function<void(const QPixmap &)> callback);

private:
    /// \brief 保证 avatar_manager_ 已创建（主线程）
    QNetworkAccessManager *avatarManager();
    /// \brief 处理头像下载完成
    void onAvatarDownloadFinished();

private:
    AppImageCache() = default;
    AppImageCache(const AppImageCache &) = delete;
    AppImageCache &operator=(const AppImageCache &) = delete;

    QMutex cl_mutex_; ///< 保护 write 路径（updateBackgroundCache），读路径无锁

    // 头像缓存
    QHash<QString, QPixmap> cl_avatar_cache_;                    ///< url → pixmap
    QHash<QString, QList<std::function<void(const QPixmap &)>>> cl_avatar_callbacks_; ///< 下载中的回调队列
    QNetworkAccessManager *clp_avatar_manager_ = nullptr;        ///< 头像下载管理器（懒创建）
};

#endif // APP_IMAGE_CACHE_H
