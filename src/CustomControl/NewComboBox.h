#ifndef NEWCOMBOBOX_H
#define NEWCOMBOBOX_H

#include "qevent.h"
#include <QComboBox>
#include <QStylePainter>
#include <QStyleOptionComboBox>
#include <QGraphicsDropShadowEffect>
#include <QListView>
#include <QFrame>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>
#include "modules/Common/elide_text.h"  ///< DeSheng::elideTextWithDots

class NewComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit NewComboBox(QWidget* parent = nullptr)
        : QComboBox(parent)
        , m_popupOffsetX(0)
        , m_popupOffsetY(0)
        , m_popup(nullptr)
    {
        // 默认隐藏滚动条（内部 listView 由外部 NoSelectListView 接管）
        view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        // 在 NewComboBox 构造函数中添加（或外部连接）
        connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, [this]() { update(); });
    }

    void setDisplayIcon(const QIcon &icon) {
        m_icon = icon;
        update();
    }
    //下拉框弹窗的位置
    void setPopupOffsetXY(int offset_x, int offset_y) {
        m_popupOffsetX = offset_x;
        m_popupOffsetY = offset_y;
    }

    // 外部设置阴影容器（在 M_SetCBoxShadow 中调用）
    void setPopupContainer(QWidget* container) {
        m_popup = container;
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QStylePainter painter(this);
        painter.setPen(palette().color(QPalette::Text));

        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        painter.drawComplexControl(QStyle::CC_ComboBox, opt);

        QRect textRect = style()->subControlRect(
            QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);

        // ---------- 获取当前项的图标 ----------
        QIcon currentIcon;
        const int idx = currentIndex();
        if (idx >= 0 && idx < model()->rowCount()) {
            if (!m_icon.isNull()) {
                currentIcon = m_icon;
            } else {
                QVariant decoration = model()->data(model()->index(idx, 0), Qt::DecorationRole);
                if (decoration.canConvert<QIcon>()) {
                    currentIcon = decoration.value<QIcon>();
                }
            }
        }

        // ---------- 绘制图标（固定 32 x 32）----------
        if (!currentIcon.isNull()) {
            const int iconSize = 32;                        // 固定尺寸
            // 确保图标不超出编辑区域高度（若控件太矮则适当缩小）
            int actualSize = qMin(iconSize, textRect.height() - 4);
            if (actualSize < iconSize) actualSize = iconSize; // 如果高度不够也会强制绘制（可能轻微溢出）

            QRect iconRect(textRect.x() + 0,
                           textRect.y() + (textRect.height() - actualSize) / 2,
                           actualSize, actualSize);
            QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
            currentIcon.paint(&painter, iconRect, Qt::AlignCenter, mode);
            // 将文本起始位置移到图标右侧
            textRect.setX(iconRect.right() + 6);
        }

        // ---------- 绘制文本 ----------
        QString text = currentText();
        text = DeSheng::elideTextWithDots(text, painter.font(), textRect.width());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }

    void showPopup() override
    {
        if (!m_popup) {
            m_popup = view()->parentWidget();
        }
        if (!m_popup) {
            QComboBox::showPopup();
            return;
        }

        QListView *listView = qobject_cast<QListView*>(view());
        if (listView && currentIndex() >= 0) {
            QModelIndex idx = model()->index(currentIndex(), 0);
            listView->setCurrentIndex(idx);
            listView->selectionModel()->select(idx, QItemSelectionModel::Select);
            //强制滚动到当前项，并立即重绘
            listView->scrollTo(idx, QAbstractItemView::EnsureVisible);
            listView->viewport()->update();
        }

        // ========== 1. 根据样式表精确计算内容高度 ==========
        const int rowCount = model()->rowCount();
        // 样式表中定义：
        // QListView::item  height:25px; margin-top:4px; margin-bottom:4px;
        // QListView        padding-top:6px; padding-bottom:6px;
        const int itemHeight = 25;
        const int itemMarginTop = 4;
        const int itemMarginBottom = 4;
        const int listPaddingTop = 6;
        const int listPaddingBottom = 6;

        int contentHeight = 0;
        if (rowCount > 0) {
            contentHeight = listPaddingTop + listPaddingBottom
                            + rowCount * (itemHeight + itemMarginTop + itemMarginBottom);
        }

        // ========== 2. 限制最大高度，超出时显示滚动条 ==========
        QScreen *screen = QGuiApplication::screenAt(mapToGlobal(QPoint(0,0)));
        if (!screen) screen = QGuiApplication::primaryScreen();
        const int shadowMarginTop = 8;       // 与 M_SetCBoxShadow 中 layout margins 上边距一致
        const int shadowMarginBottom = 8;    // 下边距
        const int maxContainerHeight = screen->availableGeometry().height() / 3;
        const int maxContentHeight = maxContainerHeight - shadowMarginTop - shadowMarginBottom;

        if (contentHeight > maxContentHeight) {
            contentHeight = maxContentHeight;
            if (listView) {
                listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            }
        } else {
            if (listView) {
                listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            }
        }

        // ========== 3. 设置 QListView 的固定高度 ==========
        if (listView) {
            listView->setFixedHeight(contentHeight);
        }

        // ========== 4. 让容器根据布局和子控件自动调整大小 ==========
        m_popup->adjustSize();  // 容器高度 = contentHeight + 上下阴影边距 (layout margins)

        // ========== 5. 位置计算（与原逻辑一致） ==========
        QStyleOptionComboBox opt;
        initStyleOption(&opt);
        QRect popupRect = style()->subControlRect(
            QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxListBoxPopup, this);

        QSize popupSize = m_popup->sizeHint();
        QPoint below = mapToGlobal(QPoint(popupRect.left(), popupRect.bottom()));
        QPoint above = mapToGlobal(QPoint(popupRect.left(), popupRect.top() - popupSize.height()));

        QRect screenGeom = screen->availableGeometry();
        bool showAbove = false;
        if (below.y() + popupSize.height() > screenGeom.bottom() && above.y() >= screenGeom.top()) {
            showAbove = true;
        }

        QPoint finalPos;
        if (showAbove) {
            finalPos = above + QPoint(m_popupOffsetX, m_popupOffsetY);
        } else {
            finalPos = below + QPoint(m_popupOffsetX, m_popupOffsetY);
        }

        m_popup->move(finalPos);
        m_popup->show();
    }

    void hidePopup() override {
        if (m_popup) {
            m_popup->hide();
        }
        QComboBox::hidePopup();
    }

private:
    QIcon m_icon;
    int m_popupOffsetX;
    int m_popupOffsetY;
    QWidget* m_popup = nullptr;
};

#endif // NEWCOMBOBOX_H

#ifndef NOSELECTLISTVIEW_H
#define NOSELECTLISTVIEW_H

#include <QListView>
#include <QItemSelectionModel>

class NoSelectListView : public QListView {
    Q_OBJECT
public:
    using QListView::QListView;

protected:
    // 禁止用户交互改变选择
    QItemSelectionModel::SelectionFlags selectionCommand(
        const QModelIndex &index, const QEvent *event) const override
    {
        Q_UNUSED(index);
        Q_UNUSED(event);
        return QItemSelectionModel::NoUpdate;
    }
};

#endif // NOSELECTLISTVIEW_H
