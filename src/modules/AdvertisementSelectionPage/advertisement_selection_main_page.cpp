#include "modules/AdvertisementSelectionPage/advertisement_selection_main_page.h"
#include "ui_advertisement_selection_main_page.h"

#include "network/http_client.h"
#include "network/request_options.h"

#include <QAbstractAnimation>
#include <QEasingCurve>

namespace {
constexpr int kImageTransitionDurationMs = 500;
constexpr int kOldImageExitDelayMs = 300;
constexpr qreal kOldImageExitStartProgress =
    static_cast<qreal>(kOldImageExitDelayMs) / kImageTransitionDurationMs;
} // namespace

AdvertisementSelectionMainPage::AdvertisementSelectionMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AdvertisementSelectionMainPage)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

AdvertisementSelectionMainPage::~AdvertisementSelectionMainPage()
{
    delete ui;
}

void AdvertisementSelectionMainPage::updateAdvertisementList()
{
    // 清除旧缓存目录
    QDir dir(cl_cache_path_);
    if (dir.exists() && !dir.removeRecursively()) {
        qWarning() << "[ad] 删除旧缓存目录失败:" << cl_cache_path_;
        emit advertisementListReady();
        return;
    }
    if (!dir.mkpath(cl_cache_path_)) {
        qWarning() << "[ad] 创建缓存目录失败:" << cl_cache_path_;
        emit advertisementListReady();
        return;
    }

    // 构建请求
    DeSheng::AdvertisementsListRequest req;
    req.scene = "home_banner";
    req.device_type = "headset";

    QUrlQuery t_query;
    QString t_error;
    if (!DeSheng::buildAdvertisementsListQuery(req, t_query, t_error)) {
        qDebug() << "[ad] buildQuery error:" << t_error;
        emit advertisementListReady();
        return;
    }

    // 通过 ApiClient 异步请求
    QNetworkReply *reply = HttpClient::instance().get(
        "/advertisements", RequestOptions{}.withQuery(t_query).withTag("ad"));
    qDebug() << "[ad] URL:" << reply->url().toString();

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[ad] 请求失败:" << reply->errorString();
            emit advertisementListReady();
            return;
        }

        QByteArray t_data = reply->readAll();
        QJsonDocument t_doc = QJsonDocument::fromJson(t_data);
        if (t_doc.isNull()) {
            qDebug() << "[ad] JSON解析错误";
            emit advertisementListReady();
            return;
        }

        DeSheng::AdvertisementsListResponse t_resp;
        if (!DeSheng::ProcessAdvertisementsListResult(t_resp, t_doc)
            || t_resp.code != "success") {
            qDebug() << "[ad] 业务失败:" << t_resp.code << t_resp.message;
            emit advertisementListReady();
            return;
        }

        qDebug() << "[ad] 获取成功, total:" << t_resp.data.total;

        // 排序 — sort_order 降序
        std::sort(t_resp.data.list.begin(), t_resp.data.list.end(),
                  [](const DeSheng::AdvertisementItem &a, const DeSheng::AdvertisementItem &b) {
                      return a.sort_order > b.sort_order;
                  });

        // 清理旧数据
        cl_advertisement_list_.clear();
        {
            while (cl_advertisement_pushButton_scrollArea_->cl_hBoxLayout_->count() > 0) {
                QLayoutItem *item = cl_advertisement_pushButton_scrollArea_->cl_hBoxLayout_->takeAt(0);
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            qDeleteAll(cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_list_);
            cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_list_.clear();
            auto t_buttons = cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_buttonGroup_->buttons();
            for (auto *btn : t_buttons) {
                cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_buttonGroup_->removeButton(btn);
                btn->deleteLater();
            }
        }

        // 填充数据
        for (const auto &t_item : t_resp.data.list)
            cl_advertisement_list_.append(std::make_shared<DeSheng::AdvertisementItem>(t_item));

        // 异步递归下载图片
        cl_image_download_index_ = 0;
        downloadAdImages(0);
    });
}

