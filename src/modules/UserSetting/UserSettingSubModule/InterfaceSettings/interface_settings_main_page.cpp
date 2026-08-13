#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/interface_settings_main_page.h"
#include "ui_interface_settings_main_page.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QListView>

#include "LoadLib.h"                             ///< globalSettings, LanguageIdx, tran
#include "modules/UserSetting/UserSettingSubModule/InterfaceSettings/InterfaceSettingCustomUI/custom_QScrollArea_background_images_component.h" ///< 背景图片预览

QTranslator tran;//翻译器

/// \brief 构造函数
InterfaceSettingsMainPage::InterfaceSettingsMainPage(QWidget *parent, int theme)
    : QWidget(parent)
    , cl_theme_(theme)
    , ui(new Ui::InterfaceSettingsMainPage)
{
    ui->setupUi(this);
    InitUIInformation(theme); ///< 初始化UI的默认信息
    InitMember();             ///< 初始化内部成员
    InitConnect();            ///< 连接默认的信号槽

    applyTheme(theme);
}

InterfaceSettingsMainPage::~InterfaceSettingsMainPage()
{
    delete ui;
}

/// \brief 按主题更新样式
void InterfaceSettingsMainPage::applyTheme(int theme)
{
    cl_theme_ = theme;
    if (clp_background_component_view_) {
        // 背景预览组件暂不需要主题样式切换
    }
}

int InterfaceSettingsMainPage::getLanguageIndex()
{
    return ui->cBox_language->currentIndex();
}

void InterfaceSettingsMainPage::setLanguageIndex(int targetIndex)
{
    if (targetIndex < 0 || targetIndex >= ui->cBox_language->count()) {
        return;
    }
    ui->cBox_language->setCurrentIndex(targetIndex);
}

int InterfaceSettingsMainPage::getThemeIndex()
{
    return ui->cBox_Theme->currentIndex();
}

void InterfaceSettingsMainPage::setThemeIndex(int targetIndex) {
    if (targetIndex < 0 || targetIndex >= ui->cBox_Theme->count()) {
        return;
    }
    ui->cBox_Theme->setCurrentIndex(targetIndex);
}

//给QComBobox设置阴影
// void InterfaceSettingsMainPage::M_SetCBoxShadow(NewComboBox *cBox)
// {
//     QWidget* container = cBox->view()->parentWidget();
//     if (!container) return;

//     container->setWindowFlags(container->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
//     container->setAttribute(Qt::WA_TranslucentBackground);
//     container->setFixedWidth(146);
//     container->setMinimumHeight(66);
//     if (container->layout())
//         container->layout()->setContentsMargins(0, 4, 8, 0);

