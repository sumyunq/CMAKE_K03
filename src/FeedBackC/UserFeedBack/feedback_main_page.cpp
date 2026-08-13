#include "FeedBackC/UserFeedBack/feedback_main_page.h"
#include "ui_feedback_main_page.h"

#include <QDateTime>
#include <QScrollArea>

#include "LoadLib.h"             ///< g_user_information
#include "data/api_global.h" ///< DeSheng 数据类型 + g_api_server_switch
#include "APOThread/ApoManager.h"
#include "network/http_client.h"
#include "network/request_options.h"

FeedbackMainPage::FeedbackMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FeedbackMainPage)
{
    ui->setupUi(this);

    // setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    //关闭时隐藏而不是销毁，以便复用
    setAttribute(Qt::WA_DeleteOnClose, false);

    InitUIInformation();
    InitMember();
    InitConnect();

    // setWindowFlags(Qt::FramelessWindowHint);
    // setAttribute(Qt::WA_TranslucentBackground,true);
    // setModal(true);

    ui->stackedWidget->setCurrentIndex(0);
}

FeedbackMainPage::~FeedbackMainPage()
{
    if (clp_Scrollarea_widget) {
        clp_Scrollarea_widget->setParent(nullptr);
        delete clp_Scrollarea_widget;
        clp_Scrollarea_widget = nullptr;
    }

    delete ui;
    ui = nullptr;
}

void FeedbackMainPage::LanguageSet()
{
    ui->retranslateUi(this);
    ui->label_title->setText(tr("联系我们"));
    if (clp_Scrollarea_widget) {
        clp_Scrollarea_widget->LanguageSet();
    }
}

void FeedbackMainPage::InitUIInformation()
{
    // 标题
    ui->label_title->setText(tr("联系我们"));
    ui->label_title->setStyleSheet("QLabel {"
                                   "   font-family: \" Noto Sans S Chinese \";"
                                   "   font-weight: 700;"
                                   "   font-size: 14px;"
                                   "   color: #A1A8B3;"
                                   "   background-color: transparent;"
                                   "}");
    // 主背景
    this->setStyleSheet("QWidget#widget {"
                        "background-position: center;"
                        "border-radius: 0px;"
                        "}");

    // // 水平滚动条策略
    // ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    clp_Scrollarea_widget = new FeedbackScrollareaWidget(ui->widget_detailed_info);
    ui->widget_detailed_info->layout()->addWidget(clp_Scrollarea_widget);

    // 提交按钮
    ui->pBn_submit->setObjectName("FeedbackMainPage_pBn_submit");
    ui->pBn_submit->setStyleSheet(R"(
        QPushButton#FeedbackMainPage_pBn_submit {
            color: #FFFFFF;
            border-radius: 15px;
            text-align: center;
            font-family: "Noto Sans S Chinese";
                font-weight: 500;
            font-size: 14px;
            border-image: url(:/Skin/Images/Popup/confirm-no.png);
        }
        QPushButton#FeedbackMainPage_pBn_submit:hover {
            border-image: url(:/Skin/Images/Popup/confirm-ho.png);
        }
    )");

    // ui->pBn_close
}

void FeedbackMainPage::InitMember() {}

