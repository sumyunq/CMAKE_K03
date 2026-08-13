#include "modules/HomePage/HomePageCustomUI/custom_QWidget_product_display.h"
#include "ui_custom_QWidget_product_display.h"

namespace {
const QSize kProductImageBaseSize(526, 616);
} // namespace

CustomQWidgetProductDisplay::CustomQWidgetProductDisplay(QWidget *parent)
    : CumtomQWidgetGlobalBase(parent)
    , ui(new Ui::CustomQWidgetProductDisplay)
{
    ui->setupUi(this);
    InitUIInformation(); // 初始化UI的默认信息
    InitMember();        // 初始化内部成员
    InitConnect();       // 连接默认的信号槽
}

CustomQWidgetProductDisplay::~CustomQWidgetProductDisplay()
{
    delete ui;
}

void CustomQWidgetProductDisplay::UpdateBackgroundImage(const QString &imagePath,
                                                        QPoint globalStartPoint,
                                                        QSize startSize,
                                                        QPoint globalTargetPoint,
                                                        QSize targetSize)
{
    QPoint localPos = this->mapFromGlobal(globalStartPoint);
    QPoint targetPos = this->mapFromGlobal(globalTargetPoint);

    cl_images_path_ = imagePath;

    if (!cl_images_path_.isEmpty()) {
        cl_background_pixmap_ = QPixmap();
    }

    cl_pixmap_label_->hide();

    // 直接加载资源
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        QPixmap scaled = scaledProductPixmap(pixmap);
        flyLabel->setPixmap(scaled);
    }

    flyLabel->move(localPos);
    flyLabel->resize(startSize);
    flyLabel->raise();
    flyLabel->show();

    posAnim->setStartValue(localPos);
    posAnim->setEndValue(targetPos);

    sizeAnim->setStartValue(flyLabel->size());
    sizeAnim->setEndValue(targetSize);

    animGroup->start();

    // 超时处理
    QTimer::singleShot(500, this, [=]() {
        if (animGroup->state() == QAbstractAnimation::Running) {
            animGroup->stop(); // 强制停止动画
        }
        flyLabel->hide();
        // 执行后备操作：直接显示图片
        QImage Image;
        Image.load(imagePath);
        cl_background_pixmap_ = QPixmap::fromImage(Image);
        updateProductPixmapLabel();
        cl_pixmap_label_->show();
    });
}

void CustomQWidgetProductDisplay::UpdateBackgroundImageImmediately(const QString &imagePath)
{
    cl_images_path_ = imagePath;
    cl_background_pixmap_ = QPixmap(imagePath);
    updateProductPixmapLabel();
    cl_pixmap_label_->show();
    update();
}

QRect CustomQWidgetProductDisplay::productImageRect() const
{
    if (rect().isEmpty()) {
        return rect();
    }

    QSize imageSize = kProductImageBaseSize;
    if (rect().width() < kProductImageBaseSize.width()
        || rect().height() < kProductImageBaseSize.height()) {
        imageSize.scale(rect().size(), Qt::KeepAspectRatio);
    }

    const int x = rect().x() + (rect().width() - imageSize.width()) / 2;
    const int y = rect().y() + (rect().height() - imageSize.height()) / 2;
    return QRect(QPoint(x, y), imageSize);
}

