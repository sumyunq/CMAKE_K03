#include "modules/DeviceSelectionPage/DeviceSelectionPageCustomUI/custom_QWidget_single_device_information.h"
#include "ui_custom_QWidget_single_device_information.h"

#include "modules/Common/DeviceRegistry.h" ///< DeSheng::DeviceRegistry

CustomQWidgetSingleDeviceInfo::CustomQWidgetSingleDeviceInfo(QWidget *parent, bool showExtraTags)
    : FrostedPanel(parent)
    , ui(new Ui::CustomQWidgetSingleDeviceInfo)
{
    ui->setupUi(this);
    cl_extra_tags_is_show_ = showExtraTags;
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

CustomQWidgetSingleDeviceInfo::CustomQWidgetSingleDeviceInfo(const QString deviceTypeName,
                                                             QWidget *parent,
                                                             bool showExtraTags)
    : FrostedPanel(parent)
    , ui(new Ui::CustomQWidgetSingleDeviceInfo)
{
    ui->setupUi(this);
    cl_extra_tags_is_show_ = showExtraTags;
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽

    cl_device_info_.DeviceTypeName = deviceTypeName;
}

CustomQWidgetSingleDeviceInfo::~CustomQWidgetSingleDeviceInfo()
{
    delete ui;
}

void CustomQWidgetSingleDeviceInfo::updatePushButtonScrollArea()
{
    // 取决于设备名称
    // T10
    if (cl_device_info_.DeviceTypeName.contains("T10")) {
        // 无线
        if (cl_device_info_.DeviceTypeName.contains("wireless")
            || cl_device_info_.DeviceTypeName.contains("无线")) {
            updatePushButtonList("T10无线");

            // 有线
        } else {
            updatePushButtonList("T10有线");
        }
    }
    // K03S
    if (cl_device_info_.DeviceTypeName.contains("K03S")) {
        // K03S超竞版
        if (cl_device_info_.DeviceTypeName.contains("超竞版")) {
            updatePushButtonList("K03S超竞版");
            // K03S
        } else {
            updatePushButtonList("K03S");
        }
    }

    // K03 有线版
    if (cl_device_info_.DeviceTypeName.contains("K03")) {
            updatePushButtonList("K03有线版");
    }

    // K06S
    if (cl_device_info_.DeviceTypeName.contains("K06S")) {
        updatePushButtonList("K06S");
    }

    // T7 GT
    if (cl_device_info_.DeviceTypeName.contains("T7 GT"))
    {
        updatePushButtonList("T7 GT");
    // T7
    } else if (cl_device_info_.DeviceTypeName.contains("T7")) {
        updatePushButtonList("T7");
    }

    // S21无线智充版
    if (cl_device_info_.DeviceTypeName.contains("S21")) {
        updatePushButtonList("S21无线智充版");
    }
}

void CustomQWidgetSingleDeviceInfo::updatePushButtonList(QString device_info_DeviceTypeName)
{
    // 按键列表置空
    qDeleteAll(cl_color_pushButtons_scrollArea_->cl_all_color_pushButton_list_);
    cl_color_pushButtons_scrollArea_->cl_all_color_pushButton_list_.clear();

    // 根据全局 hash 添加
    auto &t_map = DeSheng::DeviceRegistry::instance().deviceMap();
    auto it = t_map.begin();
    while (it != t_map.end()) {
        QPair<QString, int> key_device_type_colorIndex = it.key();
        std::shared_ptr<DeSheng::DeviceInfo> infoPtr = it.value();

        // 匹配设备型号
        if (key_device_type_colorIndex.first == device_info_DeviceTypeName) {
            CustomQPushButtonSingleDeviceColor *t_pBn = new CustomQPushButtonSingleDeviceColor(
                cl_color_pushButtons_scrollArea_->cl_content_widget_,
                infoPtr->DeviceColorRGB,
                infoPtr->DeviceColorRGB,
                infoPtr->DeviceColorRGB);
            t_pBn->cl_device_info_ = infoPtr; // 直接赋值 shared_ptr
            cl_color_pushButtons_scrollArea_->cl_all_color_pushButton_list_.append(t_pBn);
        }
        ++it;
    }
    cl_color_pushButtons_scrollArea_->updateView();

    QObject::disconnect(cl_color_pushButtons_scrollArea_->cl_all_color_pushButton_buttonGroup_,
                        nullptr,
                        this,
                        nullptr);
    // 绑定按键组 点击事件
    QObject::connect(cl_color_pushButtons_scrollArea_->cl_all_color_pushButton_buttonGroup_,
                     QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled),
                     this,
                     [this](QAbstractButton *button, bool checked) {
                         // if (!checked)
                         //     return; // 只处理选中事件，忽略取消选中

                         // 转换为自定义按钮类型
                         CustomQPushButtonSingleDeviceColor *colorBtn
                             = qobject_cast<CustomQPushButtonSingleDeviceColor *>(button);

                         if (colorBtn && colorBtn->cl_device_info_) {
                             {
                                 // 更新状态( 从按键的信息体中拿取有用的信息 )
                                 colorBtn->cl_device_info_->isChecked = checked;
                             }

                             // 如果选中
                             if (checked) {
                                 // 获取颜色信息
                                 QString colorName = colorBtn->cl_device_info_->DeviceColorName;
                                 cl_text_device_info_DeviceColorName_->setText(colorName);
                                 cl_text_device_info_DeviceTypeName_->setText(
                                     colorBtn->cl_device_info_->DeviceTypeName);
                                 updateTextLayout(); // 文本变化后,重新按文本自适应并居中

                                 cl_device_info_.DeviceHomePagePixmapPath
                                     = colorBtn->cl_device_info_->DeviceHomePagePixmapPath;// 首页耳机图片
                                 cl_device_info_.DeviceHomePageTopLeftPixmapPath_normal
                                     = colorBtn->cl_device_info_
                                           ->DeviceHomePageTopLeftPixmapPath_normal;// 首页左上角耳机 正常状态
                                 cl_device_info_.DeviceHomePageTopLeftPixmapPath_abnormal
                                     = colorBtn->cl_device_info_
                                           ->DeviceHomePageTopLeftPixmapPath_abnormal;// 首页左上角耳机 异常状态
                                 cl_device_info_.DeviceMoreSetPixmapPath
                                     = colorBtn->cl_device_info_->DeviceMoreSetPixmapPath;// 设置页面 耳机 图片
                                 cl_device_info_.DeviceMoreSetQrCodePixmapPath
                                     = colorBtn->cl_device_info_
                                           ->DeviceMoreSetQrCodePixmapPath; // 设置页面 社区二维码
                                 cl_device_info_.DeviceManualUrl
                                     = colorBtn->cl_device_info_
                                           ->DeviceManualUrl; // 设备说明书 URL（首页说明书按钮）

                                 cl_device_pixmap_
                                     = QPixmap(colorBtn->cl_device_info_->DeviceColorPixmapPath)
                                           .scaled(cl_device_pixmap_default_size_
                                                       * cl_current_scaling_factor_,
                                                   Qt::KeepAspectRatio, // 保持比例
                                                   Qt::SmoothTransformation);
                                 cl_device_pixmap_show_label_->setPixmap(cl_device_pixmap_);
                                 update();
                             }
                         }
                     });
}

