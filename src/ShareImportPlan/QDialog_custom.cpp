#include "ShareImportPlan/QDialog_custom.h"
#include "ui_QDialog_custom.h"
#include "LoadLib.h"
#include "data/api_global.h"
#include "network/http_client.h"
#include "network/request_options.h"

QDialogCustom::QDialogCustom(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QDialogCustom)
{
    ui->setupUi(this);
    InitUIInformation();
    InitMember();
    InitConnect();
}

QDialogCustom::~QDialogCustom()
{
    delete ui;
}

void QDialogCustom::setTitle(QString title)
{
    cl_title_->setText(title);
}

void QDialogCustom::set_btn_ok_text(QString text)
{
    cl_cancelBtn_->setText(text);
}

void QDialogCustom::set_btn_cancel_text(QString text)
{
    cl_okBtn_->setText(text);
}

void QDialogCustom::InitUIInformation()
{
    cl_title_ = new QLabel(this);
    cl_lineEdit_ = new QLineEdit(this);
    cl_error_icon_ = new QLabel(this);
    cl_error_message_ = new QLabel(this);
    cl_okBtn_ = new QPushButton(this);
    cl_cancelBtn_ = new QPushButton(this);
    cl_closeBtn_ = new QPushButton(this);

    setMinimumSize(351, 241);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    setStyleSheet(R"(
    QDialog {
        background: #000000;
        border: 1px solid #555;
        border-radius: 8px;
    }
)");

    cl_title_->move(cl_title_point_); ///< 对话标题
    cl_title_->setMinimumSize(cl_title_min_size_);
    cl_title_->setText(tr("导入方案"));
    cl_title_->setStyleSheet(R"(
  QLabel {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 16px;
        text-align: center;
        color: #A1A8B3;
        border: none;
    }
)");

    cl_lineEdit_->move(cl_lineEdit_point_); ///< 可编辑行
    cl_lineEdit_->setMinimumSize(cl_lineEdit_min_size_);
    cl_lineEdit_->setPlaceholderText(tr("请输入分享码"));
    cl_lineEdit_->installEventFilter(this); // 安装过滤器
    cl_lineEdit_->setStyleSheet(R"(

  QLineEdit {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        background: rgba(81, 96, 122, 0.2);
        color: #616975;
        border-radius: 4px;
        font-size: 14px;
        padding:6px;

    }
)");

    cl_error_icon_->move(cl_error_icon_point_); ///< 错误图标
    cl_error_icon_->setFixedSize(cl_error_icon_min_size_);
    // cl_error_icon_->setText(tr("X"));   // 仅图标
    cl_error_icon_->setStyleSheet(R"(

  QLabel {
        image: url(:/Skin/Images/home/error.png);
    }

)");
    cl_error_icon_->hide(); //默认隐藏

    cl_error_message_->move(cl_error_message_point_); // 错误信息
    cl_error_message_->setMinimumSize(cl_error_message_min_size_);
    cl_error_message_->setText(tr("分享码错误"));
    cl_error_message_->setStyleSheet(R"(

  QLabel {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 10px;
        text-align: center;
        color: #D44040;
        border: none;
    }

)");
    cl_error_message_->hide(); //默认隐藏

    cl_okBtn_->move(cl_okBtn_point_); ///< 确认按钮
    cl_okBtn_->setMinimumSize(cl_okBtn_min_size_);
    cl_okBtn_->setText(tr("导入方案"));
    cl_okBtn_->setCursor(QCursor(Qt::PointingHandCursor)); //鼠标变成手型
    cl_okBtn_->setStyleSheet(R"(
        QPushButton
        {
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 14px;
            color: #FFFFFF;

            background:transparent;
            border-image: url(:/Skin/Images/Popup/confirm-no.png);
        }
        QPushButton:hover
        {
            border-image: url(:/Skin/Images/Popup/confirm-ho.png);
        }
)");

    cl_cancelBtn_->move(cl_cancelBtn_point_); ///< 取消按钮
    cl_cancelBtn_->setMinimumSize(cl_cancelBtn_min_size_);
    cl_cancelBtn_->setText(tr("取消"));
    cl_cancelBtn_->setCursor(QCursor(Qt::PointingHandCursor)); //鼠标变成手型
    cl_cancelBtn_->setStyleSheet(R"(
        QPushButton
        {
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 14px;
            color: #0091DA;

            background:transparent;
            border-image: url(:/Skin/Images/Popup/cancel-no.png);
        }
        QPushButton:hover
        {
            border-image: url(:/Skin/Images/Popup/cancel-ho.png);
        }
)");

    cl_closeBtn_->move(cl_closeBtn_point_); ///< 右上角关闭按钮
    cl_closeBtn_->setFixedSize(cl_closeBtn_min_size_);
    cl_closeBtn_->setCursor(QCursor(Qt::PointingHandCursor)); //鼠标变成手型
    cl_closeBtn_->setStyleSheet(R"(
        QPushButton
        {
            border-radius:0px;
            border-image: url(:/Skin/Images/Popup/close-no.png);
            background:transparent;
        }
        QPushButton:hover
        {
            border-image: url(:/Skin/Images/Popup/close-ho.png);
        }
)");
}

