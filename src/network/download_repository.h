#ifndef DOWNLOAD_REPOSITORY_H
#define DOWNLOAD_REPOSITORY_H

#include <QObject>
#include <QString>

class DownloadRepository : public QObject {
  Q_OBJECT

 public:
  explicit DownloadRepository(QObject* parent = nullptr);

  void downloadToTempFile(const QString& url, int requestId);

 signals:
  void downloadProgress(int requestId, int percent);
  void fileSaved(int requestId, const QString& filePath);
  void errorOccurred(const QString& reason);
};

#endif  // DOWNLOAD_REPOSITORY_H