void CustomQWidgetSingleDeviceInfo::setCheckedDevice(int index)
{
    auto *buttonGroup = cl_color_pushButtons_scrollArea_->cl_all_color_pushButton_buttonGroup_;
    if (buttonGroup && !buttonGroup->buttons().isEmpty()) {
        if (index == -1) {
            // 遍历按键,按配置文件中是否选中来设置
            QList<QAbstractButton *> buttons = buttonGroup->buttons();
            for (int i = 0; i < buttons.size(); ++i) {
                QAbstractButton *btn = buttons.at(i);
                // 转换为自定义按钮类型
                CustomQPushButtonSingleDeviceColor *colorBtn
                    = qobject_cast<CustomQPushButtonSingleDeviceColor *>(btn);

                if (colorBtn && colorBtn->cl_device_info_->isChecked) {
                    colorBtn->setChecked(true);
                }
            }
        } else {
            // 指定 index 按键为选中状态
            buttonGroup->buttons().at(index)->setChecked(true);
        }
    }
    update();
}

// void CustomQWidgetSingleDeviceInfo::showHintWidget(QPoint pBn_centre_point)
// {
//     if (!cl_hint_widget_) {
//         return;
//     }
//     // cl_hint_widget_->setGeometry(this->mapFromGlobal(pBn_centre_point), );
// }

