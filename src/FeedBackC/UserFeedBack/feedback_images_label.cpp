#include "FeedBackC/UserFeedBack/feedback_images_label.h"
#include "data/api_global.h"

#include <QMessageBox>

FeedbackImagesLabel::FeedbackImagesLabel(QWidget *parent)
    : QLabel(parent)
{
    InitUIInformation();
    InitMember();
    InitConnect();
}

FeedbackImagesLabel::~FeedbackImagesLabel()
{
    cl_feedback_pixmap_ = QPixmap();
    cl_feedback_file_name_.clear();

    cl_is_selected_ = false;
    cl_is_hover_ = false;
}

void FeedbackImagesLabel::InitMember()
{
    cl_minrect_w_ = 80;
    cl_minrect_h_ = 80;
    cl_maxrect_w_ = 80;
    cl_maxrect_h_ = 80;
    cl_is_selected_ = false; ///默认非选中状态
    cl_is_hover_ = false;    ///默认非悬停状态
    side = cl_minrect_w_ / 6;
    this->setMinimumSize(QSize(cl_minrect_w_, cl_maxrect_h_));
    this->setMaximumSize(QSize(cl_maxrect_w_, cl_maxrect_h_));
    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_Hover, true);

    clp_pushButotn_del_ = new QPushButton(this);
    clp_pushButotn_del_->setCursor(Qt::PointingHandCursor);
    clp_pushButotn_del_->setGeometry(59, 5, 16, 16);
    clp_pushButotn_del_->setFixedSize(QSize(16, 16));
    // QPixmap pixmap(":/Skin/Images/userFeedback/label_delete_1x.png");
    // if (!pixmap.isNull()) {
    //     QIcon icon(pixmap);
    //     clp_pushButotn_del_->setIcon(icon);
    //     // clp_pushButotn_del_->setIconSizeQSize(16, 16);
    // }
    /// 默认隐藏
    clp_pushButotn_del_->hide();
    clp_pushButotn_del_->raise(); // 将按钮提升到父控件的最上层

    clp_pushButotn_del_->setStyleSheet(
        R"(
        QPushButton {
        border-image: url(:/Skin/Images/userFeedback/label_delete_1x.png);
        }
        QPushButton:hover {
        border-image: url(:/Skin/Images/userFeedback/label_delete_hover.png);
        }
)");
}

void FeedbackImagesLabel::InitUIInformation()
{
    setStyleSheet(R"(
    QMessageBox {
       background-color: #2D2D30;      // 对话框背景色
       border: 1px solid #555555;
       border-radius: 6px;
    }
    QMessageBox QLabel {
       color: #00ff00;                 // 所有文本颜色
       font-size: 12px;
    }
    QMessageBox QPushButton {
       background-color: #3A3A3A;
       border: 1px solid #555555;
       border-radius: 4px;
       color: #FFFFFF;
       font-size: 12px;
       padding: 6px 16px;
       min-width: 70px;
    }
    QMessageBox QPushButton:hover {
       background-color: #505050;
    }
    QMessageBox QPushButton:pressed {
       background-color: #2A2A2A;
    }
    QMessageBox QPushButton:default {
       background-color: #0078D7;     /* 默认按钮（回车触发）高亮 */
       border-color: #0078D7;
    }

)");
}

void FeedbackImagesLabel::InitConnect()
{
    QObject::connect(clp_pushButotn_del_, &QPushButton::clicked, this, [this]() {
        qDebug() << "点击删除";
        /// 检查是否选择了图片，如果有，进行删除
        if (!cl_feedback_file_name_.isEmpty()) {
            cl_feedback_file_name_.clear();
            cl_feedback_pixmap_ = QPixmap();
            emit DelFeedbackImagesLabelSucceed();
            clp_pushButotn_del_->hide();
            update();
        }
    });
}

bool FeedbackImagesLabel::isImageSizeValid(QString imagefileName)
{
    /// 验证文件格式
    QFileInfo fileInfo(imagefileName);
    QString suffix = fileInfo.suffix().toLower();
    QStringList supportedFormats = {"png", "jpg", "jpeg", "bmp", "gif"};
    if (!supportedFormats.contains(suffix)) {
        QMessageBox::warning(this, tr("格式不支持"), tr("仅支持 PNG、JPG、JPEG、BMP、GIF 格式"));
        return false;
    }

    /// 验证文件大小
    qint64 fileSize = fileInfo.size();
    double sizeMB = fileSize / (1024.0 * 1024.0);
    const double maxSizeMB = 2.0;

    if (sizeMB > maxSizeMB) {
        showImageLimitDialog();
        return false;
    }
    return true;
}