void AdvertisementSelectionMainPage::downloadAdImages(int t_idx)
{
    if (t_idx >= cl_advertisement_list_.size()) {
        // 全部图片下载完成 → 创建导航按钮
        for (int i = 0; i < cl_advertisement_list_.size(); ++i) {
            auto *btn = new CustomQPushButtonForSingleAdvertisement(
                cl_advertisement_pushButton_scrollArea_->cl_content_widget_);
            btn->setCheckable(true);
            cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_list_.append(btn);
            cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_buttonGroup_->addButton(btn, i);
        }

        // 居中布局
        cl_advertisement_pushButton_scrollArea_->cl_hBoxLayout_->addStretch();
        for (auto *btn : cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_list_)
            cl_advertisement_pushButton_scrollArea_->cl_hBoxLayout_->addWidget(btn);
        cl_advertisement_pushButton_scrollArea_->cl_hBoxLayout_->addStretch();

        // 默认选中第一个
        if (!cl_advertisement_list_.isEmpty()
            && !cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_list_.isEmpty()) {
            cl_advertisement_pushButton_scrollArea_->setVisible(cl_advertisement_list_.size() > 1);
            cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_list_.first()->setChecked(true);
            cl_current_index_.store(0);
            updateAdvertisementIndex(0);
        }

        update();
        emit advertisementListReady();
        return;
    }

    const auto &t_item = cl_advertisement_list_.at(t_idx);
    if (t_item->img_url.isEmpty()) {
        downloadAdImages(t_idx + 1); // 跳过空 URL
        return;
    }

    const QString t_path = cl_cache_path_ + t_item->img_url.section("/", -1);
    auto *t_nam = new QNetworkAccessManager(this);
    QNetworkReply *t_reply = t_nam->get(QNetworkRequest(QUrl(t_item->img_url)));

    connect(t_reply, &QNetworkReply::finished, this, [this, t_reply, t_nam, t_path, t_idx]() {
        t_nam->deleteLater();
        if (t_reply->error() == QNetworkReply::NoError) {
            QFile t_file(t_path);
            if (t_file.open(QIODevice::WriteOnly)) {
                t_file.write(t_reply->readAll());
                t_file.close();
            }
        } else {
            qDebug() << "[ad] 图片下载失败:" << t_reply->errorString();
        }
        t_reply->deleteLater();
        downloadAdImages(t_idx + 1); // 下一张
    });
}

void AdvertisementSelectionMainPage::InitUIInformation()
{
    {
        setObjectName("cl_AdvertisementSelectionMainPage");
        setStyleSheet(R"(
    QWidget#cl_AdvertisementSelectionMainPage {
        border-radius: 12px;
    }
)");
    }
    {
        ui->widget_pushButtons->setStyleSheet(R"(
    QWidget#cl_AdvertisementSelectionMainPage {
    }
)");
    }
    {
        cl_advertisement_pushButton_scrollArea_ = new CustomQScrollAreaForAdvertisementPushButton(
            ui->widget_pushButtons);
        ui->widget_pushButtons->layout()->addWidget(cl_advertisement_pushButton_scrollArea_);
    }
}

void AdvertisementSelectionMainPage::InitMember()
{
    {
        cl_change_timer_ = new QTimer(this);
        connect(cl_change_timer_, &QTimer::timeout, this, [this]() {
            auto *group = cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_buttonGroup_;
            if (!group) return;

            int total = group->buttons().size();
            if (total == 0) return;

            int currentChecked = group->checkedId();
            int nextIndex = (currentChecked + 1) % total;
            group->buttons().at(nextIndex)->setChecked(true);

            cl_current_index_.store(nextIndex);
            updateAdvertisementIndex(cl_current_index_);
        });
    }
    {
        cl_image_transition_anim_ = new QVariantAnimation(this);
        cl_image_transition_anim_->setDuration(kImageTransitionDurationMs);
        cl_image_transition_anim_->setEasingCurve(QEasingCurve::OutCubic);
        cl_image_transition_anim_->setStartValue(0.0);
        cl_image_transition_anim_->setEndValue(1.0);
        connect(cl_image_transition_anim_,
                &QVariantAnimation::valueChanged,
                this,
                [this](const QVariant &value) {
                    cl_image_transition_progress_ = value.toReal();
                    update();
                });
        connect(cl_image_transition_anim_, &QVariantAnimation::finished, this, [this]() {
            if (!cl_next_background_image_.isNull()) {
                cl_background_image_ = cl_next_background_image_;
                cl_next_background_image_ = QPixmap();
            }
            cl_image_transition_progress_ = 1.0;
            update();
        });
    }
    {
        cl_cache_path_ = QFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).absolutePath()
                        + "/XIBERIA X HUB/ProgramData/advertisement_cache/";
    }
}

void AdvertisementSelectionMainPage::InitConnect()
{
    {
        connect(cl_advertisement_pushButton_scrollArea_,
                &CustomQScrollAreaForAdvertisementPushButton::changeGameTypeVideos,
                this,
                [=](int index) {
                    cl_current_index_.store(index);
                    updateAdvertisementIndex(cl_current_index_);
                });
    }
}