void CustomQWidgetSingleDeviceInfo::InitUIInformation()
{
    {
        this->setMinimumSize(344, 426);
        this->setAttribute(Qt::WA_StyledBackground, true);
        this->setObjectName("customQWidgetSingleDeviceInfo_");
        this->setStyleSheet(R"(
    QWidget#customQWidgetSingleDeviceInfo_{
        border-radius: 10px;
        background: rgba(81, 96, 122, 0.2);
    }
        )");
        this->setCornerRadius(10);
    }
    {
        // 电池图标
        cl_battery_icon_ = new QLabel(this);
        cl_battery_icon_->setMinimumSize(cl_battery_icon_size_);
        cl_battery_icon_->move(cl_battery_icon_point_);
        cl_battery_icon_->setStyleSheet(R"()");
    }
    {
        // 设备图片（显示）
        cl_device_pixmap_show_label_ = new QLabel(this);
        cl_device_pixmap_show_label_->setMinimumSize(cl_device_pixmap_label_default_size_);
        cl_device_pixmap_show_label_->move(cl_device_pixmap_default_point_);
    }
    {
        // 机型图片（光标位置判断）
        cl_device_pixmap_label_ = new QLabel(this);
        cl_device_pixmap_label_->setMinimumSize(cl_device_pixmap_label_default_size_);
        cl_device_pixmap_label_->move(cl_device_pixmap_label_default_point_);
        cl_device_pixmap_label_->installEventFilter(this);          // 安装事件过滤器
        cl_device_pixmap_label_->setCursor(Qt::PointingHandCursor); // 手型光标
    }
    {
        // 设备名称(宽度按文本自适应,整体水平居中见 updateTextLayout)
        cl_text_device_info_DeviceTypeName_ = new QLabel(this);
        cl_text_device_info_DeviceTypeName_->setAlignment(Qt::AlignRight
                                                          | Qt::AlignVCenter); // 文本靠右,垂直居中
        cl_text_device_info_DeviceTypeName_->setStyleSheet(R"(
          QLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 22px;
                color: #A1A8B3;
                border: none;
            }
        )");
    }
    {
        //额外标签 2.0
        // 颜色中文名称
        cl_text_device_info_ExtraTags_ = new QLabel(this);
        cl_text_device_info_ExtraTags_->setStyleSheet(R"(
          QLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #FFCD7C;
                border: none;
            }
        )");
        cl_text_device_info_ExtraTags_->setText(tr("2.0"));
    }
    {
        // 颜色中文名称
        cl_text_device_info_DeviceColorName_ = new QLabel(this);
        cl_text_device_info_DeviceColorName_->setStyleSheet(R"(
          QLabel {
                font-family: "Noto Sans S Chinese";
                font-weight: 500;
                font-size: 12px;
                color: #A1A8B3;
                border: none;
            }
        )");
    }
    {
        // 按键滚动区域
        cl_color_pushButtons_scrollArea_ = new CustomQScrollAreaColorPushButtons(this);
        cl_color_pushButtons_scrollArea_->setFixedSize(cl_color_pushButtons_scrollArea_size_);
        cl_color_pushButtons_scrollArea_->move(cl_color_pushButtons_scrollArea_point_);
        cl_color_pushButtons_scrollArea_->setStyleSheet(R"(
            QScrollArea {
            background: transparent;
            border: none;
            }
        )");

        // border: none;
        //     background-color: green;
    }
}

