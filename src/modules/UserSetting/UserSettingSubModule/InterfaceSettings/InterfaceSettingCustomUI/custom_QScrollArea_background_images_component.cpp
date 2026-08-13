#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/InterfaceSettingCustomUI/custom_QScrollArea_background_images_component.h"
#include "ui_custom_QScrollArea_background_images_component.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>

#include "LoadLib.h" ///< g_user_information

/// \brief 构造函数
CustomQScrollAreaBackgroundComponent::CustomQScrollAreaBackgroundComponent(QWidget *parent, int theme)
    : QScrollArea(parent)
    , cl_theme_(theme)
    , ui(new Ui::BackgroundComponentView)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽

    applyTheme(theme);
    updateView();
}

CustomQScrollAreaBackgroundComponent::~CustomQScrollAreaBackgroundComponent() {}

/// \brief 按主题更新样式
void CustomQScrollAreaBackgroundComponent::applyTheme(int theme)
{
    cl_theme_ = theme;
}

/// \brief 初始化UI的默认信息
void CustomQScrollAreaBackgroundComponent::InitUIInformation(int theme)
{
    cl_content_widget_ = new QWidget(this);
    cl_grid_layout_ = new QGridLayout(cl_content_widget_);
    cl_grid_layout_->setContentsMargins(0, 0, 0, 0);
    cl_grid_layout_->setSpacing(20);
    this->setWidget(cl_content_widget_);
    this->setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    cl_content_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cl_grid_layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
}

/// \brief 初始化内部成员
void CustomQScrollAreaBackgroundComponent::InitMember()
{
    auto t_addWidgets = [this](const QSharedPointer<QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>> &map) {
        if (!map) return;
        for (auto it = map->constBegin(); it != map->constEnd(); ++it) {
            auto *t_widget = new CustomQWidgetBackgroundImages(
                it.value()->mode, map, it.key(), cl_content_widget_);
            QPixmap t_pm = loadWallpaperThumb(it.value()->path);  // 缩放解码 + 缓存（避免全尺寸解码卡顿）
            if (!t_pm.isNull())
                t_widget->setBackground(t_pm, it.value()->path);
            cl_background_widget_list_.append(t_widget);
        }
    };
    // 默认壁纸（header，不持久化）— 无条件创建，default_wallpaper_map 此时尚未初始化
    cl_background_widget_header_ = new CustomQWidgetBackgroundImages(
        BackgroundImageMode::DefaultTheme, cl_content_widget_);
    cl_background_widget_header_->setCl_icon_size(QSize(44,44));
    cl_background_widget_header_->setCenterIcon(QPixmap(":/Skin/Images/more/interface_settings/default_background_2x_darkBlue.png"));
    cl_background_widget_header_->setCenterText(tr("默认背景"));
    // 系统壁纸（持久化）
    t_addWidgets(g_user_information.local.system_wallpaper_map);
    // 自定义壁纸（按壁纸存储策略持久化）
    t_addWidgets(g_user_information.local.custom_wallpaper_map);
    // 添加背景卡片（tail，不持久化）
    cl_background_widget_tail_ = new CustomQWidgetBackgroundImages(
        BackgroundImageMode::AddCustom, cl_content_widget_);
    cl_background_widget_tail_->setCl_icon_size(QSize(48,38));
    cl_background_widget_tail_->setCenterIcon(QPixmap(":/Skin/Images/more/interface_settings/upload_background_2x_darkBlue.png"));
    cl_background_widget_tail_->setCenterText(tr("上传背景"));

    // 自定义弹窗
    clp_dialog_tips_ = new CustomQDialogGeneralTips(this);
    clp_dialog_tips_->setCl_texts(tr("确认删除此自定义背景吗？"), tr("取消"), tr("删除"));
}

/// \brief 弹出删除确认弹窗（exec() 安全封装，防止 exec 期间 this 被销毁导致崩溃）
void CustomQScrollAreaBackgroundComponent::showDeleteDialog(CustomQWidgetBackgroundImages *widget)
{
    QPointer<CustomQScrollAreaBackgroundComponent> t_self(this);
    QPointer<CustomQWidgetBackgroundImages> t_guard(widget);

    QObject::disconnect(clp_dialog_tips_, &CustomQDialogGeneralTips::confirmed,
                        this, nullptr);
    QObject::connect(clp_dialog_tips_, &CustomQDialogGeneralTips::confirmed,
                     this, [t_self, t_guard]() {
                         if (t_self && t_guard)
                             t_self->removeWidgetFromList(t_guard);
                     });

    clp_dialog_tips_->exec();

    if (t_self) {
        QObject::disconnect(clp_dialog_tips_, &CustomQDialogGeneralTips::confirmed,
                            this, nullptr);
    }
}