void AdvertisementSelectionMainPage::updateAdvertisementIndex(int index)
{
    if (index < 0 || index >= cl_advertisement_list_.size())
        return;

    QString imagePath = cl_cache_path_
                        + cl_advertisement_list_.at(index).get()->img_url.section("/", -1);

    if (QFile::exists(imagePath)) {
        QPixmap nextImage = QPixmap(imagePath).scaled(rect().size(),
                                                      Qt::IgnoreAspectRatio,
                                                      Qt::SmoothTransformation);
        if (nextImage.isNull())
            return;

        if (cl_background_image_.isNull()) {
            cl_background_image_ = nextImage;
            cl_next_background_image_ = QPixmap();
            cl_image_transition_progress_ = 1.0;
        } else {
            if (cl_image_transition_anim_ && cl_image_transition_anim_->state() == QAbstractAnimation::Running)
                cl_image_transition_anim_->stop();

            cl_next_background_image_ = nextImage;
            cl_image_transition_progress_ = 0.0;
            if (cl_image_transition_anim_)
                cl_image_transition_anim_->start();
        }
        ui->widget_pushButtons->raise();
        update();
    }
}

void AdvertisementSelectionMainPage::mousePressEvent(QMouseEvent *event)
{
    if (ui->widget_pushButtons->geometry().contains(event->pos())) {
        return;
    }

    auto *group = cl_advertisement_pushButton_scrollArea_->cl_all_advertisement_buttonGroup_;
    if (!group || group->buttons().isEmpty()) return;

    int currentChecked = group->checkedId();
    if (currentChecked < 0 || currentChecked >= cl_advertisement_list_.size()) return;

    // 广告点击上报 → ApiClient
    int t_id = cl_advertisement_list_.at(currentChecked)->id;
    QString t_path = QString("/advertisements/%1/click").arg(t_id);
    QNetworkReply *t_reply = HttpClient::instance().post(
        t_path, RequestOptions{}.withTag("ad"));
    connect(t_reply, &QNetworkReply::finished, t_reply, [t_reply]() {
        if (t_reply->error() == QNetworkReply::NoError) {
            qDebug() << "[ad] 点击上报成功";
        } else {
            qDebug() << "[ad] 点击上报失败:" << t_reply->errorString();
        }
        t_reply->deleteLater();
    });

    QWidget::mousePressEvent(event);
}

void AdvertisementSelectionMainPage::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);
    if (cl_change_timer_ && cl_change_timer_->isActive())
        cl_change_timer_->stop();
    QWidget::enterEvent(event);
}

void AdvertisementSelectionMainPage::leaveEvent(QEvent *event)
{
    setCursor(Qt::ArrowCursor);
    if (cl_change_timer_ && !cl_change_timer_->isActive())
        cl_change_timer_->start(2000);
    QWidget::leaveEvent(event);
}

void AdvertisementSelectionMainPage::mouseMoveEvent(QMouseEvent *event)
{
    if (ui->widget_pushButtons->geometry().contains(event->pos())) {
        setCursor(Qt::ArrowCursor);
        if (cl_change_timer_ && !cl_change_timer_->isActive())
            cl_change_timer_->start(2000);
    } else {
        setCursor(Qt::PointingHandCursor);
        if (cl_change_timer_ && cl_change_timer_->isActive())
            cl_change_timer_->stop();
    }
    QWidget::mouseMoveEvent(event);
}

void AdvertisementSelectionMainPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void AdvertisementSelectionMainPage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), 15, 15);
    painter.setClipPath(path);

    if (!cl_next_background_image_.isNull()) {
        const qreal progress = qBound<qreal>(0.0, cl_image_transition_progress_, 1.0);

        if (!cl_background_image_.isNull()) {
            qreal oldImageExitProgress = 0.0;
            if (progress > kOldImageExitStartProgress) {
                oldImageExitProgress =
                    (progress - kOldImageExitStartProgress) / (1.0 - kOldImageExitStartProgress);
            }

            painter.setOpacity(1.0 - oldImageExitProgress);
            const int oldX = static_cast<int>(-oldImageExitProgress * width());
            painter.drawPixmap(QRect(oldX, 0, width(), height()), cl_background_image_);
        }

        painter.setOpacity(progress);
        const int nextX = static_cast<int>((1.0 - progress) * width());
        painter.drawPixmap(QRect(nextX, 0, width(), height()), cl_next_background_image_);
        painter.setOpacity(1.0);
    } else if (!cl_background_image_.isNull()) {
        painter.drawPixmap(rect(), cl_background_image_);
    }

    QWidget::paintEvent(event);
}