void CustomQWidgetSingleDeviceInfo::InitMember()
{
    {
        cl_size_anim_ = std::make_unique<QVariantAnimation>(this);
        cl_size_anim_->setDuration(200); // 动画时间
    }
    {
        cl_point_anim_ = std::make_unique<QVariantAnimation>(this);
        cl_point_anim_->setDuration(200); // 动画时间
    }
    {
        cl_scaling_factor_anim_ = std::make_unique<QVariantAnimation>(this);
        cl_scaling_factor_anim_->setDuration(200); // 动画时间
    }
    {
        cl_parallel_group_ = new QParallelAnimationGroup(this);
        // 添加动画（同时执行）
        cl_parallel_group_->addAnimation(cl_size_anim_.get());
        cl_parallel_group_->addAnimation(cl_point_anim_.get());
        cl_parallel_group_->addAnimation(cl_scaling_factor_anim_.get());
    }
}

void CustomQWidgetSingleDeviceInfo::InitConnect()
{
    // 尺寸
    connect(cl_size_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_device_pixmap_label_current_size_ = value.value<QSize>();
                update();
            });
    // 位置
    connect(cl_point_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_device_pixmap_label_current_point_ = value.value<QPoint>();
                update();
            });
    // 图片放大系数
    connect(cl_scaling_factor_anim_.get(),
            &QVariantAnimation::valueChanged,
            this,
            [this](const QVariant &value) {
                cl_current_scaling_factor_ = value.value<double>();
                update();
            });
    // // 按键状态变化时
    // connect(this, &QPushButton::toggled, this, [this](bool checked) {
    //     if (checked) {
    //         cl_size_anim_->setKeyValues({{0.0, rect().size()}, {1.0, cl_checked_size_}});
    //         cl_color_anim_->setKeyValues({{0.0, cl_default_color_}, {1.0, cl_checked_color_}});

    //     } else {
    //         cl_size_anim_->setKeyValues({{0.0, rect().size()}, {1.0, cl_default_size_}});
    //         cl_color_anim_->setKeyValues({{0.0, cl_checked_color_}, {1.0, cl_default_color_}});
    //     }
    //     // 启动动画
    //     cl_parallel_group_->start();
    // });
}

void CustomQWidgetSingleDeviceInfo::updateTextLayout()
{
    // 三个文字控件宽度均依据文本内容自适应,整体水平居中:
    //   设备名称 | 4px | 额外标签(2.0) | 14px | 颜色中文名称
    // 额外标签隐藏时,颜色名称保持与设备名称间距 14px
    const int t_type_w = cl_text_device_info_DeviceTypeName_->sizeHint().width();
    const int t_extra_w = cl_extra_tags_is_show_
                              ? cl_text_device_info_ExtraTags_->sizeHint().width()
                              : 0;
    const int t_color_w = cl_text_device_info_DeviceColorName_->sizeHint().width();

    const int t_gap_extra = cl_extra_tags_is_show_ ? 4 : 0; // 设备名称 与 额外标签 间距
    const int t_gap_color = 14;                             // (额外标签/设备名称) 与 颜色名称 间距

    // 整体水平居中;文本总宽超过控件宽度时左对齐,防止越界到负坐标
    const int t_total_w = t_type_w + t_gap_extra + t_extra_w + t_gap_color + t_color_w;
    const int t_start_x = qMax(0, (width() - t_total_w) / 2);

    cl_text_device_info_DeviceTypeName_->setGeometry(t_start_x,
                                                     cl_text_device_info_DeviceTypeName_point_.y(),
                                                     t_type_w,
                                                     cl_text_device_info_DeviceTypeName_size_.height());

    const int t_x_after_type = t_start_x + t_type_w + t_gap_extra;
    if (cl_extra_tags_is_show_) {
        cl_text_device_info_ExtraTags_->setGeometry(t_x_after_type,
                                                    cl_text_device_info_ExtraTags_point_.y(),
                                                    t_extra_w,
                                                    cl_text_device_info_ExtraTags_size_.height());
    }
    cl_text_device_info_ExtraTags_->setVisible(cl_extra_tags_is_show_);

    cl_text_device_info_DeviceColorName_->setGeometry(t_x_after_type + t_extra_w + t_gap_color,
                                                      cl_text_device_info_DeviceColorName_point_.y(),
                                                      t_color_w,
                                                      cl_text_device_info_DeviceColorName_size_.height());
}