/// \brief 从 list 中移除 widget 并清理关联数据（map + 磁盘文件）
void CustomQScrollAreaBackgroundComponent::removeWidgetFromList(CustomQWidgetBackgroundImages *widget)
{
    if (!widget)
        return;
    // 拒绝 header/tail（不在 list 中），防止外部误调用导致崩溃
    if (!cl_background_widget_list_.contains(widget))
        return;
    if (widget->clp_wallpaper_map_ && widget->cl_wallpaper_index_ >= 0) {
        auto t_it = widget->clp_wallpaper_map_->find(widget->cl_wallpaper_index_);
        if (t_it != widget->clp_wallpaper_map_->end()) {
            QFile::remove(t_it.value()->path);
            cl_wallpaper_thumb_cache_.remove(t_it.value()->path);  ///< 同步清理缩略图缓存
            widget->clp_wallpaper_map_->erase(t_it);
        }
    }
    // 如果删除的是当前选中项，提前置空
    if (cl_current_item_ == widget) {
        cl_current_item_ = nullptr;
    }

    cl_background_widget_list_.removeOne(widget);
    widget->deleteLater();
    g_user_information.saveWallpaperConfig();
    updateView();

    // 删除后若无选中项，默认选中 header
    if (!cl_current_item_) {
        setCurrentItem(cl_background_widget_header_);
    }
}

/// \brief 连接默认的信号槽
void CustomQScrollAreaBackgroundComponent::InitConnect()
{
    // Header — 默认背景
    if (cl_background_widget_header_) {
        QObject::connect(cl_background_widget_header_, &CustomQWidgetBackgroundImages::defaultClicked,
                         this, [this]() {
                             setCurrentItem(cl_background_widget_header_);
                         },
                         Qt::UniqueConnection);
    }

    // List — 系统壁纸 + 用户自定义壁纸
    for (auto *t_widget : cl_background_widget_list_) {
        switch (t_widget->cl_mode_) {
        case BackgroundImageMode::DefaultTheme: {
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::defaultClicked, this,
                             [this, t_widget]() {
                                 setCurrentItem(t_widget);
                             },
                             Qt::UniqueConnection);
        } break;
        case BackgroundImageMode::SystemTheme: {
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::systemClicked, this,
                             [this, t_widget]() { setCurrentItem(t_widget); },
                             Qt::UniqueConnection);
        } break;
        case BackgroundImageMode::Custom: {
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::customClicked, this,
                             [this, t_widget]() { setCurrentItem(t_widget); },
                             Qt::UniqueConnection);

            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::deleteRequested, this,
                             [this, t_widget]() { showDeleteDialog(t_widget); },
                             Qt::UniqueConnection);
        } break;
        default: break;
        }
    }

    // Tail — 添加背景
    if (cl_background_widget_tail_) {
        QObject::connect(cl_background_widget_tail_, &CustomQWidgetBackgroundImages::backgroundAdded,
                         this, [this](const QString &path) {
                             QString t_dest_dir = g_user_information.customBackgroundDir();
                             if (t_dest_dir.isEmpty())
                                 return;
                             QDir().mkpath(t_dest_dir);

                             if (!g_user_information.local.custom_wallpaper_map)
                                 g_user_information.local.custom_wallpaper_map
                                     .reset(new QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>);
                             int t_idx = g_user_information.local.custom_wallpaper_map->isEmpty()
                                             ? 0
                                             : g_user_information.local.custom_wallpaper_map->lastKey() + 1;
                             QString t_dest = t_dest_dir + "/" + QString::number(t_idx) + ".png";

                             // 加载并压缩图片（限制最大尺寸，避免巨图导致渲染卡顿）
                             QPixmap t_pm(path);
                             if (t_pm.isNull())
                                 return;
                             constexpr int kMaxWidth = 1920;
                             constexpr int kMaxHeight = 1080;
                             if (t_pm.width() > kMaxWidth || t_pm.height() > kMaxHeight) {
                                 t_pm = t_pm.scaled(kMaxWidth, kMaxHeight,
                                                    Qt::KeepAspectRatio, Qt::SmoothTransformation);
                             }
                             if (!t_pm.save(t_dest, "PNG", 80))
                                 return;

                             g_user_information.local.custom_wallpaper_map->insert(
                                 t_idx, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>::create(t_dest, BackgroundImageMode::Custom));
                             g_user_information.saveWallpaperConfig();

                             // 插入列表（tail 之前）
                             auto *t_new = new CustomQWidgetBackgroundImages(
                                 BackgroundImageMode::Custom,
                                 g_user_information.local.custom_wallpaper_map, t_idx,
                                 cl_content_widget_);
                             t_new->setBackground(t_pm, t_dest);
                             cl_background_widget_list_.append(t_new);

                             // 连接新 widget 的信号
                             QObject::connect(t_new, &CustomQWidgetBackgroundImages::customClicked,
                                              this, [this, t_new]() {
                                                  setCurrentItem(t_new);
                                              },
                                              Qt::UniqueConnection);
                             QObject::connect(t_new, &CustomQWidgetBackgroundImages::deleteRequested,
                                              this, [this, t_new]() { showDeleteDialog(t_new); },
                                              Qt::UniqueConnection);

                             updateView();
                         });
    }
}

