#include "modules/UserSetting/UserSettingCustomUI/custom_QScrollArea_topbuttons.h"

#include <QTimer>

CustomQScrollAreaTopButtons::CustomQScrollAreaTopButtons(QWidget *parent)
    : QScrollArea(parent)
{
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽

    updateLayoutView();
}

CustomQScrollAreaTopButtons::~CustomQScrollAreaTopButtons() {}

/// @brief 动画属性 setter，驱动浮动指示器 widget 的几何变更
void CustomQScrollAreaTopButtons::setIndicatorRect(const QRect &r)
{
    if (cl_indicator_rect_ == r)
        return;
    cl_indicator_rect_ = r;
    if (clp_indicator_widget_) {
        clp_indicator_widget_->setGeometry(r);
        clp_indicator_widget_->show();
        clp_indicator_widget_->lower(); ///< 置于按钮底层
    }
}

/// @brief 将按钮几何从 content_widget 坐标映射到 viewport 坐标系
QRect CustomQScrollAreaTopButtons::calcIndicatorRect(QAbstractButton *btn) const
{
    if (!btn || !clp_content_widget_)
        return {};

    const QRect btnGeo = btn->geometry();
    // 从 content_widget 坐标映射到 viewport
    const QPoint vpPos = clp_content_widget_->mapTo(viewport(), btnGeo.topLeft());

    const int sideMargin = 0;  ///< 左右缩进
    return {vpPos.x() + sideMargin,
            vpPos.y(),         ///< 与按钮顶部对齐
            btnGeo.width() - 2 * sideMargin,
            btnGeo.height ()};
}

/// @brief 移动选中指示器到目标按钮底部
/// @param btn      目标按钮指针
/// @param animated 是否启用平移动画
void CustomQScrollAreaTopButtons::updateIndicatorForButton(QAbstractButton *btn,
                                                           bool animated)
{
    if (!btn || !clp_indicator_widget_)
        return;

    const QRect target = calcIndicatorRect(btn);
    if (!target.isValid())
        return;

    clp_indicator_widget_->lower(); ///< 置于按钮底层

    if (animated && clp_indicator_widget_->isVisible() && clp_indicator_anim_) {
        clp_indicator_anim_->stop();
        clp_indicator_anim_->setStartValue(clp_indicator_widget_->geometry());
        clp_indicator_anim_->setEndValue(target);
        clp_indicator_anim_->start();
    } else {
        if (clp_indicator_anim_)
            clp_indicator_anim_->stop();
        clp_indicator_widget_->setGeometry(target);
        clp_indicator_widget_->show();
    }

    cl_indicator_rect_ = target;
}

/// @brief 根据 cl_all_settings_type_ 刷新布局和按钮组
void CustomQScrollAreaTopButtons::updateLayoutView(int defaultIndex)
{
    if (!clp_hBoxLayout_ || !clp_all_settings_type_buttonGroup_)
        return;

    // 移除布局项（不删除 widget，由 cl_all_settings_type_ 持有）
    QLayoutItem *item;
    while ((item = clp_hBoxLayout_->takeAt(0)) != nullptr) {
        delete item;
    }

    // 移除按钮组中的按钮（不删除 widget）
    const QList<QAbstractButton *> buttons
        = clp_all_settings_type_buttonGroup_->buttons();
    for (QAbstractButton *btn : buttons) {
        clp_all_settings_type_buttonGroup_->removeButton(btn);
    }

    if (cl_all_settings_type_.size() == 0)
        return;

    // clp_hBoxLayout_->addStretch();
    for (int i = 0; i < cl_all_settings_type_.size(); ++i) {
        auto *btn = cl_all_settings_type_.at(i);
        if (btn->cl_is_show()) {
            btn->show();
            clp_all_settings_type_buttonGroup_->addButton(btn, i);
            clp_hBoxLayout_->addWidget(btn);
        } else {
            btn->hide();
        }
    }
    // clp_hBoxLayout_->addStretch();

    // 强制布局激活，确保按钮 geometry 有效
    clp_hBoxLayout_->activate();

    // 按索引选中默认按钮（ID 即 cl_all_settings_type_ 下标）
    if (defaultIndex >= 0 && defaultIndex < cl_all_settings_type_.size()) {
        auto *targetBtn = qobject_cast<CustomQPushButtonSingleSettingsType *>(
            clp_all_settings_type_buttonGroup_->button(defaultIndex));
        if (targetBtn && targetBtn->cl_is_show()) {
            targetBtn->setChecked(true);
            updateIndicatorForButton(targetBtn, false); ///< 初始定位，无动画
        }
    }

    update();
}