void FeedbackMainPage::InitConnect()
{
    QObject::connect(this, &FeedbackMainPage::success, [](const QString &response) {
        qDebug() << "操作成功！";
    });

    QObject::connect(this, &FeedbackMainPage::error, [](const QString &errorMsg) {
        qDebug() << "操作失败:" << errorMsg;
    });

    // 点击提交用户反馈按钮
    QObject::connect(ui->pBn_submit, &QPushButton::clicked, this, [this]() {
        qDebug() << "点击提交用户反馈按钮: ";
        //点击提交反馈后，清空一下反馈信息数组
        resetFileUploadsResponseList();

        //做必填字段校验
        bool ret = clp_Scrollarea_widget->requiredFieldVerify();
        if (!ret) {
            // QMessageBox::warning(this, tr("用户反馈"), tr("必填字段未填写!!!"));
            return;
        }

        // ui->widget->setGraphicsEffect(shadow);   ///
        emit FeedBackSubmitting();
        if (clp_Scrollarea_widget->getFeedBackImagesSize() == 0) {
            onAllImagesUploaded();
            return;
        }

        // 发送文件上传请求
        for (int i = 0; i < clp_Scrollarea_widget->getFeedBackImagesSize(); ++i) {
            QString filePath = clp_Scrollarea_widget->getFeedBackImagesName(i);
            QFile *file = new QFile(filePath);
            if (!file->open(QIODevice::ReadOnly)) {
                emit error(tr("文件打开失败"));
                delete file;
                return;
            }

            QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
            QHttpPart filePart;

            // 上传文件名：时间戳_用户ID.扩展名
            QString t_timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
            QString t_user_id = g_user_information.network.id;
            QString t_extension = QFileInfo(filePath).suffix();
            QString t_upload_name = QString("%1_%2.%3").arg(t_timestamp, t_user_id, t_extension);

            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QVariant("form-data; name=\"file\"; filename=\""
                                        + t_upload_name + "\""));
            filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                               QVariant("image/png")); // 根据实际文件类型修改

            filePart.setBodyDevice(file);
            file->setParent(multiPart);
            multiPart->append(filePart);

            QNetworkReply *reply = HttpClient::instance().upload(
                "/user/uploads", multiPart, RequestOptions{}.withTag("user"));
            multiPart->setParent(reply); // 防止内存泄露

            qDebug() << multiPart->boundary();
            // 对应的
            connect(reply, &QNetworkReply::finished, this, [=]() {


                if (reply->error() == QNetworkReply::NoError) {
                    //测试 回显信息解析
                    // 处理用户反馈 回显消息
                    // 读取响应数据
                    QByteArray responseData = reply->readAll();
                    qDebug() << "回显原始数据:" << QString::fromUtf8(responseData);

                    // 解析 JSON
                    QJsonParseError parseError;
                    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
                    if (parseError.error != QJsonParseError::NoError) {
                        QString errorMsg = tr("JSON解析错误: ") + parseError.errorString();
                        emit error(tr("JSON解析错误: %1").arg(errorMsg));

                        // 其他处理
                        reply->deleteLater();
                        return;
                    }

                    // 检查业务状态码
                    QJsonObject rootObj = jsonDoc.object();
                    QString code = rootObj["code"].toString();
                    if (code != "success") {
                        QString message = rootObj["message"].toString();
                        QString errorMsg = QString("API error: code=%1, message=%2")
                                               .arg(code, message);
                        emit ApoManager::instance()->requestlogWithTime(
                            QString("上传图片： %1").arg(errorMsg));
                        reply->deleteLater();

                        emit FeedBackSubmitFail();

                        return;
                    }

                    DeSheng::FileUploadsResponse ret_info; // 回显信息
                    if (DeSheng::ProcessFileUploadsResult(ret_info, jsonDoc)) {
                        addFileUploadsResponseToList(ret_info);
                        qDebug() << "对比： " << getFileUploadsResponseListSize()
                                 << " == " << clp_Scrollarea_widget->getFeedBackImagesSize();
                        if (getFileUploadsResponseListSize()
                            == clp_Scrollarea_widget->getFeedBackImagesSize()) {
                            //发送反馈请求
                            qDebug() << "发送反馈请求: ";
                            onAllImagesUploaded();
                        }
                        // emit success(QString("回显JSON解析成功:   url:%1").arg(ret_info.data.url));
                    } else {
                        //请求回显消息异常处理
                        emit error(tr("回显JSON数据 解析失败 code: %1\nmessage:%2")
                                       .arg(ret_info.code)
                                       .arg(ret_info.message));
                    };

                } else {
                    emit FeedBackSubmitFail();
                    emit error(reply->errorString());
                }
                reply->deleteLater();
            });
        }
    });

}