void CustomQWidgetSingleDeviceInfo::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // 1. 抗锯齿（使边缘平滑）
    // 2. 文本抗锯齿（使文字边缘平滑）
    // 3. 平滑图片变换（缩放图片时更平滑）
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                           | QPainter::SmoothPixmapTransform);


    // // 绘制图片
    // painter.drawPixmap(cl_device_pixmap_current_point_.x(),
    //                    cl_device_pixmap_current_point_.y(),
    //                    cl_device_pixmap_current_size_.width(),
    //                    cl_device_pixmap_current_size_.height(),
    //                    cl_device_pixmap_);
    if (!cl_device_pixmap_.isNull()) {
        cl_device_pixmap_label_->setGeometry(cl_device_pixmap_label_current_point_.x(),
                                             cl_device_pixmap_label_current_point_.y(),
                                             cl_device_pixmap_label_current_size_.width(),
                                             cl_device_pixmap_label_current_size_.height());

        cl_device_pixmap_show_label_->setGeometry(cl_device_pixmap_default_rect_.center().x()
                                                      - cl_device_pixmap_default_size_.width() / 2
                                                            * cl_current_scaling_factor_,
                                                  cl_device_pixmap_default_rect_.center().y()
                                                      - cl_device_pixmap_default_size_.height() / 2
                                                            * cl_current_scaling_factor_,
                                                  cl_device_pixmap_default_size_.width()
                                                      * cl_current_scaling_factor_,
                                                  cl_device_pixmap_default_size_.height()
                                                      * cl_current_scaling_factor_);
        cl_device_pixmap_show_label_->setPixmap(
            cl_device_pixmap_.scaled(cl_device_pixmap_default_size_ * cl_current_scaling_factor_,
                                     Qt::KeepAspectRatio, // 保持比例
                                     Qt::SmoothTransformation));

        // //  绘制实际边框
        // painter.setPen(Qt::NoPen);
        // painter.setBrush(Qt::green);
        // painter.drawRect(QRect(cl_device_pixmap_label_current_point_.x(),
        //                        cl_device_pixmap_label_current_point_.y(),
        //                        cl_device_pixmap_label_current_size_.width(),
        //                        cl_device_pixmap_label_current_size_.height()));

        // painter.setPen(Qt::NoPen);
        // painter.setBrush(Qt::blue);
        // painter.drawRect(QRect(cl_device_pixmap_default_rect_.center().x()
        //                            - cl_device_pixmap_default_size_.width() / 2
        //                                  * cl_current_scaling_factor_,
        //                        cl_device_pixmap_default_rect_.center().y()
        //                            - cl_device_pixmap_default_size_.height() / 2
        //                                  * cl_current_scaling_factor_,
        //                        cl_device_pixmap_default_size_.width()
        //                            * cl_current_scaling_factor_,
        //                        cl_device_pixmap_default_size_.height()
        //                            * cl_current_scaling_factor_));

        // // 绘制图片
        // painter.drawPixmap(cl_device_pixmap_default_rect_.center().x()
        //                        - cl_device_pixmap_default_size_.width() / 2
        //                              * cl_current_scaling_factor_,
        //                    cl_device_pixmap_default_rect_.center().y()
        //                        - cl_device_pixmap_default_size_.height() / 2
        //                              * cl_current_scaling_factor_,
        //                    cl_device_pixmap_default_size_.width() * cl_current_scaling_factor_,
        //                    cl_device_pixmap_default_size_.height() * cl_current_scaling_factor_,
        //                    cl_device_pixmap_.scaled(cl_device_pixmap_default_size_
        //                                                 * cl_current_scaling_factor_,
        //                                             Qt::KeepAspectRatio, // 保持比例
        //                                             Qt::SmoothTransformation));
    }

    FrostedPanel::paintEvent(event);
}