void FeedbackImagesLabel::showImageLimitDialog()
{
    CustomQDialogGeneralTips t_dialog(this);
    t_dialog.setCl_dialog_size(QSize(402, 222));
    t_dialog.setCl_title_geometry(QRect(0, 50, 402, 24));
    t_dialog.setCl_message_geometry(QRect(45, 91, 312, 40));
    t_dialog.setCl_close_button_geometry(QRect(361, 10, 31, 31));
    t_dialog.setCl_confirm_button_geometry(QRect(149, 149, 104, 30));
    t_dialog.setCl_cancel_visible(false);
    t_dialog.setCl_texts(tr("图片大小超出限制"),
                         tr("当前图片已超过2M限制，请压缩图片后再次尝试上传"),
                         QString(),
                         tr("我知道了"));
    t_dialog.exec();
}

void FeedbackImagesLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 4px 圆角边框
    {
        QPainterPath clipPath;
        clipPath.addRoundedRect(rect(), 4, 4); // 4px 圆角
        painter.setClipPath(clipPath);
    }

    if (cl_feedback_file_name_.isEmpty()) {
        if (cl_is_hover_) {
            QPixmap pixmap(":/Skin/Images/more/contact_settings/userFeedback/"
                           "problemScreenshot_80_80_hover_darkBlue.png");
            /// 绘制默认矩形背景
            pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int x = (width() - pixmap.width()) / 2;
            int y = (height() - pixmap.height()) / 2;
            painter.drawPixmap(x, y, pixmap);

        } else {
            QPixmap pixmap(":/Skin/Images/more/contact_settings/userFeedback/"
                           "problemScreenshot_80_80_normal_darkBlue.png");
            /// 绘制默认矩形背景
            pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            int x = (width() - pixmap.width()) / 2;
            int y = (height() - pixmap.height()) / 2;
            painter.drawPixmap(x, y, pixmap);
        }

    } else {
        /// 文件名不为空：显示背景图
        if (!cl_feedback_pixmap_.isNull()) {
            // 绘制背景图，根据需求选择缩放模式
            // 拉伸填充（会变形）
            // painter.drawPixmap(rect(), cl_background_pixmap_);
            // 保持宽高比居中裁剪（覆盖）
            QPixmap scaled = cl_feedback_pixmap_.scaled(size(),
                                                        Qt::KeepAspectRatioByExpanding,
                                                        Qt::SmoothTransformation);
            int x = (width() - scaled.width()) / 2;
            int y = (height() - scaled.height()) / 2;

            painter.drawPixmap(x, y, scaled);

        } else {
            /// 图片加载失败时的默认显示
            painter.fillRect(rect(), QColor(200, 200, 200));

            // ///如果是选中状态或者鼠标悬停在边框内,高亮显示边框
            // if (cl_is_selected_ || cl_is_hover_) {
            //     painter.setPen(QPen(Qt::blue, 2));
            //     qDebug() << "绘制边框";
            //     painter.drawRect(rect().adjusted(1, 1, -1, -1));
            // }
            painter.drawText(rect(), Qt::AlignCenter, tr("图片加载失败"));
        }
    }
}

int FeedbackImagesLabel::cl_minrect_w()
{
    return cl_minrect_w_;
}

int FeedbackImagesLabel::cl_minrect_h()
{
    return cl_minrect_h_;
}

int FeedbackImagesLabel::cl_maxrect_w()
{
    return cl_maxrect_w_;
}

int FeedbackImagesLabel::cl_maxrect_h()
{
    return cl_maxrect_h_;
}