void QDialogCustom::InitMember()
{
    saveDir = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath()
              + "/XIBERIA X HUB/ProgramData/"; /// 下载的方案 临时保存路径读取完成后删除
    savePath = saveDir + "/download_tmp.ini";
}

void QDialogCustom::InitConnect()
{
    // 点击确定按键
    QObject::connect(cl_okBtn_, &QPushButton::clicked, this, [=]() {
        // 校验分享码: 支持原始码 或 名称+机型+场景+ys/sq码 格式
        QString t_input = cl_lineEdit_->text().trimmed();
        QString shareCode;

        if (t_input.isEmpty()) {
            cl_error_message_->setText(tr("分享码不能为空"));
            cl_error_message_->show();
            cl_error_icon_->show();
            return;
        }

        // 提取前缀：ys=预设库(schemes), sq=社区(userConfig)
        cl_share_prefix_.clear();
        if (t_input.startsWith("ys") || t_input.startsWith("sq")) {
            cl_share_prefix_ = t_input.left(2);
            shareCode = t_input.mid(2); ///< 去掉前缀，用纯码
        } else {
            int t_pos = t_input.length();
            while ((t_pos = t_input.lastIndexOf('+', t_pos - 1)) >= 0) {
                QString t_suffix = t_input.mid(t_pos + 1);
                if (t_suffix.length() >= 3
                    && (t_suffix.startsWith("ys") || t_suffix.startsWith("sq"))) {
                    cl_share_prefix_ = t_suffix.left(2);
                    shareCode = t_suffix.mid(2);
                    break;
                }
            }
            if (cl_share_prefix_.isEmpty())
                shareCode = t_input;
        }

        QRegularExpression rx("^[A-Za-z0-9+/=._-]+$"); ///< sq 为 Base64 RawURL，含 _ -
        if (!rx.match(shareCode).hasMatch()) {
            cl_error_message_->setText(tr("分享码无效"));
            cl_error_message_->show();
            cl_error_icon_->show();
            return;
        }


        // sq 前缀 或 无前缀但含 .(Base64) → userConfig；其余 → schemes
        bool t_is_sq = (cl_share_prefix_ == "sq")
                       || (cl_share_prefix_.isEmpty() && shareCode.contains('.'));

        QString t_path = t_is_sq
            ? QString("/user-configs/share/%1/download").arg(shareCode)
            : QString("/schemes/resolve/%1").arg(shareCode);
        QString t_tag = t_is_sq ? "userConfig" : "schemes";
        QNetworkReply *reply = HttpClient::instance().get(t_path, RequestOptions{}.withTag(t_tag));
        // 显示处理中状态
        cl_okBtn_->setEnabled(false);
        cl_okBtn_->setText(tr("处理中..."));

        connect(reply, &QNetworkReply::finished, this, [=]() {
            QByteArray t_raw = reply->readAll();
            QJsonDocument t_doc = QJsonDocument::fromJson(t_raw);
            reply->deleteLater();

            qDebug() << "响应状态:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "响应体:" << QString::fromUtf8(t_raw);

            auto t_show_error = [this](const QString &msg) {
                cl_error_message_->setText(msg);
                cl_error_message_->show();
                cl_error_icon_->show();
                cl_okBtn_->setEnabled(true);
                cl_okBtn_->setText(tr("确定"));
            };

            QString t_download_url;

            if (t_is_sq) {
                // 社区: 手动取 code 校验（失败时 data 为 null）
                QString t_code = t_doc.object().value("code").toString();
                if (t_code == "unauthorized")       t_show_error(tr("请先登录"));
                else if (t_code == "share_code_format_error") t_show_error(tr("分享码格式错误"));
                else if (t_code == "share_code_decode_error") t_show_error(tr("分享码解码失败"));
                else if (t_code == "share_code_invalid")      t_show_error(tr("分享码无效或已篡改"));
                else if (t_code == "share_code_not_found")    t_show_error(tr("分享码对应的配置不存在"));
                else if (t_code == "request_rate_limited")    t_show_error(tr("操作过于频繁，请稍后重试"));
                else if (t_code != "success")                 t_show_error(t_doc.object().value("message").toString());
                if (t_code != "success") return;

                DeSheng::DownloadConfigByShareCodeResponse t_sq_resp;
                DeSheng::ProcessDownloadConfigByShareCodeResult(t_sq_resp, t_doc);
                t_download_url = t_sq_resp.data.config_url;
            } else {
                // 预设库
                if (!ProcessResolveShareCodeResult(resp, t_doc)) {
                    t_show_error(tr("解析失败")); return;
                }
                if (resp.code == "unauthorized")        t_show_error(tr("请先登录"));
                else if (resp.code == "invalid_param")  t_show_error(resp.message);
                else if (resp.code != "success" || resp.data.status != "active")
                                                         t_show_error(tr("该分享码对应的配置已失效"));
                if (resp.code != "success" || resp.data.status != "active") return;
                t_download_url = resp.data.url;
            }

            if (t_download_url.isEmpty()) {
                t_show_error(tr("配置文件下载地址为空"));
                return;
            }

            // 下载配置文件
            cl_okBtn_->setText(tr("下载中..."));
            QNetworkReply *t_dl_reply = HttpClient::instance().manager()->get(
                QNetworkRequest(QUrl(t_download_url)));
            connect(t_dl_reply, &QNetworkReply::finished, this, [=]() {
                t_dl_reply->deleteLater();

                if (t_dl_reply->error() != QNetworkReply::NoError) {
                    t_show_error(tr("下载失败，请检查网络"));
                    return;
                }

                QDir().mkpath(saveDir);
                QFile t_file(savePath);
                if (!t_file.open(QIODevice::WriteOnly)) {
                    t_show_error(tr("方案导入失败"));
                    return;
                }
                t_file.write(t_dl_reply->readAll());
                t_file.close();

                // 校验文件格式
                QSettings t_settings(savePath, QSettings::IniFormat);
                if (!t_settings.contains("LoadPlan/Name")) {
                    t_show_error(tr("文件格式不匹配，无法导入"));
                    return;
                }

                cl_okBtn_->setEnabled(true);
                cl_okBtn_->setText(tr("确定"));
                ret_info = QString(tr("方案导入成功"));
                accept();
            });
        });
    });

    // 点击取消按键
    QObject::connect(cl_cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);

    // 点击关闭按键
    QObject::connect(cl_closeBtn_, &QPushButton::clicked, this, &QDialog::reject);

    // 可编辑行字段校验
    QObject::connect(cl_lineEdit_, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString filtered;
        for (const QChar &ch : text) {
            if ((ch.isLetterOrNumber() || ch == '.' || ch == '+' || ch == '/' || ch == '='
                 || ch == '_' || ch == '-')
                && filtered.size() < 64) {
                filtered.append(ch);
            }
        }
        if (filtered != text) {
            cl_lineEdit_->blockSignals(true);
            cl_lineEdit_->setText(filtered);
            cl_lineEdit_->setCursorPosition(filtered.size());
            cl_lineEdit_->blockSignals(false);
        }
    });
}

