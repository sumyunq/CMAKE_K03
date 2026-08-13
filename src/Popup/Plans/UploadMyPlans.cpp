#include "Popup/Plans/UploadMyPlans.h"
#include "qscrollbar.h"
#include "ui_UploadMyPlans.h"
#include <QAction>
#include "LoadLib.h"
#include "data/api_global.h"
#include "network/http_client.h"
#include "network/request_options.h"


int radioWight = 243, radioHeight = 126;

UploadMyPlans::UploadMyPlans(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UploadMyPlans)
    , UploadPlansRequest(new DeSheng::UserConfigsCreateRequest())
    , UploadPlansResponse(new DeSheng::UserConfigsCreateResponse())
    , cl_network_manager_(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(20);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 128));    // 黑色半透明 rgba(0,0,0,0.5)
    // shadow->setColor(Qt::red);
    ui->frame->setGraphicsEffect(shadow);

    tip_HideEq = new NewCustomToolTip(this);
    tip_HideEq->setLabelStyle(0);
    tip_HideEq->AddToolTip(ui->lab_hideEqTip,tr("勾选后，上传该方案时将均衡器（EQ）配置隐藏不可见。"),Qt::AlignHCenter);

    //不添加多个机型
    ui->widget_Dev2->hide();
    ui->widget_Dev3->hide();
    ui->pBt_addDev->hide();

    //搜索框创建图标
    QAction *searchAction = new QAction(ui->lEdit_search);
    searchAction->setIcon(QIcon(":/Skin/Images/search/icon.png")); // 资源文件中的图标
    // 添加到 LineEdit 左侧
    ui->lEdit_search->addAction(searchAction, QLineEdit::LeadingPosition);

    //设置下拉框ui->cBox_Scene样式
    {
        M_SetCBoxShadow(ui->cBox_Scene);
        ui->cBox_Scene->setPopupOffsetXY(-8,2);
        QString styleSheet = (R"(QComboBox{
                                        font-family: "Noto Sans S Chinese";
                font-weight: 500;
                                        font-size: 14px;
                                        border-radius: 4px;
                                        combobox-popup: 0;
                                        background-color:rgba(81, 96, 122, 0.2);
                                        padding-left: 10px;
                                        color: #616975;
                            }
                            QComboBox::drop-down{
                                        border-image: url(:/Skin/Images/cBox/selfDroptriangle_no.png);
                                        margin-top:0px;
                                        subcontrol-origin: padding;
                                        subcontrol-position: center right;
                                        margin-right:10px;height:14px;width:11px;
                            }
                            QComboBox::drop-down:checked{
                                        border-image: url(:/Skin/Images/cBox/selfDroptriangle_se.png);
                                        margin-top:0px;
                                        margin-right:10px;
                                        subcontrol-origin: padding;
                                        subcontrol-position: center right;
                                        height:14px;
                                        width:11px;
                            }
                            )"
                              );
        ui->cBox_Scene->setStyleSheet(styleSheet);
        //创建自定义的 NoSelectListView 并设置下拉列表样式 -----
        QListView* listView = new QListView();
        QListView* listView_planType_Idx_ = new QListView();  ///< 分类选择下拉框

        listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
        listView->setAutoScroll(false);  // 禁用边缘自动滚动
        // 同上 listView
        listView_planType_Idx_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
        listView_planType_Idx_->setAutoScroll(false);  // 禁用边缘自动滚动

        //一个 QListView 不能同时被两个 QComboBox 使用
        // 样式表
        QString listStyle = R"(
    QListView {
        font-family: "Noto Sans S Chinese";
                font-weight: 500;
        font-size: 14px;
        background: #0D0F14;
        border-radius: 6px;
        padding-left: 6px;
        padding-right: 6px;
        padding-top: 6px;
        padding-bottom: 6px;
        outline: 0;/*移除焦点轮廓*/

    }
    QListView::item {
        width: 148px;
        height: 25px;
        margin-top: 4px;
        margin-bottom: 4px;
        margin-left: 6px;          /* 添加左间距 */
        margin-right: 6px;         /* 添加右间距 */
        color: #A1A8B3;
        background-color: transparent;
        outline: 0;/*移除焦点轮廓*/
    }
    QListView::item:hover {
        background-color: rgba(223, 243, 255, 0.2);
        border-radius: 4px;
        /* 无需再设置 margin-left/right，会继承普通 item 的 */
    }
    QListView::item:selected {
        background-color: #0091DA;
        border-radius: 4px;
        color: #FFFFFF;
        /* 同理，删除 margin-left/right */
    }
)";


        listView->setStyleSheet(listStyle);            // 样式只给 listView
        ui->cBox_Scene->setView(listView);          // 替换下拉视图
        //让下拉高度随项数自动增加（取消最大可见项限制）
        ui->cBox_Scene->setMaxVisibleItems(INT_MAX);   // 或一个足够大的数，例如 1000
    }

   // 创建 QScrollArea
    ScrollArea_MyPlans = new CustomQScrollAreaGeneralLayout(ui->widget_MyPlans,ScrollAreaDisplayMode::GridDisplay);
    ScrollArea_MyPlans->setObjectName("UploadMyPlans");
    ScrollArea_MyPlans->setMarginAndWidth(0,38,16,radioWight);//与左侧边框间距，与右侧边框间距，项之间间距，项最小宽度
    ScrollArea_MyPlans->setScrollbar(10);
    // ui->widget_MyPlans->setLayout(new QVBoxLayout());
    // // 可选：设置布局边距为0，避免额外空白
    // ui->widget_MyPlans->layout()->setContentsMargins(0, 0, 0, 0);
    ui->widget_MyPlans->layout()->addWidget(ScrollArea_MyPlans);

    // 给选择机型弹窗，创建垂直布局,按钮,信号事件
    {
        //设置添加机型弹窗的样式
        ui->widget_AddDevBox->setStyleSheet(
            "    background: #283243;"
            "    border-radius: 6px;"
            );

        // 添加阴影
        QGraphicsDropShadowEffect *shadow1 = new QGraphicsDropShadowEffect(ui->widget_AddDevBox);
        shadow1->setBlurRadius(8);
        shadow1->setXOffset(0);
        shadow1->setYOffset(4);
        shadow1->setColor(QColor(0, 0, 0, 128));
        ui->widget_AddDevBox->setGraphicsEffect(shadow1);

        QVBoxLayout *layout = new QVBoxLayout(ui->widget_AddDevBox);

        // 设置边距，上16，左14，下16，右0
        layout->setContentsMargins(14, 16, 0, 16); //left, top, right, bottom

        // 设置控件间距为6
        layout->setSpacing(6);

        group_dev = new QButtonGroup(this);
        group_dev->setExclusive(false);  // 允许多选
        // 添加按钮
        auto createStyledButton = [&](const QString &text, QWidget *parent) -> QPushButton* {
            QPushButton *btn = new QPushButton(text, parent);
            btn->setFixedHeight(24);
            btn->setStyleSheet("QPushButton{"
                               "background: rgba(81, 96, 122, 0.2);"
                               "border-radius: 12px;"
                               "color: #FFFFFF;"
                               "font-family: \"Noto Sans S Chinese\";"
                               "font-weight: 500;"
                               "font-size: 14px;"
                               "padding: 0 12px; "
                               "}"
                               "QPushButton::checked{"
                               "background: #009FEF;"

                               "}"
                               );
            btn->setCheckable(true);
            btn->setChecked(false);
            btn->setEnabled(true);
            group_dev->addButton(btn);  // 加入组
            return btn;
        };

        QPushButton *btn1 =  createStyledButton("T10有线", ui->widget_AddDevBox);
        QPushButton *btn2 =  createStyledButton("T10无线", ui->widget_AddDevBox);
        QPushButton *btn3 =  createStyledButton("K03S", ui->widget_AddDevBox);
        QPushButton *btn4 =  createStyledButton("K03S超竞版", ui->widget_AddDevBox);
        QPushButton *btn5 =  createStyledButton("K06S", ui->widget_AddDevBox);
        QPushButton *btn6 =  createStyledButton("T7", ui->widget_AddDevBox);
        QPushButton *btn7 =  createStyledButton("T7 GT", ui->widget_AddDevBox);
        QPushButton *btn8 =  createStyledButton("S21无线智充版", ui->widget_AddDevBox);

        layout->addWidget(btn1, 0, Qt::AlignLeft);
        layout->addWidget(btn2, 1, Qt::AlignLeft);
        layout->addWidget(btn3, 2, Qt::AlignLeft);
        layout->addWidget(btn4, 3, Qt::AlignLeft);
        layout->addWidget(btn5, 4, Qt::AlignLeft);
        layout->addWidget(btn6, 5, Qt::AlignLeft);
        layout->addWidget(btn7, 6, Qt::AlignLeft);
        layout->addWidget(btn8, 7, Qt::AlignLeft);

        ui->widget_AddDevBox->setFixedHeight(ui->widget_AddDevBox->sizeHint().height());

        connect(group_dev,
                static_cast<void(QButtonGroup::*)(QAbstractButton*, bool)>(&QButtonGroup::buttonToggled),
                this,
                [this](QAbstractButton *button, bool checked)
                {
                    if(checked)
                    {
                        checkedCount++;
                        if(checkedCount == 2)
                        {
                            ui->lab_Dev2->setText(button->text());
                            ui->widget_Dev2->show();
                        }else if(checkedCount == 3)
                        {
                            ui->lab_Dev3->setText(button->text());
                            ui->widget_Dev3->show();
                        }
                    }else
                    {
                        checkedCount--;
                    }
                    ui->lab_warning->setVisible(checkedCount >= 3);
                    // 根据数量决定 pBt_addDev 的可见性
                    // ui->pBt_addDev->setVisible(checkedCount < 3);  // 少于3个时显示，≥3时隐藏
                    ui->pBt_addDev->hide();
                    if(ui->widget_AddDevBox->isVisible())
                    {
                        ui->widget_AddDevBox->setVisible(checkedCount < 3);
                    }
                });
    }


    ui->widget_AddDevBox->hide();
    ui->widget_Dev2->hide();
    ui->widget_Dev3->hide();
    ui->lab_warning->hide();
    ui->lab_Dev2->setText("");
    ui->lab_Dev3->setText("");

    installEventFilter(this);   // 安装自身过滤器
}