void FeedbackImagesLabel::mousePressEvent(QMouseEvent *event)
{
    /// 右键
    //     {
    //     if (!cl_feedback_file_name_.isEmpty() && event->button() == Qt::RightButton) {
    //         QMenu menu(this);

    //         {
    //             menu.setStyleSheet(R"(
    //     QMenu {
    //         background-color: #2D2D30;
    //         border: 1px solid #555555;
    //         border-radius: 4px;
    //         padding: 4px 0px;
    //     }
    //     QMenu::item {
    //         background-color: transparent;
    //         color: #CCCCCC;
    //         padding: 6px 24px;
    //         font-size: 13px;
    //     }
    //     QMenu::item:selected {
    //         background-color: #0078D7;
    //         color: #FFFFFF;
    //     }
    //     QMenu::item:disabled {
    //         color: #666666;
    //     }
    //     QMenu::separator {
    //         height: 1px;
    //         background-color: #555555;
    //         margin: 4px 8px;
    //     }
    // )");
    //         }

    //         QAction *changeAction = menu.addAction(tr("更换反馈图片"));
    //         QAction *delAction = menu.addAction(tr("删除反馈图片"));
    //         QAction *selectedAction = menu.exec(event->globalPos());
    //         if (selectedAction == changeAction) {
    //             // 更换反馈图片
    //             QString fileName
    //                 = QFileDialog::getOpenFileName(this,
    //                                                tr("选择反馈图片"),
    //                                                ".",
    //                                                tr("Images (*.png *.jpg *.jpeg *.bmp)"));
    //             if (!fileName.isEmpty() && isImageSizeValid(fileName)) {
    //                 QPixmap pix(fileName);
    //                 if (!pix.isNull()) {
    //                     cl_feedback_file_name_ = fileName;
    //                     cl_feedback_pixmap_ = pix;
    //                     update();
    //                 } else {
    //                     QMessageBox::warning(this, tr("错误"), tr("无法加载图片"));
    //                 }
    //             }
    //         } else if (selectedAction == delAction) {
    //             // 按钮：是、否
    //             QScopedPointer<QMessageBox> msgBox(createStyledMessageBox(this));
    //             msgBox->setWindowTitle(tr("删除反馈图片"));
    //             msgBox->setText(tr("是否删除选中的反馈图片？"));
    //             msgBox->setIcon(QMessageBox::Question);
    //             msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    //             switch (msgBox->exec()) {
    //             case QMessageBox::Yes:
    //                 cl_feedback_file_name_.clear();
    //                 cl_feedback_pixmap_ = QPixmap();
    //                 emit DelFeedbackImagesLabelSucceed();
    //                 update();
    //                 break;
    //             case QMessageBox::No:
    //                 // 不删除，什么都不做
    //                 break;
    //             default:
    //                 break;
    //             }
    //         }
    //     }
    //     }

    /// 左键，且未添加设置背景图片
    if (cl_feedback_file_name_.isEmpty() && event->button() == Qt::LeftButton) {
        /// 判断点击位置是否在加号区域内
        int side = 50;
        int x = (width() - side) / 2;
        int y = (height() - side) / 2;

        QRegion plusRegion(x, y, side, side, QRegion::Ellipse);
        if (plusRegion.contains(event->pos())) {
            // qDebug() << "点击加号";
            // 选择图片文件
            QString fileName
                = QFileDialog::getOpenFileName(this,
                                               tr("选择图片"),
                                               XIBERIA_X_HUB_Utils::getDefaultScreenshotPath(),
                                               tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)"));
            if (!fileName.isEmpty() && isImageSizeValid(fileName)) {
                QPixmap pixmap(fileName);
                if (!pixmap.isNull()) {
                    setCl_feedback_file_name(fileName);
                    setCl_feedback_pixmap(pixmap);
                    emit AddFeedbackImagesLabelSucceed();
                    clp_pushButotn_del_->show(); ///显示删除按钮
                    update();
                } else {
                    QMessageBox::warning(this,
                                         tr("错误"),
                                         tr("无法加载图片文件，请检查文件格式是否支持。"));
                }
            }
            event->accept();
            return;
        }
    }

    /// 左键，且已添加设置背景图片,如果点击到删除按钮，则进行删除
    // if (!cl_feedback_file_name_.isEmpty() && event->button() == Qt::LeftButton) {

    //     QRect plusRect(59, 5, 16, 16);
    //     if (plusRect.contains(event->pos())) {
    //         qDebug() << "点击删除";
    //         /// 检查是否选择了图片，如果有，进行删除
    //         if (!cl_feedback_file_name_.isEmpty()) {
    //             cl_feedback_file_name_.clear();
    //             cl_feedback_pixmap_ = QPixmap();
    //             emit DelFeedbackImagesLabelSucceed();
    //             clp_pushButotn_del_->hide();
    //             update();
    //         }
    //         return;
    //     }

    // }

    QWidget::mousePressEvent(event);
}

void FeedbackImagesLabel::enterEvent(QEvent *event)
{
    cl_is_hover_ = true;
    update();
    QLabel::enterEvent(event);
}

void FeedbackImagesLabel::leaveEvent(QEvent *event)
{
    cl_is_hover_ = false;
    update();
    QLabel::leaveEvent(event);
}

QString FeedbackImagesLabel::cl_feedback_file_name() const
{
    return cl_feedback_file_name_;
}

void FeedbackImagesLabel::setCl_feedback_file_name(const QString &newCl_feedback_file_name)
{
    cl_feedback_file_name_ = newCl_feedback_file_name;
}

QPixmap FeedbackImagesLabel::cl_feedback_pixmap() const
{
    return cl_feedback_pixmap_;
}

void FeedbackImagesLabel::setCl_feedback_pixmap(const QPixmap &newCl_feedback_pixmap)
{
    cl_feedback_pixmap_ = newCl_feedback_pixmap;
}

void FeedbackImagesLabel::resetLabel()
{
    /// 用于清空图片
    cl_feedback_file_name_.clear();
    cl_feedback_pixmap_ = QPixmap();
    clp_pushButotn_del_->hide();
    emit DelFeedbackImagesLabelSucceed();
}