void CustomQScrollAreaBackgroundComponent::updateView()
{
    if (!cl_grid_layout_)
        return;

    int t_spacing = cl_grid_layout_->spacing();
    int t_item_width = 224;
    int t_viewport_width = viewport()->width();
    int t_column_count = (t_viewport_width + t_spacing) / (t_item_width + t_spacing);
    if (t_column_count < 1)
        t_column_count = 1;
    cl_column_count_ = t_column_count;

    while (QLayoutItem *t_item = cl_grid_layout_->takeAt(0))
        delete t_item;

    cl_grid_layout_->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    int t_pos = 0;
    auto t_place = [&](QWidget *w) {
        int t_row = t_pos / cl_column_count_;
        int t_col = t_pos % cl_column_count_ + 1;
        cl_grid_layout_->addWidget(w, t_row, t_col);
        ++t_pos;
    };

    // Header
    if (cl_background_widget_header_)
        t_place(cl_background_widget_header_);

    // List
    for (int i = 0; i < cl_background_widget_list_.size(); ++i)
        t_place(cl_background_widget_list_.at(i));

    // Tail
    if (cl_background_widget_tail_)
        t_place(cl_background_widget_tail_);
}

void CustomQScrollAreaBackgroundComponent::refreshListIfDirty()
{
    if (cl_list_dirty_) {
        refreshList();
        cl_list_dirty_ = false;
    } else {
        updateView();
    }
}

void CustomQScrollAreaBackgroundComponent::markDirty()
{
    cl_list_dirty_ = true;
}

QPixmap CustomQScrollAreaBackgroundComponent::loadWallpaperThumb(const QString &path)
{
    // 缓存命中直接返回（二次进入列表零解码）
    auto t_it = cl_wallpaper_thumb_cache_.constFind(path);
    if (t_it != cl_wallpaper_thumb_cache_.constEnd())
        return t_it.value();

    // 按缩略尺寸解码（QImageReader 降采样，避免全尺寸解码 + 内存常驻全图导致点击卡顿）
    QImageReader t_reader(path);
    const QSize t_orig = t_reader.size();
    constexpr int kMaxThumb = 448;  // 2× 网格卡片宽 224，显示清晰度足够
    if (t_orig.isValid() && (t_orig.width() > kMaxThumb || t_orig.height() > kMaxThumb)) {
        if (t_orig.width() >= t_orig.height()) {
            t_reader.setScaledSize(
                QSize(kMaxThumb, qMax(1, t_orig.height() * kMaxThumb / t_orig.width())));
        } else {
            t_reader.setScaledSize(
                QSize(qMax(1, t_orig.width() * kMaxThumb / t_orig.height()), kMaxThumb));
        }
    }
    QPixmap t_pm = QPixmap::fromImage(t_reader.read());
    if (!t_pm.isNull())
        cl_wallpaper_thumb_cache_.insert(path, t_pm);
    return t_pm;
}

