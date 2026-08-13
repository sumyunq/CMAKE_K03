#ifndef QDIALOG_CUSTOM_H
#define QDIALOG_CUSTOM_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <QPainter>
#include <QPainterPath>


#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>

#include <QStandardPaths>

#include "data/api_global.h"

namespace Ui {
class QDialogCustom;
}

class QDialogCustom : public QDialog
{
    Q_OBJECT

public:
    explicit QDialogCustom(QWidget *parent = nullptr);
    ~QDialogCustom();

    void setTitle(QString title);
    void set_btn_ok_text(QString text);
    void set_btn_cancel_text(QString text);

private:
    void InitUIInformation();
    void InitMember();
    void InitConnect();

public:
    QLabel *cl_title_;     ///< 对话标题
    QPoint cl_title_point_ = QPoint(144, 40); ///< 对话标题 位置
    QSize cl_title_min_size_ = QSize(64, 23); ///< 对话标题 最小尺寸
    QSize cl_title_max_size_ = QSize(64, 23); ///< 对话标题 最大尺寸

    QLineEdit *cl_lineEdit_; ///< 可编辑行
    QPoint cl_lineEdit_point_ = QPoint(40, 87);    ///< 可编辑行 位置
    QSize cl_lineEdit_min_size_ = QSize(271, 32);  ///< 可编辑行 最小尺寸
    QSize cl_lineEdit_max_size_ = QSize(271, 32);  ///< 可编辑行 最大尺寸

    QLabel *cl_error_icon_;     ///< 信息错误图标
    QPoint cl_error_icon_point_ = QPoint(40, 127); ///< 信息错误图标 位置
    QSize cl_error_icon_min_size_ = QSize(14, 14); ///< 信息错误图标 最小尺寸
    QSize cl_error_icon_max_size_ = QSize(14, 14); ///< 信息错误图标 最大尺寸

    QLabel *cl_error_message_;     ///< 信息错误提示
    QPoint cl_error_message_point_ = QPoint(60, 127); ///< 信息错误提示 位置
    QSize cl_error_message_min_size_ = QSize(220, 14); ///< 信息错误提示 最小尺寸
    QSize cl_error_message_max_size_ = QSize(220, 14); ///< 信息错误提示 最大尺寸

    QPushButton *cl_okBtn_; ///< 确认按钮
    QPoint cl_okBtn_point_ = QPoint(195, 171);      ///< 确认按钮 位置
    QSize cl_okBtn_min_size_ = QSize(104, 30);      ///< 确认按钮 最小尺寸
    QSize cl_okBtn_max_size_ = QSize(104, 30);      ///< 确认按钮 最大尺寸

    QPushButton *cl_cancelBtn_; ///< 取消按钮
    QPoint cl_cancelBtn_point_ = QPoint(52, 171);       ///< 取消按钮 位置
    QSize cl_cancelBtn_min_size_ = QSize(104, 30);      ///< 取消按钮 最小尺寸
    QSize cl_cancelBtn_max_size_ = QSize(104, 30);      ///< 取消按钮 最大尺寸

    QPushButton *cl_closeBtn_; ///< 右上角关闭按钮
    QPoint cl_closeBtn_point_ = QPoint(310, 10);       ///< 右上角关闭按钮 位置
    QSize cl_closeBtn_min_size_ = QSize(31, 31);       ///< 右上角关闭按钮 最小尺寸
    QSize cl_closeBtn_max_size_ = QSize(31, 31);       ///< 右上角关闭按钮 最大尺寸

    QString ret_info;   ///回显信息

    DeSheng::ResolveShareCodeRequest req;   ///< 解析分享码回显 请求数据
    DeSheng::ResolveShareCodeResponse resp; ///< 解析分享码回显 请求数据

    // 临时保存到本地文件
    QString saveDir; ///方案保存路径
    QString savePath;
    QString cl_share_prefix_; ///< 分享码前缀（ys/sq），路由用

private:
    Ui::QDialogCustom *ui;

    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // QDIALOG_CUSTOM_H
