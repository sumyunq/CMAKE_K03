#include "modules/Common/AppImageCache.h"

#include <QApplication>
#include <QImage>
#include <QNetworkReply>
#include <QObject>
#include <QPainter>

#include "data/api_global.h" ///< XIBERIA_X_HUB_Utils::blurPixmap

/// \brief 单例访问（C++11 Magic Static，线程安全）
AppImageCache &AppImageCache::instance()
{
    static AppImageCache t_instance;
    return t_instance;
}

/// \brief 按视口尺寸预缩放所有壁纸缓存（线程安全）
/// \param t_viewSize 视口尺寸
void AppImageCache::updateBackgroundCache(const QSize &t_viewSize)
{
    QMutexLocker t_locker(&cl_mutex_);

    const qreal t_dpr = qApp->devicePixelRatio();

    // ── 默认底部背景 ──
    cl_default_background_cache_ = QPixmap(":/Skin/Images/home/background.png")
                                       .scaled(t_viewSize * t_dpr,
                                               Qt::KeepAspectRatioByExpanding,
                                               Qt::SmoothTransformation);
    cl_default_background_cache_.setDevicePixelRatio(t_dpr);

    // ── 用户壁纸覆盖层 ──
    if (cl_background_pixmap_.isNull()) {
        cl_background_scaled_cache_ = QPixmap();
    } else {
        cl_background_scaled_cache_ = cl_background_pixmap_.scaled(
            t_viewSize * t_dpr, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        cl_background_scaled_cache_.setDevicePixelRatio(t_dpr);
    }
}

/// \brief 重算面板毛玻璃模糊快照（合成默认底图+壁纸后整体模糊，线程安全）
/// \param t_viewSize 视口尺寸
/// \param t_blur_radius 模糊半径（逻辑像素，≤0 = 清空缓存）
/// \param t_wallpaper_opacity 壁纸叠加不透明度（与 paintEvent 一致）
void AppImageCache::updateBlurredBackdrop(const QSize &t_viewSize, int t_blur_radius,
                                          double t_wallpaper_opacity)
{
    QMutexLocker t_locker(&cl_mutex_);

    // 半径为 0 或无效尺寸 → 清空缓存，paintEvent 会跳过毛玻璃
    if (t_blur_radius <= 0 || t_viewSize.isEmpty()) {
        cl_background_blurred_cache_ = QPixmap();
        return;
    }

    const qreal t_dpr = qApp->devicePixelRatio();

    // 合成整窗背景（逻辑尺寸；与 MainWindow::paintEvent 相同的居中/透明度规则）
    QImage t_composite(t_viewSize, QImage::Format_ARGB32_Premultiplied);
    t_composite.fill(Qt::transparent);
    {
        QPainter t_painter(&t_composite);

        // 第一层：默认底图（居中）
        if (!cl_default_background_cache_.isNull()) {
            const QSizeF t_logical = cl_default_background_cache_.size() / t_dpr;
            int t_x = qRound((t_viewSize.width() - t_logical.width()) / 2.0);
            int t_y = qRound((t_viewSize.height() - t_logical.height()) / 2.0);
            t_painter.drawPixmap(t_x, t_y, cl_default_background_cache_);
        }
        // 第二层：用户壁纸（居中 + 透明度）
        if (!cl_background_scaled_cache_.isNull()) {
            t_painter.setOpacity(t_wallpaper_opacity);
            const QSizeF t_logical = cl_background_scaled_cache_.size() / t_dpr;
            int t_x = qRound((t_viewSize.width() - t_logical.width()) / 2.0);
            int t_y = qRound((t_viewSize.height() - t_logical.height()) / 2.0);
            t_painter.drawPixmap(t_x, t_y, cl_background_scaled_cache_);
        }
    }

    // 降采样后模糊：成本按面积降 ~16 倍；绘制端拉伸铺回，放大失真被模糊掩盖
    const int t_scale = 4;
    const QSize t_smallSize(qMax(1, t_viewSize.width() / t_scale),
                            qMax(1, t_viewSize.height() / t_scale));
    const QImage t_small = t_composite.scaled(t_smallSize,
                                              Qt::IgnoreAspectRatio,
                                              Qt::SmoothTransformation);
    const int t_small_radius = qMax(1, t_blur_radius / t_scale);
    cl_background_blurred_cache_
        = XIBERIA_X_HUB_Utils::blurPixmap(QPixmap::fromImage(t_small), t_small_radius);
}

QPixmap AppImageCache::clippedBlur(const QPainterPath &t_path) const
{
    if (cl_background_blurred_cache_.isNull())
        return {};

    QRect t_rect = t_path.boundingRect().toRect();
    if (t_rect.isEmpty())
        return {};

    QPixmap t_result(t_rect.size());
    t_result.fill(Qt::transparent);

    QPainter t_painter(&t_result);
    t_painter.setRenderHint(QPainter::Antialiasing, true);
    t_painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    t_painter.setClipPath(t_path.translated(-t_rect.topLeft()));
    t_painter.drawPixmap(-t_rect.topLeft(), cl_background_blurred_cache_);
    t_painter.end();

    return t_result;
}

// ── 头像缓存 ──

QNetworkAccessManager *AppImageCache::avatarManager()
{
    if (!clp_avatar_manager_) {
        clp_avatar_manager_ = new QNetworkAccessManager();
        clp_avatar_manager_->setTransferTimeout(30000);
    }
    return clp_avatar_manager_;
}

void AppImageCache::requestAvatar(const QString &url,
                                   std::function<void(const QPixmap &)> callback)
{
    if (url.isEmpty()) {
        if (callback) callback(QPixmap());
        return;
    }

    // 已缓存 → 直接回调
    if (cl_avatar_cache_.contains(url)) {
        if (callback) callback(cl_avatar_cache_.value(url));
        return;
    }

    // 正在下载中 → 加入回调队列
    if (cl_avatar_callbacks_.contains(url)) {
        cl_avatar_callbacks_[url].append(callback);
        return;
    }

    // 首次请求 → 记录回调 + 发起下载
    cl_avatar_callbacks_[url].append(callback);

    QNetworkReply *t_reply = avatarManager()->get(QNetworkRequest(QUrl(url)));
    QObject::connect(t_reply, &QNetworkReply::finished, t_reply,
                     [this, t_reply, url]() {
        t_reply->deleteLater();

        QPixmap t_pixmap;
        if (t_reply->error() == QNetworkReply::NoError) {
            t_pixmap.loadFromData(t_reply->readAll());
        }

        // 缓存（即使失败也存空 pixmap，避免重复下载）
        cl_avatar_cache_.insert(url, t_pixmap);

        // 通知全部等待者
        const auto t_callbacks = cl_avatar_callbacks_.take(url);
        for (const auto &t_cb : t_callbacks) {
            if (t_cb) t_cb(t_pixmap);
        }
    });
}
