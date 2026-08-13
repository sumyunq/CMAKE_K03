#ifndef LOGINANDACTIVATIONCODE_H
#define LOGINANDACTIVATIONCODE_H

#include <QAction>
#include <QJsonObject>
#include <QLineEdit>
#include <QIcon>
#include <QDialog>
#include <QElapsedTimer>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "modules/AdvertisementSelectionPage/advertisement_selection_main_page.h" ///< 子部件：广告页面
#include "modules/GeneralCustomUI/CustomQWidget/custom_QWidget_loading.h" ///< 加载动画

namespace Ui {
class LoginAndActivationCode;
}

class LoginAndActivationCode : public QDialog
{
    Q_OBJECT

public:
    explicit LoginAndActivationCode(QWidget *parent = nullptr);
    ~LoginAndActivationCode();

    void reLogin();///< 重新登录

    void showTimer(bool en);
    void showErrorFAR(bool en);
    void showError(bool en);
    void showAError(bool en, QString text);
    void GetWechatCode();
    void handleThirdPartyLoginSuccess(const QJsonObject &dataObj); ///< 第三方登录成功公共处理（微信/Google）
    // void GetActivationCode();
    void CheckServerMaintenanceSta(const QString &originalError);

    // void ShowActivationCode();
    void showAvatar(QPixmap pixmap_Avatar);
    // void UploadACode();//上传激活码
    void UpdateDev();
    void GetDevMsg();

    void En_pBt_Activate();

    bool TokenEn();

    // void showActivation();
    void showWaitPage();
    void LanguageSet();

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

public slots:
    void on_pBt_backLogin_FAR_clicked();

private slots:
    void on_pBt_Activate_clicked();

    // 处理网络回复的槽函数
    // void onReplyFinished(QNetworkReply *reply);
    void onReplyFinished(QNetworkReply *reply,
                         QString requestTime,
                         QElapsedTimer timer,
                         QByteArray requestBody,
                         qint64 elapsedMs,
                         QString requestId);

    void on_pBt_wechat_clicked();

    void on_pBt_ELogin_clicked();

    void on_pBt_forgot_clicked();

    void on_pBt_register_clicked();

    void on_pBt_SendVerifyCode_FAR_clicked();

    void onTimeout();

    void on_pBt_ok_FAR_clicked();

public:
    AdvertisementSelectionMainPage *cl_advertisement_aelection_main_page_ = nullptr; ///< 广告页面

private:
    Ui::LoginAndActivationCode *ui;

    void LoginRequest();
    QString evidence; //用户判断微信登录状态

    QString user_psw_old;
    QString user_psw_new;

    QString user_verifyCode;

    int Countdown = 60; //发送验证码，60s倒计时
    QTimer *m_timer;

    void clearText();

    bool m_checkingMaintenance = false;

    // QRect cl_pBt_wechat_old_rect_; ///< 记录 pBt_wechat 原始位置

    QAction *lEdit_password_Action = nullptr;   ///< 登录密码
    QAction *lEdit_oldpsw_FAR_Action = nullptr; ///< 密码
    QAction *lEdit_newpsw_FAR_Action = nullptr; ///< 确认密码

    CustomQWidgetLoading *clp_login_loading_ = nullptr; ///< 登录加载动画
    void hideLoginLoading(); ///< 隐藏登录加载动画

signals:
    void MainPageChange();
    void LoginAgain();

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupActionHover(QLineEdit *lineEdit, QAction *action); ///< 为 QLineEdit 内部的 action 按钮安装 hover 事件过滤器
};

#endif // LOGINANDACTIVATIONCODE_H