UploadMyPlans::~UploadMyPlans()
{
    delete ui;
}

void UploadMyPlans::M_SetCBoxShadow(NewComboBox *cBox)
{
    QWidget* container = cBox->view()->parentWidget();
    if (!container) return;

    container->setWindowFlags(container->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    container->setAttribute(Qt::WA_TranslucentBackground);
    container->setFixedWidth(160 + 8);
    if (container->layout())
        container->layout()->setContentsMargins(8, 8, 8, 8);  // 四周留出阴影空间

    // 阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 128));
    shadow->setOffset(0, 4);
    container->setGraphicsEffect(shadow);

    //把容器告诉 NewComboBox
    cBox->setPopupContainer(container);
}

bool UploadMyPlans::eventFilter(QObject *watched, QEvent *event)
{
    if ((event->type() == QEvent::MouseButtonPress ||
         event->type() == QEvent::MouseButtonDblClick) &&
        ui->widget_AddDevBox && ui->widget_AddDevBox->isVisible())
    {
        //点击到非widget_AddDevBox区域，则隐藏
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        // 将全局坐标映射到 widget_AddDevBox 的局部坐标
        QPoint localPos = ui->widget_AddDevBox->mapFromGlobal(me->globalPos());
        // 判断是否在控件的 rect() 内
        if (!ui->widget_AddDevBox->rect().contains(localPos)) {
            ui->widget_AddDevBox->hide();
        }
    }
    return false;   // 始终传递事件
}
//描述最多为50个字
void UploadMyPlans::on_pTextEdit_Description_textChanged()
{
    QString text = ui->pTextEdit_Description->toPlainText();
    if (text.length() > 50) {
        // 阻止信号递归，避免再次触发 textChanged
        ui->pTextEdit_Description->blockSignals(true);
        // 截断到最大长度
        text = text.left(50);
        ui->pTextEdit_Description->setPlainText(text);
        // 将光标移动到文本末尾
        QTextCursor cursor = ui->pTextEdit_Description->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->pTextEdit_Description->setTextCursor(cursor);
        ui->pTextEdit_Description->blockSignals(false);
    }
}

//添加机型
void UploadMyPlans::on_pBt_addDev_clicked()
{
    qDebug("显示\n");
    ui->widget_AddDevBox->show();
}

//删除机型2
void UploadMyPlans::on_pBt_Dev2_clicked()
{
    QList<QAbstractButton*> allBtns = group_dev->buttons();
    for (QAbstractButton *btn : allBtns) {
        if (btn->text() == ui->lab_Dev2->text()) {
            btn->setChecked(false);
            break;  // 找到后退出循环
        }
    }
    ui->lab_Dev2->setText("");
    ui->widget_Dev2->hide();
}

//删除机型3
void UploadMyPlans::on_pBt_Dev3_clicked()
{
    QList<QAbstractButton*> allBtns = group_dev->buttons();
    for (QAbstractButton *btn : allBtns) {
        if (btn->text() == ui->lab_Dev3->text()) {
            btn->setChecked(false);
            break;  // 找到后退出循环
        }
    }
    ui->lab_Dev3->setText("");
    ui->widget_Dev3->hide();
}
//显示当前设备的机型
void UploadMyPlans::ShowDev()
{
    if(SelDev_DeviceName.contains("K03S",Qt::CaseInsensitive))
    {
        if(SelDev_PID == 0xF016 || SelDev_PID == 0xF017)
        {
            ui->lab_Dev->setText(tr("K03S超竞版"));

        }else
        {
            ui->lab_Dev->setText("K03S");
        }

    }else if(SelDev_DeviceName.contains("K06S",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText("K06S");
    }else if(SelDev_DeviceName.contains("T10",Qt::CaseInsensitive))
    {
        if(SelDev_DeviceName.contains("Wireless",Qt::CaseInsensitive))
        {
            ui->lab_Dev->setText(tr("T10无线"));
        }else
        {
            ui->lab_Dev->setText(tr("T10有线"));
        }

    }else if(SelDev_DeviceName.contains("T7 GT",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText("T7 GT");
    }else if(SelDev_DeviceName.contains("T7",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText("T7");
    }else if(SelDev_DeviceName.contains("S21",Qt::CaseInsensitive))
    {
        ui->lab_Dev->setText(tr("S21无线智充版"));
    }else
    {
        ui->lab_Dev->setText("");
    }

    QList<QAbstractButton*> allBtns = group_dev->buttons();
    for (QAbstractButton *btn : allBtns) {
        if (btn->text() == ui->lab_Dev->text()) {
            btn->setChecked(true);
            btn->setEnabled(false);//不可取消掉
            break;  // 找到后退出循环
        }
    }
}
//显示我的方案（自创）
void UploadMyPlans::showMyPlans()
{
    qDebug() << "[UploadMyPlans] showMyPlans: MyPlanRadioList.size="
             << MovieVal.MyPlanRadioList.size() << "AllPlan=" << MovieVal.AllPlanRadioList.size();
    int cnt = 0;
    QString name;
    ScrollArea_MyPlans->removeAllWidgets();
    for (NewRadioBtn* btn : MovieVal.MyPlanRadioList)//AllPlanRadioList_Dev
    {
        if(btn->IsSys)
        {
            continue;
        }
        NewRadioBtn *radio = new NewRadioBtn(btn->getAllPlanValue(),ui->widget_MyPlans);
        name = btn->lab_name->text();
        radio->updateElidedText(btn->property("fullText").toString(),name);
        ScrollArea_MyPlans->addWidget(radio);
        QStringList Lab1 = btn->getLabDevs();
        if (Lab1.isEmpty()) {
            radio->lab1->hide();
        } else {
            radio->lab1->show();
            radio->lab1->setText(Lab1[0]);//填写
            radio->setLabDevs(Lab1);
            radio->setLab1Style(Lab1[0]);
        }
        radio->lab2->show();
        radio->setLabel2(btn->lab2->text());
        radio->setFixedSize(radioWight,radioHeight);

        radio->AllpBt_fav->hide();
        radio->AllpBt_edit->hide();

        MyPlanRadioHash_Upload.insert(name,radio);

        //点击（右侧显示当前方案的数据，且可以被修改，不同步修改预设库界面的方案）
        connect(radio, &QRadioButton::toggled, [this, radio]()
                {
                    ui->lEdit_Name->setText(radio->lab_name->text());//名称
                    ui->cBox_Scene->setCurrentText(radio->lab2->text());//场景
                    // ShowDev();//机型
                    ui->lab_Dev->setText(radio->lab1->text());//根据方案机型标签，显示机型
                    ui->pTextEdit_Description->setPlainText(radio->property("fullText").toString());//方案描述
                    UploadPlanVal = radio->getAllPlanValue();
                });
        cnt++;
        if(cnt == 1)
        {
            radio->setChecked(true);
        }

    }
    ScrollArea_MyPlans->updateView();
    //

}
//关闭
void UploadMyPlans::on_pBt_exit_clicked()
{
    reject();
}
//上传方案到服务器
void UploadMyPlans::on_pBt_upload_clicked()
{
    QString upload_file_path = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath() + "/XIBERIA X HUB/ProgramData/upload_tmp.ini";  // 上传文件路径,上传完毕后删除
    // 确保目录存在
    QDir dir = QFileInfo(upload_file_path).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    writeExportPlanIni(upload_file_path);
    qDebug() << "File to save:" << upload_file_path;

    user_token = globalSettings->value("Login/Account")
                     .toMap()
                     .value("access_token")
                     .toString(); ///用户tonken


    // // 将当前配置信息写入文件
    QFile * file = new QFile(upload_file_path);
    if (!file->open(QIODevice::ReadOnly)) {
        emit ApoManager::instance()->requestlogWithTime(
            QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg("file open failed"));
        delete file;
        return;
    }


    // 上传文件，然后获得下载链接
    // 上传文件指定ini文件
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"file\"; filename=\""
                                + QFileInfo(upload_file_path).fileName() + "\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                       QVariant("application/octet-stream")); // INI 文件的 MIME 类型

    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QNetworkReply *reply = HttpClient::instance().upload("/user/uploads", multiPart, RequestOptions{}.withTag("user"));
    multiPart->setParent(reply);

    QObject::connect(reply, &QNetworkReply::finished, this, [=]() mutable {
        reply->deleteLater();

        file->close();
        file->deleteLater();

        // 不管上传成不成功，都删除文件，下次重新打开再写
        if (QFile::remove(upload_file_path)) {
            qDebug() << "本地 upload_tmp.ini 文件删除成功:" << upload_file_path;
        } else {
            qDebug() << "本地 upload_tmp.ini 文件删除失败:" << upload_file_path;
        }

        if (reply->error() != QNetworkReply::NoError) {
            emit ApoManager::instance()->requestlogWithTime(QString("%1 %2 netWorkReply error:%3")
                                                                .arg(__FUNCTION__)
                                                                .arg(__LINE__)
                                                                .arg(reply->errorString()));

            return;
        }

        if (reply->error() == QNetworkReply::NoError) {
            /// 读取响应数据
            QByteArray responseData = reply->readAll();
            qDebug() << "文件上传请求 回显原始数据:" << QString::fromUtf8(responseData);

            /// 解析 JSON
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = "JSON解析错误: " + parseError.errorString();
                emit ApoManager::instance()->requestlogWithTime(
                    QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg(errorMsg));

                return;
            }

            /// 检查业务状态码
            QJsonObject rootObj = jsonDoc.object();
            QString code = rootObj["code"].toString();
            if (code != "success") {
                QString message = rootObj["message"].toString();
                QString errorMsg = QString("API error: code=%1, message=%2").arg(code, message);
                emit ApoManager::instance()->requestlogWithTime(QString("上传文件 %1").arg(errorMsg));
                return;
            }

            DeSheng::FileUploadsResponse ret_info; /// 文件上传回显信息
            if (DeSheng::ProcessFileUploadsResult(ret_info, jsonDoc)) {
                /// 文件上传成功后，调用 创建方案库分享码 接口
                /// 再将返回的 分享码 复制到 粘贴板

                DeSheng::CreateShareCodeRequest req;
                req.url = ret_info.data.url;                    // 文件上传回显信息中的 URL
                qDebug() << "上传方案文件成功,文件url："<<req.url;
                UploadPlanUrl = req.url;
                CreateConfigs();

            } else {
                ///请求回显消息异常处理
                qDebug() << QString("回显JSON数据 解析失败 code: %1\nmessage:%2")
                                .arg(ret_info.code)
                                .arg(ret_info.message);
            };
        }
    });



}
//上传方案--创建配置
void UploadMyPlans::CreateConfigs()
{
    UploadPlansRequest->device_id = DevId;                                          //设备id（服务器返回的绑定设备的id）
    UploadPlansRequest->drive_version = SoftWareVer;                                //驱动版本号
    UploadPlansRequest->firmware_version = FWId;                                    //固件版本号
    UploadPlansRequest->device_name = SelDev_DeviceName;                            //设备名称
    UploadPlansRequest->device_type = DevType;                                      //设备类型
    UploadPlansRequest->title = ui->lEdit_Name->text();                             //方案名称
    UploadPlansRequest->description = ui->pTextEdit_Description->toPlainText();     //方案描述
    UploadPlansRequest->user_tags = { ui->cBox_Scene->currentText()};               //用户标签数组
    UploadPlansRequest->config_url = UploadPlanUrl;                                 //配置文件下载URL

    //结构体传递得到QJsonObject
    QJsonObject json = DeSheng::UserConfigsCreateRequestToJson(*UploadPlansRequest);
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    QByteArray t_data = doc.toJson();
    qDebug() << "Request body:" << t_data;
    QNetworkReply *reply = HttpClient::instance().post("/user-configs", RequestOptions{}.withBody(t_data).withTag("userConfig"));
    connect(reply, &QNetworkReply::finished, this, [=]() mutable {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit ApoManager::instance()->requestlogWithTime(QString("%1 %2 netWorkReply error:%3")
                                                                .arg(__FUNCTION__)
                                                                .arg(__LINE__)
                                                                .arg(reply->errorString()));

            return;
        }
        if (reply->error() == QNetworkReply::NoError) {
            // 读取响应数据
            QByteArray responseData = reply->readAll();
            qDebug() << "创建配置请求 回显原始数据:" << QString::fromUtf8(responseData);



            // 解析 JSON
            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                QString errorMsg = "JSON解析错误: " + parseError.errorString();
                emit ApoManager::instance()->requestlogWithTime(
                    QString("%1 %2 %3").arg(__FUNCTION__).arg(__LINE__).arg(errorMsg));

                return;
            }

            //UploadPlansResponse传递空值
            bool result = DeSheng::ProcessUserConfigsCreateResult(*UploadPlansResponse,jsonDoc);
            if(result)
            {
                qDebug("上传方案成功\n");
                /// 通知个人中心刷新"已上传"列表（DataSync 联动）
                emit planUploaded(UploadPlansResponse->data.id);
                accept();
            }else
            {
                qDebug("上传方案失败\n");
                if(UploadPlansResponse->code.contains("user_config_sensitive_word"))
                {
                    auto *t_notif = new CustomQWidgetNotification(tr("上传失败，方案名称或描述包含敏感词"), QString(), m);
                    QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                                     t_notif, &QWidget::deleteLater);
                    t_notif->show();
                }else if(UploadPlansResponse->code.contains("user_config_too_many_per_device"))
                {
                    auto *t_notif = new CustomQWidgetNotification(tr("今日上传方案已达上限，请明天再来"), QString(), m);
                    QObject::connect(t_notif, &CustomQWidgetNotification::accepted,
                                     t_notif, &QWidget::deleteLater);
                    t_notif->show();
                }
            }

        }
    });

}
//保存预设到文件
int UploadMyPlans::writeExportPlanIni(QString filePath)
{
    QSettings settings(filePath,QSettings::IniFormat);
    //保存当前选中预设 相关信息
    // 保存按钮属性
    //名称
    settings.setValue(QString("LoadPlan/Name"), ui->lEdit_Name->text());
    //描述
    settings.setValue(QString("LoadPlan/Description"),ui->pTextEdit_Description->toPlainText());
    //标签1
    QStringList lab_devs;
    lab_devs.clear();
    for (QLabel *lab : {ui->lab_Dev, ui->lab_Dev2, ui->lab_Dev3}) {
        if (!lab->text().isEmpty())
            lab_devs << lab->text();
    }
    settings.setValue("LoadPlan/Lab1",lab_devs);
    //标签2
    settings.setValue(QString("LoadPlan/Lab2"),ui->cBox_Scene->currentText());

    //保密
    QVariantMap planValMap;
    planValMap["DataVisibleEn"] = !ui->rBtn_hideEq->isChecked();//UploadPlanVal.DataVisibleEn;
    planValMap["ParentPlanName"] = UploadPlanVal.ParentPlanName;
    planValMap["AlgoOpenEn"] = UploadPlanVal.AlgoOpenEn;
    planValMap["spaceOpenEn"] = UploadPlanVal.spaceOpenEn;
    planValMap["eqOpenEn"] = UploadPlanVal.eqOpenEn;
    planValMap["drcOpenEn"] = UploadPlanVal.drcOpenEn;
    planValMap["lowVal"] = UploadPlanVal.lowVal;
    planValMap["spaceVal"] = UploadPlanVal.spaceVal;
    planValMap["spaceReverb"] = UploadPlanVal.spaceReverb;
    planValMap["GainVal"] = UploadPlanVal.GainVal;
    planValMap["drcVal"] = UploadPlanVal.drcVal;

    //均衡器
    QVariantList t_ExtraEq;
    for (int extraEq : UploadPlanVal.ExtraEq) {
        t_ExtraEq.append(extraEq);
    }
    planValMap["ExtraEq"] = t_ExtraEq;

    //频点
    QVariantList freqValList;
    for (int j = 0; j < 10; ++j) {
        freqValList.append(UploadPlanVal.freqVal[j]);
    }
    planValMap["freqVal"] = freqValList;
    //增益
    QVariantList eqValList;
    for (int j = 0; j < 10; ++j) {
        eqValList.append(UploadPlanVal.eqVal[j]);
    }
    planValMap["eqVal"] = eqValList;
    // q值
    QVariantList qValList;
    for (int j = 0; j < 10; ++j) {
        qValList.append(UploadPlanVal.qVal[j]);
    }
    planValMap["qVal"] = qValList;
    //滤波器
    QVariantList filterValList;
    for (int j = 0; j < 10; ++j) {
        filterValList.append(UploadPlanVal.filterVal[j]);
    }
    planValMap["filterVal"] = filterValList;

    //二创内容
    //添加频点值
    QVariantList freqValList_deriv;
    for (int j = 0; j < 10; ++j) {
        freqValList_deriv.append(UploadPlanVal.freqVal_deriv[j]);
    }
    planValMap["freqVal_deriv"] = freqValList_deriv;
    // 添加eqVal数组 int eqVal[10]
    QVariantList eqValList_deriv;
    for (int j = 0; j < 10; ++j) {
        eqValList_deriv.append(UploadPlanVal.eqVal_deriv[j]);
    }
    planValMap["eqVal_deriv"] = eqValList_deriv;
    // 添加qVal数组 double qVal[10]
    QVariantList qValList_deriv;
    for (int j = 0; j < 10; ++j) {
        qValList_deriv.append(UploadPlanVal.qVal_deriv[j]);
    }
    planValMap["qVal_deriv"] = qValList_deriv;
    //滤波器
    QVariantList filterValList_deriv;
    for (int j = 0; j < 10; ++j) {
        filterValList_deriv.append(UploadPlanVal.filterVal_deriv[j]);
    }
    planValMap["filterVal_deriv"] = filterValList_deriv;

    settings.setValue("WritePlan",planValMap);


    return 1;
}



void UploadMyPlans::on_lEdit_search_textChanged(const QString &arg1)
{
    const QString searchText = arg1.toLower();   //去除首尾空格，避免纯空格触发无意义搜索

    // 遍历哈希表中的所有按钮
    for (NewRadioBtn* radio : MyPlanRadioHash_Upload.values()) {
        if (!radio) continue;

        QString radioText = radio->lab_name->text().toLower();
        bool textMatch = searchText.isEmpty() || radioText.contains(searchText);//搜索

        if (textMatch) {
            radio->setVisible(true);
        } else {
            radio->setVisible(false);
        }
    }

    ScrollArea_MyPlans->updateView();

}