void CustomQWidgetSingleDeviceInfo::resizeEvent(QResizeEvent *event)
{
    {
        // 电池图标
        cl_battery_icon_->setGeometry(cl_battery_icon_point_.x(),
                                      cl_battery_icon_point_.y(),
                                      cl_battery_icon_size_.width(),
                                      cl_battery_icon_size_.height());
    }
    {
        // 机型图片
        cl_device_pixmap_label_->setGeometry(cl_device_pixmap_label_current_point_.x(),
                                             cl_device_pixmap_label_current_point_.y(),
                                             cl_device_pixmap_label_current_size_.width(),
                                             cl_device_pixmap_label_current_size_.height());
    }
    {
        // 设备名称
        cl_text_device_info_DeviceTypeName_
            ->setGeometry(cl_text_device_info_DeviceTypeName_point_.x(),
                          cl_text_device_info_DeviceTypeName_point_.y(),
                          cl_text_device_info_DeviceTypeName_size_.width(),
                          cl_text_device_info_DeviceTypeName_size_.height());
    }
    {
        // 文字行: 宽度依据文本内容自适应,整体水平居中
        updateTextLayout();
    }
    {
        // 按键滚动区域
        cl_color_pushButtons_scrollArea_->setGeometry(cl_color_pushButtons_scrollArea_point_.x(),
                                                      cl_color_pushButtons_scrollArea_point_.y(),
                                                      cl_color_pushButtons_scrollArea_size_.width(),
                                                      cl_color_pushButtons_scrollArea_size_.height());
    }

    FrostedPanel::resizeEvent(event);
}

bool CustomQWidgetSingleDeviceInfo::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == cl_device_pixmap_label_) {
        switch (event->type()) {
        case QEvent::MouseButtonPress: {
            // 处理点击事件
            emit sendSignalsDeviceInfo(cl_device_info_); ///< 点击时，发送该设备相关信息
            // qDebug() << "设备图片被点击";
            break;
        }
        case QEvent::Enter: {
            // qDebug() << "Enter";
            cl_size_anim_->setKeyValues({{0.0, cl_device_pixmap_label_current_size_},
                                         {1.0, cl_device_pixmap_label_enlarge_size_}});
            cl_point_anim_->setKeyValues({{0.0, cl_device_pixmap_label_current_point_},
                                          {1.0, cl_device_pixmap_label_enlarge_point_}});
            cl_scaling_factor_anim_->setKeyValues(
                {{0.0, cl_current_scaling_factor_}, {1.0, cl_target_scaling_factor_}});
            cl_parallel_group_->start();
            break;
        }
        case QEvent::Leave: {
            // qDebug() << "Leave";
            cl_size_anim_->setKeyValues({{0.0, cl_device_pixmap_label_current_size_},
                                         {1.0, cl_device_pixmap_label_default_size_}});
            cl_point_anim_->setKeyValues({{0.0, cl_device_pixmap_label_current_point_},
                                          {1.0, cl_device_pixmap_label_default_point_}});
            cl_scaling_factor_anim_->setKeyValues(
                {{0.0, cl_current_scaling_factor_}, {1.0, cl_default_scaling_factor_}});
            cl_parallel_group_->start();
            break;
        }
        default:
            break;
        }
    }
    return FrostedPanel::eventFilter(watched, event);
}