void FeedbackMainPage::sendFeedback(const DeSheng::UserFeedBacksRequest &req)
{
    QUrlQuery query;
    QString errorMsg;
    if (!DeSheng::buildFeedbackQuery(req, query, errorMsg)) {
        emit error(errorMsg);
        return;
    }

    QByteArray t_body = QJsonDocument(DeSheng::userFeedbackRequestToJson(req)).toJson();
    QNetworkReply *reply = HttpClient::instance().post(
        cl_feedback_path_, RequestOptions{}.withBody(t_body).withTag("feedback"));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit error(tr("网络错误: %1").arg(reply->errorString()));
            emit FeedBackSubmitFail();
            return;
        }
        QJsonDocument t_doc = QJsonDocument::fromJson(reply->readAll());
        DeSheng::UserFeedbackResponse t_ret;
        if (DeSheng::ProcessUserFeedbackResult(t_ret, t_doc)) {
            emit FeedBackSubmitSucceed();
            emit success(tr("工号:%1").arg(t_ret.data.ticket_no));
        } else {
            emit FeedBackSubmitFail();
            emit error(QString("code: %1, message: %2")
                           .arg(t_ret.code, t_ret.message));
        }
    });
}

void FeedbackMainPage::setCl_feedback_path(const QString &path)
{
    cl_feedback_path_ = path;
}

QString FeedbackMainPage::cl_access_token() const
{
    return cl_access_token_;
}

void FeedbackMainPage::setCl_access_token(const QString &newCl_access_token)
{
    cl_access_token_ = newCl_access_token;
}

void FeedbackMainPage::addFileUploadsResponseToList(DeSheng::FileUploadsResponse res)
{
    QMutexLocker locker(&cl_upload_images_mutex_);
    cl_uploadImagesResponse_.append(res);
}

void FeedbackMainPage::resetFileUploadsResponseList()
{
    QMutexLocker locker(&cl_upload_images_mutex_);
    cl_uploadImagesResponse_.clear();
}

int FeedbackMainPage::getFileUploadsResponseListSize()
{
    QMutexLocker locker(&cl_upload_images_mutex_);
    return cl_uploadImagesResponse_.size();
}

void FeedbackMainPage::onAllImagesUploaded()
{
    // 反馈构建请求信息
    {
        //构建请求结构体
        DeSheng::UserFeedBacksRequest req = this->clp_Scrollarea_widget->getReq();
        /* /// 添加其他信息(缺少如下,有些是必填的)
        {
            // req.device_id = "string";   ///设备ID
            // req.drive_id = "string";    ///驱动ID
            // req.firmware_id = "string"; ///固件ID
            // req.device_name = "雷柏VT950";    ///设备名称
            // req.type = "bug";   ///反馈类型
            // req.config_url = "https://hubsystest.xiberia.net/api/v1/uploads/1773807616410153674/3334.json"; ///配置文件
        }
        // 测试用数据
        {
            req.device_id = DevId;
            req.drive_id = DriId;
            req.firmware_id = FWId;
            req.drive_version = SVer;
            req.firmware_version = QString(EarVer).split('_').last().toInt();;
            req.receiver_version = QString(DongleVer).split('_').last().toInt();;
            req.device_name = SelDev_DeviceName;
            req.device_type = DevType;
            req.title = "反馈标题";
            req.description = "详细描述";
            req.type = "bug";
            req.contact_info = "135xxxx1234";
            req.os_info = "Windows 11";
            req.config_url
                = "https://hubsystest.xiberia.net/api/v1/uploads/1773807616410153674/3334.json";

            // 添加反馈信息中的url路径
            for (auto t : cl_uploadImagesResponse_) {
                req.images.append(t.data.url);
            }

        }*/

        // 添加反馈信息中的url路径
        for (auto t : cl_uploadImagesResponse_) {
            req.images.append(t.data.url);
        }

        //设置Url
        setCl_feedback_path("/feedbacks");

        // 发送请求
        sendFeedback(req);
    }
}

void FeedbackMainPage::resetForNewFeedback()
{
    resetFileUploadsResponseList();
    ui->stackedWidget->setCurrentIndex(0);
    ui->pBn_submit->setEnabled(true);
    if (clp_Scrollarea_widget)
        clp_Scrollarea_widget->resetForNewFeedback();
}

void FeedbackMainPage::showOutInfo()
{
    resetForNewFeedback();
}

void FeedbackMainPage::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}
