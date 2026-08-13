#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/InterfaceSettingCustomUI/custom_QWidget_background_images.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

/// \brief 构造函数
CustomQWidgetBackgroundImages::CustomQWidgetBackgroundImages(BackgroundImageMode t_mode,
                                                             QWidget *parent,
                                                             int theme)
    : QWidget(parent)
    , cl_mode_(t_mode)
    , cl_theme_(theme)
{
    InitUIInformation(theme); ///< 初始化 UI 默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认信号槽
}

CustomQWidgetBackgroundImages::CustomQWidgetBackgroundImages(
    BackgroundImageMode t_mode,
    QSharedPointer<QMap<int, QSharedPointer<UserInformation::UserInfo_Local::WallpaperEntry>>> map,
    int index,
    QWidget *parent,
    int theme)
    : QWidget(parent)
    , cl_mode_(t_mode)
    , cl_theme_(theme)
    , clp_wallpaper_map_(map)
    , cl_wallpaper_index_(index)
{
    InitUIInformation(theme);
    InitMember();
    InitConnect();
}

CustomQWidgetBackgroundImages::~CustomQWidgetBackgroundImages() {}

/// \brief 按主题更新样式
void CustomQWidgetBackgroundImages::applyTheme(int theme)
{
    cl_theme_ = theme;
}

/// \brief 设置背景图
void CustomQWidgetBackgroundImages::setBackground(const QPixmap &pixmap, const QString &filePath)
{
    cl_background_pixmap_ = pixmap;
    cl_file_path_ = filePath;
    cl_cached_scaled_bg_ = QPixmap(); ///< 清缓存，下次 paintEvent 重新缩放
    update();
}