void QDialogCustom::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(13, 15, 20));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 16, 16);

    QPainterPath path;
    path.addRoundedRect(rect(), 16, 16);
    setMask(path.toFillPolygon().toPolygon());
}

void QDialogCustom::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    QPainterPath path;
    path.addRoundedRect(rect(), 16, 16);
    setMask(path.toFillPolygon().toPolygon());
}

bool QDialogCustom::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == cl_lineEdit_ && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Paste)) {
            QClipboard *clipboard = QApplication::clipboard();
            QString text = clipboard->text().trimmed();
            QString shareCode;

            cl_share_prefix_.clear();
            if (text.startsWith("ys") || text.startsWith("sq")) {
                cl_share_prefix_ = text.left(2);
                shareCode = text;
            } else {
                int t_pos = text.length();
                while ((t_pos = text.lastIndexOf('+', t_pos - 1)) >= 0) {
                    QString t_suffix = text.mid(t_pos + 1);
                    if (t_suffix.length() >= 3
                        && (t_suffix.startsWith("ys") || t_suffix.startsWith("sq"))) {
                        cl_share_prefix_ = t_suffix.left(2);
                        shareCode = t_suffix.mid(2);
                        break;
                    }
                }
                if (shareCode.isEmpty())
                    shareCode = text.contains("-") ? text.section('-', -1) : text;
            }

            if (!shareCode.isEmpty())
                cl_lineEdit_->setText(shareCode.trimmed());
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

