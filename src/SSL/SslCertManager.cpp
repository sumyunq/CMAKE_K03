//公有
#include "SSL/SslCertManager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslError>
#include <QSslSocket>
#include <QTimer>
#include <QUrl>

SslCertManager::SslCertManager(QObject *parent)
    : QObject(parent)
{}

bool SslCertManager::initialize(const QString &certFileName)
{
    // 证书文件位于可执行文件同目录
    QString certPath = QCoreApplication::applicationDirPath() + QDir::separator() + certFileName;

    // 第一步：探测系统证书是否可用
    bool systemOk = probeSystemCertificates();

    if (!systemOk) {
        qDebug() << "System certificates broken, loading local bundle...";
        if (!loadLocalCertificates(certPath)) {
            qWarning() << "Cannot load local certs, SSL may fail entirely.";
        }
    } else {
        qDebug() << "System certificates OK.";
    }

    // 第二步：无论用哪个，都确保本地证书文件不过期（为以后备用，或供其他模块使用）
    checkAndUpdateCertBundle(certPath);

    return systemOk || QFileInfo::exists(certPath);
}

bool SslCertManager::probeSystemCertificates()
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("https://www.google.com"));
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        if (reply->isRunning()) {
            reply->abort();
            loop.quit();
        }
    });
    timeoutTimer.start(5000); // 5秒超时

    loop.exec();

    bool ok = (reply->error() == QNetworkReply::NoError);
    if (!ok) {
        qDebug() << "Probe failed:" << reply->errorString();
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            qDebug() << "-> SSL handshake error, system certs likely broken.";
        }
    }
    reply->deleteLater();
    return ok;
}

bool SslCertManager::loadLocalCertificates(const QString &certPath)
{
    QFile file(certPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open local cert file:" << certPath;
        return false;
    }

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> certs = QSslCertificate::fromDevice(&file, QSsl::Pem);
    file.close();

    if (certs.isEmpty()) {
        qWarning() << "No certificates found in local pem file.";
        return false;
    }

    config.setCaCertificates(certs);
    QSslConfiguration::setDefaultConfiguration(config);
    qDebug() << "Loaded local certs, count:" << certs.size();
    return true;
}

bool SslCertManager::checkAndUpdateCertBundle(const QString &certPath)
{
    QFileInfo fi(certPath);
    bool needUpdate = false;
    if (!fi.exists()) {
        needUpdate = true;
    } else {
        QDateTime lastMod = fi.lastModified();
        // 若最后修改时间距今超过30天，则需要更新
        if (lastMod.addMonths(1) < QDateTime::currentDateTime()) {
            needUpdate = true;
        }
    }

    if (!needUpdate) {
        qDebug() << "Local cert bundle is up to date.";
        return true;
    }

    // 官方推荐的 curl 维护的 CA 证书包
    const QUrl primaryUrl("https://curl.haxx.se/ca/cacert.pem");
    const QUrl fallbackUrl("http://curl.haxx.se/ca/cacert.pem"); // 降级 HTTP

    qDebug() << "Updating cert bundle from" << primaryUrl.toString();
    bool ok = downloadFileWithFallback(primaryUrl, fallbackUrl, certPath);
    if (!ok) {
        qWarning() << "Failed to update cert bundle. Will continue with existing one.";
    } else {
        // 可选：验证下载的文件是否合法 PEM
        QFile file(certPath);
        if (file.open(QIODevice::ReadOnly)) {
            QList<QSslCertificate> certs = QSslCertificate::fromDevice(&file, QSsl::Pem);
            if (certs.isEmpty()) {
                qWarning() << "Downloaded file is not a valid PEM, discarding...";
                file.close();
                file.remove(); // 删除无效文件，防止后续误用
                return false;
            }
            qDebug() << "Certificate update successful, loaded" << certs.size() << "CAs.";
        }
    }
    return ok;
}

