#include "FeedBackC/UserFeedBack/feedback_scrollarea_widget.h"
#include "LoadLib.h"
#include "ui_feedback_scrollarea_widget.h"

#include <QApplication>
#include <QScrollBar>

namespace {
constexpr int kDescriptionMaxLength = 150;
}

FeedbackScrollareaWidget::FeedbackScrollareaWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FeedbackScrollareaWidget)
{
    ui->setupUi(this);
    InitUIInformation();
    InitMember();
    InitConnect();
}

FeedbackScrollareaWidget::~FeedbackScrollareaWidget()
{
    delete ui;
    ui = nullptr;

    if (clp_FirmwareTool_) {
        delete clp_FirmwareTool_;
        clp_FirmwareTool_ = nullptr;
    }

    if (clp_FeedbackImages_) {
        delete clp_FeedbackImages_;
        clp_FeedbackImages_ = nullptr;
    }
}

void FeedbackScrollareaWidget::InitUIInformation()
{
    // scrollArea 垂直按需滚动，水平不滚动
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->installEventFilter(this);

    // scrollAreaWidgetContents 背景
    ui->scrollAreaWidgetContents->setObjectName("FeedbackScrollareaWidget_contents");
    ui->scrollAreaWidgetContents->setStyleSheet(R"(
        QWidget#FeedbackScrollareaWidget_contents {
            background-color: transparent;
            border:none;
        }

        QLabel{
           font-family: "Noto Sans S Chinese";
           font-weight: 500;
           font-size: 12px;
           color: #A1A8B3;
        }
        QLineEdit{
           padding-left:8px;
           font-size: 12px;
           font-family: "Noto Sans S Chinese";
           font-weight: 500;
           border-radius: 4px;
           background-color: rgba(0, 0, 0, 0.2);
           selection-background-color: #0078d4;
           selection-color: white;
           color: #616871;
        }
        QLineEdit:focus {
           border: 2px solid #0078d4;
        }
QPlainTextEdit {
   padding: 7px 8px;
   font-family: "Noto Sans S Chinese";
   font-weight: 500;
   font-size: 12px;
   border-radius: 4px;
   background-color: rgba(0, 0, 0, 0.2);
   color: #616871;

   border: 2px solid transparent;

   outline: none;

   selection-background-color: #0078d4;
   selection-color: #ffffff;
}

QPlainTextEdit:focus {
   border: none;
}
QPlainTextEdit QScrollBar:vertical {
   width: 7px;
   background: transparent;
   border-radius: 3px;
}
QPlainTextEdit QScrollBar::handle:vertical {
   background: rgba(0, 0, 0, 0.2);
   border-radius: 3px;
   min-height: 42px;
}
QPlainTextEdit QScrollBar::add-line:vertical,
QPlainTextEdit QScrollBar::sub-line:vertical {
   height: 0px;
}
QPlainTextEdit QScrollBar::add-page:vertical,
QPlainTextEdit QScrollBar::sub-page:vertical {
   background: transparent;
}

    )");


    QPixmap pixmap(":/Skin/Images/userFeedback/must_1x.png");
    pixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    // * 图标
    ui->label_must_1->setPixmap(pixmap);
    ui->label_must_2->setPixmap(pixmap);
    ui->label_must_3->setPixmap(pixmap);
    ui->label_must_4->setPixmap(pixmap);
    ui->label_must_5->setPixmap(pixmap);
    ui->label_must_6->setPixmap(pixmap);
    ui->label_must_9->setPixmap(pixmap);
    ui->label_must_10->setPixmap(pixmap);
    // ui->label_must_10->setPixmap(pixmap);

    // 标题
    ui->label_type->setText(tr("产品类型"));
    ui->label_class->setText(tr("产品型号"));
    ui->label_title->setText(tr("标题"));
    ui->label_drive_version->setText(tr("驱动版本"));
    ui->label_firmware_version->setText(tr("固件版本"));
    ui->label_receiver_version->setText(tr("接收器版本"));
    ui->label_contact_info->setText(tr("联系方式"));
    // ui->label_os_info->setText(tr("操作系统类型"));
    ui->label_description->setText(tr("问题描述"));
    ui->label_images->setText(tr("问题截图"));

    // 提示 默认不显示
    ui->label_title_hint->hide();
    ui->label_contact_info_hint->hide();
    // ui->label_os_info_hint->hide();
    ui->label_description_hint->hide();

    // 可编辑行内容
    ui->lineEdit_type->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_class->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_title->setPlaceholderText(tr("请输入标题"));
    ui->lineEdit_drive_version->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_firmware_version->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_receiver_version->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_contact_info->setPlaceholderText(tr("请输入您的联系方式"));
    // ui->lineEdit_os_info->setPlaceholderText(tr("请输入您的操作系统类型"));
    ui->plainTextEdit_description->setPlaceholderText(tr("请描述你遇到的问题"));

    // 字符限制
    // ui->lineEdit_type->setMaxLength(50);
    // ui->lineEdit_class->setMaxLength();
    ui->lineEdit_title->setMaxLength(20);
    // ui->lineEdit_drive_version->setMaxLength();
    // ui->lineEdit_firmware_version->setMaxLength();
    // ui->lineEdit_receiver_version->setMaxLength();
    // ui->lineEdit_contact_info->setMaxLength();
    // ui->lineEdit_os_info->setMaxLength();

    // 安装事件过滤器
    ui->plainTextEdit_description->installEventFilter(this);

    ui->lineEdit_type->setReadOnly(true); // 只读
    ui->lineEdit_class->setReadOnly(true);
    ui->lineEdit_drive_version->setReadOnly(true);
    ui->lineEdit_firmware_version->setReadOnly(true);
    ui->lineEdit_receiver_version->setReadOnly(true);

}