QPixmap CustomQWidgetProductDisplay::scaledProductPixmap(const QPixmap &pixmap) const
{
    if (pixmap.isNull()) {
        return QPixmap();
    }

    return pixmap.scaled(productImageRect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CustomQWidgetProductDisplay::updateProductPixmapLabel()
{
    if (cl_pixmap_label_ == nullptr) {
        return;
    }

    cl_pixmap_label_->setGeometry(productImageRect());
    cl_pixmap_label_->setPixmap(scaledProductPixmap(cl_background_pixmap_));
    cl_pixmap_label_->lower();
}

void CustomQWidgetProductDisplay::InitUIInformation()
{
    setAttribute(Qt::WA_StyledBackground, true);

    cl_pBt_devSel_ = new CustomQPushButtonHoverLeftward(this->cl_content_widget_);    ///< 选择机型
    cl_pBt_SysVloSet_ = new CustomQPushButtonHoverLeftward(this->cl_content_widget_); ///< 声音设置
    cl_pBt_UserGuide_ = new CustomQPushButtonHoverLeftward(this->cl_content_widget_); ///< 说明书
    {
        // 选择机型
        cl_pBt_devSel_->setCl_text(tr("选择机型"));
        cl_pBt_devSel_->move(cl_pBt_devSel_default_point_);
        cl_pBt_devSel_->setCl_reference_point(cl_pBt_devSel_default_point_); // 更新内部动画参考点
        cl_pBt_devSel_->setCl_text_rect(cl_pBt_devSel_->rect().adjusted(37, 7, -14, -8));
        cl_pBt_devSel_->setCl_pixmap(
            QPixmap(":/Skin/Images/homePage/Change_model_hover_2x.png"));
        cl_pBt_devSel_->setCl_default_pixmap(
            QPixmap(":/Skin/Images/homePage/Change_model_2x.png"));
    }
    {
        // 声音设置
        cl_pBt_SysVloSet_->setCl_text(tr("声音设置"));
        cl_pBt_SysVloSet_->move(cl_pBt_SysVloSet_default_point_);
        cl_pBt_SysVloSet_->setCl_reference_point(
            cl_pBt_SysVloSet_default_point_); // 更新内部动画参考点
        cl_pBt_SysVloSet_->setCl_text_rect(cl_pBt_SysVloSet_->rect().adjusted(36, 7, -14, -8));
        cl_pBt_SysVloSet_->setCl_pixmap(
            QPixmap(":/Skin/Images/homePage/sound_setting_hover_2x.png"));
        cl_pBt_SysVloSet_->setCl_default_pixmap(
            QPixmap(":/Skin/Images/homePage/sound_setting_2x.png"));
    }
    {
        // 说明书
        cl_pBt_UserGuide_->setCl_text(tr("说明书"));
        cl_pBt_UserGuide_->move(cl_pBt_UserGuide_default_point_);
        cl_pBt_UserGuide_->setCl_expand_size(QSize(85, 32));
        cl_pBt_UserGuide_->setCl_reference_point(
            cl_pBt_UserGuide_default_point_); // 更新内部动画参考点
        cl_pBt_UserGuide_->setCl_text_rect(cl_pBt_UserGuide_->rect().adjusted(35, 7, -14, -8));
        cl_pBt_UserGuide_->setCl_pixmap(
            QPixmap(":/Skin/Images/homePage/instruction_book_hover_2x.png"));
        cl_pBt_UserGuide_->setCl_default_pixmap(
            QPixmap(":/Skin/Images/homePage/instruction_book_2x.png"));

        // cl_pBt_UserGuide_->hide();
    }
    {
        // 按键 — 功能说明
        cl_pBt_explain_ = new QPushButton(this->cl_content_widget_);
        cl_pBt_explain_->setMinimumSize(cl_pBt_explain_default_size_);
        cl_pBt_explain_->setObjectName("CustomQWidgetProductDisplay_cl_pBt_new");
        cl_pBt_explain_->setCursor(Qt::PointingHandCursor);
        cl_pBt_explain_->installEventFilter(this);
        cl_pBt_explain_->move(cl_pBt_explain_default_point_);
        cl_pBt_explain_->setStyleSheet(R"(
        QPushButton{
            border-image: url(:/Skin/Images/homePage/annotation_25_25_2x.png);
        }
)");
        {
            // cl_pBt_explain_ 自定义提示按键
            clp_tip_explain_ = new NewCustomToolTip(cl_pBt_explain_);
            clp_tip_explain_->setLabelStyle(0);
            clp_tip_explain_->AddToolTip(cl_pBt_explain_, tr("显示已连接的耳机型号及实时状态。点击右上角按钮可以切换设备，跳转系统声卡设置、耳机说明书。"), Qt::AlignHCenter);
        }
    }
}

void CustomQWidgetProductDisplay::retranslateTexts()
{
    cl_pBt_devSel_->setCl_text(tr("选择机型"));
    cl_pBt_SysVloSet_->setCl_text(tr("声音设置"));
    cl_pBt_UserGuide_->setCl_text(tr("说明书"));
}

void CustomQWidgetProductDisplay::InitMember()
{
    {
        cl_pixmap_label_ = new QLabel(this);
        cl_pixmap_label_->setMinimumSize(526, 616);
        cl_pixmap_label_->setAlignment(Qt::AlignCenter);
        cl_pixmap_label_->move(0, 0);
        cl_pixmap_label_->hide();
    }
    {
        // 动画标签
        flyLabel = new QLabel(this);
        flyLabel->setStyleSheet(R"(
QLabel{
background-color: transparent;
}
)");
        flyLabel->setAlignment(Qt::AlignCenter);
        flyLabel->hide();
    }
    {
        posAnim = new QPropertyAnimation(flyLabel, "pos");
        posAnim->setDuration(300);                      //播放时间 300 ms
        posAnim->setEasingCurve(QEasingCurve::OutQuad); // 添加缓动曲线使动画更平滑
    }

    {
        sizeAnim = new QPropertyAnimation(flyLabel, "size");
        sizeAnim->setDuration(300);                      //播放时间 300 ms
        sizeAnim->setEasingCurve(QEasingCurve::OutQuad); // 添加缓动曲线使动画更平滑
    }

    {
        animGroup = new QParallelAnimationGroup(this);

        animGroup->addAnimation(posAnim);  // 添加位置动画
        animGroup->addAnimation(sizeAnim); // 添加尺寸动画
    }
}

void CustomQWidgetProductDisplay::InitConnect()
{
    // 动画完成后的处理
    connect(animGroup, &QParallelAnimationGroup::finished, [=]() {
        flyLabel->hide();

        if (!cl_images_path_.isEmpty()) {
            QImage Image;
            Image.load(cl_images_path_);
            cl_background_pixmap_ = QPixmap::fromImage(Image);
            updateProductPixmapLabel();
            cl_pixmap_label_->show();
            update();
        }
    });
}

void CustomQWidgetProductDisplay::resizeEvent(QResizeEvent *event)
{
    CumtomQWidgetGlobalBase::resizeEvent(event); // 调用基类，保持基本行为
    //子部件
    {
        // 按键:选择机型
        if (cl_pBt_devSel_ != nullptr) {
            cl_pBt_devSel_default_point_.setX(
                rect().width() - 23 - cl_pBt_devSel_->cl_default_size().width()); // 更新x坐标
            cl_pBt_devSel_->move(cl_pBt_devSel_default_point_);
            cl_pBt_devSel_->setCl_reference_point(
                cl_pBt_devSel_default_point_); // 更新内部动画参考点
        }
    }
    {
        // 按键:声音设置
        if (cl_pBt_SysVloSet_ != nullptr) {
            cl_pBt_SysVloSet_default_point_.setX(
                rect().width() - 23 - cl_pBt_SysVloSet_->cl_default_size().width()); // 更新x坐标
            cl_pBt_SysVloSet_->move(cl_pBt_SysVloSet_default_point_);
            cl_pBt_SysVloSet_->setCl_reference_point(
                cl_pBt_SysVloSet_default_point_); // 更新内部动画参考点
        }
    }
    {
        // 按键:说明书
        if (cl_pBt_UserGuide_ != nullptr) {
            cl_pBt_UserGuide_default_point_.setX(
                rect().width() - 23 - cl_pBt_UserGuide_->cl_default_size().width()); // 更新x坐标
            cl_pBt_UserGuide_->move(cl_pBt_UserGuide_default_point_);
            cl_pBt_UserGuide_->setCl_reference_point(
                cl_pBt_UserGuide_default_point_); // 更新内部动画参考点
        }
    }
    {
        // 按键:功能说明
        cl_pBt_explain_->setGeometry(23,
                                     24,
                                     cl_pBt_explain_default_size_.width(),
                                     cl_pBt_explain_default_size_.height());
    }
    updateProductPixmapLabel();
}

// void CustomQWidgetProductDisplay::paintEvent(QPaintEvent *event)
// {
//     if (!cl_background_pixmap_.isNull()) {
//         QPainter painter(this);
//         // 按比例缩放
//         QPixmap scaled = cl_background_pixmap_.scaled(size(),
//                                                       Qt::KeepAspectRatio,
//                                                       Qt::SmoothTransformation);
//         // 居中绘制
//         int x = (width() - scaled.width()) / 2;
//         int y = (height() - scaled.height()) / 2;
//         // painter.drawPixmap(x, y, scaled);
//     }
//     CumtomQWidgetGlobalBase::paintEvent(event);
// }

bool CustomQWidgetProductDisplay::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == cl_pBt_explain_) {
        if (event->type() == QEvent::Enter) {
            // WBLIU: 悬停进入样式由用户填写
        } else if (event->type() == QEvent::Leave) {
            // WBLIU: 悬停离开样式由用户填写
        }
    }
    return CumtomQWidgetGlobalBase::eventFilter(watched, event);
}