/// \brief 设置中心图标
void CustomQWidgetBackgroundImages::setCenterIcon(const QPixmap &pixmap)
{
    if (clp_center_icon_) {
        clp_center_icon_->setPixmap(
            pixmap.scaled(cl_icon_size_, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

/// \brief 设置中心文字
void CustomQWidgetBackgroundImages::setCenterText(const QString &text)
{
    if (clp_center_text_)
        clp_center_text_->setText(text);
}

/// \brief 设置图标尺寸
void CustomQWidgetBackgroundImages::setCl_icon_size(const QSize &size)
{
    cl_icon_size_ = size;
    if (clp_center_icon_)
        clp_center_icon_->setFixedSize(size);
    update();
}


/// \brief 设置文字高度
void CustomQWidgetBackgroundImages::setCl_text_height(int h)
{
    cl_text_size_.setHeight(h);
    update();
}

/// \brief 设置图标默认坐标
void CustomQWidgetBackgroundImages::setCl_icon_default_point(const QPoint &point)
{
    cl_icon_default_point_ = point;
    cl_icon_current_point_ = point;
    if (clp_center_icon_) clp_center_icon_->move(point);
}

/// \brief 设置图标悬停坐标
void CustomQWidgetBackgroundImages::setCl_icon_hover_point(const QPoint &point)
{
    cl_icon_hover_point_ = point;
}

/// \brief 设置文字默认坐标
void CustomQWidgetBackgroundImages::setCl_text_default_point(const QPoint &point)
{
    cl_text_default_point_ = point;
    cl_text_current_point_ = point;
    if (clp_center_text_) clp_center_text_->move(point);
}

/// \brief 设置文字悬停坐标
void CustomQWidgetBackgroundImages::setCl_text_hover_point(const QPoint &point)
{
    cl_text_hover_point_ = point;
}

/// \brief 设置删除按钮尺寸
void CustomQWidgetBackgroundImages::setCl_delete_btn_size(const QSize &size)
{
    cl_delete_btn_size_ = size;
    if (clp_delete_btn_)
        clp_delete_btn_->setFixedSize(size);
}

/// \brief 切换为指定模式
void CustomQWidgetBackgroundImages::switchMode(BackgroundImageMode targetMode)
{
    if (cl_mode_ == targetMode)
        return;

    // 清理旧模式的专属控件
    if (clp_hover_mask_) {
        clp_hover_mask_->deleteLater();
        clp_hover_mask_ = nullptr;
    }
    if (clp_delete_btn_) {
        clp_delete_btn_->deleteLater();
        clp_delete_btn_ = nullptr;
    }

    // 销毁动画
    clp_icon_anim_.reset();
    clp_text_pos_anim_.reset();
    clp_text_opacity_anim_.reset();

    // 隐藏图标和文字（后续按新模式决定是否显示）
    if (clp_center_icon_)
        clp_center_icon_->hide();
    if (clp_center_text_)
        clp_center_text_->hide();

    // 更新模式
    cl_mode_ = targetMode;

    // 按目标模式创建控件
    switch (targetMode) {
    case BackgroundImageMode::DefaultTheme:
    case BackgroundImageMode::AddCustom: {
        // 图标 + 文字 + 动画
        if (!clp_center_icon_) {
            clp_center_icon_ = new QLabel(this);
            clp_center_icon_->setObjectName("BackgroundImages_centerIcon");
            clp_center_icon_->setAlignment(Qt::AlignCenter);
            clp_center_icon_->setFixedSize(cl_icon_size_);
        }
        clp_center_icon_->show();

        if (!clp_center_text_) {
            clp_center_text_ = new QLabel(this);
            clp_center_text_->setObjectName("BackgroundImages_centerText");
            clp_center_text_->setAlignment(Qt::AlignCenter);
            clp_center_text_->setStyleSheet(R"(
                QLabel#BackgroundImages_centerText {
                    font-family: "Noto Sans S Chinese";
                    font-weight: 500;
                    font-size: 12px;
                    color: #A1A8B3;
                    background: transparent;
                }
            )");
        }
        clp_center_text_->hide();

        // 重新创建动画
        clp_icon_anim_ = std::make_unique<QVariantAnimation>(this);
        clp_icon_anim_->setDuration(200);
        clp_icon_anim_->setEasingCurve(QEasingCurve::OutCubic);

        clp_text_pos_anim_ = std::make_unique<QVariantAnimation>(this);
        clp_text_pos_anim_->setDuration(200);
        clp_text_pos_anim_->setEasingCurve(QEasingCurve::OutCubic);

        clp_text_opacity_anim_ = std::make_unique<QVariantAnimation>(this);
        clp_text_opacity_anim_->setDuration(200);
        clp_text_opacity_anim_->setEasingCurve(QEasingCurve::OutCubic);

        // 连接动画信号
        connect(clp_icon_anim_.get(), &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    cl_icon_current_point_ = value.value<QPoint>();
                    clp_center_icon_->move(cl_icon_current_point_);
                });
        connect(clp_text_pos_anim_.get(), &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    cl_text_current_point_ = value.value<QPoint>();
                    clp_center_text_->move(cl_text_current_point_);
                });
        connect(clp_text_opacity_anim_.get(), &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    cl_text_opacity_ = value.value<qreal>();
                    update();
                });
    } break;

    case BackgroundImageMode::SystemTheme: {
        // 悬停阴影遮罩
        clp_hover_mask_ = new QLabel(this);
        clp_hover_mask_->setObjectName("BackgroundImages_hoverMask");
        clp_hover_mask_->setStyleSheet(R"(
            QLabel#BackgroundImages_hoverMask {
                background-color: rgba(0, 0, 0, 0.3);
                border-radius: 8px;
            }
        )");
        clp_hover_mask_->hide();
    } break;

    case BackgroundImageMode::Custom: {
        // 悬停阴影遮罩 + 删除按钮
        clp_hover_mask_ = new QLabel(this);
        clp_hover_mask_->setObjectName("BackgroundImages_hoverMask");
        clp_hover_mask_->setStyleSheet(R"(
            QLabel#BackgroundImages_hoverMask {
                background-color: rgba(0, 0, 0, 0.3);
                border-radius: 8px;
            }
        )");
        clp_hover_mask_->hide();

        clp_delete_btn_ = new QPushButton(this);
        clp_delete_btn_->setObjectName("BackgroundImages_delete");
        clp_delete_btn_->setFixedSize(cl_delete_btn_size_);
        clp_delete_btn_->setCursor(Qt::PointingHandCursor);
        clp_delete_btn_->setStyleSheet(R"(
            QPushButton#BackgroundImages_delete {
                border: none;
                border-image: url(:/Skin/Images/more/interface_settings/del_background_normal_1x_darkBlue.png);
            }
            QPushButton#BackgroundImages_delete:hover {
                border-image: url(:/Skin/Images/more/interface_settings/del_background_hover_1x_darkBlue.png);
            }
        )");
        clp_delete_btn_->hide();

        connect(clp_delete_btn_, &QPushButton::clicked, this, [this]() {
            emit deleteRequested();
        });
    } break;

    default: break;
    }
}

