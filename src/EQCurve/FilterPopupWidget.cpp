#include "EQCurve/FilterPopupWidget.h"
#include "qbuttongroup.h"
#include "qdebug.h"
#include "qgraphicseffect.h"
#include "ui_FilterPopupWidget.h"

FilterPopupWidget::FilterPopupWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilterPopupWidget)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);


    // 添加阴影
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setBlurRadius(10);                 // 模糊半径 10px
    shadow->setXOffset(0);                     // 水平偏移 0
    shadow->setYOffset(0);                     // 垂直偏移 0
    shadow->setColor(QColor(0, 0, 0, 26));    // 黑色半透明 rgba(0,0,0,0.1)
    ui->frame->setGraphicsEffect(shadow);

    // 设置样式表
    // 六个按钮
    QList<FilterToolButton*> buttons = {
        ui->tBtn_low,
        ui->tBtn_high,
        ui->tBtn_notch,
        ui->tBtn_peaking,
        ui->tBtn_lowShelving,
        ui->tBtn_highShelving
    };

    // 未选中图标（-no）
    QStringList uncheckedIcons = {
        ":/Skin/Images/Headphones/Filter/LowPass-no.png",
        ":/Skin/Images/Headphones/Filter/HighPass-no.png",
        ":/Skin/Images/Headphones/Filter/NotchFilter-no.png",
        ":/Skin/Images/Headphones/Filter/PeakingEQ-no.png",
        ":/Skin/Images/Headphones/Filter/LowShelving-no.png",
        ":/Skin/Images/Headphones/Filter/HighShelving-no.png"
    };

    // 选中图标（-ch）
    QStringList checkedIcons = {
        ":/Skin/Images/Headphones/Filter/LowPass-ch.png",
        ":/Skin/Images/Headphones/Filter/HighPass-ch.png",
        ":/Skin/Images/Headphones/Filter/NotchFilter-ch.png",
        ":/Skin/Images/Headphones/Filter/PeakingEQ-ch.png",
        ":/Skin/Images/Headphones/Filter/LowShelving-ch.png",
        ":/Skin/Images/Headphones/Filter/HighShelving-ch.png"
    };
    QStringList texts = {
        "Low Pass", "High Pass", "Notch Filter",
        "Peaking EQ", "Low Shelving", "High Shelving"
    };

    group = new QButtonGroup(this);
    group->setExclusive(true);

    for (int i = 0; i < buttons.size(); ++i) {
        FilterToolButton *btn = buttons[i];
        btn->setIcon(QIcon(uncheckedIcons[i]));
        btn->setText(texts[i]);

        btn->setStyleSheet(
            "QToolButton {"
            "   background:transparent;"
            "}"
            "QToolButton:checked {"
            "   background: #009FEF;"
            "}"
            "QToolButton:hover {"
            "   background-color: rgb(36, 43, 52);"
            "}"
            );

        btn->setIconLeftMargin(9);
        btn->setTextSpacing(14);

        btn->setCheckable(true);
        group->addButton(btn, i);  // 加入按钮组
    }

    // 连接按钮组的点击信号
    connect(group, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, [uncheckedIcons, checkedIcons, texts, this](int id) {
                // 遍历所有按钮，根据选中状态切换图标
                QList<QAbstractButton*> allBtns = group->buttons();
                for (int i = 0; i < allBtns.size(); ++i) {
                    FilterToolButton *btn = qobject_cast<FilterToolButton*>(allBtns[i]);
                    if (!btn) continue;
                    if (btn->isChecked()) {
                        btn->setIcon(QIcon(checkedIcons[i]));
                    } else {
                        btn->setIcon(QIcon(uncheckedIcons[i]));
                    }
                }

                // 处理选中事件（例如记录当前过滤器）
                if (id >= 0 && id < texts.size()) {
                    QString selected = texts[id];
                    qDebug() << "选中过滤器:" << selected;
                    emit SwitchFilter(id);
                    close();
                }
            });


}

FilterPopupWidget::~FilterPopupWidget()
{
    delete ui;
}

void FilterPopupWidget::SetCheckedBtn(QString FilterName)
{
    if (!group) return;

    // 遍历所有按钮
    QList<QAbstractButton*> buttons = group->buttons();
    for (QAbstractButton *btn : buttons) {
        if (btn->text() == FilterName) {
            btn->blockSignals(true);
            btn->setChecked(true);
            btn->blockSignals(false);
            return;   // 找到后直接返回
        }
    }
    //未找到
    buttons[0]->blockSignals(true);
    buttons[0]->setChecked(true);
    buttons[0]->blockSignals(false);
}
