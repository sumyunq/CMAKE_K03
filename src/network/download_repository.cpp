#include "network/download_repository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QStandardPaths>
#include <QUrl>

#include "network/http_client.h"

DownloadRepository::DownloadRepository(QObject* parent) : QObject(parent) {}

void DownloadRepository::downloadToTempFile(const QString& url, int requestId) {
  if (url.isEmpty()) {
    emit errorOccurred(QStringLiteral("empty download url"));
    emit downloadProgress(requestId, -1);
    return;
  }

  QNetworkRequest req{QUrl(url)};
  auto* reply = HttpClient::instance().manager()->get(req);
  QPointer<DownloadRepository> self(this);

  QObject::connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);

  QObject::connect(reply, &QNetworkReply::downloadProgress, this,
                   [self, requestId](qint64 received, qint64 total) {
                     if (!self || total <= 0) {
                       return;
                     }
                     emit self->downloadProgress(
                         requestId, static_cast<int>(received * 100 / total));
                   });

  QObject::connect(reply, &QNetworkReply::finished, this, [self, reply, requestId] {
    if (!self) {
      return;
    }

    if (reply->error() != QNetworkReply::NoError) {
      emit self->errorOccurred(reply->errorString());
      emit self->downloadProgress(requestId, -1);
      return;
    }

    const QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
        QStringLiteral("/WidgetCMake/downloads");
    QDir().mkpath(tempDir);

    const QString name = QFileInfo(QUrl(reply->url()).path()).fileName();
    const QString fileName = name.isEmpty() ? QStringLiteral("config.json") : name;
    const QString path = tempDir + QLatin1Char('/') + QString::number(requestId) +
                         QLatin1Char('_') + fileName;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
      emit self->errorOccurred(QStringLiteral("write failed: ") + path);
      emit self->downloadProgress(requestId, -1);
      return;
    }

    file.write(reply->readAll());
    file.close();
    emit self->fileSaved(requestId, path);
  });
}