/// \brief 初始化 UI 默认信息
void CustomQWidgetBackgroundImages::InitUIInformation(int theme)
{
    setObjectName("CustomQWidgetBackgroundImages");
    setMouseTracking(true);

    {
        clp_background_ = new QLabel(this);
        clp_background_->setObjectName("BackgroundImages_bg");
        clp_background_->setScaledContents(true);
        clp_background_->setStyleSheet(R"()");
    }

    switch (cl_mode_) {
    case BackgroundImageMode::DefaultTheme: {
        // 默认主题 — 无背景图时显示图标 + 文字，点击可添加背景
        clp_center_icon_ = new QLabel(this);
        clp_center_icon_->setObjectName("BackgroundImages_centerIcon");
        clp_center_icon_->setAlignment(Qt::AlignCenter);
        clp_center_icon_->setFixedSize(cl_icon_size_);
        clp_center_icon_->setStyleSheet(R"()");

        clp_center_text_ = new QLabel(this);
        clp_center_text_->setObjectName("BackgroundImages_centerText");
        clp_center_text_->setAlignment(Qt::AlignCenter);
        clp_center_text_->hide();
        clp_center_text_->setStyleSheet(R"(
            QLabel#BackgroundImages_centerText {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background: transparent;
            }
        )");
    } break;

    case BackgroundImageMode::SystemTheme: {
        // 系统壁纸预览 — 悬停阴影遮罩
        clp_hover_mask_ = new QLabel(this);
        clp_hover_mask_->setObjectName("BackgroundImages_hoverMask");
        clp_hover_mask_->setStyleSheet(R"(
            QLabel#BackgroundImages_hoverMask {
                background-color: rgba(0, 0, 0, 0.3);
                border-radius: 8px;
            }
        )");
        clp_hover_mask_->hide();
    } break;

    case BackgroundImageMode::Custom: {
        // 用户自定义壁纸 — 悬停阴影遮罩 + 删除按钮
        clp_hover_mask_ = new QLabel(this);
        clp_hover_mask_->setObjectName("BackgroundImages_hoverMask");
        clp_hover_mask_->setStyleSheet(R"(
            QLabel#BackgroundImages_hoverMask {
                background-color: rgba(0, 0, 0, 0.3);
                border-radius: 8px;
            }
        )");
        clp_hover_mask_->hide();

        clp_delete_btn_ = new QPushButton(this);
        clp_delete_btn_->setObjectName("BackgroundImages_delete");
        clp_delete_btn_->setFixedSize(cl_delete_btn_size_);
        clp_delete_btn_->setCursor(Qt::PointingHandCursor);
        clp_delete_btn_->setStyleSheet(R"(
            QPushButton#BackgroundImages_delete {
                border: none;
                border-image: url(:/Skin/Images/more/interface_settings/del_background_normal_1x_darkBlue.png);
            }
            QPushButton#BackgroundImages_delete:hover {
                border-image: url(:/Skin/Images/more/interface_settings/del_background_hover_1x_darkBlue.png);
            }
            QPushButton#BackgroundImages_delete:pressed {
                border-image: url(:/Skin/Images/more/interface_settings/del_background_click_1x_darkBlue.png);
            }
        )");
        clp_delete_btn_->hide();
    } break;

    case BackgroundImageMode::AddCustom: {
        // "添加背景"入口 — 图标 + 文字，点击打开文件选择
        clp_center_icon_ = new QLabel(this);
        clp_center_icon_->setObjectName("BackgroundImages_centerIcon");
        clp_center_icon_->setAlignment(Qt::AlignCenter);
        clp_center_icon_->setFixedSize(cl_icon_size_);
        clp_center_icon_->setStyleSheet(R"()");

        clp_center_text_ = new QLabel(this);
        clp_center_text_->setObjectName("BackgroundImages_centerText");
        clp_center_text_->setAlignment(Qt::AlignCenter);
        clp_center_text_->hide();
        clp_center_text_->setStyleSheet(R"(
            QLabel#BackgroundImages_centerText {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background: transparent;
            }
        )");
    } break;

    default: break;
    }

    applyTheme(theme);
}