//     // 阴影：box-shadow: 0px 4px 8px 0px rgba(0, 0, 0, 0.5)
//     QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(container);
//     shadow->setBlurRadius(8);                     // 模糊半径 8px
//     shadow->setColor(QColor(0, 0, 0, 127));       // rgba(0,0,0,0.5)
//     shadow->setOffset(0, 4);                      // x:0, y:4
//     container->setGraphicsEffect(shadow);
// }
void InterfaceSettingsMainPage::M_SetCBoxShadow(NewComboBox *cBox)
{
    QWidget* container = cBox->view()->parentWidget();
    if (!container) return;

    container->setWindowFlags(container->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    container->setAttribute(Qt::WA_TranslucentBackground);

    // 原内容宽度 138（146 - 0左 - 8右），现在左右各留 8px 边距，所以容器宽度 = 138 + 8 + 8 = 154
    container->setFixedWidth(154);

    if (container->layout())
        container->layout()->setContentsMargins(8, 8, 8, 8);  // 四周留出阴影空间

    // 阴影参数（对应 box-shadow: 0px 4px 8px 0px rgba(0,0,0,0.5)）
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(container);
    shadow->setBlurRadius(8);
    shadow->setColor(QColor(0, 0, 0, 128));
    shadow->setOffset(0, 4);
    container->setGraphicsEffect(shadow);

    //把容器告诉 NewComboBox
    cBox->setPopupContainer(container);
}
/// \brief 初始化UI的默认信息
void InterfaceSettingsMainPage::InitUIInformation(int theme)
{
    {
        // 主容器
        ui->widget->setObjectName("InterfaceSettings_widget");
        ui->widget->setCornerRadius(12); // 毛玻璃圆角与下方 border-radius 对齐
        ui->widget->setStyleSheet(R"(
        QWidget#InterfaceSettings_widget {
            border-radius: 12px;
            background-color: rgba(81, 96, 122, 0.2);
        }
)");
    }
    {
        // 顶部区域
        ui->widget_3->setObjectName("InterfaceSettings_widget_3");
    }
    {
        // 语言切换 标签
        ui->label->setObjectName("InterfaceSettings_label");
        ui->label->setStyleSheet(R"(
            QLabel#InterfaceSettings_label {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #616871;
                background: transparent;
            }
        )");
    }

    QString comboBoxStyle = R"(
    QComboBox {
        font-family: "Noto Sans S Chinese";
        font-size: 12px;
        font-weight: 500;
        color: #A1A8B3;
        background-color: rgba(0, 0, 0, 0.2);
        border-radius: 2px;
        padding-left: 10px;
    }

    /* 下拉箭头 — 收起状态 */
    QComboBox::drop-down {
        image: url(:/Skin/Images/more/interface_settings/comBox_drop_down_darkBlue_.png);
        subcontrol-origin: padding;
        subcontrol-position: center right;
        margin-right: 10px;
        width: 9px;
        height: 12px;
    }

    /* 下拉箭头 — 展开状态 */
    QComboBox::drop-down:checked {
        width: 12px;
        height: 12px;
        image: url(:/Skin/Images/more/interface_settings/comBox_drop_down_checked_darkBlue.png);
    }
)";
    // 下拉框样式表
    QString listViewStyle = R"(
    QListView {
        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 12px;
        background: #0D0F14;
        border-radius: 6px;
        padding-left: 6px;
        padding-right: 6px;
        padding-top: 6px;
        padding-bottom: 6px;
        outline: 0;/*移除焦点轮廓*/

    }
    QListView::item {
        width: 243px;
        height: 25px;
        margin-top: 4px;
        margin-bottom: 4px;
        margin-left: 6px;          /* 添加左间距 */
        margin-right: 6px;         /* 添加右间距 */
        padding-left: 10px;         /* 文本与项左边缘的内边距 */
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
    }
)";
    // 下拉框样式表
    QString listViewStyle2 = R"(
    QListView {
        font-family: "Noto Sans S Chinese";
        font-weight: 500;
        font-size: 12px;
        background: #0D0F14;
        border-radius: 6px;
        padding-left: 6px;
        padding-right: 6px;
        padding-top: 6px;
        padding-bottom: 6px;
        outline: 0;/*移除焦点轮廓*/

    }
    QListView::item {
        width: 243px;
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
    }
)";

    {
        // 语言切换 下拉框
        ui->cBox_language->setObjectName("InterfaceSettings_cBox_language");
        ui->cBox_language->setStyleSheet(comboBoxStyle);
        M_SetCBoxShadow(ui->cBox_language);
        ui->cBox_language->setPopupOffsetXY(-8,2);
        int t_lang_idx = globalSettings->value("Language", 0).toInt();
        ui->cBox_language->setCurrentIndex(t_lang_idx);
        // 下拉列表样式
        QListView *t_list_view_lang = new QListView();
        t_list_view_lang->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
        t_list_view_lang->setAutoScroll(false);  // 禁用边缘自动滚动
        t_list_view_lang->setStyleSheet(listViewStyle);
        ui->cBox_language->setView(t_list_view_lang);
        //让下拉高度随项数自动增加（取消最大可见项限制）
        ui->cBox_language->setMaxVisibleItems(INT_MAX);   // 一个足够大的数
    }
    {
        // 主题切换 下拉框
        ui->cBox_Theme->setObjectName("InterfaceSettings_cBox_Theme");
        ui->cBox_Theme->setStyleSheet(comboBoxStyle);
        M_SetCBoxShadow(ui->cBox_Theme);
        ui->cBox_Theme->setPopupOffsetXY(-8,2);
        int t_theme_idx = globalSettings->value("Theme", 0).toInt();
        ui->cBox_Theme->setCurrentIndex(t_theme_idx);
        // 下拉列表样式
        QListView *t_list_view_theme = new QListView();
        t_list_view_theme->setIconSize(QSize(32, 32));
        t_list_view_theme->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//取消滚动条
        t_list_view_theme->setAutoScroll(false);  // 禁用边缘自动滚动
        t_list_view_theme->setStyleSheet(listViewStyle2);
        ui->cBox_Theme->setView(t_list_view_theme);
        //让下拉高度随项数自动增加（取消最大可见项限制）
        ui->cBox_Theme->setMaxVisibleItems(INT_MAX);   // 一个足够大的数
    }
    {
        // 主题切换 标签
        ui->label_2->setObjectName("InterfaceSettings_label_2");
        ui->label_2->setStyleSheet(R"(
            QLabel#InterfaceSettings_label_2 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #616871;
                background: transparent;
            }
        )");
    }
    {
        // 背景自定义 标签
        ui->label_3->setObjectName("InterfaceSettings_label_3");
        ui->label_3->setStyleSheet(R"(
            QLabel#InterfaceSettings_label_3 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 14px;
                color: #616871;
                background: transparent;
            }
        )");
    }
    {
        // 背景透明度 标签
        ui->label_4->setObjectName("InterfaceSettings_label_4");
        ui->label_4->setStyleSheet(R"(
            QLabel#InterfaceSettings_label_4 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background: transparent;
            }
        )");
    }
    {
        // 面板透明模糊度 标签
        ui->label_5->setObjectName("InterfaceSettings_label_5");
        ui->label_5->setStyleSheet(R"(
            QLabel#InterfaceSettings_label_5 {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #616871;
                background: transparent;
            }
        )");
    }

    {
        // 背景图片预览区域
        ui->widget_background_images->setObjectName(
            "InterfaceSettings_widget_background_images");

    }
    {
        // 面板透明模糊度 滑块
        ui->horizontalSlider_2->setRange(0, 100);
        ui->horizontalSlider_2->setType(1, 12, 4, true,true);
        ui->horizontalSlider_2->setValue(qRound(g_user_information.local.panel_blur_radius_ * 100));
        ui->horizontalSlider_2->setStyleSheet(R"(
QSlider {
    background: none;
    border: none;
}
QSlider::groove:horizontal {
    height: 10px;
    border: none;
    background: transparent;
}
QSlider::handle:horizontal {
    width: 12px;
    height: 0px;
    background: transparent;
    border: none;
    margin: 0px;
    padding: 0px;
}
QSlider::handle:horizontal:hover {
    width: 12px;
    height: 0px;
    background: transparent;
    border: none;
    margin: 0px;
    padding: 0px;
}

)");
    }
    {
        // 背景透明度 滑块
        ui->horizontalSlider->setRange(0, 100);
        ui->horizontalSlider->setType(1, 12, 4, true,true);
        ui->horizontalSlider->setValue(qRound(g_user_information.local.panel_opacity_ * 100));
        ui->horizontalSlider->setStyleSheet(R"(
QSlider {
    background: none;
    border: none;
}
QSlider::groove:horizontal {
    height: 10px;
    border: none;
    background: transparent;
}
QSlider::handle:horizontal {
    width: 12px;
    height: 0px;
    background: transparent;
    border: none;
    margin: 0px;
    padding: 0px;
}
QSlider::handle:horizontal:hover {
    width: 12px;
    height: 0px;
    background: transparent;
    border: none;
    margin: 0px;
    padding: 0px;
}
)");
    }
}

