//每次点击视频都会解析一遍视频内容
#include "SpeakerListen.h"
#include "ui_SpeakerListen.h"

#include <QDir>
#include <QFileInfo>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPixmapCache>
#include <QScrollBar>
#include <QShortcut>
#include <QTimer>
#include <QVideoWidget>
#include "APOThread/ApoManager.h"
#include "LoadLib.h"
#include "modules/Common/AppImageCache.h"
#include <thread>
//#include <QThread>
#include <QMutexLocker>

QString CurrentfilePath;
bool loop = false;

SpeakerListen::SpeakerListen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SpeakerListen)
{
    ui->setupUi(this);
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember(); ///< 初始化内部成员
    InitConnect(); ///< 连接默认的信号槽

    currentPlan_l = ui->rBt_currentPlan;
    eightPlan_l = ui->widget_eight;
}

SpeakerListen::~SpeakerListen()
{
    delete ui;
}

void SpeakerListen::LanguageSet()
{
    if (ui) {
        ui->retranslateUi(this);
    }
    if (cl_sound_test_main_page_) {
        cl_sound_test_main_page_->retranslateTexts();
    }
}

void SpeakerListen::InitUIInformation()
{
    {
        // 模糊面板圆角
        ui->widget->setCornerRadius(10);
        ui->widget_3->setCornerRadius(10);
        ui->widget_listenEn->setCornerRadius(10);
        ui->widget_all->setCornerRadius(10);
        ui->widget_listen->setCornerRadius(10);
    }

    ui->widget_background->setAttribute(Qt::WA_TranslucentBackground);
    ui->widget_background->setAutoFillBackground(false);

    cl_sound_test_main_page_ = std::make_unique<SoundTestMainPage>(ui->widget_sound_test_main_page);
    ui->widget_sound_test_main_page->layout()->addWidget(cl_sound_test_main_page_.get());

    ui->widget_sound_test_main_page->setObjectName("widget_sound_test_main_page");
    ui->widget_sound_test_main_page->setAutoFillBackground(true);
    ui->widget_sound_test_main_page->setStyleSheet(R"(
    QWidget#widget_sound_test_main_page {
        background: rgba(81, 96, 122, 0.2);
        border-radius: 10px;
    }
)");

    ui->pBt_GameListen->setCursor(Qt::PointingHandCursor);
    ui->pBt_GameListen->setCheckable(true);
    ui->pBt_GameListen->setChecked(false); ///默认非选中状态

    //     /// 文字：我的收藏
    //     ui->lab_cur_2->setStyleSheet(R"(
    //     QLabel {
    //         font-family: "Source Han Sans";
    //         font-size: 14px;
    //         font-weight: bold;
    //         color: #A1A8B3;
    //     }
    // )");

    ///不影响内部的 widget_eight
    ui->widget_all->setObjectName("widget_all");
    ui->widget_all->setAutoFillBackground(true);
    ui->widget_all->setStyleSheet(R"(
    QWidget#widget_all {
        background-color: rgba(81, 96, 122, 0.2);
        border-radius: 10px;
    }
)");

    {
        // ui->pBt_goto_plans_page 完整设置
        ui->pBt_goto_plans_page->setCl_min_size(QSize(88, 88));
        ui->pBt_goto_plans_page->setCl_icon_size(QSize(30, 29), QSize(30, 29));
        ui->pBt_goto_plans_page->setCl_icon_text_spacing(5);
        ui->pBt_goto_plans_page->setCl_icon_point(QPoint(29, 29), QPoint(29, 23));
        ui->pBt_goto_plans_page->setCl_bg_default_color(QColor(81, 96, 122, 51));
        ui->pBt_goto_plans_page->setCl_bg_hover_color(QColor(255, 255, 255, 25));
        ui->pBt_goto_plans_page->setCl_border_radius(10);
        // 显示信息 后手设置
        ui->pBt_goto_plans_page->setCl_pixmap(
            QPixmap(":/Skin/Images/soundTest/plansLib_icon_2x.png"));
        ui->pBt_goto_plans_page->setCl_classification_name(tr("预设库"));

        ui->pBt_goto_plans_page->repaint(); // 立即重绘
    }
}

void SpeakerListen::InitMember()
{
    ui->rBt_currentPlan->installEventFilter(this); // 当前方案指示器：双击跳转EQ
}