void FeedbackScrollareaWidget::InitMember()
{
    clp_FeedbackImages_ = new FeedbackImagesWidget(ui->widget_for_imagesLabel); // 反馈图片部件
    ui->widget_for_imagesLabel->layout()->addWidget(clp_FeedbackImages_);

    clp_FirmwareTool_ = new FirmwareTool(this);
}

void FeedbackScrollareaWidget::LanguageSet()
{
    ui->retranslateUi(this);
    // 代码内一次性 setText 的字段标签（.ui 之外手动设置的）
    ui->label_type->setText(tr("产品类型"));
    ui->label_class->setText(tr("产品型号"));
    ui->label_title->setText(tr("标题"));
    ui->label_drive_version->setText(tr("驱动版本"));
    ui->label_firmware_version->setText(tr("固件版本"));
    ui->label_receiver_version->setText(tr("接收器版本"));
    // 代码内一次性 setPlaceholderText 的输入框提示（随语言切换刷新）
    ui->lineEdit_type->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_class->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_title->setPlaceholderText(tr("请输入标题"));
    ui->lineEdit_drive_version->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_firmware_version->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_receiver_version->setPlaceholderText(tr("自动获取"));
    ui->lineEdit_contact_info->setPlaceholderText(tr("请输入您的联系方式"));
    ui->plainTextEdit_description->setPlaceholderText(tr("请描述你遇到的问题"));

    if (ui->label_description_hint->isVisible()) {
        const QString description = ui->plainTextEdit_description->toPlainText();
        ui->label_description_hint->setText(description.trimmed().isEmpty()
                                            ? tr("请填写问题描述")
                                            : tr("最大150字"));
    }
}

void FeedbackScrollareaWidget::InitConnect()
{
    QObject::connect(ui->plainTextEdit_description, &QPlainTextEdit::textChanged, this, [this]() {
        const QString currentText = ui->plainTextEdit_description->toPlainText();

        if (currentText.length() > kDescriptionMaxLength) {
            ui->plainTextEdit_description->blockSignals(true);
            ui->plainTextEdit_description->setPlainText(currentText.left(kDescriptionMaxLength));
            ui->plainTextEdit_description->blockSignals(false);

            QTextCursor cursor = ui->plainTextEdit_description->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->plainTextEdit_description->setTextCursor(cursor);

            ui->label_description_hint->setText(tr("最大150字"));
            ui->label_description_hint->show();
        } else if (currentText.trimmed().isEmpty() && ui->label_description_hint->isVisible()) {
            ui->label_description_hint->setText(tr("请填写问题描述"));
        } else if (!currentText.trimmed().isEmpty()) {
            ui->label_description_hint->hide();
            setQPlainTextEditError(ui->plainTextEdit_description, false);
        }
    });
}

bool FeedbackScrollareaWidget::eventFilter(QObject *watched, QEvent *event)
{
    // scrollArea resize → 同步内容宽度
    if (watched == ui->scrollArea && event->type() == QEvent::Resize) {
        int t_w = ui->scrollArea->viewport()->width();
        if (t_w > 0) {
            int t_h = ui->scrollAreaWidgetContents->layout()
                          ? ui->scrollAreaWidgetContents->layout()->sizeHint().height()
                          : ui->scrollAreaWidgetContents->height();
            ui->scrollAreaWidgetContents->resize(t_w, t_h);
        }
    }

    if (watched == ui->plainTextEdit_description) {
        // 预留 plainTextEdit 特殊事件处理
    }
    return QWidget::eventFilter(watched, event);
}

int FeedbackScrollareaWidget::getFeedBackImagesSize()
{
    return clp_FeedbackImages_->getFeedbackImages_files().size();
}