/// \brief 初始化内部成员
void InterfaceSettingsMainPage::InitMember()
{
    // 将背景图片预览组件嵌入下部区域
    clp_background_component_view_ = new CustomQScrollAreaBackgroundComponent(
        ui->widget_background_images);
    ui->widget_background_images->layout()->addWidget(clp_background_component_view_);
}

/// \brief 连接默认的信号槽
void InterfaceSettingsMainPage::InitConnect()
{
    // 语言切换
    connect(ui->cBox_language,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &InterfaceSettingsMainPage::onLanguageChanged,
            Qt::UniqueConnection);

    // 主题切换
    connect(ui->cBox_Theme,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &InterfaceSettingsMainPage::onThemeChanged,
            Qt::UniqueConnection);

    // 背景透明度 滑块 — 值变化立刻触发重绘
    connect(ui->horizontalSlider, &QSlider::valueChanged,
            this, [this](int v) {
                g_user_information.local.panel_opacity_ = v / 100.0;
        emit backgroundTransparencyChanged(g_user_information.local.panel_opacity_);
    }, Qt::UniqueConnection);

    // 面板透明模糊度 滑块 — 值变化立刻触发重绘
    // 注意：必须 v/100.0 往返一致（qMax(v,2) 会把 0/1 存成 0.02，重启后滑块显示 2 ≠ 配置值）
    connect(ui->horizontalSlider_2, &QSlider::valueChanged,
            this, [this](int v) {
                g_user_information.local.panel_blur_radius_ = v / 100.0;
                emit panelBlurChanged(g_user_information.local.panel_blur_radius_);
            }, Qt::UniqueConnection);

    // 滑块松手 — 仅持久化
    connect(ui->horizontalSlider, &QSlider::sliderReleased,
            this, [this]() { g_user_information.saveWallpaperConfigAsync(); },
            Qt::UniqueConnection);
    connect(ui->horizontalSlider_2, &QSlider::sliderReleased,
            this, [this]() { g_user_information.saveWallpaperConfigAsync(); },
            Qt::UniqueConnection);

    /// 背景壁纸变更（转发）
    connect(
        clp_background_component_view_, &CustomQScrollAreaBackgroundComponent::backgroundChanged,
        this,
        [this](const QString& path) {
            ui->horizontalSlider->setEnabled(true);
            // ui->horizontalSlider_2->setEnabled(true);

            // 水平条 渐变
            ui->horizontalSlider->animateHandleColor(QColor("#ACACAC"), QColor("#FFFFFF"), 100);
            ui->horizontalSlider->animateFillColor(QColor("#0F6796"), QColor("#009FEF"), 100);

            emit backgroundChanged(path);
        },
        Qt::UniqueConnection);

    // 恢复默认背景（转发）
    connect(
        clp_background_component_view_,
        &CustomQScrollAreaBackgroundComponent::defaultBackgroundRestored, this,
        [this]() {
            ui->horizontalSlider->setEnabled(false);
            // ui->horizontalSlider_2->setEnabled(false);
            // 水平条 渐变
            ui->horizontalSlider->animateHandleColor(QColor("#FFFFFF"), QColor("#ACACAC"), 100);
            ui->horizontalSlider->animateFillColor(QColor("#009FEF"), QColor("#0F6796"), 100);

            emit defaultBackgroundRestored();
        },
        Qt::UniqueConnection);
}