void SpeakerListen::InitConnect()
{
    QObject::connect(cl_sound_test_main_page_.get(), &SoundTestMainPage::closeSoundTest, this, [=]() {
        /// 跳转到方案库页面
        // emit ChangeToPlanPage();
        emit ChangeToSpeakerPage(1, false, 0, 0.0);
    });

    QObject::connect(cl_sound_test_main_page_.get(),
                     &SoundTestMainPage::minWidget,
                     this,
                     [=](bool isMinWidgetShow) {
                         /// 跳转到均衡器页面
                         // emit ChangeToPlanPage();
                         if (isMinWidgetShow) {
                             emit ChangeToSpeakerPage(3, false, 0, 0.0);
                         } else {
                             ///小窗口 回退
                             emit ChangeToSpeakerPage(4, false, 0, 0.0);
                         }
                     });

    /// 点击方案库按键，跳转到方案库界面
    QObject::connect(ui->pBt_goto_plans_page, &QPushButton::clicked, this, [=]() {
        /// 暂停播放
        if (!cl_sound_test_main_page_->cl_ffmpeg_main_page_->cl_ffmpeg_global_->cl_is_pause()) {
            cl_sound_test_main_page_->cl_ffmpeg_main_page_->cl_ffmpeg_global_->pause(true);
        }

        /// 小窗口隐藏并回退
        if (cl_sound_test_main_page_->cl_ffmpeg_main_page_->is_minView_.load()) {
            cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView->requestRestore();
        }

        ui->pBt_GameListen->setChecked(false);
        emit ChangeToSpeakerPage(1, false, 0, 0.0);
    });
}

void SpeakerListen::on_rBt_currentPlan_clicked()
{
    /// wbliu：保留
    // int VHIdx = GetVHIdx();
    // pause();
    // m_progressTimer->stop();
    // emit ChangeToSpeakerPage(1, false, VHIdx, 0.0);
}

bool SpeakerListen::eventFilter(QObject *watched, QEvent *event)
{
    // 当前方案指示器（NewRadioBtnText）：双击跳转 EQ 界面
    if (watched == ui->rBt_currentPlan && event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::LeftButton)
        {
            qDebug("rBt_currentPlan 双击跳转EQ\n");
            // 若视频正在播放，执行悬浮按钮的功能（切换到小窗口画中画模式，onMinWidgetSlots 幂等）
            auto *t_ffmpegGlobal = cl_sound_test_main_page_->cl_ffmpeg_main_page_->cl_ffmpeg_global_.get();
            if (t_ffmpegGlobal && !t_ffmpegGlobal->current_media_filename().isEmpty()
                && !t_ffmpegGlobal->cl_is_stop()) {
                cl_sound_test_main_page_->cl_ffmpeg_main_page_->onMinWidgetSlots(true);
            }
            // index==3：mainwindow 的 ChangeToSpeakerPage 处理链路跳转到均衡器界面（含空方案保护）
            emit ChangeToSpeakerPage(3, false, 0, 0.0);
            return true; // 事件已处理
        }
    }
    return QObject::eventFilter(watched, event);
}

//切换主题
void SpeakerListen::setThemeAndPanelTransparency_SpeakerListen(int idx, int PValue) {}
//设置面板透明度（参数：主题，透明度）
void SpeakerListen::setPanelTransparency_SpeakerListen(int idx, int PValue)
{
    double PanelTransparency = PValue / 100.0; //面板透明度(默认值是0.2)
    int r, g, b;
    QString suffix;
    switch (idx) {
    case 0:
        suffix = "" /*"_darkBlue"*/;
        break; //深蓝色（还未修改主题图片）
    case 1:
        suffix = "_white";
        break; //白色
    case 2:
        suffix = "_black";
        break; //黑色
    default:
        suffix = "";
        break;
    }
    //当前预设
    ui->rBt_currentPlan->setThemeAndPanelTransparency(idx, PValue);

    switch (idx) {
    case 0:
        r = 81;
        g = 96;
        b = 122;
        break; // 深蓝色
    case 1:
        r = 81;
        g = 96;
        b = 122;
        break; // 白色
    case 2:
        r = 81;
        g = 96;
        b = 122;
        break; // 黑色
    default:
        r = 81;
        g = 96;
        b = 122;
        break;
    }

    QString colorStr = QString("rgba(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(PanelTransparency);
    QColor background = QColor(r, g, b, PanelTransparency);
    switch (idx) {
    case 0:
        r = 255;
        g = 255;
        b = 255;
        break; // 深蓝色
    case 1:
        r = 255;
        g = 255;
        b = 255;
        break; // 白色
    case 2:
        r = 255;
        g = 255;
        b = 255;
        break; // 黑色
    default:
        r = 255;
        g = 255;
        b = 255;
        break;
    }
    QString colorStr2 = QString("rgba(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(PanelTransparency);
    //我的收藏
    ui->widget_all->setStyleSheet(
        QString("background-color: %1;border-radius: 10px;").arg(colorStr));
    //方案库
    ui->pBt_goto_plans_page->setCl_bg_default_color(colorStr);
    ui->pBt_goto_plans_page->repaint(); // 立即重绘
    //试听
    Painter_Background = background;
    update();

    //视频窗口
    ui->widget_sound_test_main_page->setStyleSheet(QString("QWidget#widget_sound_test_main_page {"
                                                           "background: %1;"
                                                           "}")
                                                       .arg(colorStr));
}
//设置面板模糊度
void SpeakerListen::setPanelBlur_SpeakerListen(int PValue) {}

void SpeakerListen::updateVideoHoverPosition()
{
    cl_sound_test_main_page_->cl_ffmpeg_main_page_->minView->updatePositionRelativeToParent();
}