/// \brief 初始化内部成员
void CustomQWidgetBackgroundImages::InitMember()
{
    setMinimumSize(224, 134);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    if (clp_center_icon_) {
        clp_icon_anim_ = std::make_unique<QVariantAnimation>(this);
        clp_icon_anim_->setDuration(200);
        clp_icon_anim_->setEasingCurve(QEasingCurve::OutCubic);

        clp_text_pos_anim_ = std::make_unique<QVariantAnimation>(this);
        clp_text_pos_anim_->setDuration(200);
        clp_text_pos_anim_->setEasingCurve(QEasingCurve::OutCubic);

        clp_text_opacity_anim_ = std::make_unique<QVariantAnimation>(this);
        clp_text_opacity_anim_->setDuration(200);
        clp_text_opacity_anim_->setEasingCurve(QEasingCurve::OutCubic);
    }

    cl_icon_current_point_ = cl_icon_default_point_;
    if (clp_center_icon_) clp_center_icon_->move(cl_icon_default_point_);

    cl_text_current_point_ = cl_text_default_point_;
    if (clp_center_text_) clp_center_text_->move(cl_text_default_point_);
}

/// \brief 连接默认信号槽
void CustomQWidgetBackgroundImages::InitConnect()
{
    if (clp_delete_btn_) {
        connect(clp_delete_btn_, &QPushButton::clicked, this, [this]() {
            emit deleteRequested();
        });
    }

    if (clp_icon_anim_) {
        connect(clp_icon_anim_.get(), &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    cl_icon_current_point_ = value.value<QPoint>();
                    clp_center_icon_->setGeometry((rect().width() - cl_icon_size_.width())/2,cl_icon_current_point_.y(),cl_icon_size_.width(),cl_icon_size_.height());
                });
    }

    if (clp_text_pos_anim_) {
        connect(clp_text_pos_anim_.get(), &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    cl_text_current_point_ = value.value<QPoint>();
                    clp_center_text_->setGeometry(cl_text_default_point_.x(),
                                                  cl_text_current_point_.y(),
                                                  (rect().width() - cl_text_default_point_.x() * 2),
                                                  cl_text_size_.height());
                });
    }

    if (clp_text_opacity_anim_) {
        connect(clp_text_opacity_anim_.get(), &QVariantAnimation::valueChanged, this,
                [this](const QVariant &value) {
                    cl_text_opacity_ = value.value<qreal>();
                    update();
                });
    }
}

void CustomQWidgetBackgroundImages::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter t_painter(this);
    t_painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath t_path;
    t_path.addRoundedRect(rect(), 8, 8);
    t_painter.setClipPath(t_path);

    if (cl_background_pixmap_.isNull()) {
        t_painter.fillRect(rect(), QColor(81, 96, 122, 51));
    } else {
        // 缓存缩放结果，避免每帧 paintEvent 都对大图做 SmoothTransformation
        if (cl_cached_scaled_bg_.size() != size() || cl_cached_scaled_bg_.isNull()) {
            cl_cached_scaled_bg_ = cl_background_pixmap_.scaled(size(),
                                                                Qt::KeepAspectRatioByExpanding,
                                                                Qt::SmoothTransformation);
        }
        int t_x = (width() - cl_cached_scaled_bg_.width()) / 2;
        int t_y = (height() - cl_cached_scaled_bg_.height()) / 2;
        t_painter.drawPixmap(t_x, t_y, cl_cached_scaled_bg_);
    }

    if (cl_is_selected_) {
        t_painter.setOpacity(1.0);
        t_painter.setPen(QPen(QColor("#009FEF"), 2));
        t_painter.setBrush(Qt::NoBrush);
        t_painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
    }
}

