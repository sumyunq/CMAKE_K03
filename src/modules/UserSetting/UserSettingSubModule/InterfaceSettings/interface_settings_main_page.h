#ifndef INTERFACE_SETTINGS_MAIN_PAGE_H
#define INTERFACE_SETTINGS_MAIN_PAGE_H

#include "CustomControl/NewComboBox.h"
#include <QTranslator>
#include <QWidget>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainter>



class CustomQScrollAreaBackgroundComponent;

// 在代码中设置自定义委托
class NoIconEffectDelegate : public QStyledItemDelegate
{
public:
    explicit NoIconEffectDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        // 保存原始字体设置
        QFont originalFont = painter->font();
        QFont m_font; // 存储字体设置
        // 初始化字体设置
        m_font.setFamily("Noto Sans S Chinese");
        m_font.setPointSize(10);
        // 设置我们自定义的字体
        painter->setFont(m_font);

        // 创建一个没有选中状态的选项，以避免默认的选中效果
        QStyleOptionViewItem opt = option;
        opt.state &= ~QStyle::State_Selected; // 移除选中状态标志
        opt.font = m_font;                    // 设置选项的字体

        // 首先，绘制默认的项（包括图标和文本），但不包含选中效果
        //QStyledItemDelegate::paint(painter, opt, index);
        // 绘制图标（20x20大小）
        QSize m_iconSize = QSize(20, 20);
        QVariant iconData = index.data(Qt::DecorationRole);
        QRect iconRect = option.rect;
        if (iconData.isValid() && iconData.canConvert<QIcon>()) {
            QIcon icon = iconData.value<QIcon>();
            iconRect.setLeft(iconRect.left() + 5); // 左侧留出5px间距
            iconRect.setTop(iconRect.top() + (iconRect.height() - m_iconSize.height()) / 2);
            iconRect.setSize(m_iconSize);

            // 绘制图标，大小为20x20
            icon.paint(painter, iconRect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
        }

        // 绘制文本，考虑图标占用的空间
        QString text = index.data(Qt::DisplayRole).toString();
        if (!text.isEmpty()) {
            QRect textRect = option.rect;
            textRect.setLeft(iconRect.right() + 10);  // 图标右侧留出10px间距
            textRect.setRight(textRect.right() - 30); // 为选中背景图片留出空间

            // 绘制文本
            painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
        }

        // 然后，如果我们想要在选中时有一个自定义的背景，我们可以手动绘制
        if (option.state & QStyle::State_Selected) {
            // 使用自定义的选中背景（例如，一个图片）
            QPixmap selectedPixmap(":/Skin/Images/cBox/item_se.png");
            if (!selectedPixmap.isNull()) {
                // 根据需要调整图片的位置和大小，这里假设图片是放在右侧的
                // 获取图片原始尺寸
                int pixmapWidth = selectedPixmap.width();
                int pixmapHeight = selectedPixmap.height();
                QRect rect = option.rect;
                int x = rect.right() - pixmapWidth;                      // 右侧对齐
                int y = rect.top() + (rect.height() - pixmapHeight) / 2; // 垂直居中
                //rect.setLeft(rect.right() - selectedPixmap.width()); // 图片放在右侧
                //painter->drawPixmap(rect, selectedPixmap);
                // 绘制图片，不进行缩放
                painter->drawPixmap(x, y, pixmapWidth, pixmapHeight, selectedPixmap);
            }
        }

        // 如果您还想在悬停时有一些效果，可以在这里添加
        // 但是根据您的样式表，悬停时背景是透明的，所以可能不需要
    }
};


namespace Ui {
class InterfaceSettingsMainPage;
}

/// \brief 界面设置 页面
/// 子控件：
///     - widget: 主容器
///     - widget_3: 上部 — 语言切换(cBox_language)、主题切换(cBox_Theme)
///     - widget_background_images: 下部 — 背景图片预览区域 BackgroundComponentView
class InterfaceSettingsMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceSettingsMainPage(QWidget *parent = nullptr, int theme = 0);
    ~InterfaceSettingsMainPage();

    void applyTheme(int theme);              ///< 按主题更新样式
    int getLanguageIndex();                  ///< 获取 语言 combox index
    void setLanguageIndex(int targetIndex);  ///< 设置 语言 combox index
    int getThemeIndex();                     ///< 获取 主题 combox index
    void setThemeIndex(int targetIndex);     ///< 设置 主题 combox index
    void UpdateInterfaceSettingsUIInformation(); ///< 更新 UI 信息（供父级调用，含壁纸刷新+滑块同步）
    void syncSlidersFromModel();                ///< 仅同步滑块值（轻量，无壁纸刷新，用于 All 路径避免卡顿）
    void LanguageSet();                          ///< 刷新翻译文本

    void M_SetCBoxShadow(NewComboBox *cBox);


signals:
    void languageChange(); ///< 语言切换请求（通知 MainWindow 刷新所有页面文本）

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽

public:
    CustomQScrollAreaBackgroundComponent *clp_background_component_view_ = nullptr; ///< 背景图片预览区域
    int cl_theme_ = 0;                                                            ///< 当前主题

signals:
    void backgroundTransparencyChanged(qreal opacity); ///< 背景透明度变化
    void panelBlurChanged(qreal radius);               ///< 面板模糊度变化
    void backgroundChanged(const QString &path);       ///< 壁纸变更（自定义/系统）
    void defaultBackgroundRestored();                  ///< 恢复默认背景

private slots:
    void onLanguageChanged(int index);   ///< 语言切换
    void onThemeChanged(int index);      ///< 主题切换

private:
    Ui::InterfaceSettingsMainPage *ui;
};

#endif // INTERFACE_SETTINGS_MAIN_PAGE_H
