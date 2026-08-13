#ifndef CUSTOM_QDIALOG_GENERAL_TIPS_H
#define CUSTOM_QDIALOG_GENERAL_TIPS_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QRect>
#include <QShowEvent>
#include <QWidget>

/// \brief 通用提示弹窗
/// 子控件：
///     - 右上角关闭按钮
///     - 中间提示标题（可设置）
///     - 中间提示内容（可选）
///     - 底部取消/确认按钮（文字可设置）
class CustomQDialogGeneralTips : public QDialog
{
    Q_OBJECT
public:
    explicit CustomQDialogGeneralTips(QWidget *parent = nullptr, int theme = 0);

    void applyTheme(int theme); ///< 按主题更新样式

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽
    void updateChildGeometry();         ///< 根据当前配置刷新子控件位置
    void centerOnOwnerWindow();         ///< 显示前居中到所属窗口

public:
    void setCl_title_text(const QString &text);
    void setCl_message_text(const QString &text);
    void setCl_cancel_text(const QString &text);
    void setCl_confirm_text(const QString &text);
    void setCl_texts(const QString &title, const QString &cancel, const QString &confirm);
    void setCl_texts(const QString &title,
                     const QString &message,
                     const QString &cancel,
                     const QString &confirm);
    void setCl_dialog_size(const QSize &size);             ///< 设置弹窗整体宽高
    void setCl_title_geometry(const QRect &geometry);      ///< 设置标题区域
    void setCl_message_geometry(const QRect &geometry);    ///< 设置内容区域
    void setCl_close_button_geometry(const QRect &geometry);   ///< 设置关闭按钮区域
    void setCl_cancel_button_geometry(const QRect &geometry);  ///< 设置取消按钮区域
    void setCl_confirm_button_geometry(const QRect &geometry); ///< 设置确认按钮区域
    void setCl_cancel_visible(bool visible);               ///< 设置是否显示取消按钮
    void setCl_message_visible(bool visible);              ///< 设置是否显示内容文字
    QString cl_title_text() const;
    QString cl_message_text() const;
    QString cl_cancel_text() const;
    QString cl_confirm_text() const;

signals:
    void confirmed();
    void cancelled();

private:
    // 控件
    QWidget *clp_container_ = nullptr;
    QPushButton *clp_close_btn_ = nullptr;
    QLabel *clp_title_label_ = nullptr;
    QLabel *clp_message_label_ = nullptr;
    QPushButton *clp_cancel_btn_ = nullptr;
    QPushButton *clp_confirm_btn_ = nullptr;

    // 文字
    QString cl_title_text_ = tr("提示");
    QString cl_message_text_;
    QString cl_cancel_text_ = tr("取消");
    QString cl_confirm_text_ = tr("确认");

    // 控件尺寸/位置
    QSize cl_dialog_size_ = QSize(350, 206);
    QSize cl_container_size_ = QSize(350, 206);
    QSize cl_close_btn_size_ = QSize(31, 31);
    QPoint cl_close_btn_point_ = QPoint(309, 10);
    QSize cl_title_label_size_ = QSize(350, 23);
    QPoint cl_title_label_point_ = QPoint(0, 54);
    QSize cl_message_label_size_ = QSize(260, 40);
    QPoint cl_message_label_point_ = QPoint(45, 86);
    QSize cl_cancel_btn_size_ = QSize(104, 30);
    QSize cl_confirm_btn_size_ = QSize(104, 30);
    QPoint cl_cancel_btn_point_ = QPoint(48, 136);
    QPoint cl_confirm_btn_point_ = QPoint(198, 136);
    bool cl_cancel_visible_ = true;
    bool cl_message_visible_ = false;

    int cl_theme_ = 0;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
};

#endif // CUSTOM_QDIALOG_GENERAL_TIPS_H
