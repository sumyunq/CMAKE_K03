#include "modules/DeviceSelectionPage/device_selection_main_page.h"
#include "ui_device_selection_main_page.h"

#include <QFileInfo>

DeviceSelectionMainPage::DeviceSelectionMainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeviceSelectionMainPage)
{
    ui->setupUi(this);
    // initGlobalDevicesInfo() 已移至 main.cpp 启动早期调用，此处不再重复
    InitUIInformation(); ///< 初始化UI的默认信息
    InitMember();        ///< 初始化内部成员
    InitConnect();       ///< 连接默认的信号槽
}

DeviceSelectionMainPage::~DeviceSelectionMainPage()
{
    DeSheng::DeviceRegistry::instance().save(
        DeSheng::DeviceRegistry::configFilePath(), 0);
    delete ui;
}

void DeviceSelectionMainPage::InitUIInformation()
{
    {
        clp_scrollArea_device_selection_ = new CustomQScrollAreaDeviceSelection(this);
        this->layout()->addWidget(clp_scrollArea_device_selection_);
    }
}

void DeviceSelectionMainPage::InitMember()
{
    {
        clp_scrollArea_roundbutton_ = new CustomQScrollAreaRoundbutton(2, this);
        clp_scrollArea_roundbutton_->move(rect().width() - 31, 0);
        clp_scrollArea_roundbutton_->raise();
        clp_scrollArea_roundbutton_->hide();
    }
}

void DeviceSelectionMainPage::InitConnect()
{
    clp_scrollArea_device_selection_->viewport()->installEventFilter(this);
}

void DeviceSelectionMainPage::smoothScrollBy(QScrollArea *scrollArea, int pixels)
{
    smoothScrollTo(scrollArea, scrollArea->verticalScrollBar()->value() + pixels);
}

void DeviceSelectionMainPage::smoothScrollTo(QScrollArea *scrollArea, int targetValue)
{
    QScrollBar *vScrollBar = scrollArea->verticalScrollBar();
    targetValue = qBound(vScrollBar->minimum(), targetValue, vScrollBar->maximum());
    QPropertyAnimation *animation = new QPropertyAnimation(vScrollBar, "value");
    animation->setDuration(500);
    animation->setStartValue(vScrollBar->value());
    animation->setEndValue(targetValue);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void DeviceSelectionMainPage::syncRowButtons()
{
    const int rowCount = clp_scrollArea_device_selection_->cl_rowCount_;
    const int rowHeight = 426 + 25;
    auto &btnList = clp_scrollArea_roundbutton_->cl_all_rows_CustomQPushButtonRoundButton_;
    if (btnList.size() == rowCount) return;
    qDeleteAll(btnList);
    btnList.clear();
    for (int i = 0; i < rowCount; ++i) {
        auto *btn = new CustomQPushButtonRoundButton();
        int targetValue = i * rowHeight;
        QObject::connect(btn, &QPushButton::toggled, this, [this, targetValue](bool checked) {
            if (checked) smoothScrollTo(clp_scrollArea_device_selection_, targetValue);
        });
        btnList.append(btn);
    }
    clp_scrollArea_roundbutton_->updateView();
    if (!btnList.isEmpty())
        clp_scrollArea_roundbutton_->cl_buttonGroup_->button(0)->setChecked(true);
}
DeSheng::DeviceInfo DeviceSelectionMainPage::cl_selected_device_information() const
{
    return cl_selected_device_information_;
}

void DeviceSelectionMainPage::setCl_selected_device_information(
    const DeSheng::DeviceInfo &newCl_selected_device_information)
{
    cl_selected_device_information_ = newCl_selected_device_information;
}

void DeviceSelectionMainPage::switchButton(int delta) {}

void DeviceSelectionMainPage::resizeEvent(QResizeEvent *event)
{
    clp_scrollArea_device_selection_->updateView();

    // 设备列表超过 1 行时才显示右侧按键区域
    if (clp_scrollArea_device_selection_->cl_rowCount_ > 1) {
        clp_scrollArea_roundbutton_->setGeometry(rect().width() - 31, 0, 8, rect().height());
        clp_scrollArea_roundbutton_->raise();
        clp_scrollArea_roundbutton_->show();
        syncRowButtons(); // 按行数同步右侧导航按键
    } else {
        clp_scrollArea_roundbutton_->hide();
    }

    QWidget::resizeEvent(event);
}

bool DeviceSelectionMainPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == clp_scrollArea_device_selection_->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        cl_wheel_accumulated_angle_ += wheelEvent->angleDelta().y();

        auto *buttonGroup = clp_scrollArea_roundbutton_->cl_buttonGroup_;
        if (!buttonGroup)
            return true;

        int currentId = buttonGroup->checkedId();
        if (currentId < 0) {
            // 没有选中任何按钮，默认选中第一个
            if (!buttonGroup->buttons().isEmpty()) {
                buttonGroup->buttons().first()->setChecked(true);
            }
            return true;
        }

        int total = buttonGroup->buttons().size();

        if (cl_wheel_accumulated_angle_ >= 120) {
            cl_wheel_accumulated_angle_ = 0;
            // smoothScrollBy(clp_scrollArea_device_selection_,
            //                -451); // 滚轮向上 → scrollBar 值减小 → 内容上移
            {
                int targetId = qMax(0, currentId - 1);
                if (buttonGroup->button(targetId)) {
                    buttonGroup->button(targetId)->setChecked(true);
                }
            }

        } else if (cl_wheel_accumulated_angle_ <= -120) {
            cl_wheel_accumulated_angle_ = 0;
            // smoothScrollBy(clp_scrollArea_device_selection_,
            //                451); // 滚轮向下 → scrollBar 值增大 → 内容下移

            {
                int targetId = qMin(currentId + 1, total - 1);
                if (buttonGroup->button(targetId)) {
                    buttonGroup->button(targetId)->setChecked(true);
                }
            }
        }

        return true; // 拦截事件，阻止默认滚动行为
    }
    return QWidget::eventFilter(watched, event);
}