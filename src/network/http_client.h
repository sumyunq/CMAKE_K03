#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "network/request_options.h"

class QHttpMultiPart;

/// \brief 共享网络客户端（饿汉单例）
///
/// Builder 模式替代 15 个重载：5 个方法 + RequestOptions
///
/// \code
/// auto &cli = HttpClient::instance();
///
/// // 简单 GET
/// auto *r = cli.get("/user-configs");
///
/// // GET + query + tag 路由
/// auto *r = cli.get("/user-configs", {
///     .query = {{"is_official_tag", "true"}, {"page", "1"}},
///     .tag   = "schemes_official"
/// });
///
/// // POST + 指定服务器 + 不带 token
/// auto *r = cli.post("/user/login", {
///     .body      = QJsonDocument(body).toJson(),
///     .serverKey = "domestic-t",
///     .auth      = false
/// });
/// \endcode
class HttpClient {
public:
  /// \brief 获取单例实例 / Get the singleton instance
  static HttpClient& instance();
  /// \brief 销毁客户端并释放网络管理器 / Destroy the client and release the network manager
  ~HttpClient();

  /// \brief 获取底层 QNetworkAccessManager / Get the underlying QNetworkAccessManager
  QNetworkAccessManager* manager();

  /// \brief 发送 GET 请求 / Send a GET request
  /// \param path API 路径 / API path
  /// \param opts 请求选项 / Request options
  /// \return QNetworkReply 指针，调用方负责 connect 信号 / Reply pointer; caller is responsible for
  /// connecting signals
  QNetworkReply* get(const QString& path, const RequestOptions& opts = {});
  /// \brief 发送 POST 请求 / Send a POST request
  /// \param path API 路径 / API path
  /// \param opts 请求选项 / Request options
  /// \return QNetworkReply 指针 / Reply pointer
  QNetworkReply* post(const QString& path, const RequestOptions& opts = {});
  /// \brief 发送 PUT 请求 / Send a PUT request
  /// \param path API 路径 / API path
  /// \param opts 请求选项 / Request options
  /// \return QNetworkReply 指针 / Reply pointer
  QNetworkReply* put(const QString& path, const RequestOptions& opts = {});
  /// \brief 发送 DELETE 请求 / Send a DELETE request
  /// \param path API 路径 / API path
  /// \param opts 请求选项 / Request options
  /// \return QNetworkReply 指针 / Reply pointer
  QNetworkReply* del(const QString& path, const RequestOptions& opts = {});
  /// \brief 上传 multipart 数据 / Upload multipart form data
  /// \param path API 路径 / API path
  /// \param mp HTTP multipart 消息体 / HTTP multipart body (ownership transferred)
  /// \param opts 请求选项 / Request options
  /// \return QNetworkReply 指针 / Reply pointer
  QNetworkReply* upload(const QString& path, QHttpMultiPart* mp, const RequestOptions& opts = {});

private:
  HttpClient() = default;
  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;

  QNetworkRequest buildRequest(const QString& path, const RequestOptions& opts);
  void applyCommonOptions(QNetworkRequest& req, const RequestOptions& opts);

  mutable QMutex cl_mutex_;
  QNetworkAccessManager* clp_manager_ = nullptr;

  static HttpClient s_instance_;
};

#endif  // API_CLIENT_H