QString FeedbackScrollareaWidget::getFeedBackImagesName(int index)
{
    return clp_FeedbackImages_->getFeedbackImages_files().at(index);
}

DeSheng::UserFeedBacksRequest FeedbackScrollareaWidget::getReq() const
{
    // 构建请求结构体
    DeSheng::UserFeedBacksRequest t_req;

    t_req.device_id = DevId;
    t_req.drive_id = DriId;
    t_req.firmware_id = FWId;
    t_req.drive_version = SoftWareVer;
    t_req.firmware_version = ui->lineEdit_firmware_version->text();
    t_req.receiver_version = ui->lineEdit_receiver_version->text();
    t_req.device_name = SelDev_DeviceName;
    t_req.device_type = DevType;
    t_req.title = ui->lineEdit_title->text();
    t_req.description = ui->plainTextEdit_description->toPlainText(); // 纯文本
    t_req.type = "bug";
    t_req.contact_info = ui->lineEdit_contact_info->text();
    t_req.os_info = QSysInfo::prettyProductName(); // 操作系统信息
    t_req.config_url = "https://hubsystest.xiberia.net/uploads/1777020291196320592/setting.ini";

    return t_req;
}

bool FeedbackScrollareaWidget::requiredFieldVerify()
{
    ui->label_title_hint->hide();
    ui->label_contact_info_hint->hide();
    // ui->label_os_info_hint->hide();
    ui->label_description_hint->hide();

    // 字段校验不为空
    bool isValid = true;

    // 校验标题（必填）
    if (ui->lineEdit_title->text().trimmed().isEmpty()) {
        setLineEditError(ui->lineEdit_title, true);
        ui->lineEdit_title->setFocus(); // 聚焦
        ui->label_title_hint->show();

        // 先处理事件队列确保布局结算，再滚动到目标控件
        QApplication::processEvents();
        ui->scrollArea->ensureWidgetVisible(ui->lineEdit_title);

        isValid = false;
        return isValid;
    } else {
        setLineEditError(ui->lineEdit_title, false);
        ui->label_title_hint->hide();
    }

    // // 校验操作系统信息（必填）
    // if (ui->lineEdit_os_info->text().trimmed().isEmpty()) {
    //     setLineEditError(ui->lineEdit_os_info, true);
    //     ui->lineEdit_os_info->setFocus();
    //     ui->label_os_info_hint->show();

    //     // 确保 lineEdit_os_info 在滚动区域内可见
    //     ui->scrollArea->ensureWidgetVisible(ui->lineEdit_os_info);

    //     isValid = false;
    //     return isValid;
    // } else {
    //     setLineEditError(ui->lineEdit_os_info, false);
    //     ui->label_os_info_hint->hide();
    // }

    // 校验描述（必填）
    if (ui->plainTextEdit_description->toPlainText().trimmed().isEmpty()) {
        setQPlainTextEditError(ui->plainTextEdit_description, true);
        ui->plainTextEdit_description->setFocus();
        ui->label_description_hint->setText(tr("请填写问题描述"));
        ui->label_description_hint->show();

        // 先处理事件队列确保布局结算，再滚动到目标控件
        QApplication::processEvents();
        ui->scrollArea->ensureWidgetVisible(ui->plainTextEdit_description);

        isValid = false;
        return isValid;
    } else {
        setQPlainTextEditError(ui->plainTextEdit_description, false);
        ui->label_description_hint->hide();
    }

    // 校验联系方式（必填）
    if (ui->lineEdit_contact_info->text().trimmed().isEmpty()) {
        setLineEditError(ui->lineEdit_contact_info, true);
        ui->lineEdit_contact_info->setFocus();
        ui->label_contact_info_hint->show();

        // 先处理事件队列确保布局结算，再滚动到目标控件
        QApplication::processEvents();
        ui->scrollArea->ensureWidgetVisible(ui->lineEdit_contact_info);

        isValid = false;
        return isValid;
    } else {
        setLineEditError(ui->lineEdit_contact_info, false);
        ui->label_contact_info_hint->hide();
    }

    return isValid;
}
void FeedbackScrollareaWidget::ShowLineInfo()
{
    ui->lineEdit_type->setText(DevType);
    ui->lineEdit_class->setText(SelDev_DeviceName);
    ui->lineEdit_drive_version->setText(SoftWareVer);

    // 耳机版本
    QString firmware = QString(EarVer).split('_').last();
    ui->lineEdit_firmware_version->setText(firmware.isEmpty() ? tr("无") : firmware);

    // 接收器版本
    QString receiver = QString(DongleVer).split('_').last();
    ui->lineEdit_receiver_version->setText(receiver.isEmpty() ? tr("无") : receiver);
}
void FeedbackScrollareaWidget::updateFWInfo()
{
    if (SelDev_DeviceName.contains("T10", Qt::CaseInsensitive)) {
        if (!SelDev_DeviceName.contains("Wireless", Qt::CaseInsensitive)) {
            // 获取一下固件信息并填入对应
            QString fwVersion;
            QString libVersion;
            bool ret = clp_FirmwareTool_
                           ->GetDeviceFirmwareVersion(SelDev_VID,
                                                      SelDev_PID,
                                                      fwVersion,
                                                      libVersion); // 这个固件信息要更新pid、vipd
            // 获取固件信息成功
            if (ret) {
                ui->lineEdit_firmware_version->setText(fwVersion);
                ui->lineEdit_receiver_version->setText(fwVersion);
            }
        }
    } else {
        ui->lineEdit_firmware_version->setText(EarVer);
        ui->lineEdit_receiver_version->setText(DongleVer);
    }
}

