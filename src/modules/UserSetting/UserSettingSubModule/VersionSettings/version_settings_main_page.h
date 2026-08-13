#ifndef VERSION_SETTINGS_MAIN_PAGE_H
#define VERSION_SETTINGS_MAIN_PAGE_H

#include "QtCore"
#include <QWidget>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class UpdateOTASuccess;
class UpdateOTA;
class UpdateError;
class InstallError;
class UpdateVersion;
class UpdateOTAFind;
class UpdateSoftWareFind;
class UserSettingMainPage;

namespace Ui {
class VersionSettingsMainPage;
}

/// \brief 版本升级 页面
/// 子控件：
///     - widget: 左侧面板 — 应用版本信息、检查更新按钮
///     - widget_2: 右侧面板 — 耳机固件升级信息、固件回退/升级按钮
class VersionSettingsMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit VersionSettingsMainPage(QWidget *parent = nullptr, UserSettingMainPage* targetObject = nullptr);
    ~VersionSettingsMainPage();

    void GetUpdateMsg();

private slots:
    void on_pBt_OTA_clicked();//固件升级
    void on_pBt_UpdateSoftware_clicked();//软件升级

private:
    void InitUIInformation(); ///< 初始化UI的默认信息
    void InitMember();        ///< 初始化内部成员
    void InitConnect();       ///< 连接默认的信号槽

    // m_manager 已移除（2026-08-04）：GetUpdateMsg 统一走 ApiClient，见 version_settings_main_page.cpp
    QNetworkAccessManager *m_manager_exe = nullptr;
    QString m_downloadedFilePath;  // 保存下载的文件路径

    bool OTARollbackEn;//ture:点击的固件回退  false:点击的固件升级
    QString lastVerStr_RX = "";
    QString lastVerStr_TX = "";

    QString DfilePath = NULL;
    QString DfilePath_RX = NULL;
    QString DfilePath_TX = NULL;
    QFile *file;
    QFile *file_RX = nullptr;
    QFile *file_TX = nullptr;
    //弹窗
    UpdateOTASuccess *uOTASuccess = nullptr;
    UpdateOTA *uota = nullptr;
    UpdateError *uError = nullptr;
    UpdateVersion *uVersion = nullptr;
    UpdateOTAFind *uOTAFind = nullptr;
    UpdateSoftWareFind *uSoftWareFind = nullptr;


    int SoftWareFindDlgRes = 0;//最新版本上位机弹窗返回值

    QPointer<QNetworkReply> SoftWareLoad_Reply;  // 使用 QPointer 自动感知 reply 是否被删除
    QSaveFile* SoftWareLoad_File = nullptr;

    // ---------- 通用下载 ----------
    void startGenericDownload(
        const QString &urlString,
        QFile *&filePtr,
        const QString &filePath,
        std::function<void(QNetworkReply*)> onSuccess,
        bool isOta);

    // ---------- 固件更新 ----------

    void GetOTAFile(int type, bool en);
    void UpdateNewOTA();
    void showDownloadError(int type);


    void startBinDownload(const QString &urlString,
                          QFile* &filePtr,
                          const QString &filePath,
                          std::function<void(QNetworkReply*)> finishCallback);
    void startOTADownloadTX(const QString &urlString);
    void startOTADownloadRX(const QString &urlString);
    void handleDownloadFinished(QNetworkReply *reply,
                                const QString &filePath,
                                int otaParam);
    void onDownloadFinishedRX(QNetworkReply *reply);
    void onDownloadFinishedTX(QNetworkReply *reply);

    QString serverOTAVerStr = "";
    QString OldOTAVerStr_Tx = "";
    QString OldOTAVerStr_Rx = "";


    void promptUserForUpdate(int type, bool en);
    void handleVersionComparison(int type, const QString &currentVerStr, bool en);
    void startDownloadByType(int type, const QString &downloadUrl);
    void handleApiSuccess(const QJsonObject &rootObj, int type, bool en, const QString &readlastVer);
    void handleApiError(const QString &code, const QString &message, int type, bool en, const QString &readlastVer);
    void sendOtaRequest(QString Name, int type, bool en, const QString &readlastVer);

    // ---------- 软件更新 ----------

    void OnSoftwareReplyFinished(QNetworkReply *reply);
    void startDownload(const QString &urlString);
    void onDownloadFinished(QNetworkReply *reply);
    void runDownloadedFile(const QString &filePath);
    void cleanupDownloadedFile();



public:
    QString getVersionUIText();                    ///< 获取界面显示的版本号文本
    void UpdateVersionSettingsUIInformation();     ///< 更新 UI 信息（供父级调用）
    void LanguageSet();                            ///< 刷新翻译文本

    void SetOTAEn(bool en);//设计固件升级是否可用

    /// \brief 清除 label_deivce_icon 当前图片并设置新的 pixmap，缩放至图标大小
    void setDeviceIconPixmap(const QPixmap &t_pixmap);

private slots:

    void onReplyFinished(QNetworkReply *reply);

    void on_pBt_Rollback_clicked();

    void cancelCurrentDownload();

private:
    Ui::VersionSettingsMainPage *ui;
    UserSettingMainPage *clp_target_user_setting_main_page_ = nullptr; ///< 需要用到其部分通用函数
};

#endif // VERSION_SETTINGS_MAIN_PAGE_H