/// \brief 从数据模型重新加载 cl_background_widget_list_
void CustomQScrollAreaBackgroundComponent::refreshList()
{
    // 清除旧 list widget
    for (auto *t_widget : cl_background_widget_list_) {
        t_widget->hide();
        t_widget->deleteLater();
    }
    cl_background_widget_list_.clear();

    // 重新从数据模型加载
    auto t_addWidgets = [this](const QSharedPointer<QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>> &map) {
        if (!map) return;
        for (auto it = map->constBegin(); it != map->constEnd(); ++it) {
            auto *t_widget = new CustomQWidgetBackgroundImages(
                it.value()->mode, map, it.key(), cl_content_widget_);
            QPixmap t_pm = loadWallpaperThumb(it.value()->path);  // 缩放解码 + 缓存（避免全尺寸解码卡顿）
            if (!t_pm.isNull())
                t_widget->setBackground(t_pm, it.value()->path);
            cl_background_widget_list_.append(t_widget);
        }
    };
    t_addWidgets(g_user_information.local.system_wallpaper_map);
    t_addWidgets(g_user_information.local.custom_wallpaper_map);

    // 连接新 widget 的信号
    for (auto *t_widget : cl_background_widget_list_) {
        switch (t_widget->cl_mode_) {
        case BackgroundImageMode::DefaultTheme: {
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::defaultClicked, this,
                             [this, t_widget]() { setCurrentItem(t_widget); },
                             Qt::UniqueConnection);
        } break;
        case BackgroundImageMode::SystemTheme: {
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::systemClicked, this,
                             [this, t_widget]() { setCurrentItem(t_widget); },
                             Qt::UniqueConnection);
        } break;
        case BackgroundImageMode::Custom: {
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::customClicked, this,
                             [this, t_widget]() { setCurrentItem(t_widget); },
                             Qt::UniqueConnection);
            QObject::connect(t_widget, &CustomQWidgetBackgroundImages::deleteRequested, this,
                             [this, t_widget]() { showDeleteDialog(t_widget); },
                             Qt::UniqueConnection);
        } break;
        default: break;
        }
    }

    // 根据数据模型恢复选中项（走 setCurrentItem 以触发信号）
    cl_current_item_ = nullptr;
    for (auto *t_widget : cl_background_widget_list_) {
        if (t_widget->clp_wallpaper_map_ && t_widget->cl_wallpaper_index_ >= 0) {
            auto t_it = t_widget->clp_wallpaper_map_->find(t_widget->cl_wallpaper_index_);
            if (t_it != t_widget->clp_wallpaper_map_->end() && t_it.value()->is_selected) {
                setCurrentItem(t_widget);
                break;
            }
        }
    }
    // 无选中项时默认选中 header
    if (!cl_current_item_) {
        setCurrentItem(cl_background_widget_header_);
    }

    updateView();
}

QPointer<CustomQWidgetBackgroundImages> CustomQScrollAreaBackgroundComponent::getCurrentItem() const
{
    return cl_current_item_;
}

void CustomQScrollAreaBackgroundComponent::setCurrentItem(CustomQWidgetBackgroundImages *item)
{
    if (cl_current_item_ == item)
        return;

    // 清除 header
    if (cl_background_widget_header_ && cl_background_widget_header_->cl_is_selected_) {
        cl_background_widget_header_->cl_is_selected_ = false;
        cl_background_widget_header_->update();
    }
    // 清除 list
    for (auto *t_widget : cl_background_widget_list_) {
        if (t_widget->cl_is_selected_) {
            t_widget->cl_is_selected_ = false;
            t_widget->update();
        }
    }
    // 清除 tail
    if (cl_background_widget_tail_ && cl_background_widget_tail_->cl_is_selected_) {
        cl_background_widget_tail_->cl_is_selected_ = false;
        cl_background_widget_tail_->update();
    }

    cl_current_item_ = item;
    if (cl_current_item_) {
        cl_current_item_->cl_is_selected_ = true;
        cl_current_item_->update();

        // 同步数据模型 + 通知 MainWindow 更换背景
        if (cl_current_item_->clp_wallpaper_map_ && cl_current_item_->cl_wallpaper_index_ >= 0) {
            // 系统壁纸 / 自定义壁纸
            g_user_information.local.selectWallpaper(cl_current_item_->clp_wallpaper_map_,
                                                     cl_current_item_->cl_wallpaper_index_);
            g_user_information.saveWallpaperConfig();

            auto t_it = cl_current_item_->clp_wallpaper_map_->find(
                cl_current_item_->cl_wallpaper_index_);
            if (t_it != cl_current_item_->clp_wallpaper_map_->end()) {
                emit backgroundChanged(t_it.value()->path);
            }
        } else if (cl_current_item_ == cl_background_widget_header_) {
            // 默认背景：清除所有壁纸选中标记 + 持久化（修复重启后自定义壁纸复活 bug）
            g_user_information.local.selectWallpaper(nullptr, -1);
            g_user_information.saveWallpaperConfig();
            emit defaultBackgroundRestored();
        }
    }
}

void CustomQScrollAreaBackgroundComponent::updateSelectedItemTransparency(qreal value)
{
    Q_UNUSED(value);
}

void CustomQScrollAreaBackgroundComponent::dealwithBackgroundWidgetClicked()
{
    auto *t_current = qobject_cast<CustomQWidgetBackgroundImages *>(sender());
    if (!t_current)
        return;
    setCurrentItem(t_current);
    update();
}

void CustomQScrollAreaBackgroundComponent::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    QTimer::singleShot(0, this, [this]() { this->updateView(); });
}