void FeedbackScrollareaWidget::clearLineOldInfo()
{
    // 内容清空
    // ui->lineEdit_type->setPlaceholderText(tr(" 自动获取"));
    // ui->lineEdit_class->setPlaceholderText(tr(" 自动获取"));
    ui->lineEdit_title->clear();
    // ui->lineEdit_drive_version->setPlaceholderText(tr(" 自动获取"));
    // ui->lineEdit_firmware_version->setPlaceholderText(tr(" 自动获取"));
    // ui->lineEdit_receiver_version->setPlaceholderText(tr(" 自动获取"));
    ui->lineEdit_contact_info->clear();
    // ui->lineEdit_os_info->clear();
    ui->plainTextEdit_description->clear();

    // 清空图片
    clp_FeedbackImages_->clearOldImages();
}

void FeedbackScrollareaWidget::resetForNewFeedback()
{
    clearLineOldInfo();

    ui->label_title_hint->hide();
    ui->label_contact_info_hint->hide();
    ui->label_description_hint->hide();

    setLineEditError(ui->lineEdit_title, false);
    setLineEditError(ui->lineEdit_contact_info, false);
    setQPlainTextEditError(ui->plainTextEdit_description, false);

    ShowLineInfo();
    updateFWInfo();

    if (ui->scrollArea->verticalScrollBar())
        ui->scrollArea->verticalScrollBar()->setValue(0);
    ui->lineEdit_title->clearFocus();
    ui->lineEdit_contact_info->clearFocus();
    ui->plainTextEdit_description->clearFocus();
}

/// \brief 设置 LineEdit 错误状态
/// \param lineEdit 目标输入框
/// \param hasError 是否有错误
void FeedbackScrollareaWidget::setLineEditError(QLineEdit *lineEdit, bool hasError)
{
    if (hasError) {
        lineEdit->setStyleSheet(
            R"(
            QLineEdit{
               padding-left:8px;
               font-size: 12px;
               font-family: "Noto Sans S Chinese";
               font-weight: 500;
               border-radius: 4px;
               background-color: rgba(0, 0, 0, 0.2);
               selection-background-color: #0078d4;
               selection-color: white;
               color: #616871;
            }
            QLineEdit:focus {
               border: 2px solid #FF0000;
            }
            )");

    } else {
        // 恢复默认样式
        lineEdit->setStyleSheet(R"(
            QLineEdit{
               padding-left:8px;
               font-size: 12px;
               font-family: "Noto Sans S Chinese";
               font-weight: 500;
               border-radius: 4px;
               background-color: rgba(0, 0, 0, 0.2);
               selection-background-color: #0078d4;
               selection-color: white;
               color: #616871;
            }
            QLineEdit:focus {
               border: 2px solid #0078d4;
            }
)");
    }
}

void FeedbackScrollareaWidget::setQPlainTextEditError(QPlainTextEdit *plainTextEdit, bool hasError)
{
    if (hasError) {
        plainTextEdit->setStyleSheet(
            R"(
                QPlainTextEdit{
                   padding-left:8px;
                   padding-top:7px;
                   font-family: "Noto Sans S Chinese";
                   font-weight: 500;
                   font-size: 12px;
                   border-radius: 4px;
                   background-color: rgba(0, 0, 0, 0.2);
                   color: #616871;
                }
                QPlainTextEdit:focus {
                   border: 2px solid #FF0000;
                }
            )");

    } else {
        // 恢复默认样式
        plainTextEdit->setStyleSheet(R"(
                QPlainTextEdit{
                   padding-left:8px;
                   padding-top:7px;
                   font-family: "Noto Sans S Chinese";
                   font-weight : 500;
                   font-size: 12px;
                   border-radius: 4px;
                   background-color: rgba(0, 0, 0, 0.2);
                   color: #616871;
                }
                QPlainTextEdit:focus {
                   border: 2px solid #0078d4;
                }
)");
    }
}
