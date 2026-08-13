#include "Popup/WeChatQRCode/WeChatCode.h"
#include "ui_WeChatCode.h"

#include "APOThread/ApoManager.h"
#include "LoadLib.h"
#include <QJsonParseError>
#include <QJsonObject>
#include <QUrlQuery>
#include <QVBoxLayout>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>

WeChatCode::WeChatCode(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WeChatCode)
{
    ui->setupUi(this);

    // 设置无边框
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    // 创建布局
    QVBoxLayout *layout = new QVBoxLayout(ui->widget_WechatCode);
    ui->widget_WechatCode->setLayout(layout);

    // 1. 创建WebEngine视图
    // m_webView = new QWebEngineView(ui->widget_WechatCode);
     m_webView = new QWebEngineView(this);

    // 启用必要的设置（解决部分白屏/崩溃问题）
    QWebEngineSettings *settings = m_webView->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
    settings->setAttribute(QWebEngineSettings::WebGLEnabled, true);

    // // 清除可能引起冲突的缓存和 cookie（可选）
    // QWebEngineProfile::defaultProfile()->cookieStore()->deleteAllCookies();
    // QWebEngineProfile::defaultProfile()->clearHttpCache();

    layout->addWidget(m_webView);
    // 在 load 完成后设置缩放因子
    connect(m_webView, &QWebEngineView::loadFinished, this, [this](bool ok) {
        if (ok) {
            // m_webView->setZoomFactor(0.5);  // 缩小到 75%
            m_webView->page()->runJavaScript(R"(
                // 假设二维码过期时页面会显示一个 class="expired" 的提示
                function checkExpired() {
                    if (document.querySelector('.expired')) {
                        // 通知 C++ 刷新
                        new QWebChannel(qt.webChannelTransport, function(channel) {
                            channel.objects.handler.refresh(); // 假设 C++ 端注册了名为 handler 的对象
                        });
                    }
                }
                setInterval(checkExpired, 1000); // 每秒检查一次
            )");
            emit ReadLoginRequest();
        }
    });
}

WeChatCode::~WeChatCode()
{
    delete ui;
}

void WeChatCode::ShowCode(QString url)
{
    emit ApoManager::instance()->requestlogWithTime("WeChatCode ShowCode");
    // 4. 加载授权页面
    // m_webView->load(QUrl(url));
    // 安全加载
    if (m_webView && !url.isEmpty()) {
        m_webView->load(QUrl(url));
    } else {
        emit ApoManager::instance()->requestlogWithTime("WeChatCode: webView is null or url empty");
    }

    emit ApoManager::instance()->requestlogWithTime("WeChatCode ShowCode load ok");
}

//刷新
void WeChatCode::on_pBt_Refresh_clicked()
{
    m_webView->reload();
}

//关闭
void WeChatCode::on_pBt_close_clicked()
{
    this->close();
}

