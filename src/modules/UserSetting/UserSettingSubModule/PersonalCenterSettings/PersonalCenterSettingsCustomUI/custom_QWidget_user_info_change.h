#ifndef CUSTOM_QWIDGET_USER_INFO_CHANGE_H
#define CUSTOM_QWIDGET_USER_INFO_CHANGE_H

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>
#include <QScrollBar>


namespace Ui {
class CustomQWidgetUserInfoChange;
}

/// \brief 用户信息修改页面
/// 子控件：
///     - 头像 QLabel
///     - 昵称/个性签名 标签
///     - 昵称 行编辑器 + 字数统计
///     - 个性签名 文本编辑器 + 字数统计
///     - 取消/确认 按钮
class CustomQWidgetUserInfoChange : public QWidget
{
    Q_OBJECT
public:
    explicit CustomQWidgetUserInfoChange(QWidget *parent = nullptr, int theme = 0);
    ~CustomQWidgetUserInfoChange();

    void applyTheme(int theme); ///< 按主题更新样式
    void LanguageSet();         ///< 语言切换：刷新代码内一次性 setText/placeholder 文本

signals:

    void cancelled();      ///< 取消按钮
    void confirmed();      ///< 确认按钮
    void editAvatarRequested(); ///< 编辑头像 — 点击头像时触发

public:
    void setCl_nickname(const QString &text);  ///< 设置昵称
    QString cl_nickname() const;               ///< 获取昵称
    void setCl_signature(const QString &text); ///< 设置个性签名
    QString cl_signature() const;              ///< 获取个性签名
    void setCl_avatar(const QPixmap &pixmap);  ///< 设置头像

private slots:
    void updateNicknameCounter();   ///< 昵称字数统计
    void updateSignatureCounter();  ///< 个性签名字数统计

private:
    void InitUIInformation(int theme); ///< 初始化UI的默认信息
    void InitMember();                 ///< 初始化内部成员
    void InitConnect();                ///< 连接默认的信号槽

public:
    /******************** UI 控件 ********************/
    QLabel *clp_avatar_label_ = nullptr;          ///< 头像
    QSize cl_avatar_size_ = QSize(80, 80);
    QPoint cl_avatar_point_ = QPoint(126, 50);
    QLabel *clp_avatar_hover_ = nullptr;          ///< 头像悬停遮罩+图标

    QLabel *clp_nickname_label_ = nullptr;        ///< 昵称 标签
    QSize cl_nickname_label_size_ = QSize(256, 20);
    QPoint cl_nickname_label_point_ = QPoint(38, 154);

    QLabel *clp_signature_label_ = nullptr;       ///< 签名 标签
    QSize cl_signature_label_size_ = QSize(256, 20);
    QPoint cl_signature_label_point_ = QPoint(38, 228);

    QLineEdit *clp_nickname_edit_ = nullptr;      ///< 昵称 编辑器
    QSize cl_nickname_edit_size_ = QSize(272, 33);
    QPoint cl_nickname_edit_point_ = QPoint(30, 179);
    QLabel *clp_nickname_counter_ = nullptr;      ///< 昵称 字数
    QSize cl_nickname_counter_size_ = QSize(27, 14);
    QPoint cl_nickname_counter_point_ = QPoint(265, 188);
    int cl_nickname_max_ = 25;///< 昵称最大字数

    QPlainTextEdit *clp_signature_edit_ = nullptr; ///< 签名 编辑器
    QSize cl_signature_edit_size_ = QSize(272, 80);
    QPoint cl_signature_edit_point_ = QPoint(30, 253);
    QLabel *clp_signature_counter_ = nullptr;      ///< 签名 字数
    QSize cl_signature_counter_size_ = QSize(27, 14);
    QPoint cl_signature_counter_point_ = QPoint(265, 311);
    int cl_signature_max_ = 50;

    QPushButton *clp_cancel_btn_ = nullptr;        ///< 取消 按钮
    QSize cl_cancel_btn_size_ = QSize(104, 30);
    QPoint cl_cancel_btn_point_ = QPoint(48, 545);

    QPushButton *clp_confirm_btn_ = nullptr;       ///< 确认 按钮
    QSize cl_confirm_btn_size_ = QSize(104, 30);
    QPoint cl_confirm_btn_point_ = QPoint(180, 545);

    int cl_theme_ = 0; ///< 当前主题

private:
    Ui::CustomQWidgetUserInfoChange *ui;

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // CUSTOM_QWIDGET_USER_INFO_CHANGE_H