/// @brief 响应按钮点击，动画移动指示器并发射切换信号
void CustomQScrollAreaTopButtons::onSettingsTypeClicked(int index)
{
    QAbstractButton *btn = clp_all_settings_type_buttonGroup_->button(index);
    if (btn)
        updateIndicatorForButton(btn, true); ///< 动画移动指示器

    emit changeSettingsType(index);
}

/// @brief 重写 QScrollArea::paintEvent
/// 指示器由浮动子 widget（clp_indicator_widget_）渲染，
/// 挂载在 viewport 上，通过 QPropertyAnimation 驱动移动。
void CustomQScrollAreaTopButtons::paintEvent(QPaintEvent *event)
{
    QScrollArea::paintEvent(event);
}

/// @brief 窗口大小变化时，延迟重定位指示器以等待布局更新完成
void CustomQScrollAreaTopButtons::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    QTimer::singleShot(0, this, [this]() {
        if (!clp_hBoxLayout_)
            return;
        clp_hBoxLayout_->activate();
        QAbstractButton *t_checked
            = clp_all_settings_type_buttonGroup_->checkedButton();
        if (t_checked)
            updateIndicatorForButton(t_checked, false);
    });
}

/// @brief 初始化UI默认信息，创建按钮、布局和浮动指示器
void CustomQScrollAreaTopButtons::InitUIInformation()
{
    this->setStyleSheet(R"(
        QScrollArea {
            background: transparent;
            border: none;
        }
    )");

    clp_content_widget_ = new QWidget(this);
    clp_content_widget_->setStyleSheet(R"(
        QWidget {
            background: transparent;
            border: none;
        }
    )");

    clp_hBoxLayout_ = new QHBoxLayout(clp_content_widget_);
    clp_hBoxLayout_->setSpacing(cl_spacing_);
    clp_hBoxLayout_->setContentsMargins(cl_left_margin_, cl_top_margin_,
                                        cl_right_margin_, cl_bottom_margin_);

    clp_all_settings_type_buttonGroup_ = new QButtonGroup(this);
    clp_all_settings_type_buttonGroup_->setExclusive(true);

    this->setWidget(clp_content_widget_);
    this->setWidgetResizable(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    clp_content_widget_->setSizePolicy(QSizePolicy::Expanding,
                                       QSizePolicy::Preferred);

    // 创建指示器浮动 widget（挂载在 viewport 上，置于按钮底层）
    clp_indicator_widget_ = new QWidget(viewport());
    clp_indicator_widget_->setStyleSheet(
        "background: #009FEF;border-radius: 6px;");
    clp_indicator_widget_->hide();

    clp_indicator_anim_ = new QPropertyAnimation(this, "cl_indicatorRect", this);
    clp_indicator_anim_->setDuration(300);
    clp_indicator_anim_->setEasingCurve(QEasingCurve::OutCubic);

    // 创建五个设置类型按键
    for (int var = 0; var < 5; ++var) {
        auto *t_btn = new CustomQPushButtonSingleSettingsType(clp_content_widget_);

        switch (var) {
        case 0: t_btn->setCl_settings_type(tr("个人中心")); break;
        case 1: t_btn->setCl_settings_type(tr("系统设置")); break;
        case 2: t_btn->setCl_settings_type(tr("界面设置")); break;
        case 3: t_btn->setCl_settings_type(tr("版本升级")); break;
        case 4: t_btn->setCl_settings_type(tr("联系我们")); break;
        default: t_btn->setCl_settings_type(tr("NULL")); break;
        }

        t_btn->setCl_is_show(true);
        t_btn->setCheckable(true);
        cl_all_settings_type_.append(t_btn);
    }
}

void CustomQScrollAreaTopButtons::InitMember() {}

void CustomQScrollAreaTopButtons::InitConnect()
{
    connect(clp_all_settings_type_buttonGroup_,
            &QButtonGroup::idClicked,
            this,
            &CustomQScrollAreaTopButtons::onSettingsTypeClicked,
            Qt::UniqueConnection);
}

void CustomQScrollAreaTopButtons::LanguageSet()
{
    for (int i = 0; i < cl_all_settings_type_.size(); ++i)
    {
        switch (i) {
            case 0: cl_all_settings_type_.at(i)->setCl_settings_type(tr("个人中心")); break;
            case 1: cl_all_settings_type_.at(i)->setCl_settings_type(tr("系统设置")); break;
            case 2: cl_all_settings_type_.at(i)->setCl_settings_type(tr("界面设置")); break;
            case 3: cl_all_settings_type_.at(i)->setCl_settings_type(tr("版本升级")); break;
            case 4: cl_all_settings_type_.at(i)->setCl_settings_type(tr("联系我们")); break;
            default: cl_all_settings_type_.at(i)->setCl_settings_type(tr("NULL")); break;
        }
         cl_all_settings_type_.at(i)->adjustSize();
    }
}