bool SslCertManager::downloadFileWithFallback(const QUrl &primaryUrl,
                                              const QUrl &fallbackUrl,
                                              const QString &savePath)
{
    QNetworkAccessManager manager;

    auto tryDownload = [&](const QUrl &url) -> bool {
        QNetworkRequest request(url);
        QNetworkReply *reply = manager.get(request);

        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
            if (reply->isRunning()) {
                reply->abort();
                loop.quit();
            }
        });
        timeoutTimer.start(15000); // 15秒超时

        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Download failed:" << reply->errorString();
            reply->deleteLater();
            return false;
        }

        QByteArray data = reply->readAll();
        reply->deleteLater();

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qWarning() << "Cannot write to" << savePath;
            return false;
        }
        file.write(data);
        file.close();
        return true;
    };

    if (tryDownload(primaryUrl))
        return true;

    qWarning() << "Primary HTTPS download failed, trying HTTP fallback...";
    return tryDownload(fallbackUrl);
}


/*//公有+私有
#include "SSL/SslCertManager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QSslError>
#include <QSslSocket>
#include <QTimer>
#include <QUrl>

SslCertManager::SslCertManager(QObject *parent)
    : QObject(parent)
{}

bool SslCertManager::initialize(const QString &certFileName,
                                const QString &privCertFileName)
{
    QString certPath = QCoreApplication::applicationDirPath() + QDir::separator() + certFileName;

    // 1. 始终加载私有证书（确保内部服务可访问）
    mergePrivateCAs(privCertFileName);

    // 2. 探测系统公共证书是否可用
    bool systemOk = probeSystemCertificates();

    // 3. 如果系统证书不可用，回退加载本地公共证书
    if (!systemOk) {
        qDebug() << "System certificates broken, loading local bundle...";

        // 若本地公共证书文件不存在，从资源释放
        if (!QFileInfo::exists(certPath)) {
            qDebug() << "Local cert file missing, extracting from resource...";
            if (!extractResourceFile(":/cacert.pem", certPath)) {
                qWarning() << "Failed to extract public cert from resource!";
            }
        }

        // 加载本地公共证书（内部会合并私有证书）
        if (!loadLocalCertificates(certPath, privCertFileName)) {
            qWarning() << "Cannot load local public certs, public HTTPS may fail.";
        }
    } else {
        qDebug() << "System certificates OK.";
    }

    // 4. 每月自动更新本地公共证书文件（无论当前用系统还是本地）
    bool updated = false;
    checkAndUpdateCertBundle(certPath, &updated);

    // 若更新了且当前正在使用本地证书，需要重新加载使新公共证书生效
    if (updated && !systemOk) {
        qDebug() << "Reloading updated local public certs...";
        loadLocalCertificates(certPath, privCertFileName);
    }

    return systemOk || QFileInfo::exists(certPath);
}

// ---------- 工具函数实现 ----------

QList<QSslCertificate> SslCertManager::loadCertificatesFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open certificate file:" << path;
        return {};
    }
    QList<QSslCertificate> certs = QSslCertificate::fromDevice(&file, QSsl::Pem);
    file.close();
    return certs;
}

QList<QSslCertificate> SslCertManager::loadPrivateCAs(const QString &privCertFileName)
{
    // 先尝试磁盘路径
    QString diskPath = QCoreApplication::applicationDirPath() + QDir::separator() + privCertFileName;
    if (QFileInfo::exists(diskPath)) {
        return loadCertificatesFromFile(diskPath);
    }
    // 回退到 Qt 资源
    return loadCertificatesFromFile(":/private.pem");
}

void SslCertManager::mergePrivateCAs(const QString &privCertFileName)
{
    QList<QSslCertificate> privateCerts = loadPrivateCAs(privCertFileName);
    if (privateCerts.isEmpty()) {
        qDebug() << "No private CA certificates found (this is OK if not needed).";
        return;
    }

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> existingCAs = config.caCertificates();
    existingCAs.append(privateCerts);
    config.setCaCertificates(existingCAs);
    QSslConfiguration::setDefaultConfiguration(config);
    qDebug() << "Merged" << privateCerts.size() << "private CA certificates.";
}

bool SslCertManager::probeSystemCertificates()
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("https://www.google.com"));
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
        if (reply->isRunning()) {
            reply->abort();
            loop.quit();
        }
    });
    timeoutTimer.start(5000);

    loop.exec();

    bool ok = (reply->error() == QNetworkReply::NoError);
    if (!ok) {
        qDebug() << "Probe failed:" << reply->errorString();
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            qDebug() << "-> SSL handshake error, system certs likely broken.";
        }
    }
    reply->deleteLater();
    return ok;
}

bool SslCertManager::loadLocalCertificates(const QString &certPath,
                                           const QString &privCertFileName)
{
    // 加载公共证书
    QList<QSslCertificate> publicCerts = loadCertificatesFromFile(certPath);
    if (publicCerts.isEmpty()) {
        qWarning() << "No public certificates found in" << certPath;
        return false;
    }

    // 加载私有证书
    QList<QSslCertificate> privateCerts = loadPrivateCAs(privCertFileName);

    // 合并
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setCaCertificates(publicCerts + privateCerts);
    QSslConfiguration::setDefaultConfiguration(config);

    qDebug() << "Loaded local CA config:" << publicCerts.size()
             << "public +" << privateCerts.size() << "private CAs.";
    return true;
}

bool SslCertManager::checkAndUpdateCertBundle(const QString &certPath,
                                              bool *outUpdated)
{
    if (outUpdated) *outUpdated = false;

    QFileInfo fi(certPath);
    bool needUpdate = false;
    if (!fi.exists()) {
        needUpdate = true;
    } else {
        QDateTime lastMod = fi.lastModified();
        // 超过30天则更新
        if (lastMod.addDays(30) < QDateTime::currentDateTime()) {
            needUpdate = true;
        }
    }

    if (!needUpdate) {
        qDebug() << "Local public cert bundle is up to date.";
        return true;
    }

    const QUrl primaryUrl("https://curl.haxx.se/ca/cacert.pem");
    const QUrl fallbackUrl("http://curl.haxx.se/ca/cacert.pem");

    qDebug() << "Updating public cert bundle from" << primaryUrl.toString();
    bool ok = downloadFileWithFallback(primaryUrl, fallbackUrl, certPath);
    if (!ok) {
        qWarning() << "Failed to update public cert bundle.";
        return false;
    }

    // 验证下载的是有效 PEM
    QFile file(certPath);
    if (file.open(QIODevice::ReadOnly)) {
        QList<QSslCertificate> certs = QSslCertificate::fromDevice(&file, QSsl::Pem);
        if (certs.isEmpty()) {
            qWarning() << "Downloaded file is not valid PEM, discarding.";
            file.close();
            file.remove();
            return false;
        }
        qDebug() << "Certificate update successful, loaded" << certs.size() << "CAs.";
    }

    if (outUpdated) *outUpdated = true;
    return true;
}

bool SslCertManager::downloadFileWithFallback(const QUrl &primaryUrl,
                                              const QUrl &fallbackUrl,
                                              const QString &savePath)
{
    QNetworkAccessManager manager;

    auto tryDownload = [&](const QUrl &url) -> bool {
        QNetworkRequest request(url);
        QNetworkReply *reply = manager.get(request);

        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, [&]() {
            if (reply->isRunning()) {
                reply->abort();
                loop.quit();
            }
        });
        timeoutTimer.start(15000);

        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Download failed:" << reply->errorString();
            reply->deleteLater();
            return false;
        }

        QByteArray data = reply->readAll();
        reply->deleteLater();

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qWarning() << "Cannot write to" << savePath;
            return false;
        }
        file.write(data);
        file.close();
        return true;
    };

    if (tryDownload(primaryUrl))
        return true;

    qWarning() << "Primary HTTPS download failed, trying HTTP fallback...";
    return tryDownload(fallbackUrl);
}

bool SslCertManager::extractResourceFile(const QString &resourcePath,
                                         const QString &destPath)
{
    QFile resFile(resourcePath);
    if (!resFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open resource file" << resourcePath;
        return false;
    }
    QByteArray data = resFile.readAll();
    resFile.close();

    QFile destFile(destPath);
    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Cannot write to" << destPath;
        return false;
    }
    destFile.write(data);
    destFile.close();
    qDebug() << "Extracted resource" << resourcePath << "to" << destPath;
    return true;
}
*/
