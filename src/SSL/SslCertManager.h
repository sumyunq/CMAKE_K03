//公有，修改个别用户电脑上系统证书库为旧版本或不完整
#ifndef SSLCERTMANAGER_H
#define SSLCERTMANAGER_H

#include <QObject>
#include <QString>

class SslCertManager : public QObject
{
    Q_OBJECT
public:
    explicit SslCertManager(QObject *parent = nullptr);

    // 执行完整的初始化流程：
    // 1. 探测系统证书是否可用
    // 2. 不可用则加载程序目录下的 cacert.pem
    // 3. 检查本地证书文件是否需要更新（每月一次），需要则自动下载
    // 参数 certFileName: 证书文件名（默认为 cacert.pem），位于程序所在目录
    // 返回 true 表示当前已具备可用的 SSL 配置（系统或本地）
    bool initialize(const QString &certFileName = QStringLiteral("cacert.pem"));

private:
    // 探测系统证书库是否正常
    static bool probeSystemCertificates();

    // 加载本地 PEM 证书文件，并设置为全局默认 CA 证书
    static bool loadLocalCertificates(const QString &certPath);

    // 检查本地证书文件的年龄，若超过 30 天或不存在则下载更新
    static bool checkAndUpdateCertBundle(const QString &certPath);

    // 使用 HTTPS/HTTP 双重降级方式下载文件，保存到 savePath
    static bool downloadFileWithFallback(const QUrl &primaryUrl,
                                         const QUrl &fallbackUrl,
                                         const QString &savePath);
};

#endif // SSLCERTMANAGER_H

/*//公有+私有
#ifndef SSLCERTMANAGER_H
#define SSLCERTMANAGER_H

#include <QObject>
#include <QString>
#include <QSslCertificate>

class SslCertManager : public QObject
{
    Q_OBJECT
public:
    explicit SslCertManager(QObject *parent = nullptr);

    // 初始化 SSL 证书环境（主线程调用）
    // certFileName: 公共证书文件名（默认 cacert.pem）
    // privCertFileName: 私有证书文件名（默认 private_ca.pem）
    bool initialize(const QString &certFileName = QStringLiteral("cacert.pem"),
                    const QString &privCertFileName = QStringLiteral("private.pem"));

private:
    // 从任意路径（磁盘或 Qt 资源）加载 PEM 证书列表
    static QList<QSslCertificate> loadCertificatesFromFile(const QString &path);

    // 加载私有 CA 证书（先查磁盘，再查资源）
    static QList<QSslCertificate> loadPrivateCAs(const QString &privCertFileName);

    // 将私有证书合并到默认 SSL 配置中（保留已有 CA 并追加）
    static void mergePrivateCAs(const QString &privCertFileName);

    // 探测系统证书库是否正常工作
    static bool probeSystemCertificates();

    // 加载本地公共证书，并与私有证书合并后设为默认配置
    static bool loadLocalCertificates(const QString &certPath,
                                      const QString &privCertFileName);

    // 检查并更新本地公共证书文件（若不存在或超过30天则下载）
    static bool checkAndUpdateCertBundle(const QString &certPath,
                                         bool *outUpdated = nullptr);

    // 同步下载文件，优先 HTTPS，失败后降级 HTTP
    static bool downloadFileWithFallback(const QUrl &primaryUrl,
                                         const QUrl &fallbackUrl,
                                         const QString &savePath);

    // 从 Qt 资源提取文件到磁盘（用于首次启动）
    static bool extractResourceFile(const QString &resourcePath,
                                    const QString &destPath);
};

#endif
*/
