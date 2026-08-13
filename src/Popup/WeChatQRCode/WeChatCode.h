#ifndef WECHATCODE_H
#define WECHATCODE_H

#include <QDialog>
#include <QWebEngineView>

namespace Ui {
class WeChatCode;
}

class WeChatCode : public QDialog
{
    Q_OBJECT

public:
    explicit WeChatCode(QWidget *parent = nullptr);
    ~WeChatCode();

    void ShowCode(QString url);


private:
    Ui::WeChatCode *ui;

    QWebEngineView *m_webView;

    // QNetworkAccessManager *m_manager_login = nullptr;
    // void LoginRequest();
    // QString evidence;//用户判断微信登录状态
    // QString user_id;//用户ID（唯一且不可更改）
    // QString user_nickname;//用户昵称
// private slots:
//     // 处理网络回复的槽函数
//     void onReplyFinished(QNetworkReply *reply);

signals:
    void ReadLoginRequest();
private slots:
    void on_pBt_Refresh_clicked();
    void on_pBt_close_clicked();
};

#endif // WECHATCODE_H