void CustomQWidgetBackgroundImages::enterEvent(QEvent *event)
{
    cl_is_hover_ = true;
    QWidget::enterEvent(event);

    switch (cl_mode_) {
    case BackgroundImageMode::DefaultTheme:
    case BackgroundImageMode::AddCustom: {
        clp_center_text_->show();

        clp_icon_anim_->stop();
        clp_icon_anim_->setKeyValues({{0.0, cl_icon_current_point_},
                                       {1.0, cl_icon_hover_point_}});
        clp_icon_anim_->start();

        clp_text_pos_anim_->stop();
        clp_text_pos_anim_->setKeyValues({{0.0, cl_text_current_point_},
                                           {1.0, cl_text_hover_point_}});
        clp_text_pos_anim_->start();

        clp_text_opacity_anim_->stop();
        clp_text_opacity_anim_->setKeyValues({{0.0, cl_text_opacity_},
                                               {1.0, 1.0}});
        clp_text_opacity_anim_->start();
    } break;

    case BackgroundImageMode::SystemTheme: {
        clp_hover_mask_->setGeometry(rect());
        clp_hover_mask_->show();
        clp_hover_mask_->raise();
    } break;

    case BackgroundImageMode::Custom: {
        clp_hover_mask_->setGeometry(rect());
        clp_hover_mask_->show();
        clp_hover_mask_->raise();

        clp_delete_btn_->move(width() - cl_delete_btn_size_.width () - 6, 6);
        clp_delete_btn_->show();
        clp_delete_btn_->raise();
    } break;

    default: break;
    }
}

void CustomQWidgetBackgroundImages::leaveEvent(QEvent *event)
{
    cl_is_hover_ = false;
    QWidget::leaveEvent(event);

    switch (cl_mode_) {
    case BackgroundImageMode::DefaultTheme:
    case BackgroundImageMode::AddCustom: {
        clp_icon_anim_->stop();
        clp_icon_anim_->setKeyValues({{0.0, cl_icon_current_point_},
                                       {1.0, cl_icon_default_point_}});
        clp_icon_anim_->start();

        clp_text_pos_anim_->stop();
        clp_text_pos_anim_->setKeyValues({{0.0, cl_text_current_point_},
                                           {1.0, cl_text_default_point_}});
        clp_text_pos_anim_->start();

        clp_text_opacity_anim_->stop();
        clp_text_opacity_anim_->setKeyValues({{0.0, cl_text_opacity_},
                                               {1.0, 0.0}});
        // 动画结束后再隐藏文字
        QMetaObject::Connection *t_conn = new QMetaObject::Connection;
        *t_conn = connect(clp_text_opacity_anim_.get(), &QVariantAnimation::finished, this,
                          [this, t_conn]() {
                              clp_center_text_->hide();
                              disconnect(*t_conn);
                              delete t_conn;
                          });
        clp_text_opacity_anim_->start();
    } break;

    case BackgroundImageMode::SystemTheme: {
        clp_hover_mask_->hide();
    } break;

    case BackgroundImageMode::Custom: {
        clp_hover_mask_->hide();
        clp_delete_btn_->hide();
    } break;

    default: break;
    }
}

void CustomQWidgetBackgroundImages::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    switch (cl_mode_) {
    case BackgroundImageMode::DefaultTheme: {
        cl_is_selected_ = true;
        emit defaultClicked();
    } break;

    case BackgroundImageMode::SystemTheme: {
        cl_is_selected_ = true;
        emit systemClicked();
    } break;

    case BackgroundImageMode::Custom: {
        cl_is_selected_ = true;
        emit customClicked();
    } break;

    case BackgroundImageMode::AddCustom: {
        QString t_file = QFileDialog::getOpenFileName(
            this, tr("选择背景图片"),
            XIBERIA_X_HUB_Utils::getDefaultScreenshotPath(),
            tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));
        if (!t_file.isEmpty()) {
            QPixmap t_pm(t_file);
            if (!t_pm.isNull()) {
                // setBackground(t_pm, t_file);
                // switchMode(BackgroundImageMode::Custom);
                emit backgroundAdded(t_file);
            }
        }
        return;
    } break;

    default: break;
    }

    QWidget::mousePressEvent(event);
}

void CustomQWidgetBackgroundImages::resizeEvent(QResizeEvent *event)
{
    clp_background_->setGeometry(rect());

    if (clp_center_icon_) clp_center_icon_->setGeometry((rect().width() - cl_icon_size_.width())/2,cl_icon_current_point_.y(),cl_icon_size_.width(),cl_icon_size_.height());

    if (clp_center_text_)
        clp_center_text_->setGeometry(cl_text_default_point_.x(),
                                      cl_text_current_point_.y(),
                                      (rect().width() - cl_text_default_point_.x() * 2),
                                      cl_text_size_.height());

    QWidget::resizeEvent(event);
}