/// \brief 语言切换
void InterfaceSettingsMainPage::onLanguageChanged(int index)
{
    // WBLIU: 原版本
    // {
    //     // 卸载当前翻译
    //     qApp->removeTranslator(&tran);
    //     LanguageIdx = index;
    //     if (index == 0) //简体中文
    //     {
    //         tran.load(":/LanguageDemo_zh_CN.qm");
    //     } else if (index == 1) //繁体中文
    //     {
    //         tran.load(":/LanguageDemo_zh_TC.qm");
    //     } else if (index == 2) //英语
    //     {
    //         tran.load(":/LanguageDemo_en_US.qm");
    //     }
    //     qApp->installTranslator(&tran);
    //     //刷新文本
    //     ui->retranslateUi(this);
    //     emit LanguageChange();
    // }

    // 卸载当前翻译
    qApp->removeTranslator(&tran);
    LanguageIdx = index;
    if (index == 0) {
        // 简体中文
        tran.load(":/LanguageDemo_zh_CN.qm");
    } else if (index == 1) {
        // 繁体中文
        tran.load(":/LanguageDemo_zh_TC.qm");
    } else if (index == 2) {
        // 英语
        tran.load(":/LanguageDemo_en_US.qm");
    }
    qApp->installTranslator(&tran);
    // 刷新文本
    ui->retranslateUi(this);
    globalSettings->setValue("Language", index);
    emit languageChange();

}

/// \brief 刷新翻译文本
void InterfaceSettingsMainPage::LanguageSet()
{
    ui->retranslateUi(this);
}

void InterfaceSettingsMainPage::UpdateInterfaceSettingsUIInformation()
{
    // 仅在数据变更时才重建列表（登录后加载自定义壁纸等），否则仅刷新布局
    clp_background_component_view_->refreshListIfDirty();
    syncSlidersFromModel();
}

void InterfaceSettingsMainPage::syncSlidersFromModel()
{
    // 仅同步滑块值（轻量操作，不刷新壁纸列表，避免 All 路径卡顿）
    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setValue(qRound(g_user_information.local.panel_opacity_ * 100));
    ui->horizontalSlider->blockSignals(false);

    ui->horizontalSlider_2->blockSignals(true);
    ui->horizontalSlider_2->setValue(qRound(g_user_information.local.panel_blur_radius_ * 100));
    ui->horizontalSlider_2->blockSignals(false);
}

/// \brief 主题切换
void InterfaceSettingsMainPage::onThemeChanged(int index)
{
    globalSettings->setValue("Theme", index);
    // 主题切换逻辑预留，由 MainWindow 统一处理样式
    qDebug() << "Theme changed to index:" << index;
}

